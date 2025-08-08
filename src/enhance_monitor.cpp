/**
# @File     : enhance_monitor.cpp
# @Author   : jade
# @Date     : 2025/8/7 17:15
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : enhance_monitor.cpp
*/
//

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
#include <iomanip>
#elif defined(__linux__)
#include <sys/resource.h>
#include <cstring>
#include <fstream>
#endif
#if defined(__has_include)
#  if __has_include(<nvml.h>)  // 标准化的头文件存在性检查
#    include <nvml.h>
#    define NVML_ENABLED 1
#  endif
#endif
#include "include/enhance_monitor.h"
#include <algorithm>
#include <string>
#include <mutex>
#include "include/system_monitor_impl.h"
#include "include/dynamic_system_monitor_impl.h"

using namespace jade;

// 新增采样数据结构
struct DiskSample
{
    uint64_t reads{};
    uint64_t writes{};
    std::chrono::system_clock::time_point timestamp;
};

EnhancedTimeProfiler::EnhancedTimeProfiler(): dynamicSystemMonitor(new DynamicSystemMonitorImpl())
{
}

EnhancedTimeProfiler::~EnhancedTimeProfiler()
{
    delete dynamicSystemMonitor;
}

void SystemMonitorImpl::getMetrics(ResourceMetrics& metrics)
{
#if defined(_WIN32)
    HANDLE hProcess = GetCurrentProcess();
    static std::mutex cpuMutex; // 添加互斥锁确保线程安全
    // 1. 内存使用监控（优化错误处理）
    PROCESS_MEMORY_COUNTERS_EX pmc = {sizeof(pmc)};
    if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc)))
    {
        metrics.memoryUsage = bytesToMB(pmc.PrivateUsage);
    }
    else
    {
        metrics.memoryUsage = 0;
    }
    // 2. CPU使用率计算（完全线程安全重构）
    static struct CpuState
    {
        ULARGE_INTEGER lastCPU = {};
        ULARGE_INTEGER lastSysCPU = {};
        ULARGE_INTEGER lastUserCPU = {};
        DWORD numProcessors = 0;
        bool initialized = false;
    } cpuState;

    {
        std::lock_guard lock(cpuMutex);

        if (!cpuState.initialized)
        {
            // 一次性初始化
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            cpuState.numProcessors = sysInfo.dwNumberOfProcessors;

            FILETIME ftime, f_sys, f_user;
            GetSystemTimeAsFileTime(&ftime);
            memcpy(&cpuState.lastCPU, &ftime, sizeof(FILETIME));

            if (!GetProcessTimes(hProcess, &ftime, &ftime, &f_sys, &f_user))
            {
                // 初始化失败处理
                metrics.cpuUsage = 0.0;
                return;
            }
            memcpy(&cpuState.lastSysCPU, &f_sys, sizeof(FILETIME));
            memcpy(&cpuState.lastUserCPU, &f_user, sizeof(FILETIME));
            cpuState.initialized = true;
            metrics.cpuUsage = 0.0; // 首轮无基准数据
            return;
        }

        // 获取当前时间点
        FILETIME ftime, f_sys, f_user;
        ULARGE_INTEGER now, sys, user;
        GetSystemTimeAsFileTime(&ftime);
        memcpy(&now, &ftime, sizeof(FILETIME));

        if (!GetProcessTimes(hProcess, &ftime, &ftime, &f_sys, &f_user))
        {
            // 调用失败时保持上次数据
            return;
        }
        memcpy(&sys, &f_sys, sizeof(FILETIME));
        memcpy(&user, &f_user, sizeof(FILETIME));

        // 计算增量
        const auto sysDiff = static_cast<double>(sys.QuadPart - cpuState.lastSysCPU.QuadPart);
        const auto userDiff = static_cast<double>(user.QuadPart - cpuState.lastUserCPU.QuadPart);
        const auto timeDiff = static_cast<double>(now.QuadPart - cpuState.lastCPU.QuadPart);

        // 更新状态
        cpuState.lastCPU = now;
        cpuState.lastSysCPU = sys;
        cpuState.lastUserCPU = user;

        // 计算使用率
        if (timeDiff > 0 && cpuState.numProcessors > 0)
        {
            double percent = sysDiff + userDiff / timeDiff;
            percent /= cpuState.numProcessors;
            metrics.cpuUsage = percent * 100.0;
        }
        else
        {
            metrics.cpuUsage = 0.0; // 防止除零错误
        }
    }

    // 3. 磁盘IO监控（改进错误处理）
    static DiskSample lastSample = {0, 0, std::chrono::system_clock::now()};
    {
        IO_COUNTERS ioCounters = {0};
        if (GetProcessIoCounters(hProcess, &ioCounters))
        {
            metrics.diskReads = ioCounters.ReadTransferCount;
            metrics.diskWrites = ioCounters.WriteTransferCount;
        }
        else
        {
            // 可选：保留原值或重置为0
            metrics.diskReads = 0;
            metrics.diskWrites = 0;
        }
        // 计算磁盘速度
        const auto now = std::chrono::system_clock::now();
        if (const auto timeDiff = static_cast<double>(std::chrono::duration_cast<
            std::chrono::milliseconds>(now - lastSample.timestamp).count()) / 1000.0; timeDiff > 0.001)
        {
            // 避免除零
            const uint64_t readDiff = metrics.diskReads - lastSample.reads;
            const uint64_t writeDiff = metrics.diskWrites - lastSample.writes;
            // 转换为MB/s (1 MB = 1,048,576 bytes)
            metrics.readSpeed = bytesToMB(readDiff);
            metrics.writeSpeed = bytesToMB(writeDiff);
        }
        else
        {
            // std::cout << "时间不对" << std::endl;
        }
        lastSample = {metrics.diskReads, metrics.diskWrites, now};
    }

    // 4. GPU监控占位（添加实现建议）
    /*
    建议方案：
    - NVIDIA GPU：使用NVML库 (nvmlDeviceGetUtilizationRates)
    - AMD GPU：使用ADL SDK
    - 通用方案：DXGI接口（IDXGIAdapter::GetDesc获取显存）
    */
    // 4. GPU监控 - NVML实现
#ifdef NVML_ENABLED
    static nvmlReturn_t nvmlStatus = NVML_ERROR_UNINITIALIZED;
    static nvmlDevice_t nvmlDevice = nullptr;
    // 一次性初始化NVML
    if (nvmlStatus == NVML_ERROR_UNINITIALIZED)
    {
        nvmlStatus = nvmlInit_v2();
        if (nvmlStatus == NVML_SUCCESS)
        {
            nvmlStatus = nvmlDeviceGetHandleByIndex(0, &nvmlDevice);
        }
    }
    // 获取GPU利用率
    if (nvmlStatus == NVML_SUCCESS)
    {
        nvmlUtilization_t utilization;
        if (nvmlDeviceGetUtilizationRates(nvmlDevice, &utilization) == NVML_SUCCESS)
        {
            metrics.gpuUsage = static_cast<double>(utilization.gpu);
        }
        else
        {
            metrics.gpuUsage = 0.0;
        }

        // 获取GPU显存信息
        nvmlMemory_t memoryInfo;
        if (nvmlDeviceGetMemoryInfo(nvmlDevice, &memoryInfo) == NVML_SUCCESS)
        {
            metrics.gpuMemory = static_cast<double>(memoryInfo.used) / (1024 * 1024);
        }
        else
        {
            metrics.gpuMemory = 0.0;
        }
    }
#else
    metrics.gpuMemory = static_cast<uint64_t>(0.0);
    metrics.gpuUsage = static_cast<uint64_t>(0.0);
#endif

#elif defined(__linux__)
    // 内存使用
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        uint64_t size, resident;
        statm >> size >> resident;
        metrics.memoryUsage = resident * sysconf(_SC_PAGESIZE);
    }

    // CPU使用
    static uint64_t lastTotalTime = 0, lastProcessTime = 0;
    // 获取进程CPU时间
    std::ifstream procStat("/proc/self/stat");
    if (procStat.is_open()) {
        std::string ignore;
        uint64_t utime, stime;
        for (int i = 0; i < 13; i++) procStat >> ignore;
        procStat >> utime >> stime;
        uint64_t processTime = utime + stime;

        // 获取系统总CPU时间
        std::ifstream globalStat("/proc/stat");
        std::string cpuLabel;
        uint64_t user, nice, system, idle;
        globalStat >> cpuLabel >> user >> nice >> system >> idle;
        uint64_t totalTime = user + nice + system + idle;

        if (lastTotalTime != 0 && lastProcessTime != 0) {
            double deltaTotal = totalTime - lastTotalTime;
            double deltaProcess = processTime - lastProcessTime;
            metrics.cpuUsage = (deltaProcess / deltaTotal) * 100.0;
        }

        lastTotalTime = totalTime;
        lastProcessTime = processTime;
    }

    // 磁盘IO
    std::ifstream ioStat("/proc/self/io");
    if (ioStat.is_open()) {
        std::string line;
        while (std::getline(ioStat, line)) {
            if (line.find("rchar") != std::string::npos)
                sscanf(line.c_str(), "rchar: %lu", &metrics.diskReads);
            else if (line.find("wchar") != std::string::npos)
                sscanf(line.c_str(), "wchar: %lu", &metrics.diskWrites);
        }
    }

    // GPU监控在Linux上通常需要NVML
    // 这里仅作为占位符
    metrics.gpuUsage = 0.0;
    metrics.gpuMemory = 0.0;
#endif
}

// DynamicSystemMonitorImpl 实现
DynamicSystemMonitorImpl::~DynamicSystemMonitorImpl()
{
    stop();
}

void DynamicSystemMonitorImpl::start(const string& name, int interval_ms, const size_t max_history)
{
    if (running_) return;
    running_ = true;
    max_history_ = max_history;
    monitor_thread_ = std::thread(&DynamicSystemMonitorImpl::monitorLoop, this, interval_ms);
    StepMetrics metrics;
    metrics.startTime = std::chrono::high_resolution_clock::now();
    stepData[name].push_back(metrics);
}

void DynamicSystemMonitorImpl::stop()
{
    if (!running_) return;
    snapshots_.clear();
    running_ = false;
    cv_.notify_all();
    if (monitor_thread_.joinable())
    {
        monitor_thread_.join();
    }
}

SystemMonitorImpl::ResourceMetrics DynamicSystemMonitorImpl::getLatestSnapshot() const
{
    std::lock_guard lock(data_mutex_);
    if (snapshots_.empty())
    {
        return SystemMonitorImpl::ResourceMetrics{};
    }
    return snapshots_.back();
}


std::deque<SystemMonitorImpl::ResourceMetrics> DynamicSystemMonitorImpl::getHistory() const
{
    std::lock_guard lock(data_mutex_);
    return snapshots_;
}

void DynamicSystemMonitorImpl::setUpdateCallback(
    const std::function<void(const SystemMonitorImpl::ResourceMetrics&)>& callback)
{
    std::lock_guard lock(callback_mutex_);
    callback_ = callback;
}

void DynamicSystemMonitorImpl::monitorLoop(const int interval_ms)
{
    while (running_)
    {
        // 获取系统指标
        auto snapshot = SystemMonitorImpl::getMetrics();
        // 存储数据（带锁保护）
        {
            std::lock_guard lock(data_mutex_);
            snapshots_.push_back(snapshot);
            // 保持历史数据长度
            while (snapshots_.size() > max_history_)
            {
                snapshots_.pop_front();
            }
        }
        // 执行回调
        {
            std::lock_guard lock(callback_mutex_);
            if (callback_)
            {
                callback_(snapshot);
            }
        }
        // 等待下一个采样周期
        std::unique_lock<std::mutex> lock(cv_mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(interval_ms), [this] { return !running_; });
    }
}


void EnhancedTimeProfiler::startStep(const std::string& stepName, const int interval_ms) const
{
    dynamicSystemMonitor->start(stepName, interval_ms);
    dynamicSystemMonitor->setUpdateCallback([](const auto& snapshot)
    {
        // std::cout << "Cpu use:" << snapshot.cpuUsage << " ,Mem:" << snapshot.memoryUsage
        // << " ,readSpeed:" << snapshot.readSpeed << " ,writeSpeed:" << snapshot.writeSpeed
        // << " ,GPU use:" << snapshot.gpuUsage << " ,GPU Mem:" << snapshot.gpuMemory  << std::endl;
    });
}

void EnhancedTimeProfiler::endStep(const std::string& stepName) const
{
    auto& steps = dynamicSystemMonitor->stepData[stepName];
    if (steps.empty()) return;
    auto& [startTime, duration,metrics] = steps.back();
    const auto endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    metrics.assign(dynamicSystemMonitor->getHistory().begin(), dynamicSystemMonitor->getHistory().end());
    dynamicSystemMonitor->stop();
}

std::vector<SystemMonitorImpl::ResourceMetrics> EnhancedTimeProfiler::extractMiddleElements(
    const std::vector<SystemMonitorImpl::ResourceMetrics>& arr)
{
    const int n = static_cast<int>(arr.size());
    // 处理空数组情况
    if (n == 0) return {};

    // 计算要取的元素数量 x（至少1个，最多n个）
    const int x = std::max(1, n / 3);

    // 计算起始位置（确保左右两侧数量最多相差1）
    const int start_index = (n - x) / 2;

    // 注意：原始数组是原生数组，我们可以用指针作为迭代器
    return {
        arr.begin() + start_index,
        arr.begin() + start_index + x
    };
}

std::vector<std::vector<std::string>> EnhancedTimeProfiler::getDatas() const
{
    vector<vector<string>> datas = {};
    for (const auto& [name, metricsList] : dynamicSystemMonitor->stepData)
    {
        if (metricsList.empty()) continue;
        // 计算各指标的平均值
        double totalTime = 0;
        double totalCPU = 0;
        double totalMem = 0;
        uint64_t totalReads = 0;
        uint64_t totalWrites = 0;
        double totalSpeedReads = 0;
        double totalSpeedWrites = 0;
        double totalGPU = 0;
        double totalGPUMem = 0;
        for (const auto& [startTime, duration,metrics] : metricsList)
        {
            totalTime += static_cast<double>(duration.count());
            std::vector<SystemMonitorImpl::ResourceMetrics> extractMetrics = extractMiddleElements(metrics);
            for (const auto& [cpuUsage, memoryUsage, diskReads, diskWrites,readSpeed,writeSpeed, gpuUsage, gpuMemory] :
                 extractMetrics)
            {
                totalCPU += cpuUsage;
                totalMem += memoryUsage;
                totalReads += diskReads;
                totalWrites += diskWrites;
                totalSpeedReads += readSpeed;
                totalSpeedWrites += writeSpeed;
                totalGPU += gpuUsage;
                totalGPUMem += gpuMemory;
            }
            if (!extractMetrics.empty())
            {
                const int extractMetricsSize = static_cast<int>(extractMetrics.size());
                totalCPU /= extractMetricsSize;
                totalMem /= extractMetricsSize;
                totalSpeedWrites /= extractMetricsSize;
                totalSpeedReads /= extractMetricsSize;
                totalReads /= extractMetricsSize;
                totalWrites /= extractMetricsSize;
                totalGPU /= extractMetricsSize;
                totalGPUMem /= extractMetricsSize;
            }
        }
        const int count = static_cast<int>(metricsList.size());
        vector data = {
            name, formatValue(count / 1.0), formatValue(totalTime / 1000.0, 3),
            formatValue(totalTime / (count * 1000.0), 3),
            formatValue(totalCPU / count), formatValue(totalMem / count), formatValue(totalSpeedReads / count),
            formatValue(totalSpeedWrites / count), formatValue(totalGPU / count), formatValue(totalGPUMem / count)
        };
        datas.push_back(data);
    }
    return datas;
}

void EnhancedTimeProfiler::printTable() const
{
    printPrettyTable(dynamicSystemMonitor->headers, getDatas());
}

std::string EnhancedTimeProfiler::getTable() const
{
    return getPrettyTable(dynamicSystemMonitor->headers, getDatas());
}


void EnhancedTimeProfiler::reset() const
{
    dynamicSystemMonitor->stepData.clear();
}

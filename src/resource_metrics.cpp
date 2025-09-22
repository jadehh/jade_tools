/**
# @File     : resource_metrics.cpp
# @Author   : jade
# @Date     : 2025/9/3 12:57
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : resource_metrics.cpp
*/
#include <chrono>
#include <mutex>

#include "include/jade_tools.h"
#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
#include <iomanip>
#elif defined(__linux__)
#include <sys/resource.h>
#include <cstring>
#include <fstream>
#include <unistd.h>
#endif
#if defined(__has_include)
#  if __has_include(<nvml.h>)  // 标准化的头文件存在性检查
#    include <nvml.h>
#    define NVML_ENABLED 1
#  endif
#endif
using namespace jade;

// 新增采样数据结构
struct DiskSample
{
    uint64_t reads{};
    uint64_t writes{};
    std::chrono::system_clock::time_point timestamp;
};

SystemMonitorImpl::ResourceMetrics SystemMonitorImpl::getMetrics()
{
    ResourceMetrics metrics{};
    getMetrics(metrics);
    return metrics;
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
        const auto sysDiff = sys.QuadPart - cpuState.lastSysCPU.QuadPart;
        const auto userDiff =user.QuadPart - cpuState.lastUserCPU.QuadPart;
        const auto timeDiff = now.QuadPart - cpuState.lastCPU.QuadPart;

        // 更新状态
        cpuState.lastCPU = now;
        cpuState.lastSysCPU = sys;
        cpuState.lastUserCPU = user;

        // 计算使用率
        if (timeDiff > 0 && cpuState.numProcessors > 0)
        {
            double percent = static_cast<double>((sysDiff + userDiff) )/ static_cast<double>(timeDiff);
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
            metrics.readSpeed = bytesToMB(static_cast<SIZE_T>(readDiff));
            metrics.writeSpeed = bytesToMB(static_cast<SIZE_T>(writeDiff));
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
    if (std::ifstream statm("/proc/self/statm"); statm.is_open())
    {
        uint64_t size, resident;
        statm >> size >> resident;
        metrics.memoryUsage = resident * sysconf(_SC_PAGESIZE);
    }

    // CPU使用
    static uint64_t lastTotalTime = 0, lastProcessTime = 0;
    // 获取进程CPU时间
    if (std::ifstream procStat("/proc/self/stat"); procStat.is_open())
    {
        std::string ignore;
        uint64_t utime, stime;
        for (int i = 0; i < 13; i++)
            procStat >> ignore;
        procStat >> utime >> stime;
        uint64_t processTime = utime + stime;

        // 获取系统总CPU时间
        std::ifstream globalStat("/proc/stat");
        std::string cpuLabel;
        uint64_t user, nice, system, idle;
        globalStat >> cpuLabel >> user >> nice >> system >> idle;
        uint64_t totalTime = user + nice + system + idle;

        if (lastTotalTime != 0 && lastProcessTime != 0)
        {
            double deltaTotal = totalTime - lastTotalTime;
            double deltaProcess = processTime - lastProcessTime;
            metrics.cpuUsage = (deltaProcess / deltaTotal) * 100.0;
        }

        lastTotalTime = totalTime;
        lastProcessTime = processTime;
    }

    // 磁盘IO
    if (std::ifstream ioStat("/proc/self/io"); ioStat.is_open())
    {
        std::string line;
        while (std::getline(ioStat, line))
        {
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

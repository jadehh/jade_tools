/**
# @File     : enhance_monitor.cpp
# @Author   : jade
# @Date     : 2025/8/7 17:15
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : enhance_monitor.cpp
*/
//

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif
#include "include/jade_tools.h"
#include "include/dynamic_system_monitor.h"
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>

namespace jade
{
    class DynamicSystemMonitorImpl
    {
    public:
        DynamicSystemMonitorImpl() = default;
        ~DynamicSystemMonitorImpl()
        {
            monitor.reset();
        };
        std::unique_ptr<DynamicSystemMonitor> monitor;
    };
}

using namespace jade;
EnhancedTimeProfiler::EnhancedTimeProfiler(): dynamic_system_monitor_impl_(new DynamicSystemMonitorImpl())
{
}

EnhancedTimeProfiler::~EnhancedTimeProfiler()
{
    delete dynamic_system_monitor_impl_;
}


void EnhancedTimeProfiler::startStep(const std::string& stepName, const int interval_ms) const
{
    dynamic_system_monitor_impl_->monitor->start(stepName, interval_ms);
    dynamic_system_monitor_impl_->monitor->setUpdateCallback([](const auto& snapshot)
    {
        // std::cout << "Cpu use:" << snapshot.cpuUsage << " ,Mem:" << snapshot.memoryUsage
        // << " ,readSpeed:" << snapshot.readSpeed << " ,writeSpeed:" << snapshot.writeSpeed
        // << " ,GPU use:" << snapshot.gpuUsage << " ,GPU Mem:" << snapshot.gpuMemory  << std::endl;
    });
}

void EnhancedTimeProfiler::endStep(const std::string& stepName, const int count)
{
    // std::cout << "end Step" << std::endl;
    // std::cout << "count:" << count << "history size:"<< dynamicSystemMonitor->getHistory().size() << std::endl;
    count_ = count;
    auto& steps = dynamic_system_monitor_impl_->monitor->stepData[stepName];
    if (steps.empty()) return;
    auto& [startTime, duration,metrics] = steps.back();
    const auto endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    metrics.assign(dynamic_system_monitor_impl_->monitor->getHistory().begin(), dynamic_system_monitor_impl_->monitor->getHistory().end());
    dynamic_system_monitor_impl_->monitor->stop();
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
    std::vector<std::vector<std::string>> datas = {};
    for (const auto& [name, metricsList] : dynamic_system_monitor_impl_->monitor->stepData)
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
        std::vector data = {
            name, formatValue(count * count_ / 1.0), formatValue(totalTime / 1000.0, 3),
            formatValue(totalTime / (count * count_), 1),
            formatValue(totalCPU / count), formatValue(totalMem / count), formatValue(totalSpeedReads / count),
            formatValue(totalSpeedWrites / count), formatValue(totalGPU / count), formatValue(totalGPUMem / count)
        };
        datas.push_back(data);
    }
    return datas;
}

void EnhancedTimeProfiler::printTable() const
{
    printPrettyTable(dynamic_system_monitor_impl_->monitor->headers, getDatas());
}

std::string EnhancedTimeProfiler::getTable() const
{
    return getPrettyTable(dynamic_system_monitor_impl_->monitor->headers, getDatas());
}


void EnhancedTimeProfiler::reset() const
{
    dynamic_system_monitor_impl_->monitor->stepData.clear();
}

void* EnhancedTimeProfilerCreate()
{
    return new EnhancedTimeProfiler();
}

void EnhancedTimeProfilerStart(void* obj, const char* name)
{
    const auto instance = static_cast<EnhancedTimeProfiler*>(obj);
    instance->startStep(name);
}

void EnhancedTimeProfilerStop(void* obj, const char* name, const int count)
{
    const auto instance = static_cast<EnhancedTimeProfiler*>(obj);
    instance->endStep(name, count);
}

const char* EnhancedTimeProfilerResult(void* obj)
{
    const auto instance = static_cast<EnhancedTimeProfiler*>(obj);
    return instance->getTable().c_str();
}


void EnhancedTimeProfilerDestroy(void* obj)
{
    delete static_cast<EnhancedTimeProfiler*>(obj);
}

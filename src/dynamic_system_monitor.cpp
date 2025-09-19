/**
# @File     : dynamic_system_monitor.cpp
# @Author   : jade
# @Date     : 2025/9/3 13:16
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : dynamic_system_monitor.cpp
*/
#include "include/dynamic_system_monitor.h"

std::deque<jade::SystemMonitorImpl::ResourceMetrics> DynamicSystemMonitor::getHistory() const
{
    std::lock_guard lock(data_mutex_);
    return snapshots_;
}

jade::SystemMonitorImpl::ResourceMetrics DynamicSystemMonitor::getLatestSnapshot() const
{
    std::lock_guard lock(data_mutex_);
    if (snapshots_.empty())
    {
        return jade::SystemMonitorImpl::ResourceMetrics{};
    }
    return snapshots_.back();
}

void DynamicSystemMonitor::monitorLoop(const int interval_ms)
{
    while (running_)
    {
        // 获取系统指标
        auto snapshot = jade::SystemMonitorImpl::getMetrics();
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
        std::unique_lock lock(cv_mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(interval_ms), [this] { return !running_; });
    }
}

void DynamicSystemMonitor::start(const std::string& name, int interval_ms, size_t max_history)
{
    if (running_) return;
    running_ = true;
    max_history_ = max_history;
    monitor_thread_ = std::thread(&DynamicSystemMonitor::monitorLoop, this, interval_ms);
    StepMetrics metrics;
    metrics.startTime = std::chrono::high_resolution_clock::now();
    stepData[name].push_back(metrics);
}

void DynamicSystemMonitor::setUpdateCallback(
    const std::function<void(const jade::SystemMonitorImpl::ResourceMetrics&)>& callback)
{
    std::lock_guard lock(callback_mutex_);
    callback_ = callback;
}

void DynamicSystemMonitor::stop()
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

DynamicSystemMonitor::~DynamicSystemMonitor()
{
    stop();
}


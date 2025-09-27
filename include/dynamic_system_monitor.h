/**
# @File     : dynamic_system_monitor_impl.h
# @Author   : jade
# @Date     : 2025/8/7 18:09
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : dynamic_system_monitor_impl.h
*/
//
#pragma once
#include <mutex>
#include <thread>
#include <deque>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <unordered_map>

#include "jade_tools.h"

class DynamicSystemMonitor
{
public:
    std::vector<std::string> headers = {
        "步骤名称", "调用次数", "总耗时(s)", "平均耗时(ms)", "CPU(%)", "内存(MB)", "磁盘读取(MB/s)", "磁盘写入(MB/s)", "GPU(%)", "显存(MB)"
    };

    struct StepMetrics
    {
        [[maybe_unused]] std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        [[maybe_unused]] std::chrono::milliseconds duration{0};
        std::vector<jade::SystemMonitorImpl::ResourceMetrics> metricsList;
    };

    std::unordered_map<std::string, std::vector<StepMetrics>> stepData;
    // 资源数据存储结构
    jade::SystemMonitorImpl::ResourceMetrics metrics = {};
    DynamicSystemMonitor() = default;
    ~DynamicSystemMonitor();
    // 启动监控线程 如果CPU资源占满会影响线程的速度
    void start(const std::string& name, int interval_ms = 100, size_t max_history = 100);
    // 停止监控线程
    void stop();
    // 获取当前最新快照
    [[maybe_unused]] jade::SystemMonitorImpl::ResourceMetrics getLatestSnapshot() const;
    // 获取历史数据（线程安全）
    std::deque<jade::SystemMonitorImpl::ResourceMetrics> getHistory() const;
    // 注册回调函数
    void setUpdateCallback(const std::function<void(const jade::SystemMonitorImpl::ResourceMetrics&)>& callback);

private:
    // 监控循环
    void monitorLoop(int interval_ms);
    std::atomic<bool> running_{false};
    std::thread monitor_thread_;
    mutable std::mutex data_mutex_;
    std::deque<jade::SystemMonitorImpl::ResourceMetrics> snapshots_;
    size_t max_history_ = 60;
    mutable std::mutex callback_mutex_;
    std::function<void(const jade::SystemMonitorImpl::ResourceMetrics&)> callback_;
    mutable std::mutex cv_mutex_;
    std::condition_variable cv_{};
};

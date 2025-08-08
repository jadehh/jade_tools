/**
# @File     : enhance_monitor.h
# @Author   : jade
# @Date     : 2025/8/7 11:14
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : enhance_monitor.h
*/
//

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H
#include "jade_tools.h"
#include <functional>
#include "system_monitor_impl.h"

namespace jade
{
    class DynamicSystemMonitorImpl; // 前向声明
}


namespace jade
{
    class JADE_API EnhancedTimeProfiler
    {
        int count_ = 1;
        DynamicSystemMonitorImpl * dynamicSystemMonitor;
        [[nodiscard]] std::vector<std::vector<std::string>> getDatas() const;
    public:
        EnhancedTimeProfiler();
        ~EnhancedTimeProfiler();
        // 开始计时并监控资源
        void startStep(const std::string& stepName, int interval_ms=1000) const;
        // 结束计时并记录资源使用情况
        void endStep(const std::string& stepName,int count=1);
        static std::vector<SystemMonitorImpl::ResourceMetrics> extractMiddleElements(const std::vector<SystemMonitorImpl::ResourceMetrics>& arr);
        // 输出统计报告
        void printTable() const;
        [[nodiscard]] std::string getTable() const;
        // 重置所有监控数据
        void reset() const;
    };
}
#endif //SYSTEM_MONITOR_H

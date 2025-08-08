/**
# @File     : system_monitor_impl.h
# @Author   : jade
# @Date     : 2025/8/7 18:06
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : system_monitor_impl.h
*/
//

#ifndef SYSTEM_MONITOR_IMPL_H
#define SYSTEM_MONITOR_IMPL_H
class SystemMonitorImpl
{
public:
    struct ResourceMetrics
    {
        double cpuUsage; // CPU使用率（百分比）
        double memoryUsage; // 内存使用量（字节）
        uint64_t diskReads; // 磁盘读取次数
        uint64_t diskWrites; // 磁盘写入次数
        // 新增速度指标
        double readSpeed = 0.0;      // 读取速度（MB/s）
        double writeSpeed = 0.0;     // 写入速度（MB/s）
        double gpuUsage; // GPU使用率（百分比）
        double gpuMemory; // GPU内存使用量（MB）
    };

    // 获取系统资源指标
    static ResourceMetrics getMetrics()
    {
        ResourceMetrics metrics{};
        getMetrics(metrics);
        return metrics;
    }

private:
    static void getMetrics(ResourceMetrics& metrics);
};
#endif //SYSTEM_MONITOR_IMPL_H

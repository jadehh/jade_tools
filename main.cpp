/**
# @File     : main.cpp
# @Author   : jade
# @Date     : 2025/7/31 11:01
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : main.cpp
*/
#include <iomanip>
#include <iostream>
#include "include/jade_tools.h"
#include "include/utils.h"
#include "include/logger.h"
#include "include/enhance_monitor.h"
#include  <thread>
using namespace jade;
void cleanup1() {
    std::cout << "执行清理函数1" << std::endl;
}

void cleanup2() {
    std::cout << "执行清理函数2" << std::endl;
}

// 高性能计算任务 - 每个线程独立执行
void cpuIntensiveTask(){
    volatile double result = 0.0; // volatile 防止编译器优化
    int count = 0;
    while (count < 10000) {
        // 混合计算操作：浮点、三角函数、开方等
        for (int i = 1; i < 10000; i++) {
            result += std::sqrt(std::sin(i) * std::cos(i)) / std::log(i+1);
        }
        count++;
    }
}
int main(const int argc, char* argv[]) {
    // 注册清理函数
    // atexit(cleanup1);
    // atexit(cleanup2);
    Utils::setConsole();
    // 初始化日志系统
    Logger::getInstance().init("myapp","info", "Logs",Logger::S_DEBUG, true, true);
    // 设置日志级别 (可选)
    Logger::getInstance().setLevel(Logger::S_DEBUG);
    LOG_TRACE("trace");
    LOG_DEBUG("DEBUG");
    LOG_ERROR("错误");
    LOG_INFO("Application started");
    LOG_WARN("This is a warning message");
    LOG_ERROR("This is an error message");
    void* profiler = EnhancedTimeProfilerCreate();
    // Logger::getInstance().critical("严重的错误",-200);
    // 获取CPU核心数
    const u_int numCores = std::thread::hardware_concurrency();
    std::cout << "启动" << numCores << "个工作线程占满CPU..." << std::endl;
    EnhancedTimeProfilerStart(profiler,"处理阶段");
    std::vector<std::thread> threads;
    threads.reserve(12);
for (int i = 0; i < 12; ++i) {
        threads.emplace_back(cpuIntensiveTask);
    }
    // 无限运行，按Ctrl+C退出
    for (auto& thread : threads) {
        thread.join();
    }
    EnhancedTimeProfilerStop(profiler,"处理阶段",12);
    LOG_DEBUG(EnhancedTimeProfilerResult(profiler));
    EnhancedTimeProfilerDestroy(profiler);
    Logger::getInstance().shutDown();
    // 暂停10s
    return 0;
}


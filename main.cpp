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
    Logger::getInstance("").init("app1","info", "Logs",Logger::S_DEBUG, true, true);
    // 设置日志级别 (可选)
    LOG_DEBUG("debug");
    Logger::getInstance("app2");
    DLL_LOG_DEBUG("app2","app2 debug");
    LOG_WARN("使用的是默认的输出啊");
    Logger::getInstance("app2").shutDown();
    Logger::getInstance().shutDown();
    // 暂停10s
    return 0;
}


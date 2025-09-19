/**
# @File     : testCrash.cpp
# @Author   : jade
# @Date     : 2025/9/10 15:23
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testCrash.cpp
*/
#ifdef JADE_TOOLS
#include "jade_tools/jade_tools.h"
#else
#include "include/jade_tools.h"
#endif
#include "test/include/testCrash.h"

#include <thread>


void testCrash(const std::function<void()>& func)
{
    LOG_INFO() << "=====================================breakpad测试开始" << "=====================================";
    // 初始化崩溃处理器
    // crash_handler.setupHandler();
    // 设置清理函数
    jade::CrashHandler::getInstance().init("./crash/",func);
    // 设置自定义信息（线程安全）
    jade::CrashHandler::getInstance().setCustomInfo("AppVersion", "1.0.0");
    jade::CrashHandler::getInstance().setCustomInfo("UserID", "12345");

    // 创建工作线程
    std::thread worker([] {
       jade::CrashHandler::getInstance().setCustomInfo("ThreadID", "WorkerThread");
   });
    // 主线程逻辑...
    //  jade::CrashHandler::getInstance().triggerTestCrash();
    // 测试崩溃（仅用于调试）
    // 清理
    worker.join();
    LOG_INFO() << "=====================================breakpad结束" << "=====================================";
}

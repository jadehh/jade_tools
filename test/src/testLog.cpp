/**
# @File     : testLog.cpp
# @Author   : jade
# @Date     : 2025/9/8 10:21
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testLog.cpp
*/
#ifdef JADE_TOOLS
#include "jade_tools/jade_tools.h"
#else
#include "include/jade_tools.h"
#endif
#include "test/include/testLog.h"
#include <thread>

void testThreadLog()
{
    for (int i = 0; i < 2; ++i)
    {
        LOG_DEBUG() << "Thread Log";
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void testLog()
{
    jade::Logger::getInstance().init("app1", "info", "Logs", jade::Logger::S_TRACE, true, true, 1024 * 1024, 30);

    LOG_INFO() << "=====================================LOG日志测试开始" << "=====================================";
    LOG_TRACE() << "LogTrace" << 2 << 2.1;
    DLL_LOG_TRACE("App") << "App DLL LOG Trace";
    DLL_LOG_DEBUG("App") << "App DLL LOG DEBUG";
    LOG_DEBUG() << "LOG DEBUG";
    try
    {
        throw std::runtime_error("runtime error");
    }
    catch (std::exception& e)
    {
        LOG_EXCEPTION(0, e.what()) << "Message";
    }

    std::thread t1(testThreadLog);
    LOG_DEBUG() << "Start Thread";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    t1.join();
    LOG_INFO() << "=====================================LOG日志测试结束" << "=====================================";
    // LOG_CRITICAL(-200) << "LogTrace" << "拼接";
}

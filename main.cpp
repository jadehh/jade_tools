/**
# @File     : main.cpp
# @Author   : jade
# @Date     : 2025/7/31 11:01
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : main.cpp
*/
#ifdef JADE_TOOLS
#include "jade_tools/jade_tools.h"
#else
#include "include/jade_tools.h"
#endif
#include "test/test.h"

void cleanup()
{
    LOG_TRACE() << "================================清理==================================";
    jade::SqliteHelper::getInstance().close();  // Sqlite数据库释放必须放在主线程的最后
    jade::MultiRtspManager::getInstance().stopAll();
    jade::SocketServer::getInstance().stop();
    jade::HaspAdapter::getInstance().shutDown();
    jade::ApplicationController::getInstance().stop();
    jade::Logger::getInstance().shutDown();
    std::cout << "================================清理完成==================================" << std::endl;
}

int main(const int argc, char* argv[])
{
#ifdef _WIN32
    jade::Utils::setConsole();
#endif
    testLog(); // 单例类
    testOpencv();
    testCrash(cleanup); // 崩溃监听单例类
    testSqlite3(); // 数据库单例类，支持多线程操作
    testSocketServer(); // Socket 服务类
    // testAdapter();// 单例类 加密狗监听类
    testInIReader();
    // jade::CrashHandler::getInstance().triggerTestCrash();
    // // 主线程工作...
    // std::this_thread::sleep_for(std::chrono::milliseconds(13300));
    jade::ApplicationController::getInstance().run();
    jade::CrashHandler::getInstance().shutDown();
    cleanup();
    return 0;
}
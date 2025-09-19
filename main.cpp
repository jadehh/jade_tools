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
    jade::CrashHandler::getInstance().shutDown();
    jade::SqliteHelper::getInstance().close();
    jade::SocketServer::getInstance().stop();
    jade::HaspAdapter::getInstance().shutDown();
    jade::Logger::getInstance().shutDown();
}

int main(const int argc, char* argv[]) {
    atexit(cleanup);
#ifdef _WIN32
    jade::Utils::setConsole();
#endif
    testLog(); // 单例类
    testOpencv();
    testCrash(cleanup);// 单例类
    testSqlite3(); // 单例类
    testSocketServer(); // 单例类
    // testAdapter();// 单例类
    testInIReader();
    // // 主线程工作...
    jade::ApplicationController::getInstance().run();
    return 0;
}


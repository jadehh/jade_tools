/**
# @File     : testSocket.cpp
# @Author   : jade
# @Date     : 2025/9/15 16:41
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testSocket.cpp
*/
#include "test/include/testSocket.h"

void MessageHandle(const int socket, const std::string& message)
{
    LOG_DEBUG() << "处理socket自定义信息" << socket << message;
}

void testSocketServer()
{
    LOG_INFO() << "=====================================Socket Server测试开始" << "=====================================";
    jade::SocketServer::getInstance().init(8099, MessageHandle);
    jade::SocketServer::getInstance().start();
    LOG_INFO() << "=====================================Socket Server测试结束" << "=====================================";
}

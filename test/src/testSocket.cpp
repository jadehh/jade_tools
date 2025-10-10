/**
# @File     : testSocket.cpp
# @Author   : jade
# @Date     : 2025/9/15 16:41
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testSocket.cpp
*/
#include "test/include/testSocket.h"

#include <thread>

void MessageHandle(SOCKET_TYPE socket, const char*data, size_t size)
{
    LOG_DEBUG() << "处理socket自定义信息" << socket << data;
}

void testSocketServer()
{
    LOG_INFO() << "=====================================Socket Server测试开始" << "=====================================";
    std::shared_ptr<jade::SocketServer> socket_server = std::make_shared<jade::SocketServer>(8099,MessageHandle);
    socket_server->start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    socket_server->stop();
    LOG_INFO() << "=====================================Socket Server测试结束" << "=====================================";
}

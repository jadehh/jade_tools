/**
# @File     : socket_server.cpp
# @Author   : jade
# @Date     : 2025/9/15 16:59
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : socket_server.cpp
*/
#include <algorithm>
#include <thread>
#include <mutex>
#include <utility>
#include <vector>
#include <functional>
#include <atomic>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#define SOCKET_TYPE SOCKET
#define INVALID_SOCKET_TYPE INVALID_SOCKET
#define SOCKET_ERROR_TYPE SOCKET_ERROR
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET_TYPE int
#define INVALID_SOCKET_TYPE (-1)
#define SOCKET_ERROR_TYPE (-1)
#endif
#include "include/jade_tools.h"
#define MODULE_NAME "SocketServer"
using namespace jade;

class SocketServer::Impl final
{
public:
    using MessageHandler = std::function<void(SOCKET_TYPE, const std::string&)>;

    explicit Impl(const int port, MessageHandler callback):
        port_(port), serverSocket_(0), running_(false), callback_(std::move(callback))
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw std::runtime_error("WSAStartup failed");
        }
#endif
    }

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;

    void start()
    {
        if (running_)
            return;
        running_ = true;
        // 创建监听套接字
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ == INVALID_SOCKET_TYPE)
        {
            DLL_LOG_ERROR(MODULE_NAME) << "Failed to create socket";
            throw std::runtime_error("Failed to create socket");
        }

        // 设置服务器地址
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port_);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR_TYPE)
        {
            closeSocket(serverSocket_);
            DLL_LOG_CRITICAL(MODULE_NAME, -101) << "绑定端口失败,当前端口为:" << port_ << "请更换端口;";
            throw std::runtime_error("bind failed");
        }
        // 监听连接
        if (listen(serverSocket_, 5) == SOCKET_ERROR_TYPE)
        {
            closeSocket(serverSocket_);
            DLL_LOG_CRITICAL(MODULE_NAME, -102) << "监听端口失败";
            throw std::runtime_error("Listen failed");
        }
        DLL_LOG_INFO(MODULE_NAME) << "Socket服务已启动,监听端口号为:" << port_ << " ...";
        // 启动接受连接的线程
        acceptThread_ = std::thread(&Impl::acceptConnections, this);
    }

    ~ Impl()
    {
        if (!running_)
            return;
        running_ = false;
        // 关闭监听套接字
        closeSocket(serverSocket_);

        // 等待接受连接线程结束
        if (acceptThread_.joinable())
        {
            acceptThread_.join();
        }

        // 关闭所有客户端连接
        std::lock_guard lock(clientsMutex_);
        for (const auto client : clients_)
        {
            closeSocket(client);
        }
        clients_.clear();
#ifdef _WIN32
        WSACleanup();
#endif
        DLL_LOG_TRACE(MODULE_NAME) << "停止Socket服务完成 ...";
    }

private:
    int port_;
    SOCKET_TYPE serverSocket_;
    std::atomic<bool> running_;
    std::thread acceptThread_;
    std::vector<SOCKET_TYPE> clients_;
    std::mutex clientsMutex_;
    MessageHandler callback_;

    void acceptConnections()
    {
        while (running_)
        {
            sockaddr_in clientAddr{};
            socklen_t clientAddrLen = sizeof(clientAddr);
#ifdef _WIN32
            SOCKET clientSocket = accept(serverSocket_, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
#else
            SOCKET_TYPE clientSocket = accept(serverSocket_, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
#endif
            if (clientSocket == INVALID_SOCKET_TYPE)
            {
                if (running_)
                {
                    printError("Accept failed");
                }
                continue;
            }

            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
            DLL_LOG_DEBUG(MODULE_NAME) << "客户端连接 " << clientIP << ":" << ntohs(clientAddr.sin_port);

            // 添加客户端到列表
            {
                std::lock_guard lock(clientsMutex_);
                clients_.push_back(clientSocket);
            }

            // 为客户端创建处理线程
            std::thread clientThread(&Impl::handleClient, this, clientSocket);
            clientThread.detach();
        }
    }

    // 处理客户端消息
    void handleClient(const SOCKET_TYPE clientSocket)
    {
        char buffer[1024];

        while (running_)
        {
            // 接收数据
            const int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesReceived <= 0)
            {
                if (bytesReceived == 0)
                {
                    DLL_LOG_DEBUG(MODULE_NAME) << "客户端断开连接";
                }
                else
                {
                    printError("Receive failed");
                }
                break;
            }

            buffer[bytesReceived] = '\0';
            std::string message(buffer);

            // 调用消息处理方法
            callback_(clientSocket, message);
        }

        // 关闭客户端连接
        closeSocket(clientSocket);

        // 从客户端列表中移除
        std::lock_guard lock(clientsMutex_);
        clients_.erase(std::remove(clients_.begin(), clients_.end(), clientSocket), clients_.end());
    }

    static void printError(const std::string& message)
    {
#ifdef _WIN32
        DLL_LOG_ERROR(MODULE_NAME) << message.c_str() << ". Error code: " << WSAGetLastError();
#else
        perror(message.c_str());
#endif
    }

    static void closeSocket(SOCKET_TYPE socket)
    {
#ifdef _WIN32
        closesocket(socket);
#else
        shutdown(socket, SHUT_RDWR);
        close(socket);
#endif
    }
};

SocketServer& SocketServer::getInstance()
{
    static SocketServer instance;
    return instance;
}

SocketServer::SocketServer():
    impl_(nullptr)
{
}

void SocketServer::init(const int port, const MessageHandler& handler)
{
    impl_ = new Impl(port, handler);
}

void SocketServer::start() const
{
    if (impl_)
        impl_->start();
}

void SocketServer::stop()
{
    if (impl_)
    {
        delete impl_;
        impl_ = nullptr;
    }
}

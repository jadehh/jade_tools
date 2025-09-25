/**
# @File     : jade_tools.cpp
# @Author   : jade
# @Date     : 2025/8/1 13:51
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : jade_tools.cpp
*/

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <locale>
#include <codecvt>
#endif
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
#include "include/jade_tools.h"


using namespace std;

uint32_t jade::getIpAsInt(const char* ip)
{
    uint8_t a, b, c, d;
    if (sscanf_s(ip, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) != 4)
    {
        return 0;
    }
    return (a << 24) | (b << 16) | (c << 8) | d;
}


uint32_t jade::getIpAsInt(const std::string& ip)
{
    return getIpAsInt(ip.c_str());
}


std::string jade::getIntAsIp(const uint32_t ip)
{
    return (std::to_string((ip >> 24) & 0xFF) + "." +
        std::to_string((ip >> 16) & 0xFF) + "." +
        std::to_string((ip >> 8) & 0xFF) + "." +
        std::to_string(ip & 0xFF));
}

std::string jade::getLocalIP(const std::string& ip)
{
    // 创建UDP socket
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        throw std::runtime_error("WSAStartup failed");
    }
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        throw std::runtime_error("socket creation failed");
    }
#else
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("socket creation failed");
    }
#endif
    // 设置目标地址
    sockaddr_in targetAddr{};
    memset(&targetAddr, 0, sizeof(ip.c_str()));
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(80); // 任意端口
    if (inet_pton(AF_INET, ip.c_str(), &targetAddr.sin_addr) <= 0) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        throw std::runtime_error("invalid target IP address");
    }

    // 连接到目标IP（不实际发送数据）
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&targetAddr), sizeof(targetAddr)) < 0) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        throw std::runtime_error("connect failed");
    }

    // 获取本地绑定的IP地址
    sockaddr_in localAddr{};
    socklen_t addrLen = sizeof(localAddr);
    if (getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localAddr), &addrLen) < 0) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        throw std::runtime_error("getsockname failed");
    }

    char localIP[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &localAddr.sin_addr, localIP, INET_ADDRSTRLEN) == nullptr) {
#ifdef _WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        throw std::runtime_error("inet_ntop failed");
    }

#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd);
#endif
    return localIP;
}

std::string jade::getVersion()
{
    return {"1.0.0"};
}

void jade::printPrettyTable(const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& data,
                            const std::vector<int>& widths)
{
    cout << getPrettyTable(headers, data, widths) << endl;
}

std::string jade::getPrettyTable(const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& data,
                                 const std::vector<int>& widths)
{
    std::stringstream ss;
    std::cout << std::fixed;
    std::cout << std::setprecision(2);
    // 验证输入
    if (headers.empty())
    {
        cerr << "错误：表头不能为空！" << endl;
        return "";
    }
    const int colCount = static_cast<int>(headers.size());
    // 1. 自动计算列宽
    vector<int> colWidths = widths;
    if (colWidths.empty())
    {
        colWidths = vector(colCount, 0);

        for (int i = 0; i < colCount; ++i)
        {
            colWidths[i] = static_cast<int>(headers[i].length());
        }

        for (auto& row : data)
        {
            if (row.size() != colCount)
                continue;
            for (int i = 0; i < colCount; ++i)
            {
                if (static_cast<int>(row[i].length()) > colWidths[i])
                {
                    colWidths[i] = static_cast<int>(row[i].length());
                }
            }
        }

        // 为每个列增加4个字符的间距
        for (int& w : colWidths)
            w += 4;
    }

    // 2. 水平居中函数
    auto centerCell = [](const string& content, const int width) -> string
    {
        const int totalSpace = static_cast<int>(width - content.length());
        if (totalSpace < 0)
            return content; // 太长则截断
        const int leftSpace = totalSpace / 2;
        const int rightSpace = totalSpace - leftSpace;
        return string(leftSpace, ' ') + content + string(rightSpace, ' ');
    };

    // 4. 打印表头
    ss << "\n";
    ss << "|";
    for (int i = 0; i < colCount; ++i)
    {
        ss << centerCell(headers[i], colWidths[i]) << "|";
    }
    ss << endl;

    // 5. 打印表头分隔线
    ss << "|";
    for (int i = 0; i < colCount; ++i)
    {
        ss << string(colWidths[i], '-');
        ss << "|";
    }
    ss << endl;

    // 6. 打印表格数据
    for (auto& row : data)
    {
        if (row.size() != colCount)
            continue;

        ss << "|";
        for (int i = 0; i < colCount; ++i)
        {
            ss << centerCell(row[i], colWidths[i]) << "|";
        }
        ss << endl;
    }
    return ss.str();
}

bool jade::isImageFile(const std::string& path)
{
    string ext = path.substr(path.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), static_cast<int(*)(int)>(tolower));
    return ext == "jpg" || ext == "jpeg" || ext == "png" ||
        ext == "bmp" || ext == "gif" || ext == "tiff" ||
        ext == "webp" || ext == "svg";
}

std::string jade::formatValue(const double& value, const int precision, bool fixed)
{
    std::ostringstream oss;
    if (fixed)
        oss << std::fixed;
    oss << std::setprecision(precision) << value;
    return oss.str();
}


std::string jade::formatValue(const int& value, int precision, bool fixed)
{
    std::ostringstream oss;
    if (fixed)
        oss << std::fixed;
    oss << std::setprecision(precision) << value;
    return oss.str();
}


std::string jade::toHexString(const int code)
{
    char buffer[11]; // 足够存放"0x" + 8位十六进制数 + 结束符
    snprintf(buffer, sizeof(buffer), "0x%08X", code);
    return buffer;
}

std::string jade::toHexString(const unsigned int code)
{
    char buffer[11]; // 足够存放"0x" + 8位十六进制数 + 结束符
    snprintf(buffer, sizeof(buffer), "0x%08X", code);
    return buffer;
}


#ifdef _WIN32
std::string jade::formatBytes(const SIZE_T bytes)
{
    const char* suffixes[] = {"B", "KB", "MB", "GB"};
    int suffixIndex = 0;
    auto size = static_cast<double>(bytes);
    while (size >= 1024 && suffixIndex < 3)
    {
        size /= 1024;
        suffixIndex++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << suffixes[suffixIndex];
    return oss.str();
}

double jade::bytesToMB(const SIZE_T bytes)
{
    return static_cast<double>(bytes) / (1024 * 1024);
}


std::string jade::toHexString(const DWORD code)
{
    char buffer[11]; // 足够存放"0x" + 8位十六进制数 + 结束符
    snprintf(buffer, sizeof(buffer), "0x%08X", code);
    return buffer;
}
#endif

std::string jade::getOperatingSystemName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "macOS";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#else
    return "Other OS";
#endif
}

jade::jade_time jade::getTimeStamp()
{
    const auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    // 使用静态互斥锁保证线程安全
    static std::mutex mtx;
    jade_time result{};
    std::lock_guard lock(mtx);
    const auto since_epoch = now.time_since_epoch();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch) % 1000;
    // 安全获取本地时间
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&result, &time); // Windows安全版本
#else
    localtime_r(&time, &result); // 非Windows平台
#endif
    // 构造扩展时间对象
    result.tm_millis = static_cast<int>(millis.count());
    return result;
}

std::string jade::timePointToTimeString(const std::chrono::time_point<std::chrono::system_clock> clock, const char* fmt_arg,
                                        const bool with_milliseconds)
{
    // 使用静态互斥锁保证线程安全
    static std::mutex mtx;
    jade_time result{};
    std::lock_guard lock(mtx);
    // 1. 转换为秒级精度
    const auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(clock);
    const auto fraction = clock - seconds; // 获取小数部分
    // 2. 转换为本地时间
    auto t = std::chrono::system_clock::to_time_t(seconds);
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(fraction) % 1000;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&result, &t); // Windows安全版本
#else
    localtime_r(&t, &result); // 非Windows平台
#endif
    // 构造扩展时间对象
    result.tm_millis = static_cast<int>(millis.count());
    return getTimeStampString(result, fmt_arg, with_milliseconds);
}

std::string jade::getTimeStampString(const char* fmt_arg, const bool with_milliseconds)
{
    return getTimeStampString(getTimeStamp(), fmt_arg, with_milliseconds);

}

std::string jade::getTimeStampString(const jade_time& time, const char* fmt_arg, const bool with_milliseconds)
{
    std::ostringstream oss;
    oss << std::put_time(&time, fmt_arg);
    if (with_milliseconds)
    {
        oss << "." << std::setfill('0') << std::setw(3) << time.tm_millis;
    }
    return oss.str();
}

std::wstring jade::stringToWstring(const std::string& str)
{
#ifdef _WIN32
    if (str.empty())
        return L"";

    const int size_needed = MultiByteToWideChar(
        CP_UTF8, 0,
        str.c_str(), static_cast<int>(str.size()),
        nullptr, 0
        );

    std::wstring result(size_needed, 0);
    MultiByteToWideChar(
        CP_UTF8, 0,
        str.c_str(), static_cast<int>(str.size()),
        &result[0], size_needed
        );

    return result;
#else
    if (str.empty())
        return L"";

    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }
    catch (...)
    {
        return L"";
    }
#endif
}

std::string jade::wstringToString(const std::wstring& w_str)
{
#ifdef _WIN32
    if (w_str.empty())
        return "";

    const int size_needed = WideCharToMultiByte(
        CP_UTF8, 0,
        w_str.c_str(), static_cast<int>(w_str.size()),
        nullptr, 0, nullptr, nullptr
        );

    std::string result(size_needed, 0);
    WideCharToMultiByte(
        CP_UTF8, 0,
        w_str.c_str(), static_cast<int>(w_str.size()),
        &result[0], size_needed,
        nullptr, nullptr
        );

    return result;
#else
    if (w_str.empty())
        return "";

    try
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(w_str);
    }
    catch (...)
    {
        return "";
    }
#endif
}

std::string jade::getSeqNumber()
{
    const auto fmt_arg = "%Y%m%d%H%M%S";
    const jade_time time = getTimeStamp();
    std::ostringstream oss;
    oss << std::put_time(&time, fmt_arg);
    oss << std::setfill('0') << std::setw(3) << time.tm_millis;
    return oss.str();
}
/**
# @File     : main.cpp
# @Author   : jade
# @Date     : 2025/7/31 11:01
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : main.cpp
*/
#include <iostream>

#include "include/file_tools.h"
#include "include/jade_tools.h"
#include "include/utils.h"
#include "include/logger.h"
using namespace jade;
void cleanup1() {
    std::cout << "执行清理函数1" << std::endl;
}

void cleanup2() {
    std::cout << "执行清理函数2" << std::endl;
}
int main(const int argc, char* argv[]) {
    // 注册清理函数
    atexit(cleanup1);
    atexit(cleanup2);
    jade::Utils::SetConsoleUTF8();
    // 初始化日志系统
    Logger::getInstance().init("myapp","info", "Logs",Logger::S_DEBUG, true, true);
    // 设置日志级别 (可选)
    Logger::getInstance().setLevel(spdlog::level::debug);
    LOG_TRACE("trace");
    LOG_DEBUG("DEBUG");
    LOG_ERROR("错误");
    LOG_INFO("Application started");
    LOG_WARN("This is a warning message");
    LOG_ERROR("This is an error message");
    // Logger::getInstance().critical("严重的错误",-200);
    Logger::getInstance().shutDown();
    const char* test_char_list[2] = {"123","345"};
    // std::cout << test_char_list[0] << getStringArrayLength(test_char_list) << std::endl;
    return 0;
}


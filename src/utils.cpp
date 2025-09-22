/**
# @File     : utils.cpp
# @Author   : jade
# @Date     : 2025/7/31 17:41
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : utils.cpp
*/
//
#include "include/jade_tools.h"
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
using namespace jade;
#ifdef _WIN32
void Utils::setConsole()
{
    setConsoleUTF8();
    enableVirtualTerminal();
}

void Utils::setConsoleUTF8()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

void Utils::enableVirtualTerminal()
{
    // 启用虚拟终端处理
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#endif

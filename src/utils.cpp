/**
# @File     : utils.cpp
# @Author   : jade
# @Date     : 2025/7/31 17:41
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : utils.cpp
*/
//
#include "include/utils.h"
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
using namespace jade;
void Utils::SetConsoleUTF8()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

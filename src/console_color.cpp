/**
# @File     : console_color.cpp
# @Author   : jade
# @Date     : 2025/9/3 12:47
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : console_color.cpp
*/
#include <iostream>

#include "include/jade_tools.h"
using namespace jade;

void ConsoleColor::reset()
{
    std::cout << "\033[0m";
}

[[maybe_unused]] void ConsoleColor::bold()
{
    std::cout << "\033[1m";
}

[[maybe_unused]] void ConsoleColor::italic()
{
    std::cout << "\033[3m";
}

[[maybe_unused]] void ConsoleColor::reverse()
{
    std::cout << "\033[7m";
}

[[maybe_unused]] void ConsoleColor::setBackgroundColor(const int rgb[3])
{
    std::cout << "\033[48;2;" << rgb[0] << ";" << rgb[1] << ";" << rgb[2] << "m";
}

void ConsoleColor::setForegroundColor(const int rgb[3])
{
    std::cout << "\033[38;2;" << rgb[0] << ";" << rgb[1] << ";" << rgb[2] << "m";
}

[[maybe_unused]] void ConsoleColor::setBackgroundColor(int r, int g, int b)
{
    std::cout << "\033[48;2;" << r << ";" << g << ";" << b << "m";
}

void ConsoleColor::setForegroundColor(int r, int g, int b)
{
    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
}

[[maybe_unused]] std::string ConsoleColor::getForegroundColor() {
    return "\033[1;31m";
}


[[maybe_unused]] void ConsoleColor::underline()
{
    std::cout << "\033[4m";
}

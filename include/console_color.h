/**
# @File     : console_color.h
# @Author   : jade
# @Date     : 2025/8/8 10:30
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : console_color.h
*/
//

#ifndef CONSOLE_COLOR_H
#define CONSOLE_COLOR_H
#include "jade_tools.h"
#include <iostream>
namespace std
{
    class JADE_API ConsoleColor {
    public:
        // 重置所有样式
        static void reset() {
            std::cout << "\033[0m";
        }
        // 前景色（文本颜色）
        static void setForegroundColor(const int rgb[3]) {
            std::cout << "\033[38;2;" << rgb[0] << ";" << rgb[1] << ";" << rgb[2] <<  "m";
        }
        // 背景色
        static void setBackgroundColor(const int rgb[3]) {
            std::cout << "\033[48;2;" << rgb[0] << ";" << rgb[1] << ";" << rgb[2] << "m";
        }

        // 前景色（文本颜色）
        static void setForegroundColor(const int r, const int g, const int b) {
            std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
        }

        // 背景色
        static void setBackgroundColor(const int r, const int g, const int b) {
            std::cout << "\033[48;2;" << r << ";" << g << ";" << b << "m";
        }
        // 预定义颜色常量（RGB）
        struct Colors {
            static constexpr int RED[3] = {255, 0, 0};
            static constexpr int GREEN[3] = {0, 255, 0};
            static constexpr int BLUE[3] = {0, 0, 255};
            static constexpr int YELLOW[3] = {255, 255, 0};
            static constexpr int MAGENTA[3] = {255, 0, 255};
            static constexpr int CYAN[3] = {0, 255, 255};
            static constexpr int WHITE[3] = {255, 255, 255};
            static constexpr int BLACK[3] = {0, 0, 0};
            static constexpr int ORANGE[3] = {255, 165, 0};
            static constexpr int PURPLE[3] = {128, 0, 128};
        };

        // 文本样式
        static void bold()      { std::cout << "\033[1m"; }   // 粗体
        static void italic()    { std::cout << "\033[3m"; }   // 斜体
        static void underline() { std::cout << "\033[4m"; }   // 下划线
        static void reverse()   { std::cout << "\033[7m"; }   // 反色（交换前景和背景）
    };
}
#endif //CONSOLE_COLOR_H

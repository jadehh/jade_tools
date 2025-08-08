/**
# @File     : utils.h
# @Author   : jade
# @Date     : 2025/7/31 17:37
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : utils.h
*/
//

#ifndef UTILS_H
#define UTILS_H
#include "jade_tools.h"
namespace jade
{
    class JADE_API Utils
    {
    public:
        // 获取单例实例
        static Utils& GetInstance()
        {
            static Utils instance;
            return instance;
        }
        static void setConsole();
        static void setConsoleUTF8();
        static void enableVirtualTerminal();

    };
}
#endif //UTILS_H

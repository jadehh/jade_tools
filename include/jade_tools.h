/**
# @File     : jade_tools.h
# @Author   : jade
# @Date     : 2025/8/1 13:50
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : jade_tools.h
*/
//

#ifndef JADE_TOOLS_H
#define JADE_TOOLS_H
// 跨平台导出宏
#include <iostream>
#ifdef _WIN32
    #ifdef JADE_TOOLS_EXPORTS
        #define JADE_API __declspec(dllexport)
    #else
        #define JADE_API __declspec(dllimport)
    #endif
#else
    #define JADE_API __attribute__((visibility("default")))
#endif
using namespace std;
// 核心功能命名空间
namespace jade {
    JADE_API const char* getVersion();
    // 检查文件扩展名是否为图片格式
    JADE_API bool isImageFile(const std::string& path);
} // namespace jade
#endif // JADE_TOOLS_H

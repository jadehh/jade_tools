/**
# @File     : file_tools.h
# @Author   : jade
# @Date     : 2025/7/31 13:13
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : file_tools.h
*/
//

#ifndef FILE_TOOLS_H
#define FILE_TOOLS_H
#include "jade_tools.h"
#include <filesystem>
using namespace filesystem;
namespace jade
{
    class JADE_API FileTools
    {
    public:
        // 获取单例实例
        static FileTools& GetInstance() {
            static FileTools instance;
            return instance;
        }
        // 创建单层目录（父目录必须存在）
        static bool CreateDirectory(const string& path);

        // 递归创建多级目录（自动创建父目录）
        static bool CreateDirectories(const string& path);

        // 检查路径是否存在且是目录
        static bool Exists(const string& path);

        // 获取最后一次错误信息
        static string GetLastError();

        // 删除目录（可选功能）
        static bool Remove(const string& path);

        //处理不同操作系统下的路径分隔符差异，确保路径字符串在不同平台上都能正确工作
        static string FixPath(const string& path);


        // 获取文件夹下所有的图片文件
        static vector<string> getImageFiles(const string& path, bool fullPath = true);

        // 写入二进制文件
        static bool writeBinaryToFile(const string& path, const float* data,  int  size);
        static bool writeBinaryToFile(const string& path, const char * data,  int  size);
        // 禁止拷贝和赋值
        FileTools(const FileTools&) = delete;
        FileTools& operator=(const FileTools&) = delete;

    private:
        // 将错误信息设置为模块私有，通过静态方法访问
        static string& GetLastErrorRef() {
            static string lastError;
            return lastError;
        }
        FileTools() = default;  // 私有构造函数
        ~FileTools() = default;
    };
}

#endif //FILE_TOOLS_H

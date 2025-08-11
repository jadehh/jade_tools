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
#include <vector>
#ifdef _WIN32
#include <windows.h>
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
namespace jade
{
    struct jade_time : tm
    {
      int tm_millis;
    };
    JADE_API const char* getVersion();
    /**
    * 打印水平居中表格
    * @param headers  表头
    * @param data     表格数据
    * @param widths   列宽（可选）
    */
    JADE_API void printPrettyTable(const std::vector<std::string>& headers,
                        const  std::vector< std::vector<std::string>>& data,
                        const  std::vector<int>& widths = {}) ;
    /**
    * 输出表格
    * @param headers  表头
    * @param data     表格数据
    * @param widths   列宽（可选）
    * @return         表格字符串
    */
    JADE_API std::string getPrettyTable(const std::vector<std::string>& headers,
                        const  std::vector< std::vector<std::string>>& data,
                        const  std::vector<int>& widths = {}) ;
    // 检查文件扩展名是否为图片格式
    JADE_API bool isImageFile(const std::string& path);
    // 保留2位小数点
    std::string formatValue(const double& value, int precision = 2, bool fixed = true);
    std::string formatValue(int& value, int precision = 2, bool fixed = true);
    // 将字节转换为易读格式
    std::string formatBytes(SIZE_T bytes);

    double bytesToMB(SIZE_T bytes);
    /**
     * 获取操作系统名称
     * @reutrn   名称
     */
    std::string getOperatingSystemName();
    jade_time getTimeStamp();
    std::string getTimeStampString(const char*fmt_arg="[%Y-%m-%d %H:%M:%S]");

} // namespace jade


#endif // JADE_TOOLS_H

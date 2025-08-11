/**
# @File     : jade_tools.cpp
# @Author   : jade
# @Date     : 2025/8/1 13:51
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : jade_tools.cpp
*/
//
#include "include/jade_tools.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <mutex>
namespace jade
{
    const char* getVersion()
    {
        return "1.0.0";
    }

    void printPrettyTable(const std::vector<std::string>& headers,
                          const std::vector<std::vector<std::string>>& data,
                          const std::vector<int>& widths)
    {
        cout << getPrettyTable(headers, data, widths) << endl;
    }

    std::string getPrettyTable(const std::vector<std::string>& headers,
                               const std::vector<std::vector<std::string>>& data,
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
                if (row.size() != colCount) continue;
                for (int i = 0; i < colCount; ++i)
                {
                    if (row[i].length() > colWidths[i])
                    {
                        colWidths[i] = static_cast<int>(row[i].length());
                    }
                }
            }

            // 为每个列增加4个字符的间距
            for (int& w : colWidths) w += 4;
        }

        // 2. 水平居中函数
        auto centerCell = [](const string& content, const int width) -> string
        {
            const int totalSpace = static_cast<int>(width - content.length());
            if (totalSpace < 0) return content; // 太长则截断
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
            if (row.size() != colCount) continue;

            ss << "|";
            for (int i = 0; i < colCount; ++i)
            {
                ss << centerCell(row[i], colWidths[i]) << "|";
            }
            ss << endl;
        }
        return ss.str();
    }

    bool isImageFile(const string& path)
    {
        string ext = path.substr(path.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
        return ext == "jpg" || ext == "jpeg" || ext == "png" ||
            ext == "bmp" || ext == "gif" || ext == "tiff" ||
            ext == "webp" || ext == "svg";
    }

    std::string formatValue(const double& value, const int precision, const bool fixed)
    {
        std::ostringstream oss;
        if (fixed) oss << std::fixed;
        oss << std::setprecision(precision) << value;
        return oss.str();
    }

    std::string formatValue(const int& value, const int precision, const bool fixed)
    {
        std::ostringstream oss;
        if (fixed) oss << std::fixed;
        oss << std::setprecision(precision) << value;
        return oss.str();
    }

    std::string formatBytes(const SIZE_T bytes)
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

    double bytesToMB(const SIZE_T bytes)
    {
        return static_cast<double>(bytes) / (1024*1024);
    }

    std::string getOperatingSystemName()
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

    jade_time getTimeStamp()
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
        localtime_s(&result, &time);  // Windows安全版本
#else
        tm = *std::localtime(&time); // 非Windows平台
#endif
        // 构造扩展时间对象
        result.tm_millis = static_cast<int>(millis.count());
        return result;
    }

    std::string getTimeStampString(const char *fmt_arg)
    {
        std::ostringstream oss;
        const jade_time local_time = getTimeStamp();    // 获取毫秒部分
        oss << std::put_time(&local_time, fmt_arg)<< "." << std::setfill('0') << std::setw(3) << local_time.tm_millis;
        return oss.str();
    }


} // namespace jade

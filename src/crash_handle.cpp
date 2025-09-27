#include <csignal>
#include "include/jade_tools.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

// 平台特定的头文件
#ifdef _WIN32
#include "client/windows/handler/exception_handler.h"
#elif __APPLE__
#include "client/mac/handler/exception_handler.h"
#include <execinfo.h>  // 用于Linux/Mac的堆栈跟踪
#else
#include "client/linux/handler/exception_handler.h"
#include <execinfo.h>  // 用于Linux/Mac的堆栈跟踪
#endif

using namespace jade;
#ifdef LOW_GCC
#include <experimental/filesystem>
using namespace std::experimental;
namespace fs = filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#define MODULE_NAME "CrashHandler"

// Pimpl 实现类
class CrashHandler::Impl
{
public:
    explicit Impl(const std::string& dump_path, const std::function<void()>& func)
    {
        // 注册全局异常事件
        dump_path_ = dump_path;
        func_ = func;
        // 确保目录存在
        FileTools::createDirectories(dump_path);
#ifdef _WIN32
        const std::wstring wide_path = stringToWstring(dump_path);
        handler_ = new google_breakpad::ExceptionHandler(
            wide_path,
            nullptr, // 过滤器回调
            dumpCallback,
            this,    // 回调上下文
            google_breakpad::ExceptionHandler::HANDLER_ALL
        );
        // setup_global_handlers();
#elif __APPLE__
        handler_ = new google_breakpad::ExceptionHandler(
            dump_path,
            nullptr, // 过滤器回调
            dumpCallback,
            this,    // 回调上下文
            true,    // 安装信号处理器
            nullptr  // 服务器端口（用于上传）
        );
#else
        const google_breakpad::MinidumpDescriptor descriptor(dump_path);
        handler_ = new google_breakpad::ExceptionHandler(
            descriptor,
            nullptr, // 过滤器回调
            dumpCallback,
            this, // 回调上下文
            true, // 安装信号处理器
            -1 // 挂起线程的堆栈大小（-1 使用默认值）
            );
#endif
    }

    ~Impl()
    {
        delete handler_;
        DLL_LOG_TRACE(MODULE_NAME) << "关闭崩溃监听";
    }

    // 添加自定义信息
    void setCustomInfo(const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(custom_info_mutex_);
        custom_info_[key] = value;
    }

    // 清除自定义信息
    void clearCustomInfo()
    {
        std::lock_guard<std::mutex> lock(custom_info_mutex_);
        custom_info_.clear();
    }

    // 触发测试崩溃
    static void triggerTestCrash()
    {
        // 触发一个访问违例崩溃
        volatile int* ptr = nullptr;
        *ptr = 42;
    }

private:
    google_breakpad::ExceptionHandler* handler_ = nullptr;
    std::map<std::string, std::string> custom_info_;
    std::mutex custom_info_mutex_;
    std::string dump_path_;
    std::function<void()> func_;

    [[maybe_unused]] static std::string removeDmpSuffix(const std::string& filename)
    {
        // 查找最后一个点号的位置

        // 检查后缀是否为 .dmp（不区分大小写）
        if (const auto dot_pos = static_cast<size_t>(filename.rfind('.')); dot_pos != std::string::npos &&
            filename.size() - dot_pos == 4 &&
            (filename.substr(dot_pos + 1) == "dmp" ||
                filename.substr(dot_pos + 1) == "DMP"))
        {
            return filename.substr(0, dot_pos);
        }
        return filename;
    }

    // 生成唯一的文件名
    static std::string generateUniqueFilename(const std::string& prefix, const std::string& extension)
    {
        const auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        // 安全获取本地时间
        std::tm time_info = {};
#ifdef _WIN32
        localtime_s(&time_info, &in_time_t);
#else
        localtime_r(&in_time_t, &time_info);
#endif
        // 格式化为字符串
        std::ostringstream oss;
        oss << prefix << "_" << std::put_time(&time_info, "%Y%m%d_%H%M%S") << "." << extension;
        return oss.str();
    }

    // 重命名 minidump 文件
    [[nodiscard]] std::string renameMinidumpFile(const std::string& original_path) const
    {
        try
        {
            // 生成新的文件名
            const std::string new_filename = generateUniqueFilename("crash", "dmp");
            const fs::path new_path = fs::path(dump_path_) / new_filename;

            // 重命名文件
            fs::rename(original_path, new_path);

            // 返回新路径用于后续处理
            return new_path.string();
        }
        catch (const fs::filesystem_error& e)
        {
            std::cerr << "Failed to rename minidump file: " << e.what() << std::endl;
            return original_path; // 返回原始路径作为回退
        }
    }

    // 平台特定的回调函数
#ifdef _WIN32
    static bool dumpCallback(
        const wchar_t* dump_path,
        const wchar_t* minidump_id,
        void* context,
        [[maybe_unused]] EXCEPTION_POINTERS* ex_info,
        [[maybe_unused]] MDRawAssertionInfo* assertion,
        const bool succeeded
    ) {
        const auto self = static_cast<Impl*>(context);
        return self->handleCrash(succeeded, dump_path, minidump_id);
    }
#elif __APPLE__
    static bool dumpCallback(
        void* context,
        bool succeeded
    ) {
        const auto self = static_cast<Impl*>(context);
        return self->handleCrash(succeeded);
    }
#else
    static bool dumpCallback(
        const google_breakpad::MinidumpDescriptor& descriptor,
        void* context,
        const bool succeeded
        )
    {
        const auto self = static_cast<Impl*>(context);
        return self->handleCrash(succeeded, nullptr, nullptr, descriptor.path());
    }
#endif
    // 区分操作系统的处理逻辑

#ifdef _WIN32
    bool handleCrash(const bool succeeded,
                     const wchar_t* dump_path = nullptr,
                     const wchar_t* minidump_id = nullptr) const
    {
        if (!succeeded)
        {
            DLL_LOG_ERROR(MODULE_NAME) << "Failed to generate minidump";
            return false;
        }
        std::string original_dump_path;
        const std::wstring wpath = std::wstring(dump_path) + L"\\" + std::wstring(minidump_id) + L".dmp";
        original_dump_path = wstringToString(wpath);
        std::ostringstream oss;
        oss << "程序崩溃,崩溃原因二进制文件地址为:";
        // 重命名 minidump 文件
        const std::string new_dump_path = renameMinidumpFile(original_dump_path);
        oss << new_dump_path;
        // 获取崩溃信息文件路径（基于重命名后的文件）
        // const std::string info_path = removeDmpSuffix(new_dump_path) + ".txt";
        // oss << ",崩溃日志文件地址为:" << info_path;
        // 写入自定义信息
        // writeCustomInfo(info_path);
        DLL_LOG_ERROR(MODULE_NAME) << oss.str() << ",程序异常退出,准备清理资源 ...";
        if (func_)
            func_();
        return true;
    }
# elif __APPLE__
    bool handleCrash(const bool succeeded,
                     const wchar_t* dump_path = nullptr,
                     const wchar_t* minidump_id = nullptr,
                     [[maybe_unused]] const char* linux_path = nullptr) const {
        if (!succeeded)
        {
            DLL_LOG_ERROR(MODULE_NAME) << "Failed to generate minidump";
            return false;
        }
        // 获取原始 minidump 文件路径
        std::string original_dump_path;
        // macOS 需要特殊处理获取路径
        original_dump_path = getMacDumpPath();
        std::ostringstream oss;
        oss << "程序崩溃,崩溃原因二进制文件地址为:";
        // 重命名 minidump 文件
        const std::string new_dump_path = renameMinidumpFile(original_dump_path);
        oss << new_dump_path;
        // 获取崩溃信息文件路径（基于重命名后的文件）
        // const std::string info_path = removeDmpSuffix(new_dump_path) + ".txt";
        // oss << ",崩溃日志文件地址为:" << info_path;
        // 写入自定义信息
        // writeCustomInfo(info_path);
        DLL_LOG_ERROR(MODULE_NAME) << oss.str() << ",程序异常退出,准备清理资源 ...";
        if (func_)
            func_();
        return true;
    }
#else
    bool handleCrash(const bool succeeded,
                     const wchar_t* dump_path = nullptr,
                     const wchar_t* minidump_id = nullptr,
                     [[maybe_unused]] const char* linux_path = nullptr) const {
        if (!succeeded)
        {
            DLL_LOG_ERROR(MODULE_NAME) << "Failed to generate minidump";
            return false;
        }
        // 获取原始 minidump 文件路径
        std::string original_dump_path;
        original_dump_path = std::string(linux_path);
        std::ostringstream oss;
        oss << "程序崩溃,崩溃原因二进制文件地址为:";
        // 重命名 minidump 文件
        const std::string new_dump_path = renameMinidumpFile(original_dump_path);
        oss << new_dump_path;
        // 获取崩溃信息文件路径（基于重命名后的文件）
        // const std::string info_path = removeDmpSuffix(new_dump_path) + ".txt";
        // oss << ",崩溃日志文件地址为:" << info_path;
        // 写入自定义信息
        // writeCustomInfo(info_path);
        DLL_LOG_ERROR(MODULE_NAME) << oss.str() << ",程序异常退出,准备清理资源 ...";
        if (func_)
            func_();
        return true;
    }
#endif

#ifdef __APPLE__
    // macOS 获取崩溃文件路径
    std::string getMacDumpPath() {
        // macOS 上获取崩溃文件路径比较复杂
        // 这里简化处理，实际应用中需要更完整的实现
        return "/tmp/crashdump";
    }
#endif
    /**写入自定义信息*/
    [[maybe_unused]] void writeCustomInfo(const std::string& info_path)
    {
        // 获取当前时间
        const auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        // 复制自定义信息（减少锁的持有时间）
        std::map<std::string, std::string> local_copy;
        {
            std::lock_guard lock(custom_info_mutex_);
            local_copy = custom_info_;
        }
        // 写入文件
        if (std::ofstream outfile(info_path, std::ios::app); outfile.is_open())
        {
            std::tm time_info = {};
#ifdef _WIN32
            // Windows 安全版本
            localtime_s(&time_info, &in_time_t);
#else
            // POSIX 安全版本
            localtime_r(&in_time_t, &time_info);
#endif
            outfile << "Crash time: " << std::put_time(&time_info, "%Y-%m-%d %H:%M:%S") << "\n";
            // 写入自定义信息
            for (const auto& [key, value] : local_copy)
            {
                outfile << key << ": " << value << "\n";
            }
            outfile.close();
        }
    }
};


CrashHandler& CrashHandler::getInstance()
{
    static CrashHandler instance;
    return instance;
}

// CrashHandler 公共接口实现
CrashHandler::CrashHandler():
    impl_(nullptr)
{
}

void CrashHandler::init(const std::string& dumpPath, const std::function<void()>& func)
{
    impl_ = new Impl(dumpPath, func);
}

void CrashHandler::shutDown()
{
    if (impl_)
    {
        delete impl_;
        impl_ = nullptr;
    }
}

void CrashHandler::setCustomInfo(const std::string& key, const std::string& value) const
{
    if (impl_)
    {
        impl_->setCustomInfo(key, value);
    }
}

[[maybe_unused]] void CrashHandler::clearCustomInfo() const
{
    if (impl_)
    {
        impl_->clearCustomInfo();
    }
}

[[maybe_unused]] void CrashHandler::triggerTestCrash() const
{
    if (impl_)
    {
        Impl::triggerTestCrash();
    }
}

/**
# @File     : logger.cpp
# @Author   : jade
# @Date     : 2025/7/31 11:11
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : logger.cpp
*/
//
#include "include/jade_tools.h"
#ifdef LOW_GCC
#include <experimental/filesystem>
using namespace std::experimental::filesystem;
#else
#include <filesystem>
using namespace std::filesystem;
#endif
#include <map>
#include <sstream>
#if defined(__has_include)
#  if __has_include(<spdlog/spdlog.h>)  // 标准化的头文件存在性检查
#    include <spdlog/spdlog.h>
#    include "spdlog/sinks/stdout_color_sinks.h"
#    include "spdlog/sinks/rotating_file_sink.h"
#    include <spdlog/fmt/chrono.h> // 用于时间格式化
#    define SPDLOG_ENABLE 1
#else
#include <thread>
#  endif
#endif
#include <string>
#include <utility>
using namespace jade;
#define MODULE_NAME "Logger"

class CustomFormatter final : public spdlog::formatter
{
    std::map<spdlog::level::level_enum, const char*> level_names;
    std::map<spdlog::level::level_enum, const char*> level_colors;

public:
    explicit CustomFormatter(const bool is_console_sink):
        is_console_sink_(is_console_sink)
    {
        // 设置级别名称
        level_names = {
            {spdlog::level::trace, "TRACE"},
            {spdlog::level::debug, "DEBUG"},
            {spdlog::level::info, "INFO"},
            {spdlog::level::warn, "WARNING"},
            {spdlog::level::err, "ERROR"},
            {spdlog::level::critical, "CRITICAL"}
        };

        // 设置级别颜色
        level_colors = {
            {spdlog::level::trace, "\033[37m"}, // 灰色
            {spdlog::level::debug, "\033[36m"}, // 青色
            {spdlog::level::info, "\033[32m"}, // 绿色
            {spdlog::level::warn, "\033[33m"}, // 黄色
            {spdlog::level::err, "\033[31m"}, // 红色
            {spdlog::level::critical, "\033[1;31m"} // 粗体红色
        };
    }

    // 设置应用程序名称（在程序开始时调用一次）
    static void setAppName(const std::string& name)
    {
        appName() = name;
    }

    static void setDllName(const std::string& name)
    {
        dllName() = name;
    }

    // 设置是否显示文件行数
    static void setShowLineNumbers(const bool showLine)
    {
        showLineNumbers() = showLine;
    }

    static std::string getName()
    {
        if (!dllName().empty())
        {
            return dllName();
        }
        return appName();
    }

    void format(const spdlog::details::log_msg& msg, spdlog::memory_buf_t& dest) override
    {
        const char* RESET = is_console_sink_ ? "\033[0m" : ""; // 重置颜色
        // 1.添加时间 一行 带颜色
        if (is_console_sink_)
        {
            dest.append(level_colors[msg.level], level_colors[msg.level] + strlen(level_colors[msg.level]));
        }
        fmt::format_to(fmt::appender(dest), "{}", jade::timePointToTimeString(msg.time, "%Y-%m-%d %H:%M:%S", true));
        // 2. 添加App名称
        fmt::format_to(fmt::appender(dest), " - [{}] -", getName());
        // 3. 添加等级
        fmt::format_to(fmt::appender(dest), " {} ", level_names[msg.level]);
        fmt::format_to(fmt::appender(dest), "- ");
        // 4. 添加文件信息
        if (showLineNumbers() && msg.source.filename)
        {
            // 提取文件名（不含路径）
            const char* filename = msg.source.filename;
            const char* basename = filename;
            for (const char* p = filename; *p != '\0'; p++)
            {
                if (*p == '/' || *p == '\\')
                {
                    basename = p + 1;
                }
            }
            fmt::format_to(fmt::appender(dest), "[{}:{}] ", basename, msg.source.line);
        }

        // 5. 添加线程信息
        fmt::format_to(fmt::appender(dest), "[Thread {}] : ", msg.thread_id);

        // 6. 添加日志消息内容
        dest.append(msg.payload.data(), msg.payload.data() + msg.payload.size());
        if (is_console_sink_)
        {
            dest.append(RESET, RESET + strlen(RESET));
        }
        // 7. 添加换行符
        dest.push_back('\n');
    }

    [[nodiscard]] std::unique_ptr<formatter> clone() const override
    {
        return std::make_unique<CustomFormatter>(is_console_sink_);
    }

private:
    // 使用静态局部变量来存储AppName，保证线程安全
    static std::string& appName()
    {
        static std::string appName;
        return appName;
    }

    // 使用静态局部变量来存储DLLName，保证线程安全
    static std::string& dllName()
    {
        static std::string dllName;
        return dllName;
    }

    static bool& showLineNumbers()
    {
        static bool showLineNumbers = true;
        return showLineNumbers;
    }

    bool is_console_sink_ = false; // 新增：标识是否为控制台输出
};

class Logger::SpdLoggerIMPL
{
public:
    bool initialized_ = false;
    bool shutdown_ = false;

    ~SpdLoggerIMPL()
    {
        DLL_LOG_TRACE(MODULE_NAME) << "关闭日志完成";
        shutdown_ = true;
        initialized_ = true;
#ifdef SPDLOG_ENABLE
        sink.reset();
#endif
    }
#ifdef SPDLOG_ENABLE
    std::shared_ptr<spdlog::logger> sink = nullptr;
#endif
    // 正确的构造函数
    SpdLoggerIMPL() = default;

};

class LoggerStream::Impl
{
public:
    Impl(const Logger::Level level, const char* file, const int line, const int exitCode, std::string exceptionMsg):
        level_(level), line_(line), exitCode_(exitCode), file_(file), moduleName_(""),
        exceptionMsg_(std::move(exceptionMsg))
    {
    }

    void setModuleName(const char* moduleName) { moduleName_ = moduleName; }
    void setStream(const char* value) { buffer_ << value; }
    void setStream(const int value) { buffer_ << value; }
    void setStream(const double value) { buffer_ << value; }
    void setStream(const float value) { buffer_ << value; }
    void setStream(const std::string& value) { buffer_ << value; }
    void setStream(const bool value) { buffer_ << value; }
    void setStream(const long value) { buffer_ << value; }
    void setStream(const unsigned int value) { buffer_ << value; }
    void setStream(const std::bitset<16> value) { buffer_ << value; }
#ifdef _WIN32
    void setStream(const int64_t value) { buffer_ << value; }
#else
    void setStream(const unsigned long value) { buffer_ << value; }
#endif
    void log() const
    {
        CustomFormatter::setDllName(moduleName_);
        switch (level_)
        {
        case Logger::Level::S_TRACE:
            Logger::getInstance().trace(buffer_.str(), file_, line_);
            break;
        case Logger::Level::S_DEBUG:
            Logger::getInstance().debug(buffer_.str(), file_, line_);
            break;
        case Logger::Level::S_INFO:
            Logger::getInstance().info(buffer_.str(), file_, line_);
            break;
        case Logger::Level::S_WARNING:
            Logger::getInstance().warn(buffer_.str(), file_, line_);
            break;
        case Logger::Level::S_ERROR:
            Logger::getInstance().error(buffer_.str(), file_, line_);
            break;
        case Logger::Level::S_CRITICAL:
            Logger::getInstance().critical(buffer_.str(), exitCode_, file_, line_);
            break;
        case Logger::Level::S_EXCEPTION:
            Logger::getInstance().exception(buffer_.str(), exceptionMsg_, exitCode_, file_, line_);
            break;
        default:
            break;
        }
    }

private:
    Logger::Level level_;
    std::ostringstream buffer_;
    int line_, exitCode_;
    const char* file_;
    const char* moduleName_;
    std::string exceptionMsg_;
};

void DLLLoggerStream::setModuleName(const char* moduleName) const
{
    getImpl()->setModuleName(moduleName);
}

DLLLoggerStream::DLLLoggerStream(const Logger::Level level, const char* file, const int line,
                                 const char* moduleName, const int exitCode, const std::string& e):
    LoggerStream(level, file, line, exitCode, e)
{
    setModuleName(moduleName);
}

LoggerStream::LoggerStream(const Logger::Level level, const char* file, const int line, const int exitCode,
                           const std::string& e) :
    impl_(new Impl(level, file, line, exitCode, e))
{
}

LoggerStream::Impl* LoggerStream::getImpl() const
{
    return impl_;
}

LoggerStream& LoggerStream::operator<<(const char* value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const std::string& value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const int value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const double value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const float value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const bool value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const long value)
{
    impl_->setStream(value);
    return *this;
}

LoggerStream& LoggerStream::operator<<(const unsigned int value)
{
    impl_->setStream(value);
    return *this;
}
LoggerStream& LoggerStream::operator<<(const std::bitset<16> value)
{
    impl_->setStream(value);
    return *this;
}
#ifdef _WIN32
LoggerStream& LoggerStream::operator<<(int64_t value)
{
    impl_->setStream(value);
    return *this;
}
#else
LoggerStream&LoggerStream::operator<<(const unsigned long value)
{
    impl_->setStream(value);
    return *this;
}
#endif
LoggerStream::~LoggerStream()
{
    impl_->log();
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}


Logger::Logger() :
    logger_(new SpdLoggerIMPL())
{
}

void Logger::init(const std::string& app_name, const std::string& logName, const std::string& logDir,
                  Level logLevel, const bool consoleOutput, const bool fileOutput, size_t maxFileSize,
                  size_t maxFiles) const

{
#ifdef SPDLOG_ENABLE
    try
    {
        CustomFormatter::setAppName(app_name);
        if (logLevel <= S_DEBUG)
            CustomFormatter::setShowLineNumbers(true);

        std::vector<spdlog::sink_ptr> sinks;

        // 控制台输出
        if (consoleOutput)
        {
            const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            std::unique_ptr<spdlog::formatter> formater = std::make_unique<CustomFormatter>(true);
            consoleSink->set_formatter(std::move(formater));
            sinks.push_back(consoleSink);


        }
        // 文件输出
        if (fileOutput)
        {
            // 创建日志目录
#ifdef LOW_GCC
            std::experimental::filesystem::create_directories(logDir);
#else
                std::filesystem::create_directories(logDir);
#endif
            const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logDir + "/" + logName + ".log", maxFileSize, maxFiles);
            std::unique_ptr<spdlog::formatter> formater = std::make_unique<CustomFormatter>(false);
            fileSink->set_formatter(std::move(formater));
            sinks.push_back(fileSink);
        }
        logger_->sink = std::make_shared<spdlog::logger>(logName, begin(sinks), end(sinks));
        // 创建logger
        logger_->sink->set_level(static_cast<spdlog::level::level_enum>(logLevel)); // 默认记录所有级别
        logger_->sink->flush_on(spdlog::level::trace); // 立即刷新
        logger_->initialized_ = true;
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        // 如果初始化失败，使用默认logger
        logger_->sink = spdlog::stdout_color_mt(logName);
        logger_->sink->error("Logger initialization failed: " + std::string(ex.what()));
    }
#else
        logger_->initialized_ = true;
#endif
}

[[maybe_unused]] void Logger::setDllName(const std::string& dllName)
{
    CustomFormatter::setDllName(dllName);
}

void Logger::trace(const std::string& message, const char* file, const int line) const
{
    log(S_TRACE, message, file, line);
}

void Logger::debug(const std::string& message, const char* file, const int line) const
{
    log(S_DEBUG, message, file, line);
}

void Logger::info(const std::string& message, const char* file, const int line) const
{
    log(S_INFO, message, file, line);
}

void Logger::warn(const std::string& message, const char* file, const int line) const
{
    log(S_WARNING, message, file, line);
}

void Logger::error(const std::string& message, const char* file, const int line) const
{
    log(S_ERROR, message, file, line);
}

void Logger::getError(const std::string& message, const int exitCode, const char* file, const int line) const
{
    if (exitCode != 0)
    {
        std::ostringstream ss;
        ss << message << ",程序退出,退出代码为:" << exitCode;
        log(S_CRITICAL, ss.str(), file, line);
        exit(exitCode);
    }
}

void Logger::critical(const std::string& message, const int exitCode, const char* file, const int line) const
{
    getError(message, exitCode, file, line);
    log(S_CRITICAL, message, file, line);
}

void Logger::exception(const std::string& message, const std::string& e, const int exitCode, const char* file,
                       const int line) const
{
    std::stringstream ss;
    ss << message << ",失败的原因:" << e;
    getError(ss.str(), exitCode, file, line);
    log(S_CRITICAL, ss.str(), file, line);
}

void Logger::log(const Level level, const std::string& message, const char* file, const int line) const
{
#if SPDLOG_ENABLE
    if (logger_->sink)
        logger_->sink->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION},
                           static_cast<spdlog::level::level_enum>(level), message);
#else
        std::stringstream ss;
        ss << getTimeStampString("%Y-%m-%d %H:%M:%S")   << " - [" << logger_->app_name_ << "] - " ;
        // 根据日志级别设置不同颜色
        if (level == Logger::S_TRACE)
        {
            ConsoleColor::setForegroundColor(ConsoleColor::Colors::BLACK);
            ss << "TRACE";
        }
        else if (level == Logger::S_DEBUG)
        {
            // ConsoleColor::setBackgroundColor(ConsoleColor::Colors::WHITE);
            ss << "DEBUG";
        }
        else if (level == Logger::S_INFO)
        {
            ConsoleColor::setForegroundColor(ConsoleColor::Colors::BLUE);
            ss << "INFO";
        }
        else if (level == Logger::S_WARNING)
        {
            ConsoleColor::setForegroundColor(ConsoleColor::Colors::YELLOW);
            ss << "WARNING";
        }
        else if (level == Logger::S_ERROR)
        {
            ConsoleColor::setForegroundColor(ConsoleColor::Colors::RED);
            ss << "ERROR";
        }
        else if (level == Logger::S_CRITICAL)
        {
            ConsoleColor::setForegroundColor(ConsoleColor::Colors::PURPLE);
            ss << "CRITICAL";
        }

        // 2025-08-21 22:04:09.716 - [app1] - DEBUG - [main.cpp:46] [Thread 30696] : DEBUG

        ss   << " - [" << file << ":" << line << "] " << "[Thread " << std::this_thread::get_id() << "] : " << message << std::endl;
        std::cout << ss.str();
        ConsoleColor::reset();
#endif
}


[[maybe_unused]] void Logger::setLevel(const Level level) const
{
#if SPDLOG_ENABLE
    if (logger_->initialized_ && !logger_->shutdown_)
    {
        if (logger_->sink)
            logger_->sink->set_level(static_cast<spdlog::level::level_enum>(level));
    }
#endif
}

void Logger::shutDown()
{

    if (logger_)
    {
        delete logger_;
        logger_ = nullptr;
    }
}

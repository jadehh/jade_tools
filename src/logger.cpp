/**
# @File     : logger.cpp
# @Author   : jade
# @Date     : 2025/7/31 11:11
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : logger.cpp
*/
//
#include "include/logger.h"
#include "include/logger_impl.h"
#include <spdlog/spdlog.h>
#include "spdlog/sinks/rotating_file_sink.h"
#include <filesystem>
#include <sstream>

namespace jade
{
    Logger& Logger::getInstance()
    {
        static Logger instance;
        return instance;
    }

    Logger::Logger() : impl_(new LoggerImpl()) {}

    Logger::~Logger()
    {
        delete impl_;

    };

    void Logger::init(
        const std::string& app_name,
        const std::string& logName,
        const std::string& logDir,
        Level logLevel,
        const bool consoleOutput,
        const bool fileOutput,
        const size_t maxFileSize,
        const size_t maxFiles) const
    {
        impl_->init(app_name, logName, logDir, logLevel,consoleOutput, fileOutput, maxFileSize, maxFiles);
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

    void Logger::critical(const std::string& message, const int exitCode, const char* file, const int line) const
    {
        if (exitCode != 0)
        {
            std::ostringstream oss;
            oss << message << ", 程序退出,退出代码为:" << exitCode; // 支持混合类型
            log(S_CRITICAL, oss.str(), file, line);
            getInstance().shutDown();
            exit(exitCode);
        }
        log(S_CRITICAL, message, file, line);
    }

    void Logger::exception(const std::string& message, const std::exception& e, const int exitCode, const char* file, const int line) const
    {
        std::stringstream ss;
        ss <<  message <<",失败的原因:" << e.what();
        if (exitCode != 0)
        {
            ss << ",程序退出,退出代码为:"<<exitCode;
            log(S_CRITICAL, ss.str(), file, line);
            getInstance().shutDown();
            exit(exitCode);
        }
        log(S_CRITICAL, ss.str(), file, line);
    }

    void Logger::log(Level level, const std::string& message, const char* file, const int line) const
    {
        impl_->log(level, message, file, line);
    }


    void Logger::setLevel(const spdlog::level::level_enum level) const
    {
        impl_->setLevel(level);
    }

    void Logger::flush() const
    {
        impl_->flush();
    }

    void Logger::shutDown() const
    {
        impl_->shutDown();
    }

    // LoggerImpl 实现
    LoggerImpl::LoggerImpl() = default;

    LoggerImpl::~LoggerImpl()
    {
        shutDown();
    }


    void LoggerImpl::init(const std::string& app_name, const std::string& logName, const std::string& logDir,Logger::Level logLevel,
                          const bool consoleOutput, const bool fileOutput, size_t maxFileSize, size_t maxFiles)
    {
        try
        {
            std::vector<spdlog::sink_ptr> sinks;


            // 控制台输出
            if (consoleOutput)
            {
                const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                consoleSink->set_pattern("%Y-%m-%d %H:%M:%S.%e - [" + app_name + "] - %^%l%$ - [%s:%#] [Thread %t] %v");
                sinks.push_back(consoleSink);
            }
            // 文件输出
            if (fileOutput)
            {
                // 创建日志目录
                std::filesystem::create_directories(logDir);
                const auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logDir + "/" + logName + ".log", maxFileSize, maxFiles);
                fileSink->set_pattern("%Y-%m-%d %H:%M:%S.%e - [" + app_name + "] - %^%l%$:%v");
                sinks.push_back(fileSink);
            }

            // 创建logger
            logger_ = std::make_shared<spdlog::logger>(logName, begin(sinks), end(sinks));
            logger_->set_level(static_cast<spdlog::level::level_enum>(logLevel)); // 默认记录所有级别
            logger_->flush_on(spdlog::level::trace); // 立即刷新
            // 注册logger以便全局访问
            spdlog::register_logger(logger_);
            initialized_ = true;
        }
        catch (const spdlog::spdlog_ex& ex)
        {
            // 如果初始化失败，使用默认logger
            logger_ = spdlog::stdout_color_mt(logName);
            logger_->error("Logger initialization failed: " + std::string(ex.what()));
        }
    }

    void LoggerImpl::log(Logger::Level level, const std::string& message, const char* file, const int line) const
    {
        if (initialized_ && !shutdown_)
        {
            if (logger_) logger_->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION},
                                      static_cast<spdlog::level::level_enum>(level), message);
        }
    }


    void LoggerImpl::setLevel(const spdlog::level::level_enum level) const
    {
        if (initialized_ && !shutdown_)
        {
            if (logger_) logger_->set_level(level);
        }
    }

    void LoggerImpl::flush() const
    {
        if (initialized_ && !shutdown_)
        {
            if (logger_) logger_->flush();
        }
    }

    void LoggerImpl::shutDown()
    {
        if (shutdown_ || !initialized_)
        {
            return;
        }
        try
        {
            if (logger_)
            {
                flush();
                // 从全局注册表中移除
                spdlog::drop(logger_->name());
                // 释放logger
                logger_.reset();
            }
            // 关闭spdlog（可选，会关闭所有logger）
            // spdlog::shutdown();
            shutdown_ = true;
            initialized_ = false;
        }
        catch ([[maybe_unused]] const spdlog::spdlog_ex& ex)
        {
            // 如果使用默认logger，尝试简单释放
            logger_.reset();
        }
    }
}

/**
# @File     : logger.h
# @Author   : jade
# @Date     : 2025/7/31 11:12
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : logger.h
*/
#ifndef JADE_TOOLS_LOGGER_H
#define JADE_TOOLS_LOGGER_H

#define SPDLOG_LEVEL_NAMES {"TRACE","DEBUG","INFO","WARNING","ERROR","CRITICAL","OFF" }
#include <map>

#include "jade_tools.h"
#include <string>

namespace jade
{
    class LoggerImpl; // 前向声明
}

namespace jade
{
    class JADE_API Logger {
    public:
        // 日志级别枚举
        enum Level {
            S_TRACE = 0,
            S_DEBUG = 1,
            S_INFO = 2,
            S_WARNING = 3,
            S_ERROR = 4,
            S_CRITICAL = 5,
        };

        // 获取单例实例
        static Logger& getInstance(const std::string& module = "");

        // 初始化日志系统
        void init(
                  const std::string& app_name = "app",
                  const std::string& logName = "info",
                  const std::string& logDir = "Logs",
                  Level logLevel = S_INFO,
                  bool consoleOutput = true,
                  bool fileOutput = true,
                  size_t maxFileSize = 1024 * 1024 * 1, // 1MB
                  size_t maxFiles = 30) const;
        void initRepeat(const std::string& app_name = "app") const;
        // 日志记录方法
        void log(Level level, const std::string& message, const char* file = "", int line = 0) const;
        void trace(const std::string& message, const char* file = "", int line = 0) const;
        void debug(const std::string& message, const char* file = "", int line = 0) const;
        void info(const std::string& message, const char* file = "", int line = 0) const;
        void warn(const std::string& message, const char* file = "", int line = 0) const;
        void error(const std::string& message, const char* file = "", int line = 0) const;
        void critical(const std::string& message, int exitCode, const char* file = "", int line = 0) const;
        void exception(const std::string& message, const std::exception& e, int exitCode, const char* file = "", int line = 0) const;

        // 设置日志级别
        void setLevel(Level level) const;

        // 立即刷新日志
        void flush() const;

        // 关闭日志
        void shutDown() const;
        // 特殊成员函数
        ~Logger();
        Logger(); // 私有构造函数

    private:
        // 使用裸指针代替unique_ptr，由实现类管理生命周期
        LoggerImpl* impl_;


    };
}

// 便捷宏定义
#define LOG_TRACE(message) jade::Logger::getInstance().trace(message, __FILE__, __LINE__)
#define LOG_DEBUG(message) jade::Logger::getInstance().debug(message, __FILE__, __LINE__)
#define LOG_INFO(message) jade::Logger::getInstance().info(message, __FILE__, __LINE__)
#define LOG_WARN(message) jade::Logger::getInstance().warn(message, __FILE__, __LINE__)
#define LOG_ERROR(message) jade::Logger::getInstance().error(message, __FILE__, __LINE__)
#define LOG_CRITICAL(message,exitCode) jade::Logger::getInstance().critical(message, exitCode,__FILE__, __LINE__)
#define LOG_EXCEPTION(message,ex, exitCode) jade::Logger::getInstance().exception(message, ex, exitCode, __FILE__, __LINE__)


#define DLL_LOG_TRACE(module,message) jade::Logger::getInstance(module).trace(message, __FILE__, __LINE__)
#define DLL_LOG_DEBUG(module,message) jade::Logger::getInstance(module).debug(message, __FILE__, __LINE__)
#define DLL_LOG_INFO(module,message) jade::Logger::getInstance(module).info(message, __FILE__, __LINE__)
#define DLL_LOG_WARN(module,message) jade::Logger::getInstance(module).warn(message, __FILE__, __LINE__)
#define DLL_LOG_ERROR(module,message) jade::Logger::getInstance(module).error(message, __FILE__, __LINE__)
#define DLL_LOG_CRITICAL(module,message,exitCode) jade::Logger::getInstance(module).critical(message, exitCode,__FILE__, __LINE__)
#define DLL_LOG_EXCEPTION(module,message,ex, exitCode) jade::Logger::getInstance(module).exception(message, ex, exitCode, __FILE__, __LINE__)


#endif

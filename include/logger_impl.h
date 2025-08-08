/**
# @File     : logger_impl.h
# @Author   : jade
# @Date     : 2025/8/4 14:38
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : logger_impl.h
*/
//

#ifndef LOGGER_IMPL_H
#define LOGGER_IMPL_H
#include "logger.h"  // 包含Logger类的定义
#include <memory>
#include <string>

namespace jade
{
    class LoggerImpl{
    public:
        LoggerImpl();
        ~LoggerImpl();
        void init(
            const std::string& app_name,
            const std::string& logName,
            const std::string& logDir,
            Logger::Level logLevel,
            bool consoleOutput,
            bool fileOutput,
            size_t maxFileSize,
            size_t maxFiles);

        void log(Logger::Level level, const std::string& message, const char* file, int line) const;
        void setLevel(Logger::Level level) const;
        void flush() const;
        void shutDown();

    private:
        struct SpdLogImpl;
        std::shared_ptr<SpdLogImpl> logger_;
        bool initialized_ = false;
        bool shutdown_ = false;
    };
}


#endif

/**
# @File     : multi_rtsp_manager.cpp
# @Author   : jade
# @Date     : 2025/9/18 17:58
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : multi_rtsp_manager.cpp
*/
#include "include/jade_tools.h"
#include <mutex>
#include <opencv2/core/utils/logger.hpp>
#include <utility>
using namespace jade;
#define MODULE_NAME "MultiRtspManager"

class MultiRtspManager::Impl
{
    std::vector<std::shared_ptr<RtspVideoCapture>> captures;
    mutable std::mutex mutex;

public:
    static void setOpencvLogger()
    {
        cv::redirectError(handleError);
        cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
    }

    explicit Impl(CpuFrameCallback cpu_call_back) :
        cpu_callback_(std::move(cpu_call_back))
    {
        setOpencvLogger();
    }
#ifdef OPENCV_CUDA_ENABLED
    explicit Impl(GpuFrameCallback gpu_call_back) :
        gpu_callback_(std::move(gpu_call_back))
    {
        setOpencvLogger();
    }

    explicit Impl(CpuFrameCallback cpu_call_back, const GpuFrameCallback& gpu_call_back) :
        cpu_callback_(std::move(cpu_call_back)), gpu_callback_(gpu_call_back)
    {
        setOpencvLogger();
    }
#endif

    void addStream(const RtspVideoCapture::RtspInfo& rtsp_info)
    {
        std::lock_guard lock(mutex);
        // 检查是否已存在相同ID的流
        for (const auto& capture : captures)
        {
            if (capture->getRtspIpAddress() == rtsp_info.getIpAddress())
            {
                DLL_LOG_WARN(MODULE_NAME) << "当前ip流地址已经存在,ip地址为:" << rtsp_info.getIpAddress();
                return;
            }
        }
#ifdef OPENCV_CUDA_ENABLED
        const auto capture = std::make_shared<RtspVideoCapture>(rtsp_info, cpu_callback_, gpu_callback_);
#else
    const auto capture = std::make_shared<RtspVideoCapture>(rtsp_info, cpu_callback_);
#endif

        capture->start();
        captures.push_back(capture);
    }
    ~Impl()
    {
        for (const auto& capture : captures)
        {
            capture->stop();
        }
        captures.clear();
        DLL_LOG_TRACE(MODULE_NAME) << "多路Rtsp管理器停止成功";

    }

private:
    [[maybe_unused]] static void OpencvLog(const cv::utils::logging::LogLevel level, const char* message)
    {
        switch (level)
        {
        case cv::utils::logging::LOG_LEVEL_DEBUG:
            DLL_LOG_DEBUG("Opencv") << message;
            break;
        case cv::utils::logging::LOG_LEVEL_INFO:
            DLL_LOG_INFO("Opencv") << message;
            break;
        case cv::utils::logging::LOG_LEVEL_WARNING:
            DLL_LOG_WARN("Opencv") << message;
            break;
        case cv::utils::logging::LOG_LEVEL_VERBOSE:
            DLL_LOG_TRACE("Opencv") << message;
            break;
        case cv::utils::logging::LOG_LEVEL_ERROR:
            DLL_LOG_ERROR("Opencv") << message;
            break;
        case cv::utils::logging::LOG_LEVEL_FATAL:
            DLL_LOG_CRITICAL("Opencv", 0) << message;
            break;
        default:
            break;
        }
    }

    static int handleError(const int status, const char* func_name,
                           const char* err_msg, const char* file_name,
                           const int line, void* userdata)
    {
        DLL_LOG_ERROR_FL("Opencv", file_name, line) << err_msg << ",status:" << status << ",Function: " <<
            func_name;
        // 返回0表示不中断程序执行
        return 0;
    }

private:
    CpuFrameCallback cpu_callback_;
#ifdef OPENCV_CUDA_ENABLED
    GpuFrameCallback gpu_callback_;
#endif
};

MultiRtspManager::MultiRtspManager() :
    impl_(nullptr)
{
}

MultiRtspManager& MultiRtspManager::getInstance()
{
    static MultiRtspManager instance;
    return instance;
}

void MultiRtspManager::init(const CpuFrameCallback& cpu_callback)
{
    impl_ = new Impl(cpu_callback);
}

#ifdef OPENCV_CUDA_ENABLED
void MultiRtspManager::init(const CpuFrameCallback& cpu_callback, const GpuFrameCallback& gpu_callback)
{
    impl_ = new Impl(cpu_callback, gpu_callback);
}

void MultiRtspManager::init(const GpuFrameCallback& gpu_callback)
{
    impl_ = new Impl(gpu_callback);
}


#endif

void MultiRtspManager::addStream(const RtspVideoCapture::RtspInfo& rtsp_info) const
{
    if (impl_)
    {
        impl_->addStream(rtsp_info);
    }
}


void MultiRtspManager::stopAll()
{
    if (impl_)
    {
        delete impl_;
        impl_ = nullptr;
    }
}

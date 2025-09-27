/**
# @File     : video_capture.cpp
# @Author   : jade
# @Date     : 2025/9/17 15:50
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : video_capture.cpp
*/
#include "include/jade_tools.h"
#include <condition_variable>
#include <future>
using namespace jade;
#define MODULE_NAME "VideoCapture"
#ifdef OPENCV_ENABLED
class VideoCaptureBase::Impl
{
private:
    std::string source_;
    cv::VideoCapture cap_cpu_;
    bool use_gpu_;
    std::thread captureThread;
    int frame_interval_;
    int frame_count_;
#ifdef OPENCV_CUDA_ENABLED
    cv::Ptr<cv::cudacodec::VideoReader> cap_gpu_;
    cv::cuda::GpuMat gpu_frame;
#endif
    std::atomic<bool> isRunning_;
    cv::Mat cpu_frame;

    int reconnectAttempts_;

    void open(const VideoCaptureBase* outer,const std::string& source,const bool use_gpu)
    {
        source_ = source;
        use_gpu_ = use_gpu;
        reconnectAttempts_ += 1;
        if (use_gpu)
        {
#ifdef OPENCV_CUDA_ENABLED
            try
            {
                cap_gpu_ = cv::cudacodec::createVideoReader(source);
                if (cap_gpu_)
                {
                    DLL_LOG_INFO(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机打开成功";
                }
                else
                {
                    DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机打开失败,失败次数为:" << reconnectAttempts_;
                }
            }
            catch (const cv::Exception& e)
            {
                DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机打开异常,失败次数为:" << reconnectAttempts_  << ",异常原因:" << e.what();
            }
            return;
#else
      DLL_LOG_WARN(MODULE_NAME) << outer->getVideoInfo() << "无法使用GPU解码,退回到CPU上";
      use_gpu_ = false;
#endif
        }
        try
        {
            if (cap_cpu_.open(source))
            {
                DLL_LOG_INFO(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机打开成功";
            }
            else
            {
                DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机打开失败,失败次数为:" << reconnectAttempts_ ;
            }
        }
        catch (std::exception& e)
        {
            DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机打开异常 失败次数为:" << reconnectAttempts_ << ",异常原因:" << e.what();
        }
    }

#ifdef OPENCV_CUDA_ENABLED
    bool tryGetNextFrame(cv::cuda::GpuMat& frame) const
    {
        std::future<bool> ret = std::async(std::launch::async, [&]() {
            try
            {
                return cap_gpu_->nextFrame(frame);
            }catch (std::exception& e)
            {
                std::cout << "取流异常" << e.what() << std::endl;
                return false;
            }
        });
        // 10s内没有结果则异常退出
        return ret.wait_for(std::chrono::milliseconds(10000)) == std::future_status::ready && ret.get();
    }
#endif


    void cpuCapture(VideoCaptureBase* outer,cv::Mat &frame)
    {
        if (cap_cpu_.grab())
        {
            // 只抓取视频,不解码
            frame_count_ += 1;
            if (frame_count_ % frame_interval_ == 0)
            {
                if (cap_cpu_.retrieve(frame))
                {
                    outer->process(frame);
                    frame_count_ = 0;
                }
                else
                {
                    DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,解码图像失败,准备重新连接";
                    open(outer, source_, use_gpu_);
                }
            }
            reconnectAttempts_ = 0;
        }
        else
        {
            DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机取流异常,准备重新连接";
            open(outer, source_, use_gpu_);
        }
    }

    void captureLoop(VideoCaptureBase* outer)
    {
        open(outer, source_, use_gpu_);

        while (true)
        {
            if (!isRunning_)
            {
                DLL_LOG_TRACE(MODULE_NAME) << outer->getVideoInfo() << "相机关闭成功";
                break;
            }
#ifdef OPENCV_CUDA_ENABLED
            try
            {
                if (use_gpu_)
                {
                    if (tryGetNextFrame(gpu_frame))
                    {
                        frame_count_ += 1;
                        if (frame_count_ % frame_interval_ == 0)
                        {
                            if (!gpu_frame.empty())
                            {
                                outer->process(gpu_frame);
                            }
                            frame_count_ = 0;
                        }
                        reconnectAttempts_ = 0;
                    }
                    else
                    {
                        if (!isRunning_)
                        {
                            DLL_LOG_TRACE(MODULE_NAME) << outer->getVideoInfo() << "相机关闭成功";
                            break;
                        }
                        open(outer, source_, use_gpu_);
                    }
                }
                else
                {
                    cpuCapture(outer,cpu_frame);
                }
            }
            catch (const std::exception& e)
            {
                DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机异常:" << e.what();
                open(outer, source_, use_gpu_);
            }
#else
        // 纯 CPU 解码操作
      try {
        cpuCapture(outer,cpu_frame);
      }catch (const std::exception &e) {
        DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机异常:" << e.what();
        open(outer, source_, use_gpu_);
      }
#endif
        }
    }

public:
    Impl(std::string source, const bool use_gpu, const int frame_interval) :
        source_(std::move(source)), use_gpu_(use_gpu), frame_interval_(frame_interval),
        frame_count_(0), isRunning_(false), reconnectAttempts_(0){
    }


    void start(VideoCaptureBase* outer)
    {
        isRunning_ = true;
        captureThread = std::thread(&Impl::captureLoop, this, outer);
    };

    void stop(const VideoCaptureBase* outer)
    {
        DLL_LOG_TRACE(MODULE_NAME) << outer->getVideoInfo() << "准备关闭相机 ...";
        isRunning_ = false;
#ifdef  OPENCV_CUDA_ENABLED
        cap_gpu_.release();
        gpu_frame.release();
#endif
        if (captureThread.joinable())
        {
            captureThread.join();
        }
    }

};

VideoCaptureBase::VideoCaptureBase(const std::string& source,
                                   const bool use_gpu,
                                   const int frame_interval) :
    impl_(new Impl(source, use_gpu, frame_interval))
{
}

void VideoCaptureBase::start()
{
    if (impl_)
    {
        impl_->start(this);
    }
}

void VideoCaptureBase::stop() const
{
    if (impl_)
    {
        impl_->stop(this);
    }
}

#endif

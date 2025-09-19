/**
# @File     : video_capture.cpp
# @Author   : jade
# @Date     : 2025/9/17 15:50
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : video_capture.cpp
*/
#include "include/jade_tools.h"
using namespace jade;
#define MODULE_NAME "VideoCapture"
#ifdef OPENCV_ENABLED
class VideoCaptureBase::Impl {
private:
  std::string source_;
  cv::VideoCapture cap_cpu_;
  bool use_gpu_;
  std::thread captureThread;
  int frame_interval_;
  int frame_count_;
#ifdef OPENCV_CUDA_ENABLED
  cv::Ptr<cv::cudacodec::VideoReader> cap_gpu_;
#endif
  std::atomic<bool> isRunning_;
  int reconnectAttempts_;
  const int maxReconnectAttempts_;

  void open(const VideoCaptureBase *outer, const std::string &source, const bool use_gpu) {
    source_ = source;
    use_gpu_ = use_gpu;
    reconnectAttempts_ += 1;

    if (use_gpu) {
#ifdef OPENCV_CUDA_ENABLED
      try {
        cap_gpu_ = cv::cudacodec::createVideoReader(source);
        if (cap_gpu_) {
          DLL_LOG_INFO(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机打开成功" ;
        }else {
          DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机打开失败,失败次数为:" << reconnectAttempts_;
        }
      } catch (const cv::Exception &e) {
        DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用GPU解码,相机打开异常,异常原因:" << e.what();
      }
      return;
#else
      DLL_LOG_WARN(MODULE_NAME) << outer->getVideoInfo() << "无法使用GPU解码,退回到CPU上";
      use_gpu_ = false;
#endif
    }
    try {
      if (cap_cpu_.open(source)) {
        DLL_LOG_INFO(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机打开成功";
      } else {
        DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机打开失败,失败次数为:" << reconnectAttempts_;
      }
    } catch (std::exception &e) {
      DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机打开异常,异常原因为:" << e.what() <<  "失败次数为:" <<  reconnectAttempts_;
    }
  }

  void captureLoop(VideoCaptureBase *outer) {
    open(outer, source_, use_gpu_);
    while (isRunning_) {
      try {
        if (use_gpu_) {
#ifdef OPENCV_CUDA_ENABLED
          if (cv::cuda::GpuMat gpu_frame; cap_gpu_->nextFrame(gpu_frame)) {
            outer->process(gpu_frame);
            reconnectAttempts_ = 0;
          } else {
            open(outer, source_, use_gpu_);
          }
#endif
        } else {
          if (cv::Mat frame; cap_cpu_.read(frame)) {
            frame_count_ += 1;
            if (frame_count_ % frame_interval_ == 0) {
                outer->process(frame);
                frame_count_ = 0;
            }
            reconnectAttempts_ = 0;
          } else {
            DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机取流异常,准备重新连接";
            open(outer, source_, use_gpu_);
          }
        }
      } catch (const std::exception &e) {
        DLL_LOG_ERROR(MODULE_NAME) << outer->getVideoInfo() << "使用CPU解码,相机异常:" << e.what();
        open(outer, source_, use_gpu_);
      }
    }
  }

public:
  Impl(std::string source, const bool use_gpu,const int frame_interval)
      : source_(std::move(source)), use_gpu_(use_gpu),frame_interval_(frame_interval),frame_count_(0), isRunning_(false),
        reconnectAttempts_(0), maxReconnectAttempts_(100) {}

  void start(VideoCaptureBase *outer) {
    isRunning_ = true;
    captureThread = std::thread(&Impl::captureLoop, this, outer);
  };
  void stop() {
    isRunning_ = false;
    if (captureThread.joinable()) {
      captureThread.join();
    }
  }
  ~Impl();
};

VideoCaptureBase::VideoCaptureBase(const std::string &source,const bool use_gpu, const int frame_interval): impl_(new Impl(source, use_gpu,frame_interval)) {}

void VideoCaptureBase::start() {
  impl_->start(this);
}

void VideoCaptureBase::stop() const { impl_->stop(); }

#endif

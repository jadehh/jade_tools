/**
# @File     : rtsp_capture.cpp
# @Author   : jade
# @Date     : 2025/9/18 17:19
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : rtsp_capture.cpp
*/
#include <utility>

#include "include/jade_tools.h"
using namespace jade;
#ifdef OPENCV_ENABLED

class RtspVideoCapture::Impl
{
public:
    explicit Impl(const RtspInfo& rtsp_info):rtsp_info_(rtsp_info){}
    [[nodiscard]] RtspInfo getRtspInfo() const
    {
        return rtsp_info_;
    }
private:
    RtspInfo rtsp_info_;
};

class RtspVideoCapture::CallBackImpl
{
public:
    explicit CallBackImpl(CpuFrameCallback  cpu_frame_callback):cpu_frame_callback_(std::move(cpu_frame_callback)){};
    void runCpu(const RtspInfo& info, const cv::Mat& frame) const
    {
        cpu_frame_callback_(info,frame);
    }
#ifdef OPENCV_CUDA_ENABLED
    explicit CallBackImpl(GpuFrameCallback  gpu_frame_callback):gpu_frame_callback_(std::move(gpu_frame_callback)){};
    explicit CallBackImpl(CpuFrameCallback  cpu_frame_callback,GpuFrameCallback  gpu_frame_callback):cpu_frame_callback_(std::move(cpu_frame_callback)),gpu_frame_callback_(std::move(gpu_frame_callback)){};
    void runGpu(const RtspInfo& info, cv::cuda::GpuMat& frame) const
    {
        gpu_frame_callback_(info,frame);

    }
#endif
private:
    CpuFrameCallback cpu_frame_callback_;
#ifdef OPENCV_CUDA_ENABLED
    GpuFrameCallback gpu_frame_callback_;
#endif
};

RtspVideoCapture::RtspVideoCapture(const RtspInfo& rtsp_info, const CpuFrameCallback& cpu_frame_callback):
    VideoCaptureBase(rtsp_info.toRtspUrl(), rtsp_info.getUseGpu(), rtsp_info.getFrameInterval()), impl_(new Impl(rtsp_info)),
    call_back_impl_(new CallBackImpl(cpu_frame_callback))
{
}
#ifdef OPENCV_CUDA_ENABLED

RtspVideoCapture::RtspVideoCapture(const RtspInfo& rtsp_info, const GpuFrameCallback& gpu_frame_callback):
    VideoCaptureBase(rtsp_info.toRtspUrl(), rtsp_info.getUseGpu(), rtsp_info.getFrameInterval()), impl_(new Impl(rtsp_info)),
    call_back_impl_(new CallBackImpl(gpu_frame_callback))
{
}

RtspVideoCapture::RtspVideoCapture(const RtspInfo& rtsp_info, const CpuFrameCallback& cpu_frame_callback,
                                   const GpuFrameCallback& gpu_frame_callback):
    VideoCaptureBase(rtsp_info.toRtspUrl(), rtsp_info.getUseGpu(), rtsp_info.getFrameInterval()), impl_(new Impl(rtsp_info)),call_back_impl_(new CallBackImpl(cpu_frame_callback,gpu_frame_callback))
{
}
#endif


void RtspVideoCapture::process(cv::Mat& frame) { call_back_impl_->runCpu(impl_->getRtspInfo(), frame);}

#ifdef OPENCV_CUDA_ENABLED
void RtspVideoCapture::process(cv::cuda::GpuMat& gpu_mat) { call_back_impl_->runGpu(impl_->getRtspInfo(), gpu_mat); }
#endif

std::string RtspVideoCapture::getRtspIpAddress() const
{
    return impl_->getRtspInfo().getIpAddress();
}

std::string RtspVideoCapture::getVideoInfo() const
{
    return "相机名称为:" + impl_->getRtspInfo().getCameraName() + ",相机ip地址为:" + impl_->getRtspInfo().getIpAddress() + ",";
}
#endif

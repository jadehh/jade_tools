/**
# @File     : rtsp_capture.cpp
# @Author   : jade
# @Date     : 2025/9/18 17:19
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : rtsp_capture.cpp
*/
#include "include/jade_tools.h"
#include <utility>
using namespace jade;
#ifdef OPENCV_ENABLED
RtspVideoCapture::RtspVideoCapture(const RtspInfo& rtsp_info, CpuFrameCallback cpu_frame_callback):
    VideoCaptureBase(rtsp_info.toRtspUrl(), rtsp_info.use_gpu, rtsp_info.frame_interval), rtsp_info_(rtsp_info),
    cpu_frame_callback_(std::move(cpu_frame_callback))
{
}
#ifdef OPENCV_CUDA_ENABLED

RtspVideoCapture::RtspVideoCapture(const RtspInfo& rtsp_info, GpuFrameCallback gpu_frame_callback):
    VideoCaptureBase(rtsp_info.toRtspUrl(), rtsp_info.use_gpu, rtsp_info.frame_interval), rtsp_info_(rtsp_info),
    gpu_frame_callback_(std::move(gpu_frame_callback))
{
}

RtspVideoCapture::RtspVideoCapture(const RtspInfo& rtsp_info, CpuFrameCallback cpu_frame_callback,
                                   const GpuFrameCallback& gpu_frame_callback):
    VideoCaptureBase(rtsp_info.toRtspUrl(), rtsp_info.use_gpu, rtsp_info.frame_interval), rtsp_info_(rtsp_info),
    cpu_frame_callback_(std::move(cpu_frame_callback)), gpu_frame_callback_(gpu_frame_callback)
{
}
#endif


void RtspVideoCapture::process(cv::Mat& frame) { cpu_frame_callback_(rtsp_info_, frame); }

#ifdef OPENCV_CUDA_ENABLED
void RtspVideoCapture::process(cv::cuda::GpuMat& gpu_mat) { gpu_frame_callback_(rtsp_info_, gpu_mat); }
#endif

std::string RtspVideoCapture::getRtspIpAddress() const
{
    return rtsp_info_.ip_address;
}

std::string RtspVideoCapture::getVideoInfo() const
{
    return "相机名称为:" + rtsp_info_.camera_name + ",相机ip地址为:" + rtsp_info_.ip_address + ",";
}
#endif

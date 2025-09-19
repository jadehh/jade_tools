#include "test/include/testOpencv.h"
#include <functional>

// 使用示例 - 图像处理回调函数
void processCpuFrame(const jade::RtspVideoCapture::RtspInfo &rtsp_info, const cv::Mat &frame) {
  // 在这里处理图像帧
  LOG_TRACE() << "CPU解码成功,相机类型为:" << rtsp_info.camera_name << ",抓拍照片成功";
  // 可以在这里添加你的图像处理逻辑
  // 例如：目标检测、人脸识别、运动检测等
  // 显示处理后的图像（可选）
}
#ifdef OPENCV_CUDA_ENABLED
void processGpuFrame(const jade::RtspVideoCapture::RtspInfo &rtsp_info, const cv::cuda::GpuMat &frame) {
  // 在这里处理图像帧
  LOG_TRACE() << "Gpu解码成功,相机类型为:" << rtsp_info.camera_name << ",抓拍照片成功";
  // 可以在这里添加你的图像处理逻辑
  // 例如：目标检测、人脸识别、运动检测等
  // 显示处理后的图像（可选）
}
#endif

void testOpencv() {
#ifdef OPENCV_CUDA_ENABLED
  jade::MultiRtspManager::getInstance().init(processCpuFrame,processGpuFrame);
#else
  jade::MultiRtspManager::getInstance().init(processCpuFrame);
#endif
  const jade::RtspVideoCapture::RtspInfo front_hik_info("前相机", "admin", "samples123", "192.168.29.181", 554, "",false,30,jade::RtspVideoCapture::RtspDeviceType::HIKVISION);
  const jade::RtspVideoCapture::RtspInfo back_hik_info("后相机", "admin", "samples123", "192.168.29.180", 554, "",true,30,jade::RtspVideoCapture::RtspDeviceType::HIKVISION);
  const jade::RtspVideoCapture::RtspInfo left_hik_info("侧相机", "admin", "samples123", "192.168.29.179", 554, "",true,30,jade::RtspVideoCapture::RtspDeviceType::HIKVISION);
  // 添加多个RTSP流
  const std::vector streams = {front_hik_info,back_hik_info,left_hik_info};
  for (const auto& rtsp : streams) {
    jade::MultiRtspManager::getInstance().addStream(rtsp);
  }
}
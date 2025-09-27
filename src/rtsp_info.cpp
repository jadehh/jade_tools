/**
# @File     : rtsp_info.cpp
# @Author   : jade
# @Date     : 2025/9/18 16:41
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : rtsp_info.cpp
*/

#include "include/jade_tools.h"
using namespace jade;

class RtspVideoCapture::RtspInfo::Impl
{
public:
    Impl(const int port, const bool use_gpu, const int frame_interval):
        port(port), use_gpu(use_gpu), frame_interval(frame_interval),device_type(RtspDeviceType::UNKNOWN)
    {

    }

    Impl(const std::string& camera_name, const std::string& user,
    const std::string& pwd, const std::string& ip, const int port_num,
    const std::string& path, const bool use_gpu, const int frame_interval,
    const RtspDeviceType type):camera_name(camera_name),username(user),password(pwd),ip_address(ip),port(port_num),
    stream_path(path),use_gpu(use_gpu),frame_interval(frame_interval),device_type(type)
    {

    }
    std::string getCameraName()
    {
        return camera_name;
    }

    std::string getUserName()
    {
        return username;
    }

    std::string getPasswd()
    {
        return password;
    }

    [[nodiscard]] int getPort() const
    {
        return port;
    }

    std::string getIpAddress()
    {
        return ip_address;
    }

    std::string getStreamPath()
    {
        return stream_path;
    }

    void clear()
    {
        username.clear();
        password.clear();
        ip_address.clear();
        port = 554;
        stream_path.clear();
        device_type = RtspDeviceType::UNKNOWN;
    }

    bool getUseGpu()
    {
        return use_gpu;
    }

    RtspDeviceType getDeviceType() const
    {
        return device_type;
    }

    void setDeviceType(const RtspDeviceType type)
    {
        device_type = type;
    }

    int getFrameInterval()
    {
        return frame_interval;
    }

private:
    std::string camera_name; // 相机名称
    std::string username; // 用户名
    std::string password; // 密码
    std::string ip_address; // IP地址或域名
    int port; // 端口号，默认为554
    std::string stream_path; // 流路径
    bool use_gpu; //是否使用GPU解码
    int frame_interval; // 参数含义, 每5帧中，你只处理第1帧（或任意指定的一帧），跳过中间的4帧。
    RtspDeviceType device_type; // 设备类型
};

RtspVideoCapture::RtspInfo::RtspInfo():impl_(new Impl(554,false,5)){
}

RtspVideoCapture::RtspInfo::RtspInfo(
    const std::string& camera_name, const std::string& user,
    const std::string& pwd, const std::string& ip, const int port_num,
    const std::string& path, const bool use_gpu, const int frame_interval,
    const RtspDeviceType type):impl_(new Impl(camera_name,user,pwd,ip,port_num,path,use_gpu,frame_interval,type))
{

}


std::string RtspVideoCapture::RtspInfo::toRtspUrl() const
{
    std::string url = "rtsp://";
    if (!impl_->getUserName().empty() && !impl_->getPasswd().empty())
    {
        url += impl_->getUserName() + ":" + impl_->getPasswd() + "@";
    }

    url += impl_->getIpAddress();

    if (impl_->getPort() != 554)
    {
        url += ":" + std::to_string(impl_->getPort());
    }

    std::string actual_path = impl_->getStreamPath();
    if (actual_path.empty())
    {
        actual_path = getDefaultStreamPath();
    }
    if (!actual_path.empty())
    {
        if (actual_path[0] != '/')
        {
            url += "/";
        }
        url += actual_path;
    }
    return url;
}

std::string RtspVideoCapture::RtspInfo::getDefaultStreamPath() const
{
    switch (impl_->getDeviceType())
    {
    case RtspDeviceType::HIKVISION:
        return "h264/ch1/main/av_stream"; // 海康主码流
    case RtspDeviceType::DAHUA:
        return "cam/realmonitor?channel=1&subtype=0"; // 大华主码流
    case RtspDeviceType::ONVIF_GENERIC:
        return "live"; // 通用ONVIF流
    default:
        return ""; // 未知设备返回空路径
    }
}


[[maybe_unused]] std::string RtspVideoCapture::RtspInfo::getDeviceTypeString() const
{
    static const std::map<RtspDeviceType, std::string> type_map = {
        {RtspDeviceType::UNKNOWN, "Unknown"},
        {RtspDeviceType::HIKVISION, "Hikvision"},
        {RtspDeviceType::DAHUA, "Dahua"},
        {RtspDeviceType::ONVIF_GENERIC, "ONVIF Generic"}
    };

    const auto it = type_map.find(impl_->getDeviceType());
    return it != type_map.end() ? it->second : "Unknown";
}


[[maybe_unused]] void RtspVideoCapture::RtspInfo::setDeviceTypeFromString(
    const std::string& type_str) const
{
    static const std::map<std::string, RtspDeviceType> str_to_type = {
        {"hikvision", RtspDeviceType::HIKVISION},
        {"hik", RtspDeviceType::HIKVISION},
        {"dahua", RtspDeviceType::DAHUA},
        {"dh", RtspDeviceType::DAHUA},
        {"onvif", RtspDeviceType::ONVIF_GENERIC},
        {"generic", RtspDeviceType::ONVIF_GENERIC}
    };

    std::string lower_str;
    for (const char c : type_str)
    {
        lower_str += static_cast<char>(std::tolower(c));
    }
    const auto it = str_to_type.find(lower_str);
    impl_->setDeviceType((it != str_to_type.end()) ? it->second : RtspDeviceType::UNKNOWN);
}

[[maybe_unused]] bool RtspVideoCapture::RtspInfo::isValid() const
{
    return !impl_->getIpAddress().empty() && impl_->getPort() > 0 && impl_->getPort() <= 65535;
}


void RtspVideoCapture::RtspInfo::clear() const
{
    impl_->clear();
}

std::string RtspVideoCapture::RtspPathHelper::getHikvisionPath(const RtspStreamType stream_type, const int channel)
{
    switch (stream_type)
    {
    case RtspStreamType::MAIN_STREAM:
        return "Streaming/Channels/" + std::to_string(channel) + "01";
    case RtspStreamType::SUB_STREAM:
        return "Streaming/Channels/" + std::to_string(channel) + "02";
    case RtspStreamType::THIRD_STREAM:
        return "Streaming/Channels/" + std::to_string(channel) + "03";
    case RtspStreamType::AUDIO_STREAM:
        return "Streaming/Channels/" + std::to_string(channel) + "01?audio";
    default:
        return "Streaming/Channels/101";
    }
}

std::string RtspVideoCapture::RtspPathHelper::getDahuaPath(
    const RtspStreamType stream_type, const int channel)
{
    const std::string base = "cam/realmonitor?channel=" + std::to_string(channel);

    switch (stream_type)
    {
    case RtspStreamType::MAIN_STREAM:
        return base + "&subtype=0";
    case RtspStreamType::SUB_STREAM:
        return base + "&subtype=1";
    case RtspStreamType::THIRD_STREAM:
        return base + "&subtype=2";
    case RtspStreamType::AUDIO_STREAM:
        return base + "&subtype=0&audio=1";
    default:
        return base + "&subtype=0";
    }
}

std::string RtspVideoCapture::RtspPathHelper::getPathByDeviceType(
    const RtspDeviceType device_type, const RtspStreamType stream_type,
    const int channel)
{
    switch (device_type)
    {
    case RtspDeviceType::HIKVISION:
        return getHikvisionPath(stream_type, channel);
    case RtspDeviceType::DAHUA:
        return getDahuaPath(stream_type, channel);
    case RtspDeviceType::ONVIF_GENERIC:
        return "live";
    default:
        return "";
    }
}
bool RtspVideoCapture::RtspInfo::getUseGpu() const
{
    return impl_->getUseGpu();
}

int RtspVideoCapture::RtspInfo::getFrameInterval() const
{
    return impl_->getFrameInterval();
}

std::string RtspVideoCapture::RtspInfo::getIpAddress() const
{
    return impl_->getIpAddress();
}

std::string RtspVideoCapture::RtspInfo::getUserName() const
{
    return impl_->getUserName();
}

std::string RtspVideoCapture::RtspInfo::getCameraName() const
{
    return impl_->getCameraName();
}



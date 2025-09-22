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

RtspVideoCapture::RtspInfo::RtspInfo():
    port(554), use_gpu(false), frame_interval(5),
    device_type(RtspDeviceType::UNKNOWN)
{
}

RtspVideoCapture::RtspInfo::RtspInfo(
    const std::string& camera_name, const std::string& user,
    const std::string& pwd, const std::string& ip, const int port_num,
    const std::string& path, const bool use_gpu, const int frame_interval,
    const RtspDeviceType type):
    camera_name(camera_name), username(user), password(pwd), ip_address(ip),
    port(port_num), stream_path(path), use_gpu(use_gpu), frame_interval(frame_interval),
    device_type(type)
{
}

std::string RtspVideoCapture::RtspInfo::toRtspUrl() const
{
    std::string url = "rtsp://";
    if (!username.empty() && !password.empty())
    {
        url += username + ":" + password + "@";
    }

    url += ip_address;

    if (port != 554)
    {
        url += ":" + std::to_string(port);
    }

    std::string actual_path = stream_path;
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
    switch (device_type)
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


std::string RtspVideoCapture::RtspInfo::getDeviceTypeString() const
{
    static const std::map<RtspDeviceType, std::string> type_map = {
        {RtspDeviceType::UNKNOWN, "Unknown"},
        {RtspDeviceType::HIKVISION, "Hikvision"},
        {RtspDeviceType::DAHUA, "Dahua"},
        {RtspDeviceType::ONVIF_GENERIC, "ONVIF Generic"}
    };

    auto it = type_map.find(device_type);
    return it != type_map.end() ? it->second : "Unknown";
}


void RtspVideoCapture::RtspInfo::setDeviceTypeFromString(
    const std::string& type_str)
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
    device_type = (it != str_to_type.end()) ? it->second : RtspDeviceType::UNKNOWN;
}

bool RtspVideoCapture::RtspInfo::isValid() const
{
    return !ip_address.empty() && port > 0 && port <= 65535;
}


void RtspVideoCapture::RtspInfo::clear()
{
    username.clear();
    password.clear();
    ip_address.clear();
    port = 554;
    stream_path.clear();
    device_type = RtspDeviceType::UNKNOWN;
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

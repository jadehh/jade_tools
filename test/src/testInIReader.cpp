/**
# @File     : testInIReader.cpp
# @Author   : jade
# @Date     : 2025/9/15 15:07
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testInIReader.cpp
*/
#include "test/include/testINIReader.h"
#include <sstream>

std::string sections(const jade::INIReader& reader)
{
    std::stringstream ss;
    const std::set<std::string>& sections = reader.Sections();
    for (const auto& section : sections)
        ss << section << ",";
    return ss.str();
}

void testInIReader()
{
    LOG_INFO() << "=====================================ini文件解析测试开始" << "=====================================";
#ifdef _WIN32
    auto filename = "../../config/test.ini";
#else
    auto filename = "../config/test.ini";
#endif
    if (const jade::INIReader reader(filename); reader.ParseError() < 0)
    {
        LOG_ERROR() << "Can't load 'test.ini'";
    }
    else
    {
        auto configs = reader.GetIntegerWithPrefix("EIO","EIOOutputBitNumber");
        LOG_DEBUG() << "Config loaded from 'test.ini': found sections="
            << sections(reader)
            << " version=" << reader.GetInteger("protocol", "version", -1)
            << ", name=" << reader.Get("user", "name", "UNKNOWN")
            << ", email=" << reader.Get("user", "email", "UNKNOWN")
            << ", multi=" << reader.Get("user", "multi", "UNKNOWN")
            << ", pi=" << reader.GetReal("user", "pi", -1)
            << ", active=" << reader.GetBoolean("user", "active", true);
    }
    LOG_INFO() << "=====================================ini文件解析测试结束" << "=====================================";
}

/**
# @File     : ini_reader.cpp
# @Author   : jade
# @Date     : 2025/9/15 14:47
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : ini_reader.cpp
*/
#include "include/jade_tools.h"
#include <algorithm>
#include <cctype>
#include "include/ini_reader.h"
#include <iostream>
#define MODULE_NAME "INIReader"
#define LOG_INIREADER_WARN(section,name,default_value)  DLL_LOG_WARN(MODULE_NAME) << "节点名:\"" << (section) << "\"" << ",字段名: \"" << (name) << "\",读取异常,请检查配置文件,使用默认值:" << (default_value);

using namespace jade;


class INIReader::Impl
{
public:
    Impl():
        _error(0)
    {
    }

    explicit Impl(const std::string& filename)
    {
        _error = ini_parse(filename.c_str(), ValueHandler, this);
    };

    explicit Impl(FILE* file)
    {
        _error = ini_parse_file(file, ValueHandler, this);
    }

    std::map<std::string, std::string> _values;
    std::set<std::string> _sections;
    int _error;
    static std::string MakeKey(const std::string& section, const std::string& name){
        std::string key = section + "=" + name;
        // Convert to lower case to make section/name lookups case-insensitive
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key;
    }
    std::vector<std::string> GetKeys(const std::string& section) const
    {
        std::vector<std::string> keys;
        std::string prefix = section + "=";
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
        for (const auto& [fst, snd] : _values)
        {
            if (fst.compare(0, prefix.size(), prefix) == 0)
            {
                keys.push_back(fst.substr(prefix.size()));
            }
        }
        return keys;
    }

    std::vector<std::pair<std::string, std::string>> GetKeysWithPrefix(const std::string& section, const std::string& prefix)
    {
        std::vector<std::pair<std::string, std::string>> result;

        // 1. 获取该section下所有键
        const std::vector<std::string> keys = GetKeys(section);
        std::string prefix_lower = prefix;
        std::transform(prefix_lower.begin(), prefix_lower.end(), prefix_lower.begin(), ::tolower);

        // 2. 过滤出以prefix开头的键
        for (const auto& key : keys) {
            if (key.compare(0, prefix_lower.length(), prefix_lower) == 0) {  // 检查前缀匹配
                std::string value =  _values[MakeKey(section, key)];
                result.emplace_back(key, value);
            }
        }
        // 3. 按键名字典序排序（可选）
        std::sort(result.begin(), result.end(),[](const auto& a, const auto& b) { return a.first < b.first; });
        return result;
    }

    static int ValueHandler(void* user, const char* section, const char* name, const char* value){
        const auto impl = static_cast<Impl*>(user);
        const std::string key = MakeKey(section, name);
        auto values = impl->_values;
        if (!impl->_values[key].empty())
            impl->_values[key] += "\n";
        impl->_values[key] += value;
        impl->_sections.insert(section);
        return 1;
    }
};

INIReader::INIReader():
    impl_(new Impl())
{
}

INIReader::INIReader(const std::string& filename):
    impl_(new Impl(filename))
{
}

INIReader::INIReader(FILE* file):
    impl_(new Impl(file))
{
}

int INIReader::ParseError() const
{
    return impl_->_error;
}

const std::set<std::string>& INIReader::Sections() const
{
    return impl_->_sections;
}

std::string INIReader::Get(const std::string& section,
                           const std::string& name,
                           const std::string& default_value) const
{
    const std::string key = Impl::MakeKey(section, name);
    if (! impl_->_values.count(key) && !default_value.empty())
    {
        LOG_INIREADER_WARN(section, name, default_value)
    }
    return impl_->_values.count(key) ? impl_->_values[key] : default_value;
}




long INIReader::GetInteger(const std::string& section,
                           const std::string& name,
                           const long default_value) const
{
    const std::string valStr = Get(section, name, "");
    const char* value = valStr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    const long n = strtol(value, &end, 0);
    if (end <= value)
    {
        LOG_INIREADER_WARN(section, name, default_value)
    }
    return end > value ? n : default_value;
}
std::vector<long> INIReader::GetIntegerWithPrefix(const std::string& section, const std::string& prefix) const
{
    auto pairs = impl_->GetKeysWithPrefix(section, prefix);
    std::vector<long> result;
    for (const auto& [key, value] : pairs)
    {
        const char* valStr = value.c_str();
        char* end;
        // This parses "1234" (decimal) and also "0x4D2" (hex)
        const long n = strtol(valStr, &end, 0);
        if (end > valStr)
        {
            result.push_back(n);
        }else
        {
            result.push_back(0);
            LOG_INIREADER_WARN(section, key, 0)
        }
    }
    return result;
}

double INIReader::GetReal(const std::string& section,
                          const std::string& name,
                          const double default_value) const
{
    const std::string valStr = Get(section, name, "");
    const char* value = valStr.c_str();
    char* end;
    const double n = strtod(value, &end);
    if (end <= value)
    {
        LOG_INIREADER_WARN(section, name, default_value)
    }
    return end > value ? n : default_value;
}

float INIReader::GetFloat(const std::string& section,
                          const std::string& name,
                          const float default_value) const
{
    const std::string valStr = Get(section, name, "");
    const char* value = valStr.c_str();
    char* end;
    const float n = strtof(value, &end);
    if (end <= value)
    {
        LOG_INIREADER_WARN(section, name, default_value)
    }
    return end > value ? n : default_value;
}

bool INIReader::GetBoolean(const std::string& section,
                           const std::string& name,
                           const bool default_value) const
{
    std::string valStr = Get(section, name, "");
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(valStr.begin(), valStr.end(), valStr.begin(), ::tolower);
    if (valStr == "true" || valStr == "yes" || valStr == "on" || valStr == "1")
        return true;
    if (valStr == "false" || valStr == "no" || valStr == "off" ||
        valStr == "0")
        return false;
    LOG_INIREADER_WARN(section, name, default_value)
    return default_value;
}

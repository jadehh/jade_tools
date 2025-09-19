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
using namespace jade;


class INIReader::Impl{
public:
  Impl(): _error(0){}

  explicit Impl(const std::string &filename)
  {
    _error = ini_parse(filename.c_str(), ValueHandler, this);
  };
  explicit Impl(FILE *file)
  {
    _error = ini_parse_file(file, ValueHandler, this);
  }

  std::map<std::string, std::string> _values;
  std::set<std::string> _sections;
  int _error;
  static std::string MakeKey(const std::string& section, const std::string& name);
  static int ValueHandler(void* user, const char* section, const char* name, const char* value);
};

inline std::string INIReader::Impl::MakeKey(const std::string &section, const std::string &name) {
  std::string key = section + "=" + name;
  // Convert to lower case to make section/name lookups case-insensitive
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  return key;
}

inline int INIReader::Impl::ValueHandler(void *user, const char *section, const char *name, const char *value) {
  const auto impl = static_cast<Impl*>(user);
  const std::string key = MakeKey(section, name);
  auto values = impl->_values;
  if (impl->_values[key].size() > 0)
    impl->_values[key] += "\n";
  impl->_values[key] += value;
  impl->_sections.insert(section);
  return 1;
}
INIReader::INIReader():impl_(new Impl()){}

INIReader::INIReader(const std::string &filename):impl_(new Impl(filename)) {}
INIReader::INIReader(FILE *file):impl_(new Impl(file)) {}
int INIReader::ParseError() const
{
  return impl_->_error;
}
const std::set<std::string> &INIReader::Sections() const {
  return impl_->_sections;
}
std::string INIReader::Get(const std::string &section,
                                  const std::string &name,
                                  const std::string &default_value) const
{
  const std::string key = Impl::MakeKey(section, name);
  return impl_->_values.count(key) ? impl_->_values[key] : default_value;
}

long INIReader::GetInteger(const std::string &section,
                                  const std::string &name,
                                  const long default_value) const {
  std::string valstr = Get(section, name, "");
  const char *value = valstr.c_str();
  char *end;
  // This parses "1234" (decimal) and also "0x4D2" (hex)
  long n = strtol(value, &end, 0);
  return end > value ? n : default_value;
}

double INIReader::GetReal(const std::string &section,
                                 const std::string &name,
                                 const double default_value) const {
  std::string valstr = Get(section, name, "");
  const char *value = valstr.c_str();
  char *end;
  double n = strtod(value, &end);
  return end > value ? n : default_value;
}

float INIReader::GetFloat(const std::string &section,
                                 const std::string &name,
                                 float default_value) const {
  std::string valstr = Get(section, name, "");
  const char *value = valstr.c_str();
  char *end;
  const float n = strtof(value, &end);
  return end > value ? n : default_value;
}

bool INIReader::GetBoolean(const std::string &section,
                                  const std::string &name,
                                  const bool default_value) const {
  std::string valstr = Get(section, name, "");
  // Convert to lower case to make string comparisons case-insensitive
  std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
  if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
    return true;
  if (valstr == "false" || valstr == "no" || valstr == "off" ||
    valstr == "0")
    return false;
  return default_value;
}






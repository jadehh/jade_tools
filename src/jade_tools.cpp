/**
# @File     : jade_tools.cpp
# @Author   : jade
# @Date     : 2025/8/1 13:51
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : jade_tools.cpp
*/
//
#include "include/jade_tools.h"
#include <algorithm>
namespace jade {
    const char* getVersion() {
        return "1.0.0";
    }
    bool isImageFile(const string &path)
    {
        std::string ext = path.substr(path.find_last_of(".") + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == "jpg" || ext == "jpeg" || ext == "png" ||
               ext == "bmp" || ext == "gif" || ext == "tiff" ||
               ext == "webp" || ext == "svg";
    }

} // namespace jade

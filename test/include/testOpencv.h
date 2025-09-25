/**
# @File     : testOpencv.h
# @Author   : jade
# @Date     : 2025/9/12 16:23
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testOpencv.h
*/

#pragma once
#ifdef JADE_TOOLS
#include "jade_tools/jade_tools.h"
#else
#include "include/jade_tools.h"
#endif
#ifdef OPENCV_ENABLED
// 设备类型枚举
void testOpencv();
#endif

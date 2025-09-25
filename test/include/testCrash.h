/**
# @File     : testCrash.h
# @Author   : jade
# @Date     : 2025/9/10 15:22
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testCrash.h
*/
#pragma once

#ifdef JADE_TOOLS
#include "jade_tools/jade_tools.h"
#else
#include "include/jade_tools.h"
#endif

void testCrash(const std::function<void()>& func);

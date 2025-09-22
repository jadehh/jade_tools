/**
# @File     : testHaspAdapter.cpp
# @Author   : jade
# @Date     : 2025/9/16 10:33
# @Email    : jadehh@1ive.com
# @Software : Samples
# @Desc     : testHaspAdapter.cpp
*/
#include "test/include/testHaspAdapter.h"
#include "include/jade_tools.h"

void testAdapter()
{
    LOG_INFO() << "=====================================HaspAdapter 测试开始" << "=====================================";
    jade::HaspAdapter::getInstance().init(static_cast<jade::HaspAdapter::HaspAdapterDevice>(1), {101});
    jade::HaspAdapter::getInstance().run();
}

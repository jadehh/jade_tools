# 安装breakpad

## 代码下载

```bash
git clone https://github.com/google/breakpad.git
```

## 添加CMakeLists.txt
```text
cmake_minimum_required(VERSION 3.10)
project(breakpad)
# 强制使用Unicode
add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DBREAKPAD_EXPORTS)
# 设置源文件
set(BREAKPAD_SOURCES
    src/client/windows/crash_generation/client_info.cc
    src/client/windows/crash_generation/crash_generation_client.cc
    src/client/windows/handler/exception_handler.cc
    src/common/windows/guid_string.cc
    src/common/windows/string_utils.cc
)


# 创建静态库
add_library(breakpad STATIC ${BREAKPAD_SOURCES})


#MSVC Debug
if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
endif ()


# 设置包含目录
target_include_directories(breakpad PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/client/windows
    ${CMAKE_SOURCE_DIR}/src/common/windows
)

# 设置预处理器定义
target_compile_definitions(breakpad PUBLIC
    _CRT_SECURE_NO_WARNINGS
    WIN32
    _WINDOWS

)

# 链接Windows库
target_link_libraries(breakpad
    dbghelp
    ws2_32
)

# 安装配置
install(TARGETS breakpad DESTINATION lib)
install(DIRECTORY src/google_breakpad/ DESTINATION include/breakpad/google_breakpad FILES_MATCHING PATTERN "*.h")
install(DIRECTORY src/client/windows/ DESTINATION include/breakpad/client/windows FILES_MATCHING PATTERN "*.h")
install(DIRECTORY src/common/windows/ DESTINATION include/breakpad/common/windows FILES_MATCHING PATTERN "*.h")
```

### Windows 32 位 Release 安装
```bash
cd breakpad && mkdir build_x86 && cd build_x86
# 指定32位架构
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
cmake  -G "Visual Studio 15 2017" -A Win32   -DCMAKE_INSTALL_PREFIX="D:\SDKS\breakpad\x86" .. 
cmake --build . --config  Release --target install 
```

### Windows 32 位 Debug 安装
```bash
cd breakpad && mkdir build_debug_x86 && cd build_debug_x86
# 指定32位架构
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
cmake  -G "Visual Studio 15 2017" -A Win32   -DCMAKE_INSTALL_PREFIX="D:\SDKS\breakpad\x86" .. 
cmake --build . --config  Debug --target install 
```


## Windows 64 位 Release 安装
```bash
cd breakpad && mkdir build_x64 && cd build_x64
# 指定64位架构
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake  -G "Visual Studio 15 2017" -A x64   -DCMAKE_INSTALL_PREFIX="D:\SDKS\breakpad\x64" .. 
cmake --build . --config  Release --target install 
```


## Linux 64 位 Release 安装
```bash
git clone https://github.com/google/breakpad.git
cd breakpad
git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss
./configure --prefix=/usr/local/breakpad
make -j$(nproc)
make install
```
# jade_tools

## 组件支持
- [x] Spdlog 日志模块 (无需运行库)
- [x] Openssl 实现文件的加密解密 
- [x] Sqlite 数据库模块
- [x] nvml 显卡管理模块 (属于额外功能,仅支持64位带显卡驱动的机器)
- [x] breakpad 异常管理模块
- [x] ini配置文件读取模块
- [x] SocketServer模块
- [x] 程序管理模块
- [x] 加密狗读取监听模块  
- [x] Opencv组件支持
- [x] XML模块组件支持


## 功能实现
- [x] 日志功能，支持DLL调用日志
- [x] Cmake支持编译Debug模式和Release模式
- [x] 集成Openssl支持文件加密解密
- [x] 重新优化代码永远只保留一个头文件
- [x] 日志模块支持流式输入
- [x] 支持Sqlite数据库存储
- [x] 支持加密狗的监听
- [x] 支持Opencv Rtsp协议的流管理


## 指定参数
### Windows 通用 指定参数
```text
-D BREAKPAD_DIR="D:\SDKS\vcpkg\installed"  -D SPDLOG_DIR="D:\SDKS\vcpkg\installed" -D SQLITE3_DIR="D:\SDKS\vcpkg\installed"  -D OPENSSL_DIR="D:\SDKS\vcpkg\installed" -D HASP_ADAPTER_DIR="D:\SDKS\vcpkg\installed"
```


## 可执行文件Windows编译
### Debug
```bash
cmake 
-G "Visual Studio 15 2017" -A Win32 -D CMAKE_BUILD_TYPE=Debug -D JADE_BUILD_EXAMPLES=ON  -D BUILD_SHARED=OFF  -D OPENCV_DIR="D:\SDKS\opencv-3.4.5"  
```

### Release
```bash
cmake 
-G  "Visual Studio 15 2017" -A x64 -D CMAKE_BUILD_TYPE=Release -D JADE_BUILD_EXAMPLES=ON  -D BUILD_SHARED=OFF -D OPENCV_DIR="D:\SDKS\opencv-cuda-4.11.2" -D CUDA_DIR="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8"  
```

> 存在breakpad这个库Release模式的dll不能给examplesDebug模式下使用

## 可执行文件 Linux编译

### 指定参数

```text
-D JADE_BUILD_EXAMPLES=ON  -D SQLITE3_DIR="/usr/include" -D OPENSSL_DIR="/usr/include" -D BREAKPAD_DIR="/usr/local/breakpad" -D SPDLOG_DIR="/usr/local/" -D HASP_ADAPTER_DIR="/usr/local/" -D OPENCV_DIR="/usr/local/opencv" -D CUDA_DIR="/usr/local/cuda-10.0/targets/x86_64-linux" -D BUILD_SHARED=OFF 
```
### Debug

```bash
cmake -D CMAKE_BUILD_TYPE=Debug .. 指定参数
```

### Release
```bash
cmake -D CMAKE_BUILD_TYPE=Release .. 指定参数
```

## Windows 动态库生成(dll)

### 指定参数

## 安装 32 位 Debug
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
mkdir build_x86 && cd build_x86 
cmake -G "Visual Studio 15 2017" -A Win32 .. -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX="D:\SDKS\jade_tools" -D OPENCV_DIR="D:\SDKS\opencv-3.4.5" 
cmake --build . --config Debug --target install 
```

### 安装 32位 Release
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
mkdir build_x86 && cd build_x86 
cmake -G "Visual Studio 15 2017" -A Win32 .. -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX="D:\SDKS\jade_tools" -D OPENCV_DIR="D:\SDKS\opencv-3.4.5"  
cmake --build . --config Release --target install 
```


### 安装 64位 Release

```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
mkdir build_x64 && cd build_x64
cmake -G "Visual Studio 15 2017" -A x64 .. -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX="D:\SDKS\jade_tools" -D OPENCV_DIR="D:\SDKS\opencv-cuda-4.11.2" -D CUDA_DIR="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8"   指定参数
cmake --build . --config Release --target install 
```
> 编译NVML,只能是64位



## Linux 动态库生成(so)

### 启动
```bash
docker run  --name devel -v ${PWD}:/app -it -w /app jadehh/jade-tools:1.0.3-devel-ubuntu24.04 
```

### 指定参数
```text
-D CMAKE_INSTALL_PREFIX="D:\SDKS\jade_tools" -D SQLITE3_DIR="/usr/include" -D OPENSSL_DIR="/usr/include" -D BREAKPAD_DIR="/usr/local/breakpad"  -D SPDLOG_DIR="/usr/include"
```

### Release

```bash
mkdir build_linux_release && cd build_linux_release
cmake -DCMAKE_BUILD_TYPE=Release .. 指定参数
make -j$(nproc) && make install 
```


## Linux上使用Docker编译

> [Build Docker](.docker/devel/README.md)


## 环境变量设置
```bash
Path=D:\SDKS\vcpkg\installed\x86-windows\debug\bin\;D:\SDKS\jade_tools\x86-windows\debug\bin\;D:\SDKS\opencv-3.4.5\x86\x86\vc15\bin
```


## BUG查找

Windows上发布时需要pdb和exe一起，exe是给现场使用,pdb作为bug查询工具，通过 set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi") 可以生成PDB文件

Linux也是同时发布可执行文件和.debug文件一起，.debug作为bug查询使用，



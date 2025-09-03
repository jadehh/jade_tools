# jade_tools

## 功能实现
- [x] 日志功能，支持DLL调用日志
- [x] Cmake 支持编译Debug模式和Release模式
- [x] 集成Openssl支持文件加密解密

## 安装openssl
```bash
C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat
curl -O https://www.openssl.org/source/openssl-1.1.1w.tar.gz
perl Configure VC-WIN64A shared --prefix=D:\SDKS\openssl
nmake && nmake install
```


## 安装spdlog

```bash
git clone https://github.com/gabime/spdlog.git
cd spdlog && mkdir build && cd build
cmake -G "Visual Studio 15 2017"  -A x64  -DCMAKE_INSTALL_PREFIX="D:\SDKS\spdlog" -D SPDLOG_BUILD_EXAMPLE=OFF .. 
cmake --build . --config Release --target install
```
> spdlog是无需编译成动态库的，因为他所有的方法都是在头文件中生成的





## 安装jade_tools

### 全部功能
```bash

git clone 
mkdir build && cd build 
cmake -G "Visual Studio 15 2017"  -A x64 -DCMAKE_INSTALL_PREFIX="D:\SDKS\jade_tools"  -D JADE_BUILD_EXAMPLES=OFF -D SPDLOG_DIR="D:\SDKS\spdlog" -D CUDA_DIR="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8" -D OPENSSL_DIR="D:\SDKS\openssl-x64" ..
cmake --build . --config Release --target install 

```

### 只有Log功能

```bash
C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat
cmake -G "Visual Studio 15 2017" -D ENABLE_LOG=ON  -D CMAKE_INSTALL_PREFIX="D:\SDKS\jade_log"  -D SPDLOG_DIR="D:\SDKS\spdlog"  ..

```





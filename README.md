# jade_tools


## 安装spdlog

```bash
git clone https://github.com/gabime/spdlog.git
cd spdlog && mkdir build && cd build
cmake -G "Visual Studio 15 2017"  -A x64  -DCMAKE_INSTALL_PREFIX="D:\SDKS\spdlog" -D SPDLOG_BUILD_EXAMPLE=OFF .. 
cmake --build . --config Release --target install
```
> spdlog是无需编译成动态库的，因为他所有的方法都是在头文件中生成的
## 安装jade_tools

```bash
git clone 
mkdir build && cd build 
cmake -G "Visual Studio 15 2017"  -A x64 -DCMAKE_INSTALL_PREFIX="D:\SDKS\jade_tools"  -D JADE_BUILD_EXAMPLES=OFF -D SPDLOG_DIR="D:\SDKS\spdlog" -D CUDA_DIR="C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8" ..
cmake --build . --config Release --target install 
```


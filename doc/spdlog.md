#  安装spdlog

## 代码下载

```bash
git clone https://github.com/gabime/spdlog.git
```

## Windows 32位 Release 安装
```bash
cd spdlog && mkdir build_x86 && cd build_x86
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
cmake -G "Visual Studio 15 2017" -A Win32 -DCMAKE_INSTALL_PREFIX="D:\SDKS\spdlog\x86" -D SPDLOG_BUILD_EXAMPLE=OFF .. 
cmake --build . --config Release --target install
```



## Windows 64位 Release 安装
```bash
cd spdlog && mkdir build_x64 && cd build_x64
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -G "Visual Studio 15 2017" -A x64 -DCMAKE_INSTALL_PREFIX="D:\SDKS\spdlog\x64" -D SPDLOG_BUILD_EXAMPLE=OFF .. 
cmake --build . --config Release --target install
```


## Linux 64位 Release 安装

```bash
apt-get update && apt-get install libfmt-dev libspdlog-dev
```
> spdlog是无需编译成动态库的，因为他所有的方法都是在头文件中生成的

#  安装Opencv

## 代码下载

```bash
git clone https://github.com/jadehh/opencv.git
```

## Windows 32位 Release 安装
```bash

cd opencv && git checkout 3.4.0 && mkdir build_x86 && cd build_x86
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

cmake -G "Visual Studio 15 2017" -A Win32 -DCMAKE_BUILD_TYPE=Release -D WITH_CUDA=OFF -D OPENCV_GUI=OFF -D WITH_WIN32UI=OFF -D BUILD_opencv_world=ON -D BUILD_LIST=core,imgproc,imgcodecs,videoio,highgui,video  -DCMAKE_INSTALL_PREFIX="D:\SDKS\opencv-3.4.5\x86"  .. 
cmake --build . --config Release --target install
```
> WITH_MSM Windows Server上无法支持


## Linux 64位 Release 安装

```bash
  
```
> spdlog是无需编译成动态库的，因为他所有的方法都是在头文件中生成的

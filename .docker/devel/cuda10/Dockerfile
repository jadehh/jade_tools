FROM nvcr.io/nvidia/cuda:10.0-cudnn7-devel-ubuntu18.04
LABEL authors="jadehh"
# 更新软件包索引并安装所需工具,dev是所需要的头文件
# 设置非交互式前端以避免安装过程中的提示
ENV DEBIAN_FRONTEND=noninteractive

# 更新软件包列表并安装基础工具
RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    gnupg \
    curl \
    build-essential \
    git \
    unzip \
    gdb \
    sqlite3 \
    libsqlite3-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    libdisasm-dev \
    libfmt-dev \
    && rm -rf /var/lib/apt/lists/*

# 添加 Ubuntu Toolchain PPA
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y &&  apt-get update

# 安装最新版 CMake
# 首先添加 Kitware 官方签名密钥
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null

# 添加 Kitware 官方 APT 仓库
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main'

# 安装最新版 CMake
RUN apt-get update && apt-get install -y \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# 安装breakpoint,gcc7 支持旧版本
RUN git clone --depth 1 --branch v2023.06.01 https://github.com/google/breakpad.git && \
    cd breakpad/ && \
    git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss && \
    ./configure --prefix=/usr/local/breakpad && \
    make -j$(nproc) && make install && rm -r /breakpad/

## 安装SPDLOG
RUN git clone https://github.com/gabime/spdlog.git && \
    cd spdlog && mkdir build && cd build && \
    cmake .. -D SPDLOG_BUILD_EXAMPLE=OFF && \
    make -j$(nproc) && make install

## 安装hasp
RUN git clone https://github.com/jadehh/hasp_adapter.git && cd hasp_adapter && git checkout hasp_adapter && \
    mv  include/hasp_api.h /usr/local/include &&  mv lib/Linux/x86_64/libhasp_linux_x86_64_34033.a  /usr/local/lib/ && rm -r /hasp_adapter

## 安装hasp驱动
RUN  wget https://github.com/jadehh/pythonTools/releases/download/JadeV1.9.7/aksusbd_8.51-1_amd64.deb && dpkg -i aksusbd_8.51-1_amd64.deb && rm aksusbd_8.51-1_amd64.deb
RUN  wget https://github.com/jadehh/pythonTools/releases/download/JadeV1.9.7/hasplm.ini && cp hasplm.ini /etc/hasplm/ && rm hasplm.ini

# 安装Video Codec SDK 注意版本兼容,libnvcuvid.so和libnvidia-encode.so 需要宿主机的驱动
RUN wget https://github.com/jadehh/VideoSDK/releases/download/v1.0.0/Video_Codec_SDK_9.1.23.zip && \
    unzip Video_Codec_SDK_9.1.23.zip && rm Video_Codec_SDK_9.1.23.zip &&  \
    mv /Video_Codec_SDK_9.1.23/include/* /usr/local/cuda/include/ && \
    mv /Video_Codec_SDK_9.1.23/Lib/linux/stubs/x86_64/* /usr/local/cuda/lib64/stubs/ && rm -r /Video_Codec_SDK_9.1.23

RUN ln -sT /usr/local/cuda/lib64/stubs/libnvcuvid.so /usr/lib/x86_64-linux-gnu/libnvcuvid.so.1
RUN ln -sT /usr/local/cuda/lib64/stubs/libnvidia-encode.so /usr/lib/x86_64-linux-gnu/libnvidia-encode.so.1

# 安装Opencv所需要的库
RUN apt-get update && apt-get install -y \
    pkg-config \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libopenblas-dev \
    liblapack-dev \
    liblapacke-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer1.0-dev  \
    libtbb2 \
    libtbb-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev &&  \
    rm -rf /var/lib/apt/lists/*

# 编译OpenCV 注意 Opencv的版本
RUN git clone --depth 1 --branch 4.3.0 https://github.com/opencv/opencv.git  && \
    git clone --depth 1 --branch 4.3.0 https://github.com/opencv/opencv_contrib.git && \
    cd /opencv && mkdir build && cd build && \
    cmake  \
        -D CMAKE_BUILD_TYPE=RELEASE \
        -D CMAKE_INSTALL_PREFIX=/usr/local/opencv \
        -D WITH_OPENGL=OFF \
        -D WITH_FFMPEG=ON \
        -D WITH_NVCUVID=ON \
        -D WITH_NVENC=ON \
        -D CUDA_FAST_MATH=ON \
        -D OPENCV_ENABLE_NONFREE=ON \
        -D WITH_CUDA=ON \
        -D WITH_CUDNN=ON \
        -D OPENCV_DNN_CUDA=ON \
        -D CUDA_ARCH_BIN="6.1;7.5" \
        -D BUILD_opencv_cudacodec=ON \
        -D BUILD_opencv_cudalegacy=ON \
        -D ENABLE_FAST_MATH=ON  \
        -D WITH_TBB=ON  \
        -D OPENCV_EXTRA_MODULES_PATH=/opencv_contrib/modules/ \
        -D CMAKE_LIBRARY_PATH=/usr/local/cuda/lib64/stubs ..  && make  -j$(nproc) && make install && rm -r /opencv/ && rm -r /opencv_contrib

## 设置运行环境 包括breakpad和opencv的环境
ENV PATH=/usr/local/opencv/bin:/usr/local/breakpad/bin/:${PATH}
## 时区设置
RUN apt-get update && apt-get install tzdata &&  \
    rm -rf /var/lib/apt/lists/* && \
    ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
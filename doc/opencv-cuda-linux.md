# Opencv Cuda Linux 编译

## 环境准备

```bash
apt-get update && apt-get install -y \
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
    libavif-dev \
    libavif-bin \
    libtiff-dev &&  \
    rm -rf /var/lib/apt/lists/*
```


## 安装Video Codec SDK
```bash
wget https://github.com/jadehh/VideoSDK/releases/download/v1.0.0/Video_Codec_SDK_9.1.23.zip && \
unzip Video_Codec_SDK_9.1.23.zip && rm Video_Codec_SDK_9.1.23.zip &&  \
mv /Video_Codec_SDK_9.1.23/include/* /usr/local/cuda/include/ && \
mv /Video_Codec_SDK_9.1.23/Lib/linux/stubs/x86_64/* /usr/local/cuda/lib64/stubs/ && rm -r /Video_Codec_SDK_9.1.23
ln -sT /usr/local/cuda/lib64/stubs/libnvcuvid.so /usr/lib/x86_64-linux-gnu/libnvcuvid.so.1
ln -sT /usr/local/cuda/lib64/stubs/libnvidia-encode.so /usr/lib/x86_64-linux-gnu/libnvidia-encode.so.1
```
> Video SDK版本需要与CUDA版本一致

下表列出了经 NVIDIA 官方测试和推荐的主要版本组合。**粗体**版本表示这是一个常见且稳定的组合。

| Video SDK 版本 | 推荐 CUDA 版本 | 重要特性与说明 |
| :--- | :--- | :--- |
| **12.2** | **CUDA 12.2+** | 最新版本，支持所有最新特性和架构。 |
| **12.1** | **CUDA 12.1** | |
| **12.0** | **CUDA 11.8** | **引入对 AV1 硬件编码的支持**（需 Ada Lovelace GPU）。这是一个重要的功能版本。 |
| **11.1** | **CUDA 11.5** | 一个稳定的生产环境版本。 |
| **11.0** | CUDA 11.4 | 显著的性能提升和功能更新。 |
| **10.0** | **CUDA 10.2** | 一个非常流行且稳定的长期使用版本。 |
| **9.1** | **CUDA 10.0** | 引入对 Turing 编码器的支持。 |
| 9.0 | CUDA 9.2 | |
| 8.2 | CUDA 8.0 | 较旧的稳定版本。 |

> **注意**：此表为通用指南。对于关键项目，**务必查阅您所使用的特定 Video SDK 版本的官方发行说明 (Release Notes)**。


## 源码下载
```bash
git clone --depth 1 --branch 4.3.0 https://github.com/opencv/opencv.git  &&  git clone --depth 1 --branch 4.3.0 https://github.com/opencv/opencv_contrib.git
```

### 部分兼容的版本
| OpenCV 版本 | 兼容性 | 所需修改 |
|------------|--------|----------|
| **4.5.5** | ⚠️ 部分兼容 | 需要删除 AV1 相关代码 |
| **4.4.0** | ⚠️ 部分兼容 | 可能需要小幅度调整 |
| **4.3.0** | ✅ 完全兼容 | 无需修改 |
| **4.2.0** | ✅ 完全兼容 | 无需修改 |

## 编译
```bash
mkdir build && cd build && \
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
```
# NVIDIA GPU 计算能力完整参考表 (10系列 - 当前最新)

从经典的10系列到最新的RTX 40系列，NVIDIA GPU的计算能力经历了多次演进。以下是完整的技术规格参考。

## GPU计算能力总表
| 架构 | 计算能力 | 发布时间 | 关键特性 | 代表显卡型号 |
|------|----------|----------|----------|-------------|
| **Ada Lovelace** | **8.9** | 2022 | 第四代Tensor Core，光流加速器，DLSS 3 | RTX 4090, RTX 4080, RTX 4070 |
| **Ampere** | **8.0, 8.6** | 2020 | 第三代Tensor Core，RT Core，稀疏化 | RTX 3090, RTX 3080, RTX 3070, A100 |
| **Turing** | **7.5** | 2018 | 首次集成RT Core，第二代Tensor Core | RTX 2080 Ti, RTX 2080, RTX 2070 |
| **Volta** | **7.0** | 2017 | 专用Tensor Core，NVLink 2.0 | Tesla V100, Titan V |
| **Pascal** | **6.1** | 2016 | 16nm工艺，GDDR5X，Unified Memory | GTX 1080 Ti, GTX 1080, GTX 1070 |
| **Maxwell** | **5.0, 5.2** | 2014 | 能效优化，第二代SMM设计 | GTX 980, GTX 970, GTX 960 |

> CUDA ARCH BIN CUDA10 最高支持:7.5  最新列表 6.1;7.5;8.0;8.6;8.9;9.0
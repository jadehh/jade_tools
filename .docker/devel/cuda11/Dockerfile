FROM nvcr.io/nvidia/cuda:11.8.0-cudnn8-devel-ubuntu22.04
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
    && rm -rf /var/lib/apt/lists/*

# 安装其他开发工具
RUN apt-get update && apt-get install -y \
    sqlite3 \
    libsqlite3-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    libdisasm-dev \
    libfmt-dev  \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装breakpoint
RUN git clone https://github.com/google/breakpad.git && git clone https://github.com/jadehh/linux-syscall-support.git && \
        cp -r linux-syscall-support/lss/ breakpad/src/third_party && \
        cd breakpad && ./configure --prefix=/usr/local/breakpad && \
        make -j$(nproc) && make install && rm -r /breakpad && rm -r /linux-syscall-support

# 设置多个环境变量
ENV PATH=/usr/local/breakpad/bin:${PATH}
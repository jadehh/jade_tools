FROM nvcr.io/nvidia/cuda:10.0-runtime-ubuntu18.04
LABEL authors="jadehh"
# 更新软件包索引并安装所需工具,dev是所需要的头文件
# 设置非交互式前端以避免安装过程中的提示
ENV DEBIAN_FRONTEND=noninteractive

# 安装其他的开发工具
RUN apt-get update && apt-get install -y \
    sqlite3 \
    openssl \
    && rm -rf /var/lib/apt/lists/*



## 设置运行环境 包括breakpad和opencv的环境
ENV PATH=/usr/local/breakpad/bin:${PATH}
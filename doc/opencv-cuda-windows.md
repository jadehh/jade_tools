# Opencv

## 手动编译的优势

* 针对CPU架构定制指令集
* 使用CUDA对Opencv加速
* 精简Opencv和额外的算法

## Windows下编译Opencv CUDA

以下是**Windows平台下编译支持CUDA的OpenCV的详细文档**，整合了多篇权威指南的核心步骤和避坑要点：

---

### 一、环境准备（关键！）

1. **硬件与驱动**
    - NVIDIA显卡（支持CUDA，算力≥3.5）
    - 安装最新[NVIDIA驱动](https://www.nvidia.com/drivers)

2. **软件依赖**
    - **CUDA Toolkit**：推荐11.x或12.x（[官网下载](https://developer.nvidia.com/cuda-toolkit-archive)）
    - **cuDNN**：需与CUDA版本匹配（[官网下载](https://developer.nvidia.com/cudnn)）不需要DNN加速可不用
    - **Visual Studio**：2017/2019/2022（安装时勾选“C++桌面开发”）
    - **CMake** ≥ 3.10（[官网下载](https://cmake.org/download/)）
    - **intel mkl** ≥  2025.1.0（[官网下载](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onemkl-download.html)）
    - **oneTBB** ≥ 2022.1.0（[官网下载](https://github.com/uxlfoundation/oneTBB/releases)）
    - **video-codec-sdk** >= 12.0 （[官网下载](https://developer.nvidia.com/video-codec-sdk-archive)）需要放在cuda目录下
    - **PowerShell** >= 7.0 （[官网下载](https://github.com/PowerShell/PowerShell/releases)）需要配置环境变量
    - **nasm** >= 2.16.03（[官网下载](https://www.nasm.us/)）需要配置环境变量
    - **OpenJPG** >= 2.5.3（[官网下载](https://github.com/uclouvain/openjpeg/releases)）需要配置环境变量(提升imread性能)
    - **OpenCV源码**：包含主库和contrib模块（版本需一致）
   ```bash
   git clone https://github.com/opencv/opencv.git
   git clone https://github.com/opencv/opencv_contrib.git
   ```

---

### 二、编译配置（CMake关键步骤）

1. **打开CMake GUI**
    - `Where is the source code`：指向`opencv`源码目录
    - `Where to build the binaries`：新建一个构建目录（如`build`）

2. **首次配置**
    - 点击`Configure`，选择匹配的Visual Studio版本和`x64`架构

**启用CUDA相关选项**

| **选项**                                | **值/操作**                       | **说明**                                                    |
|---------------------------------------|--------------------------------|-----------------------------------------------------------|
| `WITH_CUDA`                           | ✅勾选                            | 启用CUDA支持                                                  |
| `OPENCV_EXTRA_MODULES_PATH`           | 指向`opencv_contrib/modules`     | 启用额外算法（如SIFT）                                             |
| `CUDA_ARCH_BIN`                       | 填写显卡算力（如`7.5;8.0;8.6;8.9;9.0`） | [算力查询表](https://developer.nvidia.com/cuda-gpus)           |
| `OPENCV_ENABLE_NONFREE`               | ✅勾选                            | 启用专利算法（如SURF）                                             |
| `ENABLE_FAST_MATH` + `CUDA_FAST_MATH` | ✅勾选                            | 加速数学计算                                                    |
| `WITH_TBB`                            | ✅勾选                            | 充分利用多核CPU（尤其是现代8核以上处理器），提升OpenCV算法的吞吐量                    |
| `MKL_ROOT_DIR`                        | 指向`Intel/oneAPI/mkl/latest`    | MKL库来加速数学运算                                               |
| `BUILD_opencv_world`                  | ✅勾选                            | 所有模块会被整合到单个库文件 opencv_world 中 大库文件可能占用过多存储/内存，需权衡便利性与资源限制 |
GPU
```bash
mkdir build && cd build 
cmake  -G "Visual Studio 15 2017" -A x64 -D BUILD_JAVA=OFF    -D WITH_CUDA=ON  -D OPENCV_EXTRA_MODULES_PATH="D:\code\opencv_contrib\modules" -D CUDA_ARCH_BIN="7.5;8.0;8.6;8.9;9.0" -D OPENCV_ENABLE_NONFREE=ON -D ENABLE_FAST_MATH=ON -D CUDA_FAST_MATH=ON  -D MKL_ROOT_DIR="C:/Program Files (x86)/Intel/oneAPI/mkl/latest" -D WITH_TBB=ON -D TBB_DIR=D:/SDKS/oneapi-tbb-2022.1.0/lib/cmake/tbb ..
```
CPU
```bash
cmake  -G "Visual Studio 15 2017" -A x64 -D BUILD_JAVA=OFF   -D ENABLE_FAST_MATH=ON -D CUDA_FAST_MATH=ON  -D MKL_ROOT_DIR="C:/Program Files (x86)/Intel/oneAPI/mkl/latest" -D WITH_TBB=ON -D TBB_DIR=D:/SDKS/oneapi-tbb-2022.1.0/lib/cmake/tbb ..
```

1. **生成VS工程**
   **- 点击`Generate`生成Visual Studio解决方案（`.sln`文件）**

---

### 三、编译与安装（Visual Studio）

1. **打开解决方案**
    - 在构建目录（如`build`）中找到`OpenCV.sln`，用VS打开

2. **编译选项**
    - 顶部菜单切换为`Release` + `x64`
    - 右键解决方案 → `生成` → 选择`ALL_BUILD`（先编译） → 再选`INSTALL`（安装）

3. **等待编译完成**
    - 耗时约1-3小时（取决于硬件）
    - 成功后在`install`目录生成库文件（默认路径：`build/install`）

---

### 四、验证安装

1. **C++项目配置**
    - 包含目录：添加`install/include`
    - 库目录：添加`install/x64/vc17/lib`（根据VS版本调整）
    - 链接器 → 输入：添加`opencv_world4xx.lib`（若启用world库）
---


### 五、优化建议

1. **多线程编译**： 在VS中启用`/MP`选项（项目属性 → C/C++ → 所有选项 → `Multi-processor Compilation`设为`Yes`）
2. **减少编译时间**：在CMake中禁用不需要的模块（如`BUILD_TESTS=OFF`）
3. **版本兼容性**：OpenCV 4.6+需CUDA ≥11.0，推荐使用最新稳定版组合

> 完整编译好的库文件（含CUDA）可移植条件：目标机器需相同CUDA版本+相同显卡算力。

**参考资料**：

- [CUDA算力对照表](https://developer.nvidia.com/cuda-gpus)
- [OpenCV官方编译指南](https://docs.opencv.org/4.x/d3/d52/tutorial_windows_install.html)


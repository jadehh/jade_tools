# Devel Docker

## CUDA 10
```bash
cd .docker/devel/cuda10
docker build -t jadehh/jade-tools:1.0.3-opencv-4.3.0-cuda-10.0-cudnn7-devel-ubuntu18.04 . 
```
> 最高支持RTX 20 系列的显卡

## CUDA 11
```bash
cd ./docker/devel/cuda11
docker build -t jadehh/jade-tools:1.0.3-opencv-4.5.0-cuda-11.8.0-cudnn8-devel-ubuntu22.04 . 
```
> 最高支持RTX 30 系列的显卡

| 显卡驱动版本最低要求    | cuda版本       | 镜像                                                                          |
|:--------------|:-------------|:----------------------------------------------------------------------------|
| **440.xx**    | **CUDA10**   | jadehh/jade-tools:1.0.3-opencv-4.3.0-cuda-10.0-cudnn7-runtime-ubuntu18.04 . |
| **520.xx.xx** | **CUDA11** | jadehh/jade-tools:1.0.3-cuda-11.8.0-cudnn8-runtime-ubuntu22.04 .            |
| **555.xx.xx** | **CUDA12** | jadehh/jade-tools:1.0.3-cuda-11.8.0-cudnn8-runtime-ubuntu22.04 .            |
* 高版本的显卡无法使用低版本的镜像，因为低版本的显卡驱动不兼容
* 低版本的显卡可以通过显卡驱动升级的办法使用最新的镜像，如果不做升级，只能使用旧的Docker镜像 
* 显卡驱动的cuda去匹配对应的镜像
## 启动
```bash
docker run  --name devel -v ${PWD}:/app -it -w /app jadehh/jade-tools:1.0.3-opencv-4.3.0-cuda-10.0-cudnn7-devel-ubuntu18.04 

```

> 使用同一套代码，只区分镜像，代码里面自动判断当前cuda的版本去使用
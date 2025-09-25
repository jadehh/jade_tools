#  安装spdlog

## Windows 32位  安装
```bash
vcpkg install spdlog:x86-windows
```



## Windows 64位  安装
```bash
vcpkg install spdlog:x64-windows
```


## Linux 64位 Release 安装

```bash
apt-get update && apt-get install libfmt-dev libspdlog-dev
```
> spdlog是无需编译成动态库的，因为他所有的方法都是在头文件中生成的

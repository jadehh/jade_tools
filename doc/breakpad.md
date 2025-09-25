# 安装breakpad

## Windows 32 安装
```bash
vcpkg install breakpad:x86-windows
```

## Windows 64 安装
```bash
vcpkg install breakpad:x64-windows
```

## Linux 64 位 Release 安装
```bash
git clone https://github.com/google/breakpad.git
cd breakpad
git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss
./configure --prefix=/usr/local/breakpad
make -j$(nproc)
make install
```
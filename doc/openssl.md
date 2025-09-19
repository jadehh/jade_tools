# 安装openssl

## 代码下载
```bash
wget  https://www.openssl.org/source/openssl-1.1.1w.tar.gz
```


## Windows 32位 Release 安装
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
perl Configure VC-WIN32 shared --prefix=D:\SDKS\openssl\x86
nmake && nmake install
```

## Windows 64位 Release 安装
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
perl Configure debug-VC-WIN64A shared --prefix=D:\SDKS\openssl\x64
nmake && nmake install
```


## Linux安装
```bash
apt-get update && apt-get install -y openssl libssl-dev 
```
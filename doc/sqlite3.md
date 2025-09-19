# 安装Sqlite3功能
[下载链接](https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip)

## Windows 32位 Release 安装
```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
cl /DSQLITE_API=__declspec(dllexport) /O2 /LD sqlite3.c -link /DLL /OUT:sqlite3.dll
```

## Windows 64位 Release 安装

```bash
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /DSQLITE_API=__declspec(dllexport) /O2 /LD sqlite3.c -link /DLL /OUT:sqlite3.dll
```
> sqlite3 是可以直接编译的


## Linux 安装
```bash
apt-get update && apt-get install -y sqlite3 libsqlite3-dev 
```
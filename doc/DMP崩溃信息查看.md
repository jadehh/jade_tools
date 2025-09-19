# DMP 文件分析
## 生成符号文件（.sym）
```bash
dump_syms ./examples > examples.sym
```
1. 查看 .sym文件的第一行：
```bash
head -n1 examples.sym 
```
输出类似：
```text
MODULE Linux x86_64 B487AC3797CCC7CC2958181F5F125B350 examples
```
- Linux：操作系统
- x86_64：CPU架构
- B487...：模块标识符（Build ID）
- my_app：模块名
2. 根据这些信息创建目录结构并放置符号文件：
```bash
mkdir -p ./symbols/examples/A6B8461E138BD7409C5A3DC3EE5239EA0/
cp examples.sym ./symbols/examples/A6B8461E138BD7409C5A3DC3EE5239EA0/ 
```
> 对于系统库（如 libc.so.6），你也需要为它们生成 .sym文件并放入 symbols目录对应的路径下，这样堆栈跟踪才能完全符号化。
#### 解析 DMP 文件
1. 使用 minidump_dump快速查看（未符号化）
```bash
minidump_dump crash/crash_*.dmp 
```
> 这会输出大量原始信息，包括系统信息、异常记录、线程列表以及每个线程的未符号化的堆栈回溯（是一堆内存地址）。
2. 使用 minidump_stackwalk生成可读的崩溃报告（符号化）
```bash
minidump_stackwalk crash/crash_*.dmp ./symbols > crash_report.txt
```
> 解析完成后，所有的堆栈跟踪都会被符号化。打开 crash_report.txt，你将看到格式清晰、可读的崩溃报告。



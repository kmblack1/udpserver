# 1 本节说明
解析数据起始位为0x7567的数据。超大文件的收发。



# 2 引用库
## 2.1 WINDOWS使用到的库
```bash
vcpkg search tiny-regex-c;
vcpkg search sqlcipher;
vcpkg search gettimeofday;
vcpkg install gettext tiny-regex-c sqlcipher gettimeofday;
```
## 2.2 ubuntu使用到的库
```bash
sudo apt-get -y install build-essential regex sqlite3
```


# 3 编译命令
## 3.1 WINDOWS
```bash
rd /S /Q D:\MyWork\2022\01\LargeFile\build
cd D:\MyWork\2022\01\LargeFile
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev -DKMBLACK_DIR=D:/MyWork/2022/01 -DUDP_SERVER_DIR=D:/MyWork/2022/01/UdpServer
cmake --build build --config Release
```
## 3.2 LINUX
```bash
cd ~/2022/LargeFile
mkdir build
cd build
rm -rf * && \
cmake -DCMAKE_BUILD_TYPE=release -DKMBLACK_DIR=/usr/local/kmblack -DUDP_SERVER_DIR=/home/kmblack/2022/udpserver -DCMAKE_INSTALL_PREFIX=/home/kmblack/2022/app/plugins ..
make
make install
objdump -tT libfive_fifty.so
```


# 4 查看导出的dll
## 4.1 WINDOWS
```bash
#查看导出的函数
dumpbin /EXPORTS D:\MyWork\2022\01\UdpServer\plugins\LargeFile.dll
#查看dll依赖项
dumpbin /DEPENDENTS D:\MyWork\2022\01\UdpServer\plugins\LargeFile.dll
```
# 1 本节说明
实现Udp Server



# 2 引用库
## 2.1 WINDOWS使用到的库
```bash
vcpkg search gettext;
vcpkg search pthread;
vcpkg search jansson;
vcpkg search libpq;
vcpkg install gettext pthreads jansson libpq;
```
## 2.2 ubuntu使用到的库
```bash
sudo apt-get -y install build-essential libtool automake cmake unzip clang git wget libjansson-dev libpq-dev
```


# 3 编译命令
## 3.1 WINDOWS
```bash
rd /S /Q D:\MyWork\2022\01\UdpServer\build
cd D:\MyWork\2022\01\UdpServer
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev -DKMBLACK_DIR=D:/MyWork/2022/01
cmake --build build --config Release
```
## 3.2 LINUX
```bash
cd ~/2022/udpserver
mkdir build
cd build
rm -rf * && \
cmake -DCMAKE_BUILD_TYPE=release -DKMBLACK_DIR=/usr/local/kmblack -DCMAKE_INSTALL_PREFIX=/home/kmblack/2022/app ..
make
make install
cd ~/2022/app
sudo ./udpserver -r
```

# 4 查看端口占用
## 4.1 WINDOWS
```bash
netstat -ano |findstr "54321"
```
## 4.2 linux
```bash
netstat -tunlp | grep 端口号
```
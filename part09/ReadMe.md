# 1 ����˵��
���������ļ��Ͳ�������ļ�



# 2 ���ÿ�
## 2.1 WINDOWSʹ�õ��Ŀ�
```bash
vcpkg search gettext;
vcpkg search pthread;
vcpkg search jansson;
vcpkg install gettext pthreads jansson;
```
## 2.2 ubuntuʹ�õ��Ŀ�
```bash
sudo apt-get -y install build-essential libtool automake cmake unzip clang git wget libjansson-dev
```

# 3 ��������
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
cmake -DCMAKE_BUILD_TYPE=release -DKMBLACK_DIR=/usr/local/kmblack ..
make
```
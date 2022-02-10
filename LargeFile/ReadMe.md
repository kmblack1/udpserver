# 1 ����˵��
����������ʼλΪ0x7567�����ݡ������ļ����շ���



# 2 ���ÿ�
## 2.1 WINDOWSʹ�õ��Ŀ�
```bash
vcpkg search tiny-regex-c;
vcpkg search sqlcipher;
vcpkg search gettimeofday;
vcpkg install gettext tiny-regex-c sqlcipher gettimeofday;
```
## 2.2 ubuntuʹ�õ��Ŀ�
```bash
sudo apt-get -y install build-essential regex sqlite3
```


# 3 ��������
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


# 4 �鿴������dll
## 4.1 WINDOWS
```bash
#�鿴�����ĺ���
dumpbin /EXPORTS D:\MyWork\2022\01\UdpServer\plugins\LargeFile.dll
#�鿴dll������
dumpbin /DEPENDENTS D:\MyWork\2022\01\UdpServer\plugins\LargeFile.dll
```
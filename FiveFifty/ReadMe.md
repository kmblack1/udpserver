# 1 ����˵��
����������ʼλΪ0x0551������



# 2 ���ÿ�



# 3 ��������
## 3.1 WINDOWS
```bash
rd /S /Q D:\MyWork\2022\01\FiveFifty\build
cd D:\MyWork\2022\01\FiveFifty
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev -DKMBLACK_DIR=D:/MyWork/2022/01 -DUDP_SERVER_DIR=D:/MyWork/2022/01/UdpServer
cmake --build build --config Release
```
## 3.2 LINUX
```bash
cd ~/2022/FiveFifty
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
dumpbin /EXPORTS D:\MyWork\2022\01\UdpServer\plugins\FiveFifty.dll
#�鿴dll������
dumpbin /DEPENDENTS D:\MyWork\2022\01\UdpServer\plugins\FiveFifty.dll
```
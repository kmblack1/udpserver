# 1 ����˵��
regex������ʽ


# 2 ���ÿ�
## 2.1 WINDOWSʹ�õ��Ŀ�
```bash
vcpkg search tre;
vcpkg search jansson;
vcpkg install gettext tre jansson;
```
## 2.2 ubuntuʹ�õ��Ŀ�
```bash
sudo apt-get -y install build-essential libjansson-dev
```


# 3 ��������
## 3.1 WINDOWS
```bash
rd /S /Q D:\MyWork\2022\01\regex\build
cd D:\MyWork\2022\01\regex
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev -DKMBLACK_DIR=D:/MyWork/2022/01
cmake --build build --config Release
copy /Y D:\MyWork\2022\01\x64\Release\kmblack.dll D:\MyWork\2022\01\regex\build\Release
copy /Y E:\vcpkg\installed\x64-windows\bin\jansson.dll D:\MyWork\2022\01\regex\build\Release
```
## 3.2 LINUX
```bash
cd ~/2022/regex
mkdir build
cd build
rm -rf * && \
cmake -DCMAKE_BUILD_TYPE=release -DKMBLACK_DIR=/usr/local/kmblack ..
make
make install
objdump -tT libfive_fifty.so
```


# 4 �鿴������dll
## 4.1 WINDOWS
```bash
#�鿴�����ĺ���
dumpbin /EXPORTS E:\vcpkg\installed\x64-windows\bin\tre.dll
#�鿴dll������
dumpbin /DEPENDENTS D:\MyWork\2022\01\x64\Release\kmblack.dll
```
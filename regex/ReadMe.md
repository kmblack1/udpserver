# 1 本节说明
regex正则表达式


# 2 引用库
## 2.1 WINDOWS使用到的库
```bash
vcpkg search tre;
vcpkg search jansson;
vcpkg install gettext tre jansson;
```
## 2.2 ubuntu使用到的库
```bash
sudo apt-get -y install build-essential libjansson-dev
```


# 3 编译命令
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


# 4 查看导出的dll
## 4.1 WINDOWS
```bash
#查看导出的函数
dumpbin /EXPORTS E:\vcpkg\installed\x64-windows\bin\tre.dll
#查看dll依赖项
dumpbin /DEPENDENTS D:\MyWork\2022\01\x64\Release\kmblack.dll
```
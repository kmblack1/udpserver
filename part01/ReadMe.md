C基本数据类型

# 1 编译命令

## WINDOWS
从开始菜单中打开"x64 Native Tools Command Prompt for VS 2019"，执行下列命令
```bah
cd D:\MyWork\2022\01\part01
rd /S /Q D:\MyWork\2022\01\part01\build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev
cmake --build build --config Release
```
## LINUX
打开终端执行下列命令
```bah
cd ~/code/2022/01/part01
mkdir build
cd build
rm -rf * && \
cmake -DCMAKE_BUILD_TYPE=release ..
make
sudo make installls 
```
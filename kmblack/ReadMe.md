# 1 dll说明
这个库是本项目的核心库，主要定义一些常用的函数；

# 2 引用库

[PostgreSQL](https://www.postgresql.org/ftp/source/v14.1/)


# 3 编译命令
## 3.1 WINDOWS
rd /S /Q D:\MyWork\2022\01\kmblack\build
cd D:\MyWork\2022\01\kmblack
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev ..
cmake --build build --config Release
cmake --install build --config Release

## 3.2 LINUX
mkdir build
cd build
rm -rf *
cmake -DCMAKE_BUILD_TYPE=release ..
make
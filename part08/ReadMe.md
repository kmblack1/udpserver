# 1 ����˵��
�ڴ�й¶���



# 2 ���ÿ�

```bash
vcpkg search gettext;
vcpkg search pthread;
vcpkg install gettext pthreads;
```


# 3 ��������
## 3.1 WINDOWS
```bash
rd /S /Q D:\MyWork\2022\01\kmblack\build
cd D:\MyWork\2022\01\kmblack
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev ..
cmake --build build --config Release
cmake --install build --config Release
```
## 3.2 LINUX
```bash
mkdir build
cd build
rm -rf *
cmake -DCMAKE_BUILD_TYPE=release -DKMBLACK_DIR=/usr/local/kmblack ..
make
```
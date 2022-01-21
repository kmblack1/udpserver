# 1 dll˵��
������Ǳ���Ŀ�ĺ��Ŀ⣬��Ҫ����һЩ���õĺ�����

# 2 ���ÿ�
[PostgreSQL](https://www.postgresql.org/ftp/source/v14.1/)

## 2.1 WINDOWSʹ�õ��Ŀ�
```bash
vcpkg search jansson
```

## 2.2 ubuntuʹ�õ��Ŀ�
```bash
sudo apt-get -y install build-essential libtool automake cmake unzip clang git wget
```

# 3 ��������
## 3.1 WINDOWS
```bash
rd /S /Q D:\MyWork\2022\01\kmblack\build
cd D:\MyWork\2022\01\kmblack
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=E:/vcpkg/scripts/buildsystems/vcpkg.cmake -Wno-dev
cmake --build build --config Release
cmake --install build --config Release
```

## 3.2 LINUX
```bash
cd ~/2022/kmblack
mkdir build
cd build
rm -rf * && \
sudo rm -rf /usr/local/kmblack && \
cmake -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=/usr/local/kmblack ..
make
sudo make install
```
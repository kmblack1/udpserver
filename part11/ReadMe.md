# 1 本节说明
SQLite3、SQLCipher的基本使用方法


# 2 引用库
## 2.1 WINDOWS使用到的库
```bash
vcpkg search gettext;
vcpkg search sqlite3;
vcpkg search sqlcipher;
vcpkg install gettext sqlite3 sqlcipher;
```
## 2.2 ubuntu使用到的库
```bash
sudo apt-get -y install build-essential libtool automake cmake unzip clang git wget sqlite3 sqlcipher
```

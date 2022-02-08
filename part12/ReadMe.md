# 1 本节说明
1.雪花算法介绍及实现

2.生成高质量随机数

3.SQLite3创建表,并写入1千万垃圾测试数据。



# 2 引用库
## 2.1 WINDOWS使用到的库
```bash
vcpkg search gettext;
vcpkg search sqlite3;
vcpkg search sqlcipher;
vcpkg install gettimeofday gettext sqlite3 sqlcipher sqlcipher[tool];
```
## 2.2 ubuntu使用到的库
```bash
sudo apt-get -y install build-essential libtool automake cmake unzip clang git wget sqlite3 sqlcipher
```

# 3 收发流程(暂定)

客户端发送文件步骤：MTU(576 - 28 = 548 ) 因此数据包最大为548字节，本功能2字节标识符为0x7567。

1.客户端向Udp Server发送数据前验证用户信息（用户名和密码最大不超过533字节 = 548-2-1-2-8-2）

	[2字节标识符 + 1字节command(0x01)+ 2字节数据长度(10) + 用户id + 用户密码(sha256) + 8字节文件长度+ 2字节crc16]

2.用户验证

如果用户名和密码错误，数据回发给客户端（9字节）
	
	[2字节标识符 + 1字节command(0x01) + 2字节数据长度(10) + 2字节状态码（0x0001） + 2字节crc16]	


用户名和密码正确，Udp Server将客户端请求写入数据库filebegin表，并生成8字节ID，Udp Server将ID回发给客户端（15字节）<br />

	[2字节标识符 + 1字节command(0x02) + 2字节数据长度(10) + 8字节ID[变种雪花算法snowflake] + 2字节crc16]

3.客户端向Udp Server循环发送数据

	[
	2字节标识符 + 1字节command(0x02) + 2字节数据长度 + 8字节ID[变种雪花算法snowflake] 
    + 8字节文件长度 + 8字节偏移量 + 文件数据(最大519字节=548-2-1-2-24-2) + 2字节crc16
	]

4.Udp Server收到数据回发给客户端（9字节）

	[2字节标识符 + 1字节command(0x02) + 2字节数据长度(10) + 2字节状态码（0x0000成功，其它为异常） + 2字节crc16]

5.客户端收到Udp Server回发的状态数据后，检查如果成功则继续发送下一个包，否则重发。

6.循环3-5，直到文件发送发送完成。

7.Udp Server根据文件长度和偏移量确认接收数据完成后，向数据库filedone表写入完成信息。

/****************************************************************************************
	Udp Server接收文件时保存文件句柄
	drop table if exists filepointers;

	客户端发送的数据格式
	2字节标识符 + 1字节command + 2字节数据长度 + 1字节编号 + 8字节文件长度 + 8字节偏移量 + 文件数据
	

	客户端首次发送文件时将udp server将打开的文件信息写入filepointers和fplasts,以后udp server每接收一次都将接收时间写入fplasts
	udp server在后台有再开一个线程，实现关闭句柄或清理资源功能
		1.每过3秒运行一次，检查fplasts最后两条记录的generate相差是否>=3秒（表示发收和接收文件失败），如>=3秒则从filepointers取出文件句柄然后关闭
		2.每天03:00时对sql进行VACUUM

	软件最多运行至36812-02-20 08:36:15.000064+08，从1970-01-01开始+1099511627775秒允许的范围

	objectid为变种的雪花算法snowflake[41字节时间戳(打开文件句柄的时间,单位为秒) + 10字节文件编号(客户端发送文件时的编号值范围(1-255))+ 12字节0]
	注意：所有使用的时间单位是秒，不是毫秒
****************************************************************************************/
 create table if not exists filepointers(
	objectid unsigned big int not null													--唯一编号
	address char[4] not null,																	--客户端IP
	port char[4] not null,																		--客户端端口
	hand unsigned bigint not null,															--UdpServer接收客户端发送文件时打开的文件句柄
	generate unsigned bigint generated always as ((objectid>>12) & 1099511627775 ) stored,		--打开文件句柄的时间(值范围0-1099511627775)
	fileid smallint generated always as cast((objectid & cast(255 as bigint)) as smallint) stored,				--客户端发送文件时的编号(值范围0-4095)，由客户端发送时设置
	constraint pk_filepointers_hand primary key(objectid)
);
create index if not exists idx_filepointers_search on filepointers(address,port,fileid);


/****************************************************************************************
	上一次打开文件句柄的时间
	drop table if exists fplasts;
	objectid为变种的雪花算法snowflake[41字节时间戳(打开文件句柄的时间,单位为秒) + 10字节文件编号(客户端发送文件时的编号值范围(1-255))+  12字节顺序号]
****************************************************************************************/
 create table if not exists fplasts(
	objectid  unsigned big int not null,													--唯一编号
	generate unsigned big int,																--最后一次使用文件句柄的时间
	constraint pk_fplasts_objectid primary key(objectid)
 );
create index if not exists idx_fplasts_parentid on fplasts(((parentid>>12)&2251799813685247),generate);

--打开数据库
E:\vcpkg\installed\x64-windows\tools\sqlcipher\sqlcipher D:\\data.db
--输入或设置数据库密码
PRAGMA key=123;
--显示帮助
.help
--显示sql执行时间
.timer on
--退出数据库
.exit 0;

/****************************************************************************************
	注意：	因为数据库没有uint32_t和uint16_t，IP地址和端口是放大后存储的
				可靠准确获取值采用以下方法
					address & 0x00000000FFFFFFFF
					port & 0x0000FFFF
					select
						(address&4294967295) as ipv4,
						(port&65535) as port
****************************************************************************************/
drop table if exists fileend;
drop table if exists filebegin;
drop table if exists registers;
/****************************************************************************************
	用户注册信息
	select (objectid&536870911) as objectid
****************************************************************************************/
 create table if not exists registers(
	objectid integer not null,																	--唯一编号(高3位为0，只使用低29位，值范围1-536870911)
	mobile bigint not null,																		--手机号(用户id)
	password char(40) not null,																--用户密码sha1
	name varchar(64) not null,																--名称
	constraint pk_registers_objectid primary key(objectid)
 );
 /*
drop index if exists idx_registers_userid;
create index idx_registers_mobile on registers(mobile);
*/

drop index if exists uidx_registers_mobile;
create unique index uidx_registers_mobile on registers(mobile);

/****************************************************************************************	
	开始接收文件时信息
	objectid为变种雪花算法snowflake	，可使用至2106-02-07 14:28:15
		时间戳0-31bit(当前时间,单位为秒，使用最高位,值范围0-4294967295)
		注册号(值范围0-536870911)
		3bit顺序号（值范围0-3,同一台机器每秒最多允许发3个文件）
			32bit时间戳 +  29bit注册号+ 3bit顺序号	
****************************************************************************************/
 create table if not exists filebegin(
	objectid bigint not null,																	--唯一编号
	address bigint not null,																	--客户端IP(发送时ip)
	port integer not null,																		--客户端端口(发送时端口)
	filesize bigint not null,																		--文件长度
	generate bigint generated always as (((objectid>>31)&4294967295)) stored,			--客户端发送文件开始时间 virtual
	regid bigint generated always as (((objectid>>1)&536870911)) stored,						--注册号 virtual
	sn bigint generated always as ((objectid&1)) stored,													--顺序号virtual
	constraint pk_filebegin_objectid primary key(objectid)
);

/****************************************************************************************
	接收文件完成时信息
****************************************************************************************/
 create table if not exists fileend(
	objectid bigint not null,																	--唯一编号	
	address bigint not null,																	--客户端IP(完成时ip)
	port integer not null,																		--客户端端口(完成时端口)
	done bigint not null,																		--文件完成时间
	generate bigint generated always as (((objectid>>31)&4294967295)) stored,			--客户端发送文件开始时间
	regid bigint generated always as (((objectid>>1)&536870911)) stored,						--注册号
	sn bigint generated always as ((objectid&1)) stored,													--顺序号
	constraint pk_fileend_objectid primary key(objectid),
	constraint fk_fileend_objectid foreign key(objectid) references filebegin(objectid) on delete cascade
);

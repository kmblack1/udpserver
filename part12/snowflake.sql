--打开数据库
E:\vcpkg\installed\x64-windows\tools\sqlcipher\sqlcipher D:\\data.db
--输入或设置数据库密码
PRAGMA key=123;
--设置缓存大小8G=(1024*1024*8)kb
PRAGMA cache_size=8388608;
--清理碎片
vacuum;
--优化数据库
PRAGMA analysis_limit=400;
PRAGMA optimize;
--显示sql执行时间
.timer on
--退出数据库
.exit 0;

--获取当前时间
select datetime('now', 'localtime'),strftime('%s','now');


/*
	标准雪花算法snowflake值范围[41bit时间戳+10bit机器编号+12bit顺序号]
	最高位永远为0
*/
select (cast(1 as bigint)<<40)-1,
	(cast(1 as bigint)<<10)-1,
	(cast(1 as bigint)<<12)-1;

select 
	datetime(1099511627775/1000, 'unixepoch', 'localtime'),			--40bit支持的最大时间
	datetime(1577808000000/1000, 'unixepoch', 'localtime'),			--2020-01-01起始时间
	datetime((1577808000000+1099511627775)/1000, 'unixepoch', 'localtime');	--从2020-01-01开始4040bit支持的最大时间

--创建和解析标准雪花算法生成的id
select
	/*时间戳,从2020-1-1日开始*/
	millisec,
	/*标准雪花算法snowflake生成的id*/
	id,
	/*41bit时间戳*/
	((id>>22)&1099511627775),
	/*10bit机器编号*/
	(id>>12)&1023,
	/*12bit顺序号*/
	id&4095,
	/*当前时间*/
	datetime((1577808000000 + ((id>>22)&1099511627775))/1000, 'unixepoch', 'localtime')
from (	
	select 
		/*时间戳,从2020-1-1日开始*/
		millisec,
		/*创建id*/
		((millisec<<22)|(1022<<12)|4094) as id
	from (
		/*时间戳,从2020-1-1日开始*/
		select  (cast(strftime('%s','now') as bigint) * 1000) - 1577808000000 as millisec
	) as x
) as t;




/*
	自定义变种的雪花算法snowflake结构
		1bit最高位0 +	32bit时间戳 +  29bit注册号+ 1bit顺序号
		32bit当前时间,单位为秒（值范围0-4294967295）
		29bit注册号(值范围0-536870911)
		1bit顺序号（值范围0-1,同一台机器每秒最多允许同时发送2个文件）
		
*/
--自定义变种的雪花算法snowflake根据bit求值范围
select 
	(cast(1 as bigint)<<32)-1, 
	(cast(1 as bigint)<<29)-1,
	(cast(1 as bigint)<<1)-1;

--自定义变种的雪花算法snowflake支持的最大时间计算方法
select datetime(4294967295, 'unixepoch', 'localtime');

--自定义变种的雪花算法snowflake计算方法
/*
16进制口决(一个16进制数占4bit)
	0x0	全为0
	0x1	低一为1
	0x3	低二、低一全为1
	0x7	低三、低二、低一全为1
	0xf	全为1
例如 : 0x1FFFFFFF
*/
select
	seconds,
	id,
	/*id解码*/
	((id>>31)&4294967295),
	datetime(((id>>31)&4294967295), 'unixepoch', 'localtime'),
	(id>>1)&536870911,
	id&1
from (
	/*创建id*/
	select seconds,((seconds<<31)|(1<<1)|1) as id
	from (select cast(strftime('%s','now') as bigint) as seconds) as x
) as t;

--00000007FFFFFFFF
--34359738367
--删除表
delete from fileend;
delete from filebegin;
delete from registers;

select * from registers order by objectid desc limit 100;
select *,datetime(generate, 'unixepoch', 'localtime') from filebegin order by objectid desc limit 100;
select *,datetime(generate, 'unixepoch', 'localtime') from fileend order by objectid desc limit 100;

select * from sqlite_master where type='table';
--清理碎片
vacuum;


--执行计划
EXPLAIN QUERY PLAN
select ROWID,*,datetime((generate)/1000, 'unixepoch', 'localtime') from filebegin ORDER by generate,regid,sn limit 1000;

EXPLAIN QUERY PLAN
select 
	*,datetime(generate, 'unixepoch', 'localtime') 
from filebegin 
where objectid=123
order by  objectid desc limit 1;



--ip v4转换方法
select 
	ip,
	((ip>>24)&255),
	((ip>>16)&255),
	((ip>>8)&255),
	(ip&255)
from (select (
	(cast(192 as bigint)<<24)|
	(cast(168 as bigint)<<16)|KC_SQLITE3_CHECK_STEP_FAIL
	(cast(2 as bigint)<<8)|
	(cast(222 as bigint))) as ip
) as t;




select * from registers order by objectid desc limit 100;

select * from registers where mobile=13357184251;


explain query plan
select objectid+1,mobile,(select count(*) from registers as t2 where t2.mobile='qnbopwzotxqfyolj') from registers as t1 order by objectid  limit 1;


select * from registers where objecid=any(select objectid from registers order by objectid  limit 1);


select count(*) from registers as t2 where t2.mobile=20000016620;

select count(*) from filebegin;
select count(*) from fileend;


select * from registers where mobile>19999999999;
select mobile from registers where objectid >= p.objectid order by objectid limit 1

 select random() % (19999999999  - 10000000000 + 1) + 10000000000

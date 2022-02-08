#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <libintl.h>
#include <sqlcipher/sqlite3.h>
#include <pg_bswap.h>
#include <stringbuffer.h>
#include <KcUtility.h>
#include  <KcStdio.h>
#include <KcShared.h>
#include <KcSqlite3.h>
/*****************************************************************************
*	在栈上创建的字符串数组，无需释放
*****************************************************************************/
struct KC_STRING {
	const char* value;
	int32_t len;
};

int32_t getTimeZone() {
	time_t t;
	struct tm gmt, local, * pgtm = &gmt, * plocal = &local;

	time(&t);		//获取系统日期和时间
#ifdef _MSC_VER				//获取当地日期和时间
	localtime_s(plocal, &t);
#else
	localtime_r(&t, plocal);
#endif

#ifdef _MSC_VER				//把日期和时间转换为格林威治(GMT)时间
	gmtime_s(pgtm, &t);
#else
	gmtime_r(&t, pgtm);
#endif
	return (plocal->tm_hour - pgtm->tm_hour) * 3600;
}


#pragma region 初始化检查
int32_t  kcStr2int(const char* buf, uint32_t len) {
	uint32_t val = kcCrc32(buf, 0, len);
	return (int32_t)(val - INT32_MAX);
}

int32_t kcGenerateFiveFifty(sqlite3* db, StringBuffer str) {
	int32_t rc, timeZone;
	char* sqlite3Error = NULL;

	timeZone = getTimeZone();
	resetStringBuffer(str);
	appendStringBuffer(str, "create table if not exists fivefifty("\
		"objectid  bigint not null,"\
		"temperature integer not null,"\
		"humidity integer not null,"\
		"generate bigint default(strftime('%%s','now')) not null,"\
		"bin blob,"\
		"constraint pk_fivefifty_objectid primary key(objectid)"\
		");"\
		"create index if not exists idx_fivefifty_generate on fivefifty((generate-(generate%86400)-%d));", timeZone);

	rc = sqlite3_exec(db, str->data, NULL, 0, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, str);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	return KC_FAIL;
}

int32_t kcGenerateFilePointers(sqlite3* db, StringBuffer str) {
	int32_t rc;
	char* sqlite3Error = NULL;
	const char* sql = "create table if not exists filepointers("\
		"id char[1] not null,"\
		"address char[4] not null,"\
		"port char[4] not null,"\
		"hand unsigned big int not null,"\
		"generate bigint default(strftime('%%s','now')) not null,"\
		"constraint pk_filepointers_hand primary key(address,port,id)"\
		");"\
		"create index if not exists idx_filepointers_generate on filepointers(generate);";

	rc = sqlite3_exec(db, sql, NULL, 0, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, str);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	return KC_FAIL;
}

int32_t kcCheckSqlite3Object(sqlite3* db, StringBuffer str) {
	int32_t rc = 0;
	sqlite3_stmt* stmt = NULL;
	char* sqlite3Error = NULL;
	char** objNotExists = NULL;
	StringBuffer elems = NULL;
	uint8_t* ptmp, * pelemsBegin;
	uint32_t ival32, arrayCout = 0;
	uint32_t dimension, currentSize, stringLen, stringVal;
	uint8_t bval32[4];


	struct KC_STRING strObjects[] = {
		{"fivefifty",0},
		{"filepointers",0}
	};
	const size_t STROBJ_ITME_SIZE = sizeof(struct KC_STRING);
	const size_t STRING_COUNT = sizeof(strObjects) / STROBJ_ITME_SIZE;


	elems = createStringBuffer();
	ptmp = elems->data;
	for (size_t i = 0; i < STRING_COUNT; ++i)
		strObjects[i].len = (int32_t)strlen(strObjects[i].value);

	KC_SAFE_ENLARGE_STRINGBUFFER(elems, 32, str);// 确保内存足够
	//数组维数
	ival32 = 1;
	KC_VAL32_TO_BYTES(ptmp, ival32);
	pelemsBegin = ptmp;
	//第一维数组中元素的数量
	ival32 = 0;
	KC_VAL32_TO_BYTES(ptmp, ival32);
	//判断表是否存在
	resetStringBuffer(str);
	appendStringBufferStr(str, "select 1 from sqlite_master where type=? and name=?");
	rc = sqlite3_prepare_v2(db, str->data, (int32_t)str->len, &stmt, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_prepare_v2), str);
	//绑定sqlite_master的对象类型为表
	rc = sqlite3_bind_text(stmt, 1, "table", (int32_t)strlen("table"), NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_text), str);
	resetStringBuffer(str);
	for (size_t i = 0; i < STRING_COUNT; ++i) {
		//绑定表名
		rc = sqlite3_bind_text(stmt, 2, strObjects[i].value, strObjects[i].len, NULL);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_text), str);
		rc = sqlite3_step(stmt);
		switch (rc) {
		case SQLITE_DONE:do {		//未找到数据-表示表不存在
			KC_SAFE_ENLARGE_STRINGBUFFER(elems, (sizeof(uint32_t) + strObjects[i].len), str);		// 确保内存足够
			KC_VAL32_TO_BYTES(ptmp, strObjects[i].len);
			KC_VAL_ZERO_TO_BYTES(ptmp, strObjects[i].value, strObjects[i].len);
			++arrayCout;
		} while (0);
		break;
		case SQLITE_ROW:do {			//找到数据-表示表已经存在,什么也不做
		} while (0);
		break;
		default:do {							//发生错误
			KC_SQLITE3_CHECK_STEP_FAIL(rc, str);
		} while (0);
		break;
		}
		KC_SQLITE3_CHECK_RC(sqlite3_reset(stmt), KC_2STR(sqlite3_reset), str);//将预准备语句对象重置回其初始状态，以便重新执行。如使用sqlite3_bind_*()API将值绑定的SQL语句变量都将保留其值
	}
	//现在知道了动态数组的大小，然后我们重新设置动态数数组大小
	KC_VAL32_TO_BYTES(pelemsBegin, arrayCout);

	//创建不存在的表
	ptmp = elems->data;
	KC_VAL32_FROM_BYTES(ptmp, bval32, dimension);				//数组维数
	for (uint32_t i = 0; i < dimension; ++i) {
		KC_VAL32_FROM_BYTES(ptmp, bval32, currentSize);			//数组大小
		for (uint32_t j = 0; j < currentSize; ++j) {
			KC_VAL32_FROM_BYTES(ptmp, bval32, stringLen);			//字符串长度	
			stringVal = kcStr2int(ptmp, stringLen);
			ptmp += stringLen;		
			switch (stringVal) {
			case 802804853: //fivefifty
				KC_CHECK_RCV2(kcGenerateFiveFifty(db, str));
			case 1814018304: //filepointers
				KC_CHECK_RCV2(kcGenerateFilePointers(db, str));
			default:
				break;
			}
		}
	}


	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt);
	KC_SAFE_STRINGBUF_FREE(elems);
	return KC_OK;
KC_ERROR_CLEAR:
	//KC_SAFE_STRING_ARRAY_FREE(objNotExists, SQLITE3_OBJ_COUNT);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt);
	KC_SAFE_STRINGBUF_FREE(elems);
	return KC_FAIL;
}
#pragma endregion

int32_t main(int32_t argc, char* argv[]) {
	int32_t rc = 0;
	sqlite3* db = NULL;
	
	StringBuffer str = NULL;
	char* sqlite3Error = NULL;

	str = createStringBuffer();

	//insert into fivefifty(objectid,temperature,humidity) values(1,24400,5100);

	//rc = sqlite3_open_v2(NULL, &db, SQLITE_OPEN_MEMORY | SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	rc = sqlite3_open_v2("file:///D:/data.db", &db, SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_open_v2), str);
	//设置密码
	rc = sqlite3_key(db, "123", 3);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_key), str);
	//开启事务
	rc = sqlite3_exec(db, "begin;", NULL, 0, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, str);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);

	rc = kcCheckSqlite3Object(db, str);
	KC_CHECK_RCV2(rc);
	//提交事务
	rc = sqlite3_exec(db, "commit;", NULL, 0, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, str);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);

	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	KC_SQLITE3_SAFE_CLOSE(db);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	//回滚事务
	sqlite3_exec(db, "rollback;", NULL, 0, NULL);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	KC_SQLITE3_SAFE_CLOSE(db);
	fprintf(stderr, "%s\n", str->data);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

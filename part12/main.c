#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <libintl.h>
#include <pthread.h>
#include <openssl/sha.h>
#include <sqlcipher/sqlite3.h>
#include <gettimeofday.h>
#include <pg_bswap.h>
#include <instr_time.h>
#include <stringbuffer.h>
#include <KcUtility.h>
#include  <KcStdio.h>
#include <KcShared.h>
#include <KcSqlite3.h>


#define KC_INSERT_COUNT (10000000)
//#define KC_INSERT_COUNT (100)


int32_t kcInsert(sqlite3* db, StringBuffer str);



int32_t main(int32_t argc, char* argv[]) {
	int32_t rc = 0;
	sqlite3* db = NULL;
	StringBuffer str = NULL;
	char* sqlite3Error = NULL;


	str = createStringBuffer();


	//rc = sqlite3_open_v2(NULL, &db, SQLITE_OPEN_MEMORY | SQLITE_OPEN_PRIVATECACHE | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	rc = sqlite3_open_v2("file:///D:/data.db", &db, SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_open_v2), str);
	//设置密码
	rc = sqlite3_key(db, "123", 3);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_key), str);
	//开启事务
	rc = sqlite3_exec(db, "begin transaction;", NULL, 0, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, str);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	//插入测试数据
	KC_CHECK_RCV2(kcInsert(db, str));
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


int32_t kcInsert(sqlite3* db, StringBuffer str) {
	int32_t rc = 0;
	sqlite3_stmt* stmt1 = NULL, * stmt2 = NULL, * stmt3 = NULL;
	static const char* sql1 = "insert into registers(objectid,mobile,password,name) values(?,?,?,?)";
	static const char* sql2 = "insert into filebegin(objectid,address,port,filesize) values(?,?,?,?)";
	static const char* sql3 = "insert into fileend(objectid,address,port,done) values(?,?,?,?)";
	struct timeval tv;
	ptheradid_t pid;
	StringBuffer password = NULL, name = NULL;
	uint64_t sencnds, regid, id, ipv4, sn, filesize, done;
	int32_t k = 0, port;
	int64_t mobile;
	instr_time before, after;
	double elapsed_msec = 0;

	unsigned char sha1Result[20];

	INSTR_TIME_SET_CURRENT(before);
	password = createStringBuffer();
	name = createStringBuffer();

	rc = sqlite3_prepare_v2(db, sql1, (int32_t)strlen(sql1), &stmt1, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_prepare_v2), str);

	rc = sqlite3_prepare_v2(db, sql2, (int32_t)strlen(sql2), &stmt2, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_prepare_v2), str);

	rc = sqlite3_prepare_v2(db, sql3, (int32_t)strlen(sql3), &stmt3, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_prepare_v2), str);
	//获取当前线程编号
	pid = KC_GET_PTHREAD_ID();
	for (int32_t i = 1; i <= KC_INSERT_COUNT; ++i, ++k) {
		if (0 == (k % 150)) { //每150条数据后更新随机因子
			gettimeofday(&tv, NULL);
			KC_SET_RANDOM_FACTOR(pid, tv);
		}
		sencnds = tv.tv_sec;//生成标准雪花算法snowflake移位时，要保证数据类型一至
		regid = i;  //生成标准雪花算法snowflake移位时，要保证数据类型一至
#pragma region 注册信息	
		//随机生成11位手机号
		mobile = 10000000000;
		for (int32_t j = 0; j < 10; ++j)
			mobile += ((int64_t)(KC_RANDOM(0, 9) * pow(10, 10 - j)));
		resetStringBuffer(password);
		resetStringBuffer(name);
		KC_CHECK_RCV2(kcRandomString(8, 16, 7, password, str));
		KC_CHECK_RCV2(kcRandomString(3, 12, 8, name, str));

		//密码使用sha1存储
		SHA1(password->data, password->len, sha1Result);
		KC_CHECK_RCV2(kcHexStringFromBytes(sha1Result, sizeof(sha1Result), 0, password));

		rc = sqlite3_bind_int(stmt1, 1, i);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int), str);

		rc = sqlite3_bind_int64(stmt1, 2, mobile);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);

		rc = sqlite3_bind_text(stmt1, 3, password->data, (int32_t)password->len, NULL);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_text), str);

		rc = sqlite3_bind_text(stmt1, 4, name->data, (int32_t)name->len, NULL);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_text), str);
#pragma endregion

#pragma region 开始接收文件时信息
		sn = KC_RANDOM(UINT64CONST(0), UINT64CONST(1));
		ipv4 = KC_RANDOM(1LL, 4294967295LL);
		port = KC_RANDOM(1, 65535);
		filesize = KC_RANDOM(1024LL, 9223372036854775807LL);
		id = ((sencnds << 31) | (regid << 1) | sn);
		done = KC_RANDOM(sencnds, sencnds + 3600LL);

		rc = sqlite3_bind_int64(stmt2, 1, id);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);

		rc = sqlite3_bind_int64(stmt2, 2, ipv4);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);

		rc = sqlite3_bind_int(stmt2, 3, port);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int), str);

		rc = sqlite3_bind_int64(stmt2, 4, filesize);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);
#pragma endregion

#pragma region 接收文件完成时信息
		rc = sqlite3_bind_int64(stmt3, 1, id);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);

		rc = sqlite3_bind_int64(stmt3, 2, ipv4);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);

		rc = sqlite3_bind_int(stmt3, 3, port);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int), str);

		rc = sqlite3_bind_int64(stmt3, 4, done);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), str);
#pragma endregion

#pragma region 执行sql
		rc = sqlite3_step(stmt1);
		KC_SQLITE3_CHECK_STEP_FAIL(rc, str);

		rc = sqlite3_step(stmt2);
		KC_SQLITE3_CHECK_STEP_FAIL(rc, str);

		rc = sqlite3_step(stmt3);
		KC_SQLITE3_CHECK_STEP_FAIL(rc, str);

		KC_SQLITE3_CHECK_RC(sqlite3_reset(stmt1), KC_2STR(sqlite3_reset), str);//将预准备语句对象重置回其初始状态，以便重新执行。如使用sqlite3_bind_*()API将值绑定的SQL语句变量都将保留其值
			//KC_SQLITE3_CHECK_RC(sqlite3_clear_bindings(stmt), KC_2STR(sqlite3_clear_bindings), str);		
		KC_SQLITE3_CHECK_RC(sqlite3_reset(stmt2), KC_2STR(sqlite3_reset), str);
		KC_SQLITE3_CHECK_RC(sqlite3_reset(stmt3), KC_2STR(sqlite3_reset), str);

#pragma endregion
	}

	KC_SQLITE3_SAFE_STMT_DESTROY(stmt1);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt2);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt3);

	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "insert registers %s\n", str->data);


	KC_SAFE_STRINGBUF_FREE(password);
	KC_SAFE_STRINGBUF_FREE(name);

	return KC_OK;
KC_ERROR_CLEAR:
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt1);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt2);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt3);

	KC_SAFE_STRINGBUF_FREE(password);
	KC_SAFE_STRINGBUF_FREE(name);

	return KC_FAIL;
}

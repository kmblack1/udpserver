#include <stdio.h>
#include <pg_bswap.h>
#ifdef _MSC_VER
# include<windows.h>
#	include <tre/regex.h>
#include <gettimeofday.h>
#else
#	include <regex.h>
#endif
#include <sqlcipher/sqlite3.h>
#include <KcStdio.h>
#include <KcShared.h>
#include <KcUtility.h>
#include <KcIpV4.h>
#include <KcSqlite3.h>
#include <KcRegex.h>
#include "KcProcessData.h"



#pragma region 用户验证


#define KC_LARGEFILE_IS_LOCK									(1<<0)					//读写锁是否加锁
#define KC_LARGEFILE_TRANSACTION_IS_BEGIN		(1<<1)					//事务是否创建

/*****************************************************************************
*   验证输入参数是否在有效范围,如果是直接跳转至异常
*****************************************************************************/
#define KC_CHECK_PARAM_LEN(para,len,str) do{\
	if ((para)<=0 || (para)>(len) ){\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("invalid argument[val<=0 or val>%d].(%s:%d)"),(len),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

int32_t kcReplyId(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, const char* identifier, uint8_t cmd, uint64_t snowflakeId, StringBuffer error);
int32_t kcReplyStatus(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, const char* identifier, uint8_t cmd, uint16_t statusCode, StringBuffer error);

int32_t kcValidateUserStep1(const char* ptmp, const char* identifier, size_t identifierLen, pthread_rwlock_t* rwlock, StringBuffer error);

int32_t kcValidateUserStep2(int64_t mobile, uint8_t passwordLen, const char* password, pthread_rwlock_t* rwlock, StringBuffer error);

#pragma endregion

int32_t kcFiveFiftyPluginProcessData(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen,/* PGconn * conn,*/ pthread_rwlock_t * rwlock, StringBuffer error) {
	char* identifier = NULL;
	uint8_t cmd, bval16[2];
	uint16_t dataCrc16 = 0, clacCrc16;
	const char* ptmp = item->recv->data;

	KC_VALIDATE_PARA_PTR(config, error);
	KC_VALIDATE_PARA_PTR(item, error);
	//KC_VALIDATE_PARA_PTR(conn, error);
	KC_VALIDATE_PARA_PTR(rwlock, error);

	//计算校验和
	ptmp += (item->recv->len - 2);
	KC_VAL16_FROM_BYTES(ptmp, bval16, dataCrc16);
	clacCrc16 = kcCrc16(item->recv->data, 0, item->recv->len, 0xffff);
	clacCrc16 = pg_bswap16(clacCrc16);
	if (dataCrc16 != clacCrc16)
		KC_GOTOERR(error, "%s", gettext("CRC validation failed."));
	ptmp = item->recv->data;


	//开始解析数据
	identifier = (char*)malloc(identifierLen);
	KC_CHECK_MALLOC(identifier, error);
	KC_VAL_NOZERO_FROM_BYTES(ptmp, identifier, identifierLen);
	//1字节命令字
	KC_VAL8_FROM_BYTES(ptmp, &cmd);
	switch (cmd) {
	case 0x01:do {  //验证用户信息
		KC_CHECK_RCV2(kcValidateUserStep1(ptmp, identifier, identifierLen, rwlock, error));
	} while (0);
	break;
	case 0x02:do {	//接收文件

	} while (0);
	break;
	default:
		break;
	}
	KC_SAFE_FREE(identifier);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(identifier);
	return KC_FAIL;
}

#pragma region 用户验证

/// <summary>
/// 使用正则表达式获取手机号
/// </summary>
/// <param name="values">要匹配的字符串</param>
/// <param name="str">[in,out]返回值为REG_OK返回手机号，为REG_NOMATCH为未匹配，其它为错误信息</param>
/// <returns>返回REG_OK表示成功，REG_NOMATCH表示字符串不匹配，其它为错误</returns>
reg_errcode_t kcCheckGetMobile(const char* values, StringBuffer str) {
	reg_errcode_t rc;
	int32_t  len;
	size_t nsub;
	regex_t* reg = NULL;
	regmatch_t* matchs = NULL;
	/* 判断是否为手机号,接收以下7种格式
	* +861xxxxxxxxxx
	* 861xxxxxxxxxx
	* (+86)1xxxxxxxxxx
	* (86)1xxxxxxxxxx
	* （+86）1xxxxxxxxxx，这里的括号是中文的括号
	* （86）1xxxxxxxxxx，这里的括号是中文的括号
	* 1xxxxxxxxxx
	*/
	static const char* PATTERN = "^((\\+86)|(86)|(\\(\\+86\\))|(\\(86\\))|(（\\+86）)|(（86）))?(1[0-9]{10})$";



	enlargeStringBuffer(str, 256);
	reg = (regex_t*)malloc(sizeof(regex_t));
	KC_CHECK_MALLOC(reg, str);
	rc = regcomp(reg, PATTERN, REG_EXTENDED);
	if (rc) {
		resetStringBuffer(str);
		appendStringBufferStr(str, "error:");
		regerror(rc, reg, str->data, str->len);
		appendStringBufferChar(str, 0x10);
		goto KC_ERROR_CLEAR;
	}
	nsub = reg->re_nsub + 1; //获取正则中项目的数量
	matchs = (regmatch_t*)malloc(nsub * sizeof(regmatch_t));
	KC_CHECK_MALLOC(matchs, str);
	rc = regexec(reg, values, nsub, matchs, 0);
	switch (rc) {
	case 0:do {
		if (-1 != matchs[reg->re_nsub].rm_so) {
			len = matchs[reg->re_nsub].rm_eo - matchs[reg->re_nsub].rm_so;  // 匹配长度	
			resetStringBuffer(str);
			appendStringBufferBinary(str, (values + matchs[reg->re_nsub].rm_so), len);
		}
	} while (0);
	break;
	case REG_NOMATCH:do {
		resetStringBuffer(str);
		appendStringBufferStr(str, "no match");
	} while (0);
	break;
	default:do {
		resetStringBuffer(str);
		appendStringBufferStr(str, "error:");
		regerror(rc, reg, str->data, str->len);
	} while (0);
	break;
	}
	KC_SAFE_FREE(matchs);
	KC_SAFE_REGEX_FREE(reg);
	return rc;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(matchs);
	KC_SAFE_REGEX_FREE(reg);
	return rc;
}

int32_t kcValidateUserStep1(const char* ptmp, const char* identifier, size_t identifierLen, pthread_rwlock_t* rwlock, StringBuffer error) {
	//1字节用户id和密码长度
	uint8_t useridLen, passwordLen, bval64[8];
	char* userid = NULL, * password = NULL;
	//8字节文件长度
	int64_t mobile, fileLen;

	//用户id
	KC_VAL8_FROM_BYTES(ptmp, &useridLen);
	KC_PARA_LESS_EQUAL_ZERO(useridLen, error);
	userid = (char*)malloc(useridLen + sizeof(char));
	KC_CHECK_MALLOC(userid, error);
	KC_VAL_NOZERO_FROM_BYTES(ptmp, userid, useridLen);
	userid[useridLen] = '\0';
	/*
	*	在本例中如果不是手机号则抛出异常
	*	在实际项目中如不是手机号继续判断是否为email或用户id等等
	*/
	if (REG_OK != kcCheckGetMobile(userid, error))
		goto KC_ERROR_CLEAR;
	//将字符串转换为数字	
	mobile = atoll(error->data);
	//#if defined(_MSC_VER)	
	//	sscanf_s(error->data, "%I64d", &mobile);
	//#else
	//#	ifdef __x86_64__
	//		sscanf(error->data, "%ld", &mobile);
	//#	elif __i386__
	//		sscanf(error->data, "%lld", &mobile);
	//#	endif
	//#endif
		//密码
	KC_VAL8_FROM_BYTES(ptmp, &passwordLen);
	KC_CHECK_PARAM_LEN(passwordLen, 40, error);
	password = (char*)malloc(passwordLen + sizeof(char));
	KC_CHECK_MALLOC(password, error);
	KC_VAL_NOZERO_FROM_BYTES(ptmp, password, passwordLen);
	password[passwordLen] = '\0';
	//8字节文件长度
	KC_VAL64_FROM_BYTES(ptmp, bval64, fileLen);



	KC_SAFE_FREE(userid);
	KC_SAFE_FREE(password);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(userid);
	KC_SAFE_FREE(password);
	return KC_FAIL;
}

int32_t kcValidateUserStep2(int64_t mobile, uint8_t passwordLen, const char* password, pthread_rwlock_t* rwlock, StringBuffer error) {
	sqlite3* db = NULL;
	char* sqlite3Error = NULL;
	int32_t rc = 0, flag = 0;

	pthread_rwlock_wrlock(rwlock);
	flag += KC_LARGEFILE_IS_LOCK;

	rc = sqlite3_open_v2("file:///D:/data.db", &db, SQLITE_OPEN_SHAREDCACHE | SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_open_v2), error);
	//设置密码
	rc = sqlite3_key(db, "123", 3);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_key), error);
	//设置缓存8G=(1024*1024*8)kb
	resetStringBuffer(error);
	appendStringBuffer(error, "PRAGMA cache_size=%u;", 8388608U);
	rc = sqlite3_exec(db, error->data, NULL, NULL, &sqlite3Error);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_exec), error);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	//开启事务
	rc = sqlite3_exec(db, "begin transaction;", NULL, NULL, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, error);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	flag += KC_LARGEFILE_TRANSACTION_IS_BEGIN;

	//提交事务
	rc = sqlite3_exec(db, "commit;", NULL, NULL, &sqlite3Error);
	KC_SQLITE3_CHECK_EXEC(rc, sqlite3Error, error);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);

	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	KC_SQLITE3_SAFE_CLOSE(db);
	pthread_rwlock_unlock(rwlock);
	return KC_OK;
KC_ERROR_CLEAR:
	if (KC_LARGEFILE_TRANSACTION_IS_BEGIN == (flag & KC_LARGEFILE_TRANSACTION_IS_BEGIN))
		sqlite3_exec(db, "rollback;", NULL, 0, NULL);
	KC_SQLITE3_SAFE_FREE(sqlite3Error);
	KC_SQLITE3_SAFE_CLOSE(db);
	if (KC_LARGEFILE_IS_LOCK == (flag & KC_LARGEFILE_IS_LOCK))
		pthread_rwlock_unlock(rwlock);
	return KC_FAIL;
}

int32_t kcValidateUserStep3(sqlite3* db, struct KC_BACKEND_ITEM* item, int64_t mobile, uint8_t passwordLen, const char* password, int64_t filesize, StringBuffer error) {
	int32_t rc = 0, pos=0, registerid;
	int64_t fileid;
	struct timeval tv;
	sqlite3_stmt* stmt1 = NULL, * stmt2 = NULL;
	static const char* sql1 = "select objectid from registers where mobile=? and password=?;";
	static const char* sql2 = "insert into filebegin(objectid,address,port,filesize) values(?,?,?,?);";

	rc = sqlite3_prepare_v2(db, sql1, (int32_t)strlen(sql1), &stmt1, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_prepare_v2), error);

	rc = sqlite3_prepare_v2(db, sql2, (int32_t)strlen(sql2), &stmt2, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_prepare_v2), error);
	//绑定参数
	rc = sqlite3_bind_int64(stmt1, pos++, mobile);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), error);
	rc = sqlite3_bind_text(stmt1, pos++, password, (int32_t)passwordLen, NULL);
	KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), error);
	if (SQLITE_ROW == (rc = sqlite3_step(stmt1))) {
		pos = 0;
		registerid = sqlite3_column_int(stmt1, pos++);
		pos = 0;
		gettimeofday(&tv, NULL);
		fileid = ((tv.tv_sec << 31) | (registerid << 1) | 1);
		rc = sqlite3_bind_int64(stmt2, pos++, fileid);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), error);
		rc = sqlite3_bind_int64(stmt2, pos++, (int64_t)(pg_bswap32(item->sender.sin_addr.s_addr)));
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), error);
		rc = sqlite3_bind_int(stmt2, pos++, (int32_t)(pg_bswap16(item->sender.sin_port)));
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int32), error);
		rc = sqlite3_bind_int64(stmt2, pos++, filesize);
		KC_SQLITE3_CHECK_RC(rc, KC_2STR(sqlite3_bind_int64), error);
	} else {

	}
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt2);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt1);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt2);
	KC_SQLITE3_SAFE_STMT_DESTROY(stmt1);
	return KC_FAIL;
}

#pragma endregion


#pragma region 发送应答包

#define KC_REPLY_ID_SIZE (13)
int32_t kcReplyId(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, const char* identifier, uint8_t cmd, uint64_t snowflakeId, StringBuffer error) {
	uint8_t* pbegin = item->sendData, * pIpTmp = pbegin;
	uint8_t* pIpV4Sum = pbegin + 10;
	uint8_t* pUdpTmp = pbegin + KC_IPV4_HEADER_SIZE;
	uint8_t* pUDPSum = pUdpTmp + (KC_UDP_HEADER_SIZE - 2);
	uint8_t* preply = pbegin + KC_IPV4_UDP_HEADER_SIZE, * ptmp = preply;

	uint16_t crc;
	uint32_t srcAddr, destAddr;
	uint16_t srcPort, destPort;
	uint16_t sum = 0;

	uint8_t reply = 1;

	srcAddr = pg_bswap32(config->addres.sin_addr.s_addr);
	destAddr = pg_bswap32(item->sender.sin_addr.s_addr);
	srcPort = pg_bswap16(config->addres.sin_port);
	destPort = pg_bswap16(item->sender.sin_port);
	/*****************************************************************************
	*	set IPV4 header
	*****************************************************************************/
	// IPv4 version or length
	KC_VAL8_TO_BYTES(pIpTmp, ((IP_VERSION_V4 << 4) | 5));
	// IP type of service
	KC_VAL8_TO_BYTES(pIpTmp, 0);
	// Total length
	KC_VAL16_TO_BYTES(pIpTmp, KC_IPV4_UDP_HEADER_SIZE + KC_REPLY_ID_SIZE);
	// Unique identifier
	KC_VAL16_TO_BYTES(pIpTmp, 0);
	// Fragment offset field
	KC_VAL16_TO_BYTES(pIpTmp, 0);
	// Time to live
	KC_VAL8_TO_BYTES(pIpTmp, 128);
	// Protocol(TCP,UDP etc)
	KC_VAL8_TO_BYTES(pIpTmp, IPPROTO_UDP);
	// IP checksum
	KC_VAL16_TO_BYTES(pIpTmp, 0);
	// Source address
	KC_VAL32_TO_BYTES(pIpTmp, srcAddr);
	// Dest. address
	KC_VAL32_TO_BYTES(pIpTmp, destAddr);

	//计算IP头校验码并重新设置IP头部校验码
	//sum = kcCheckSum((const uint16_t *)pbegin, KC_IPV4_HEADER_SIZE);
	//KC_VAL16_TO_BYTES(pIpV4Sum, sum);
	KC_SET_BUFFER_EXCEED(pbegin, pIpTmp, KC_IPV4_HEADER_SIZE, error);
	/*****************************************************************************
	*	set udp header
	*****************************************************************************/
	// Source port no.
	KC_VAL16_TO_BYTES(pUdpTmp, srcPort);
	// Dest. port no.
	KC_VAL16_TO_BYTES(pUdpTmp, destPort);
	// Udp packet length
	KC_VAL16_TO_BYTES(pUdpTmp, KC_UDP_HEADER_SIZE + KC_REPLY_ID_SIZE);
	// Udp checksum (optional),设置完成数据后再计算校验码
	KC_VAL16_TO_BYTES(pUdpTmp, 0);
	KC_SET_BUFFER_EXCEED((pbegin + KC_IPV4_HEADER_SIZE), pUdpTmp, KC_UDP_HEADER_SIZE, error);
	/*****************************************************************************
	*	set message
	*****************************************************************************/
	//2字节数据标识符,固定值“0x7567”
	KC_VAL_ZERO_TO_BYTES(ptmp, identifier, identifierLen);
	//1字节命令字
	KC_VAL8_TO_BYTES(ptmp, cmd);
	//8字节ID
	KC_VAL64_TO_BYTES(ptmp, snowflakeId);
	//2字节crc
	crc = kcCrc16(preply, 0, KC_REPLY_ID_SIZE - 2, 0xffff);
	KC_VAL16_TO_BYTES(ptmp, crc);
	KC_SET_BUFFER_EXCEED(preply, ptmp, KC_REPLY_ID_SIZE, error);
	//计算UDP校验码
	KC_CHECK_RCV2(kcCheckUdpSum(srcAddr, srcPort, destAddr, destPort, preply, KC_REPLY_ID_SIZE, error, &sum));
	KC_VAL16_TO_BYTES(pUDPSum, sum);

	sendto(item->sockRaw, item->sendData, KC_IPV4_UDP_HEADER_SIZE + KC_REPLY_ID_SIZE, 0, (const struct sockaddr*)(&item->sender), sizeof(struct sockaddr_in));

	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

#define KC_REPLY_STATUS_SIZE (7)
int32_t kcReplyStatus(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, const char* identifier, uint8_t cmd, uint16_t statusCode, StringBuffer error) {
	uint8_t* pbegin = item->sendData, * pIpTmp = pbegin;
	uint8_t* pIpV4Sum = pbegin + 10;
	uint8_t* pUdpTmp = pbegin + KC_IPV4_HEADER_SIZE;
	uint8_t* pUDPSum = pUdpTmp + (KC_UDP_HEADER_SIZE - 2);
	uint8_t* preply = pbegin + KC_IPV4_UDP_HEADER_SIZE, * ptmp = preply;

	uint16_t crc;
	uint32_t srcAddr, destAddr;
	uint16_t srcPort, destPort;
	uint16_t sum = 0;

	uint8_t reply = 1;

	srcAddr = pg_bswap32(config->addres.sin_addr.s_addr);
	destAddr = pg_bswap32(item->sender.sin_addr.s_addr);
	srcPort = pg_bswap16(config->addres.sin_port);
	destPort = pg_bswap16(item->sender.sin_port);
	/*****************************************************************************
	*	set IPV4 header
	*****************************************************************************/
	// IPv4 version or length
	KC_VAL8_TO_BYTES(pIpTmp, ((IP_VERSION_V4 << 4) | 5));
	// IP type of service
	KC_VAL8_TO_BYTES(pIpTmp, 0);
	// Total length
	KC_VAL16_TO_BYTES(pIpTmp, KC_IPV4_UDP_HEADER_SIZE + KC_REPLY_STATUS_SIZE);
	// Unique identifier
	KC_VAL16_TO_BYTES(pIpTmp, 0);
	// Fragment offset field
	KC_VAL16_TO_BYTES(pIpTmp, 0);
	// Time to live
	KC_VAL8_TO_BYTES(pIpTmp, 128);
	// Protocol(TCP,UDP etc)
	KC_VAL8_TO_BYTES(pIpTmp, IPPROTO_UDP);
	// IP checksum
	KC_VAL16_TO_BYTES(pIpTmp, 0);
	// Source address
	KC_VAL32_TO_BYTES(pIpTmp, srcAddr);
	// Dest. address
	KC_VAL32_TO_BYTES(pIpTmp, destAddr);

	//计算IP头校验码并重新设置IP头部校验码
	//sum = kcCheckSum((const uint16_t *)pbegin, KC_IPV4_HEADER_SIZE);
	//KC_VAL16_TO_BYTES(pIpV4Sum, sum);
	KC_SET_BUFFER_EXCEED(pbegin, pIpTmp, KC_IPV4_HEADER_SIZE, error);
	/*****************************************************************************
	*	set udp header
	*****************************************************************************/
	// Source port no.
	KC_VAL16_TO_BYTES(pUdpTmp, srcPort);
	// Dest. port no.
	KC_VAL16_TO_BYTES(pUdpTmp, destPort);
	// Udp packet length
	KC_VAL16_TO_BYTES(pUdpTmp, KC_UDP_HEADER_SIZE + KC_REPLY_STATUS_SIZE);
	// Udp checksum (optional),设置完成数据后再计算校验码
	KC_VAL16_TO_BYTES(pUdpTmp, 0);
	KC_SET_BUFFER_EXCEED((pbegin + KC_IPV4_HEADER_SIZE), pUdpTmp, KC_UDP_HEADER_SIZE, error);
	/*****************************************************************************
	*	set message
	*****************************************************************************/
	//2字节数据标识符,固定值“0x7567”
	KC_VAL_ZERO_TO_BYTES(ptmp, identifier, identifierLen);
	//1字节命令字
	KC_VAL8_TO_BYTES(ptmp, cmd);
	//2字节返回代码
	KC_VAL16_TO_BYTES(ptmp, statusCode);
	//2字节crc
	crc = kcCrc16(preply, 0, KC_REPLY_STATUS_SIZE - 2, 0xffff);
	KC_VAL16_TO_BYTES(ptmp, crc);
	KC_SET_BUFFER_EXCEED(preply, ptmp, KC_REPLY_STATUS_SIZE, error);
	//计算UDP校验码
	KC_CHECK_RCV2(kcCheckUdpSum(srcAddr, srcPort, destAddr, destPort, preply, KC_REPLY_STATUS_SIZE, error, &sum));
	KC_VAL16_TO_BYTES(pUDPSum, sum);

	sendto(item->sockRaw, item->sendData, KC_IPV4_UDP_HEADER_SIZE + KC_REPLY_STATUS_SIZE, 0, (const struct sockaddr*)(&item->sender), sizeof(struct sockaddr_in));

	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

#pragma endregion

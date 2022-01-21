#include <pg_bswap.h>
#ifdef _WIN32
# include<windows.h>
#endif

#include <KcStdio.h>
#include <KcShared.h>
#include <KcUtility.h>
//#include <KcLibpq.h>
#include <KcIpV4.h>
#include "KcProcessData.h"


int32_t kcFiveFiftySaveAsPostgreSQL(PGconn* conn, const StringBuffer revc, const struct KC_FIVEFIFTY* data, StringBuffer error);
int32_t kcFiveFiftyReply(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, const struct KC_FIVEFIFTY* data, uint8_t errCode, StringBuffer error);

void kcSafeFreeFifty(struct KC_FIVEFIFTY* ptr) {
	if (NULL == ptr)
		return;
	if (NULL != ptr->identifier)
		free(ptr->identifier);
	free(ptr);
}

int32_t kcFiveFiftyPluginProcessData(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, PGconn* conn, pthread_rwlock_t* rwlock, StringBuffer error) {
	struct KC_FIVEFIFTY* data = NULL;
	const char* ptmp = item->recv->data;
	uint8_t bval16[2], bval32[4];
	uint16_t dataCrc16 = 0, clacCrc16;
	uint32_t ival32;

	KC_VALIDATE_PARA_PTR(config, error);
	KC_VALIDATE_PARA_PTR(item, error);
	KC_VALIDATE_PARA_PTR(conn, error);
	KC_VALIDATE_PARA_PTR(rwlock, error);

	//计算校验和
	ptmp += (item->recv->len - 2);
	KC_VAL16_FROM_BYTES(ptmp, bval16, dataCrc16);
	clacCrc16 = kcCrc16(item->recv->data + 2, 0, item->recv->len - 4, 0xffff); //跳过我们自己手动加的标识符0x0551
	clacCrc16 = pg_bswap16(clacCrc16);
	if (dataCrc16 != clacCrc16)
		KC_GOTOERR(error, "%s", gettext("CRC validation failed."));
	ptmp = item->recv->data;

	//开始解析数据
	data = (struct KC_FIVEFIFTY*)malloc(sizeof(struct KC_FIVEFIFTY));
	KC_CHECK_MALLOC(data, error);
	//数据标识符
	data->identifier = (char*)malloc(identifierLen);
	KC_CHECK_MALLOC(data->identifier, error);
	KC_VAL_NOZERO_FROM_BYTES(ptmp, data->identifier, identifierLen);
	//1字节设备地址
	KC_VAL8_FROM_BYTES(ptmp, &data->addres);
	//1字节命令字
	KC_VAL8_FROM_BYTES(ptmp, &data->cmd);
	//1字节数据字节数
	KC_VAL8_FROM_BYTES(ptmp, &data->len);
	//4字节温度
	KC_VAL32_FROM_BYTES(ptmp, bval32, ival32);
	data->temperature = KC_UINT32_TO_FLOAT(ival32);
	//4字节湿度
	KC_VAL32_FROM_BYTES(ptmp, bval32, ival32);
	data->humidity = KC_UINT32_TO_FLOAT(ival32);
	//保存至数据库
	KC_CHECK_RCV2(kcFiveFiftySaveAsPostgreSQL(conn, item->recv, data, error));
	//伪造数据包并回发数据
	kcFiveFiftyReply(config, item, identifierLen, data, 0, error);
	KC_SAFE_FREE_FIVE_FIFTY(data);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE_FIVE_FIFTY(data);
	return KC_FAIL;
}

/*

create table fivefifty(
	objectid  bigserial not null,														--唯一编号
	temperature integer not null,													--温度,值为小数,为加快计算速度,存储时*100后转换为整数存储,计算完成后/100保留小数2位
	humidity integer not null,														--湿度,值为小数,为加快计算速度,存储时*100后转换为整数存储,计算完成后/100保留小数2位
	generate timestamptz default(now()) not null,						--上报时间
	bin bytea,																					--原始数据包
	constraint pk_fivefifty_objectid primary key (objectid)  with (fillfactor=100) using index tablespace idxdrainage
) with (fillfactor=100,
		autovacuum_enabled=true,toast.autovacuum_enabled=true,
		autovacuum_vacuum_threshold=4000,autovacuum_analyze_threshold=8000,
		toast.autovacuum_vacuum_threshold=8000);

select temperature,humidity from fivefifty order by objectid limit 100;

delete from fivefifty;

vacuum analyze fivefifty;
*/
int32_t kcFiveFiftySaveAsPostgreSQL(PGconn* conn, const StringBuffer revc, const struct KC_FIVEFIFTY* data, StringBuffer error) {
//	struct KCPQ_EXECPARAMS* params = NULL;
//	PGresult* result = NULL;
//	int32_t tran_flag;
//
//	ExecStatusType status;
//	static const char* const  CMD_FREE_FIVE_FIFTY_INS = "FiveFiftySaveAsPostgreSQL";
//
//	//创建参数
//	KC_CHECK_RCV2(kcPqParamsCreate(3, error, &params));
//	KC_CHECK_RCV2(kcPqParamsAddInt32(params, 0, (int32_t)(data->temperature * 100), error));
//	KC_CHECK_RCV2(kcPqParamsAddInt32(params, 1, (int32_t)(data->humidity * 100), error));
//	KC_CHECK_RCV2(kcPqParamsAddBytea(params, 2, revc->data, (int32_t)revc->len, error));
//
//
//	//开始事务
//	KC_CHECK_RCV2(kcPqBegin(conn, READ_COMMITTED_READ_WRITE, &tran_flag, error));
//	result = PQdescribePrepared(conn, CMD_FREE_FIVE_FIFTY_INS);
//	status = PQresultStatus(result);
//	KCPQ_CLEAR(result);
//	//预备语句不存在创建预备语句	
//	if (PGRES_FATAL_ERROR == status) {
//		//发生错误后要事务必须要先回滚然后重新创建
//		kcPqRollback(conn, &tran_flag, error);
//		KC_CHECK_RCV2(kcPqBegin(conn, READ_COMMITTED_READ_WRITE, &tran_flag, error));
//		const char* const sql = "insert into fivefifty(temperature,humidity,bin) values($1,$2,$3);";
//		result = PQprepare(conn, CMD_FREE_FIVE_FIFTY_INS, sql, params->count, params->types);
//		KCPQ_CHECK_EXEC(conn, PGRES_COMMAND_OK, result, error);
//	}
//	//执行sql
//	result = PQexecPrepared(conn, CMD_FREE_FIVE_FIFTY_INS, params->count, (const char**)params->values, params->lengths, params->formats, KCPQ_FORMAT_BIN);
//	KCPQ_CHECK_EXEC(conn, PGRES_COMMAND_OK, result, error);
//
//	KCPQ_CLEAR(result);
//	KCPQ_PARAMS_FREE(params);
//	KC_CHECK_RCV2(kcPqCommit(conn, &tran_flag, error));	
//	return KC_OK;
//KC_ERROR_CLEAR:
//	KCPQ_CLEAR(result);
//	KCPQ_PARAMS_FREE(params);
//	kcPqRollback(conn, &tran_flag, error);
//	return KC_FAIL;

	//模拟保存数据
	int64_t microsec = 3000;
#ifdef _WIN32
	SleepEx((microsec < 500 ? 1 : (microsec + 500) / 1000), 0);
#else
	struct timeval delay;
	delay.tv_sec = microsec / 1000000L;
	delay.tv_usec = microsec % 1000000L;
	(void)select(0, NULL, NULL, NULL, &delay);
#endif // _WIN32

	
}


#define KC_REPLY_OK_SIZE (8)
int32_t kcFiveFiftyReply(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, const struct KC_FIVEFIFTY* data, uint8_t errCode, StringBuffer error) {
	uint8_t* pbegin = item->sendData, * pIpTmp = pbegin;
	uint8_t* pIpV4Sum = pbegin + 10;
	uint8_t* pUdpTmp = pbegin + KC_IPV4_HEADER_SIZE;
	uint8_t* pUDPSum = pUdpTmp + (KC_UDP_HEADER_SIZE - 2);
	uint8_t* preply = pbegin + KC_IPV4_UDP_HEADER_SIZE, * ptmp = preply;

	uint16_t crc;
	uint32_t srcAddr, destAddr;
	uint16_t srcPort, destPort;
	uint16_t sum=0;

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
	KC_VAL16_TO_BYTES(pIpTmp, KC_IPV4_UDP_HEADER_SIZE + KC_REPLY_OK_SIZE);
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
	KC_VAL16_TO_BYTES(pUdpTmp, KC_UDP_HEADER_SIZE + KC_REPLY_OK_SIZE);
	// Udp checksum (optional),设置完成数据后再计算校验码
	KC_VAL16_TO_BYTES(pUdpTmp, 0);
	KC_SET_BUFFER_EXCEED((pbegin + KC_IPV4_HEADER_SIZE), pUdpTmp, KC_UDP_HEADER_SIZE, error);
	/*****************************************************************************
	*	set message
	*****************************************************************************/
	//2字节数据标识符,固定值“0x0551”
	KC_VAL_ZERO_TO_BYTES(ptmp, data->identifier, identifierLen);
	//1字节设备地址
	KC_VAL8_TO_BYTES(ptmp, data->addres);
	//1字节命令字
	KC_VAL8_TO_BYTES(ptmp, data->cmd);
	//1字节数据字节数
	KC_VAL8_TO_BYTES(ptmp, reply);
	//1字节错误代码
	KC_VAL8_TO_BYTES(ptmp, errCode);
	//2字节crc
	crc = kcCrc16(preply, 0, KC_REPLY_OK_SIZE - 2, 0xffff);
	KC_VAL16_TO_BYTES(ptmp, crc);
	KC_SET_BUFFER_EXCEED(preply, ptmp, KC_REPLY_OK_SIZE, error);
	//计算UDP校验码
	KC_CHECK_RCV2(kcCheckUdpSum(srcAddr, srcPort, destAddr, destPort, preply, KC_REPLY_OK_SIZE, error, &sum));
	KC_VAL16_TO_BYTES(pUDPSum, sum);

	sendto(item->sockRaw, item->sendData, KC_IPV4_UDP_HEADER_SIZE + KC_REPLY_OK_SIZE, 0, (const struct sockaddr*)(&item->sender), sizeof(struct sockaddr_in));

	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	导出函数定义
*****************************************************************************/
#ifndef KC_78A1AE04_5624_6B4D_99E9_C8AE78F7E3FA
#define KC_78A1AE04_5624_6B4D_99E9_C8AE78F7E3FA

#include <jansson.h>
#include "KcUtility.h"

#define KC_SAFE_MODULE_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		kcSafeFree((ptr)); (ptr) = NULL; \
	} \
} while (0)

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/

	/// <summary>
	/// 保存日志
	/// </summary>
	/// <param name="logPath">日志存储路径</param>
	/// <param name="context">日志内容</param>
	/// <param name="context">日志文件名，设置为NULL时用YYYYMMDD.log代替</param>
	extern KC_EXTERNAL void KCAPI kcSaveLog(const char* const logPath, const StringBuffer context, const char* const fileName);

#pragma region 将执行时间（毫秒）转换为人类可以阅读的日期时间格式

	/// <summary>
	/// 将执行时间（毫秒）转换为人类可以阅读的时间格式
	/// PostgreSQL 14.1 src\bin\psql\common.c(481)
	/// </summary>
	/// <param name="elapsed_msec">执行时间（毫秒）</param>
	/// <param name="str">人类可以阅读的日期时间格式</param>
	extern KC_EXTERNAL void KCAPI kcMilliseconds2String(double elapsed_msec, StringBuffer str);
#pragma endregion

#pragma region 字符串操作
	/// <summary>
	/// 安全释放本模块分配的内存
	/// </summary>
	/// <param name="ptr">待释放的内存</param>
	extern KC_EXTERNAL void KCAPI kcSafeFree(char* ptr);

	/// <summary>
	/// 安全的复制字符串
	/// </summary>
	/// <param name="source">源字符串</param>
	/// <param name="error">失败后的异常信息</param>
	/// <param name="ptr">[out]复制后的字符串，注意不使用时请使用KC_SAFE_MODULE_FREE释放内存</param>
	/// <param name="ptrLen">[out]复制后的字符串长度</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcStringCopyV1(const char* source, StringBuffer error, char** ptr, size_t* ptrLen);


	/// <summary>
	/// 安全的复制字符串
	/// </summary>
	/// <param name="source">源字符串</param>
	/// <param name="srcLen">源字符串长度</param>
	/// <param name="error">失败后的异常信息</param>
	/// <param name="ptr">[out]复制后的字符串，注意不使用时请使用KC_SAFE_MODULE_FREE释放内存</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcStringCopyV2(const char* source, size_t srcLen, StringBuffer error, char** ptr);

	/// <summary>
	/// 安全的复制字符串
	/// </summary>
	/// <param name="source">源字符串</param>
	/// <param name="error">失败后的异常信息</param>
	/// <param name="ptr">[out]复制后的字符串，注意不使用时请使用KC_SAFE_MODULE_FREE释放内存</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcStringCopyV3(const char* source, StringBuffer error, char** ptr);


	/// <summary>
	/// 替换字符串
	/// </summary>
	/// <param name="source">源字符串</param>
	/// <param name="substr">待替换的字符串</param>
	/// <param name="dest">要替换的字符串</param>
	/// <param name="error">出错则为异常信息</param>
	/// <param name="ptr">替换后的的字符串，注意不使用时请使用KC_SAFE_MODULE_FREE释放内存</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcStringReplace(const char* source, const char* substr, const char* dest, StringBuffer error, char** ptr);



	/// <summary>
	/// 将16进制字符串转换为内存字节 0xAABBCCDD
	/// </summary>
	/// <param name="hexString">16进制字符串</param>
	/// <param name="hexLen">16进制字符串长度</param>
	/// <param name="error">[in,out]如果出错则为异常信息</param>
	/// <param name="ptr">[out]内存字节，注意不使用时请使用KC_SAFE_MODULE_FREE释放内存</param>
	/// <param name="ptrLen">内存字节大小</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcHexStringToBytes(const char* hexString, size_t hexLen, StringBuffer error, uint8_t** ptr, size_t* ptrLen);


	/// <summary>
	/// 将内存中的内容转换为16进制字符串
	/// </summary>
	/// <param name="bytes">内存字节</param>
	/// <param name="bytesLen">内存字节大小</param>
	/// <param name="buffer">[in,out]如果成功为16进制字符串，失败为异常信息</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcHexStringFromBytes(const uint8_t* bytes, size_t bytesLen, StringBuffer buffer);
#pragma endregion

#pragma region
	/*****************************************************************************
	*	CRC校验
	*****************************************************************************/

	/// <summary>
	/// 计算数据的crc16校验值
	/// </summary>
	/// <param name="data">待计算的数据</param>
	/// <param name="offset">待计算的数据开始位置</param>
	/// <param name="len">待计算的数据内存大小</param>
	/// <param name="preval">之前的校验值，默认为0xffff</param>
	/// <returns></returns>
	extern KC_EXTERNAL uint16_t KCAPI kcCrc16(const uint8_t* data, size_t offset, size_t len, uint16_t preval);


	/// <summary>
	/// 计算数据的crc32校验值
	/// </summary>
	/// <param name="buf">待计算的数据</param>
	/// <param name="len">待计算的数据内存大小</param>
	/// <returns>crc32校验码</returns>
	extern KC_EXTERNAL uint32_t KCAPI kcCrc32(const char* buf, size_t offset, size_t len);

	/// <summary>
	/// 计算数据的crc64校验值
	/// </summary>
	/// <param name="buf">待计算的数据</param>
	/// <param name="len">待计算的数据内存大小</param>
	/// <returns>crc64校验码</returns>
	extern KC_EXTERNAL uint64_t  KCAPI kcCrc64(const char* buf, size_t offset, size_t len);

#pragma endregion

#pragma region 网络操作函数

	/// <summary>
	/// 将字符串转换为IP地址
	/// </summary>
	/// <param name="addresses">字符串类型的网络地址（ip、机器名、域名）</param>
	/// <param name="inaddr">转换后的网络地址</param>
	/// <param name="buffer">[in,out]如果出错则为异常信息</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcString2NetworkAddressV4(const char* const addresses, struct in_addr* const inaddr, StringBuffer buffer);
#pragma endregion

#pragma region Json操作
	/*************************************************************************************
	*	Json操作
	*  char** ptr不使用时请使用KC_SAFE_MODULE_FREE释放内存
	*************************************************************************************/
	extern KC_EXTERNAL int32_t KCAPI kcJsonGetString(const char* const key, const json_t* const value, char** ptr, StringBuffer error);
	extern KC_EXTERNAL int32_t KCAPI kcJsonGetStringN(const char* const key, const json_t* const value, char** ptr, size_t* ptrLen, StringBuffer error);
	extern KC_EXTERNAL int32_t KCAPI kcJsonGetBool(const char* const key, const json_t* const value, bool* ptr, StringBuffer error);
	extern KC_EXTERNAL int32_t KCAPI kcJsonGetInt32(const char* const key, const json_t* const value, int32_t* ptr, StringBuffer error);
	extern KC_EXTERNAL int32_t KCAPI kcJsonGetInt64(const char* const key, const json_t* const value, int64_t* ptr, StringBuffer error);
#pragma endregion

#pragma region 校验和
	/*************************************************************************************
	*	UDP校验和
	*************************************************************************************/
	/// <summary>
	/// 计算校验和
	/// </summary>
	/// <param name="addr"></param>
	/// <param name="len"></param>
	/// <returns></returns>
	extern KC_EXTERNAL uint16_t KCAPI kcCheckSum(const uint16_t* addr, int32_t len);

	/// <summary>
	/// 计算udp校验码
	/// </summary>
	/// <param name="srcAddr">发送方ip地址</param>
	/// <param name="srcPort">发送方ip端口</param>
	/// <param name="destAddr">接收方ip</param>
	/// <param name="destPort">接收方ip端口</param>
	/// <param name="msg">要发送的消息</param>
	/// <param name="msgLen">要发送的消息长度</param>
	/// <param name="error">异常信息</param>
	/// <param name="udpSum">[in,out]Udp checksum</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcCheckUdpSum(uint32_t srcAddr, uint16_t srcPort, uint32_t destAddr, uint16_t destPort,
		const uint8_t* const msg, uint16_t msgLen, StringBuffer error, uint16_t* udpSum);
#pragma endregion

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif	/* KC_78A1AE04_5624_6B4D_99E9_C8AE78F7E3FA */

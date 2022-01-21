/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	解析数据起始位为0x0551的数据
*****************************************************************************/
#ifndef KC_92BE22E9_F739_134C_9D9D_9F6C010CD013
#define KC_92BE22E9_F739_134C_9D9D_9F6C010CD013

#include <KcPlugins.h>



struct KC_FIVEFIFTY {
	char* identifier;						//数据标识符
	uint8_t addres;							//设备地址
	uint8_t cmd;								//命令字
	uint8_t len;								//数据字节数
	float temperature;					//温度
	float humidity;							//湿度
};

#define KC_SAFE_FREE_FIVE_FIFTY(ptr) do {\
	if(NULL !=(ptr)  ) { \
		kcSafeFreeFifty((ptr)); (ptr) = NULL; \
	} \
} while (0)


#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/

	/// <summary>
	/// 释放struct KC_FIVEFIFTY结构体，不要直接使用，请使用KC_SAFE_FREE_FIFTY
	/// </summary>
	/// <param name="ptr">struct KC_FIVEFIFTY结构体指针</param>
	extern KC_EXTERNAL void KCAPI kcSafeFreeFifty(struct KC_FIVEFIFTY* ptr);
	 
	/// <summary>
	/// 解析数据起始位为0x0551的数据
	/// </summary>
	/// <param name="config">配置信息</param>
	/// <param name="backendItem">接收的数据和远程端点缓冲区</param>
	/// <param name="identifierLen">数据标识符长度</param>
	/// <param name="conn">PostgreSQL链接对象</param>
	/// <param name="rwlock">读写锁</param>
	/// <param name="error">[in,out]失败后的异常信息</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcFiveFiftyPluginProcessData(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, PGconn* conn, pthread_rwlock_t* rwlock, StringBuffer error);

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif	/* KC_92BE22E9_F739_134C_9D9D_9F6C010CD013*/

/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	解析数据起始位为0x7567的数据
*****************************************************************************/
#ifndef KC_26CE95CC_10C4_4A45_B912_60346703C7BE
#define KC_26CE95CC_10C4_4A45_B912_60346703C7BE

#include <KcPlugins.h>



#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/


	/// <summary>
	/// 解析数据起始位为0x7567的数据
	/// </summary>
	/// <param name="config">配置信息</param>
	/// <param name="backendItem">接收的数据和远程端点缓冲区</param>
	/// <param name="identifierLen">数据标识符长度</param>
	/// <param name="conn">PostgreSQL链接对象</param>
	/// <param name="rwlock">读写锁</param>
	/// <param name="error">[in,out]失败后的异常信息</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcFiveFiftyPluginProcessData(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* item, size_t identifierLen, /*PGconn* conn,*/ pthread_rwlock_t* rwlock, StringBuffer error);

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif	/* KC_26CE95CC_10C4_4A45_B912_60346703C7BE*/

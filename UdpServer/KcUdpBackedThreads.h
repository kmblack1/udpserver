#ifndef KC_AB02CC14_C70C_DF40_AACD_F9D01F2801DD
#define KC_AB02CC14_C70C_DF40_AACD_F9D01F2801DD


#include "KcUdpBackedThreadsHead.h"

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/

	/// <summary>
	/// 启动后台线程池
	/// </summary>
	/// <param name="config">配置信息</param>
	/// <param name="process">处理函数</param>
	/// <returns>KC_OK表示成功，其它表示失败</returns>
	extern KC_EXTERNAL int32_t KCAPI kcUdpBackedThreadStart(const struct KC_CONFIG* const config, StringBuffer error);

	/// <summary>
	/// 停止后台线程池
	/// </summary>
	extern KC_EXTERNAL void KCAPI kcUdpBackedThreadStop();

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif	/* KC_AB02CC14_C70C_DF40_AACD_F9D01F2801DD */

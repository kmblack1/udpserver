/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	加载配置文件
*****************************************************************************/
#ifndef KC_0FB1C9F6_9F5A_FF4E_87AB_E6D5B8CB779C
#define KC_0FB1C9F6_9F5A_FF4E_87AB_E6D5B8CB779C
#include <stdint.h>
#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#endif
#include "stringbuffer.h"

struct KC_CONFIG {

	struct sockaddr_in addres;		//UDP侦听的IP和端口
	int32_t sendTimeout;				//UDP发送超时时间，单位为毫秒
	int32_t recvTimeout;				//UDP接收超时时间，单位为毫秒
	int32_t queue_max;					//队列大小
	int32_t threadPoolSize;			//处理数据的线程池大小

	char* log_path;						//日志文件完整路径
	char* runing_path;					//运用程序运行目录	
};



#define KC_SAFE_FEEE_CONFIG(ptr) do {\
	if(NULL !=(ptr)  ) { \
		kcConfigFree((ptr)); (ptr) = NULL; \
	} \
} while (0)



#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/

	/// <summary>
	/// 创建配置对象
	/// </summary>
	/// <param name="str"></param>
	/// <param name="ptr"></param>
	/// <returns></returns>
	extern KC_EXTERNAL int32_t KCAPI kcConfigCreate(const char * const stdPath,StringBuffer str, struct KC_CONFIG** ptr);

	/// <summary>
	/// 释放配置对象
	/// </summary>
	/// <param name="ptr"></param>
	/// <returns></returns>
	extern KC_EXTERNAL void KCAPI kcConfigFree(struct KC_CONFIG* ptr);


#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/



#endif	/*KC_0FB1C9F6_9F5A_FF4E_87AB_E6D5B8CB779C*/

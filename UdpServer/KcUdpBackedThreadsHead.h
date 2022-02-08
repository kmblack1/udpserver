#ifndef KC_F03ADABD_C6F6_964A_B843_57CCB514D5CE7
#define KC_F03ADABD_C6F6_964A_B843_57CCB514D5CE7
#include <stdint.h>
#include <pthread.h>
#include <jansson.h>
#ifdef _MSC_VER
#   include <winsock2.h>
#	include <libpq-fe.h>
#else
#   include <unistd.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#	include <postgresql/libpq-fe.h>
#endif
#include "stringbuffer.h"
#include "KcCore.h"
#include "KcUtility.h"
#include "KcConfig.h"






/*****************************************************************************
*   启动时创建的对象标志
*****************************************************************************/
#define KC_SHARED_MUTEX										(1<<0)
#define KC_SHARED_COND											(1<<1)
#define KC_SHARED_CONDSTART								(1<<2)
#define KC_SHARED_RWLOCK										(1<<3)




/*****************************************************************************
*	共享对象
*****************************************************************************/
/// <summary>
/// 线程共享对象结构
/// </summary>
struct KC_SHARED {
	/// <summary>
	/// 初始化标志
	/// </summary>
	int32_t flag;

	/// <summary>
	/// 互斥对象
	/// </summary>
	pthread_mutex_t* mutex;

	/// <summary>
	/// 条件变量
	/// </summary>
	pthread_cond_t* cond;
	pthread_cond_t* condStart;

	/// <summary>
	/// 读写锁-轻量级
	/// </summary>
	pthread_rwlock_t *rwlock;



	/// <summary>
	/// 池中的项目
	/// </summary>
	struct KC_BACKEND_ITEM** items;

	/// <summary>
	/// 服务是否运行
	/// </summary>
	int32_t isRuning;

	/// <summary>
	/// 解析数据使用的插件
	/// </summary>
	struct KC_PLUGINS* plugins;

	/// <summary>
	/// 配置信息
	/// </summary>
	const struct KC_CONFIG* config;


	/// <summary>
	/// 已经运行的线程数量
	/// </summary>
	uint8_t threadRunCount;


	/// <summary>
	/// 工作线程
	/// </summary>
	pthread_t** threadWorks;

	/// <summary>
	/// UDP侦听线程
	/// </summary>
	pthread_t* threadRecv;
};




#endif	/* KC_F03ADABD_C6F6_964A_B843_57CCB514D5CE7 */

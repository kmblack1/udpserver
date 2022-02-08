/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	加载、查找、卸载插件
*****************************************************************************/
#ifndef KC_EBBA23E8_31F3_CD4C_BA71_4264EA36F72A
#define KC_EBBA23E8_31F3_CD4C_BA71_4264EA36F72A

#include <stdint.h>
#ifdef _MSC_VER
#	include <Windows.h>
#	include <libpq-fe.h>
#else
#	include <dlfcn.h>
#	include <postgresql/libpq-fe.h>
#endif
#include "KcUtility.h"
#include "stringbuffer.h"
#include "KcConfig.h"
//#include "KcUdpBackedThreadsHead.h"


#define KC_SAFE_UNLOAD_SERVER_PLUGINS(ptr) do {\
	if(NULL !=(ptr)  ) { \
		kcServerPluginsUnload((ptr)); (ptr) = NULL; \
	} \
} while (0)

/*****************************************************************************
*	处理解析后的数据
*****************************************************************************/
typedef  int32_t(*kcPluginProcessData)(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM* backendItem, size_t identifierLen, PGconn* conn, StringBuffer error);


/*****************************************************************************
*	解析数据所用的插件
*****************************************************************************/
struct KC_PLUGIN_ITEM {
	uint8_t* identifier;
	size_t identifierLen;
	char* library;
	char* entry_process_data;
#ifdef _MSC_VER
	HINSTANCE handler;
#else
	void* handler;
#endif
	kcPluginProcessData funProcessData;
};

struct KC_PLUGINS {
	size_t count;
	struct KC_PLUGIN_ITEM** items;
};

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/


	/// <summary>
	/// 加载插件
	/// </summary>
	extern KC_EXTERNAL int32_t KCAPI kcServerPluginsLoad(const struct KC_CONFIG* const config, StringBuffer str, struct KC_PLUGINS** ptr);

	/// <summary>
	/// 查找插件
	/// </summary>
	extern KC_EXTERNAL const struct KC_PLUGIN_ITEM* KCAPI kcServerPluginsFind(const StringBuffer recv, const struct KC_PLUGINS* plugins, StringBuffer error);

	/// <summary>
	/// 卸载插件
	/// </summary>
	extern KC_EXTERNAL void KCAPI kcServerPluginsUnload(struct KC_PLUGINS* plugins);

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif	/* KC_EBBA23E8_31F3_CD4C_BA71_4264EA36F72A */

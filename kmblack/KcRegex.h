/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	正则表达式相关定义
*****************************************************************************/
#ifndef KC_8E966F72_4C99_2A47_A8E2_6907B35533A3
#define KC_8E966F72_4C99_2A47_A8E2_6907B35533A3

#ifdef _MSC_VER
#	include <tre/regex.h>
#else
#	include <regex.h>
#endif

#define KC_SAFE_REGEX_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		regfree((ptr)); \
		free((ptr)); \
		(ptr) = NULL; \
	} \
} while (0)

#endif	/* KC_8E966F72_4C99_2A47_A8E2_6907B35533A3 */

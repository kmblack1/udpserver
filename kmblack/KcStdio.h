/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	标准字节操作
*****************************************************************************/

#ifndef KC_4313D5B8_9FB4_D548_AF70_6589CB017804
#define KC_4313D5B8_9FB4_D548_AF70_6589CB017804

#include "KcUtility.h"
/*****************************************************************************
*	从字节中读取数据
*****************************************************************************/
#define KC_VAL8_FROM_BYTES(ptr,ptrVal8) do {\
	KC_MEMCPY((ptrVal8), (1), (ptr), (1)); \
	(ptr) += (1); \
} while (0);

#define KC_VAL16_FROM_BYTES(ptr,pbval16,ival16) do {\
	KC_MEMCPY((pbval16), (2), (ptr), (2)); \
	(ival16) = *((uint16_t*)(pbval16)); \
	(ival16) = pg_bswap16((ival16)); \
	(ptr) += (2); \
} while (0);

#define KC_VAL32_FROM_BYTES(ptr,pbval32,ival32) do {\
	KC_MEMCPY((pbval32), (4), (ptr), (4)); \
	(ival32) = *((uint32_t*)(pbval32)); \
	(ival32) = pg_bswap32((ival32)); \
	(ptr) += (4); \
} while (0);

#define KC_VAL48_FROM_BYTES(ptr,pval64,ival64) do {\
	(pval64[0])=0;(pval64[1])=0; \
	KC_MEMCPY((&(pval64[2])), (6), (ptr), (6)); \
	(ival64) = *((uint64_t*)(pval64)); \
	(ival64) = pg_bswap64((ival64)); \
	(ptr) += (6); \
} while (0);

#define KC_VAL64_FROM_BYTES(ptr,pval64,ival64) do {\
	KC_MEMCPY((pval64), (8), (ptr), (8)); \
	(ival64) = *((uint64_t*)(pval64)); \
	(ival64) = pg_bswap64((ival64)); \
	(ptr) += (8); \
} while (0);

#define KC_VAL_ZERO_FROM_BYTES(ptr,pbvals,len) do {\
	KC_MEMCPY((pbvals), (len), (ptr), (len)); \
	(pbvals)[len]='\0'; \
	(ptr) += (len); \
} while (0);

#define KC_VAL_NOZERO_FROM_BYTES(ptr,pbvals,len) do {\
	KC_MEMCPY((pbvals), (len), (ptr), (len)); \
	(ptr) += (len); \
} while (0);

/*****************************************************************************
*	将数据写入字节
*****************************************************************************/
#define KC_VAL8_TO_BYTES(pbytes,ival8) do {\
	pbytes[0] = ival8; \
	(pbytes) += (1); \
} while (0);

#define KC_VAL16_TO_BYTES(pbytes,ival16) do {\
	uint16_t tmp = pg_bswap16((ival16)); \
	KC_MEMCPY((pbytes), (2), &(tmp), (2)); \
	(pbytes) += (2); \
} while (0);

#define KC_VAL32_TO_BYTES(pbytes,ival32) do {\
	uint32_t tmp = pg_bswap32((ival32)); \
	KC_MEMCPY((pbytes), (4), &(tmp), (4)); \
	(pbytes) += (4); \
} while (0);


#define KC_VAL48_TO_BYTES(pbytes,ival64) do {\
	uint64_t tmp = pg_bswap64((ival64)); \
	uint8_t* ptmp = (uint8_t*)&tmp; \
	ptmp += 2; \
	KC_MEMCPY((pbytes), (6), ptmp, (6)); \
	(pbytes) += (6); \
} while (0);

#define KC_VAL64_TO_BYTES(pbytes,ival64) do {\
	uint64_t tmp = pg_bswap64((ival64)); \
	KC_MEMCPY((pbytes), (8), &(tmp), (8)); \
	(pbytes) += (8); \
} while (0);

#define KC_VAL_ZERO_TO_BYTES(pbytes,vals,len) do {\
	KC_MEMCPY((pbytes), (len), (vals), (len)); \
	(pbytes) += (len); \
} while (0);

#endif	/* KC_4313D5B8_9FB4_D548_AF70_6589CB017804 */

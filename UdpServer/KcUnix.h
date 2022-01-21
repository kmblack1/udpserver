/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	UNIX  以服务方式运行
*****************************************************************************/

#ifndef KC_D73FBB29_02BE_D049_A1B5_A5694B162479
#define KC_D73FBB29_02BE_D049_A1B5_A5694B162479
#ifdef __GNUC__

#include <stdint.h>
#include <signal.h>
#include <curses.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "KcUtility.h"
#include "stringbuffer.h"

void signal_handler(int32_t sig);

int32_t unixServiceRuning(const char* const stdPath, StringBuffer error);

#endif //__GNUC__
#endif	/*KC_D73FBB29_02BE_D049_A1B5_A5694B162479*/

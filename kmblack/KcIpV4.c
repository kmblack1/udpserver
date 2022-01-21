/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	网络操作函数
*****************************************************************************/
#ifdef _WIN32
#	include <conio.h>
#	include <Ws2tcpip.h>
#	include <Ws2def.h>
#else
#   include <ctype.h>
#   include <netdb.h>
#endif


#include "KcUtility.h"
#include "KcShared.h"



int32_t kcString2NetworkAddressV4(const char* const addresses, struct in_addr* const inaddr, StringBuffer buffer) {
	int32_t rc = 0;
	struct addrinfo hints, * result = NULL, * rp;

	/*if (SE_STR_ISNULLOREMPTY(addr) ) {
		inaddr->s_addr = htonl(INADDR_ANY);
		return SE_OK;
	}*/

	rc = inet_pton(AF_INET, addresses, &inaddr->s_addr);
	if (rc) {
		return KC_OK;
	} else {
		if (0 == rc) {
			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_family = AF_INET;
			rc = getaddrinfo(addresses, NULL, &hints, &result);
			if (rc) {
#ifdef _WIN32
				KC_GOTOERR(buffer, gettext("%s failed with error: %d"), KC_2STR(getaddrinfo), KC_SOCKET_GETLASTERROR);
#else
				KC_GOTOERR(buffer, gettext("%s failed with error: %d"), KC_2STR(getaddrinfo), errno);
#endif
			}
			for (rp = result; NULL != rp; rp = rp->ai_next) {
				inaddr->s_addr = ((struct sockaddr_in*)(rp->ai_addr))->sin_addr.s_addr;
				break;
			}
			freeaddrinfo(result);
		} else {
			KC_GOTOERR(buffer, gettext("call function \"%s\" fail."), KC_2STR(inet_pton));
		}
	}
	/*
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_family = AF_INET;

	rc = getaddrinfo(addr, NULL, &hints, &result);
	if (0 != rc)
		SE_GOTOERR(str,"getaddrinfo failed with error: %d.", rc);

	for (rp = result; NULL != rp; rp = rp->ai_next) {
		inaddr->s_addr = ((struct sockaddr_in*)(rp->ai_addr))->sin_addr.s_addr;
		break;
		//printf("ip addresss: %s.", inet_ntoa(addr));
	}

	freeaddrinfo(result);*/
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

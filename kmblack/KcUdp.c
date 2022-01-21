/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	计算UDP校验和
*****************************************************************************/

#include "pg_bswap.h"
#include "KcUtility.h"
#include "KcStdio.h"
#include "KcShared.h"
#include "KcIpV4.h"

#pragma region  计算校验和

/*
 * in_cksum --
 *      Checksum routine for Internet Protocol family headers (C Version)
 *      https://www.inf.pucrs.br/~benso/redes601/2004_2/in_cksum.c
 */
uint16_t kcCheckSum(const uint16_t* addr, int32_t len) {
	register uint32_t sum = 0;
	uint16_t answer = 0;
	register const uint16_t* w = addr;
	register int32_t nleft = len;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(uint8_t*)(&answer) = *(uint8_t*)w;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
	sum += (sum >> 16);                     /* add carry */
	answer = ~sum;                          /* truncate to 16 bits */
	return (answer);
}


static uint32_t kcCheckUdpSumV1(const uint16_t* addr, int32_t len, uint32_t udpSum) {
	register uint32_t sum = udpSum;
	uint16_t answer = 0;
	register const uint16_t* w = addr;
	register int32_t nleft = len;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(uint8_t*)(&answer) = *(uint8_t*)w;
		sum += answer;
	}
	return sum;
}


int32_t kcCheckUdpSum(uint32_t srcAddr, uint16_t srcPort, uint32_t destAddr, uint16_t destPort,
	const uint8_t* const msg, uint16_t msgLen, StringBuffer error, uint16_t* udpSum) {
	uint8_t buf[KC_UDP_FAKE_UDP_HEADER_SIZE], * ptmp = buf;
	uint16_t answer;
	uint32_t sum;
	static const  uint16_t MAXLEN = KC_UDP_BUFFER_SIZE - sizeof(struct udp_fake_head) - sizeof(struct udp_hdr);

	if (msgLen > MAXLEN) {
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("invalid argument.(udp message max lenght>%d)"), MAXLEN);
		goto KC_ERROR_CLEAR;
	}
	//struct udp_fake_head
	KC_VAL32_TO_BYTES(ptmp, srcAddr);
	KC_VAL32_TO_BYTES(ptmp, destAddr);
	KC_VAL8_TO_BYTES(ptmp, 0);
	KC_VAL8_TO_BYTES(ptmp, IPPROTO_UDP);
	KC_VAL16_TO_BYTES(ptmp, (KC_UDP_HEADER_SIZE + msgLen));
	//struct udp_hdr 
	KC_VAL16_TO_BYTES(ptmp, srcPort);
	KC_VAL16_TO_BYTES(ptmp, destPort);
	KC_VAL16_TO_BYTES(ptmp, (KC_UDP_HEADER_SIZE + msgLen));
	KC_VAL16_TO_BYTES(ptmp, 0); // Udp checksum (optional)
	KC_SET_BUFFER_EXCEED(buf, ptmp, KC_UDP_FAKE_UDP_HEADER_SIZE,error);

	sum = 0;
	sum = kcCheckUdpSumV1((const uint16_t*)buf, KC_UDP_FAKE_UDP_HEADER_SIZE, sum);
	sum = kcCheckUdpSumV1((const uint16_t*)msg, msgLen, sum);

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;

	(*udpSum) = pg_bswap16(answer);
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}


#pragma endregion

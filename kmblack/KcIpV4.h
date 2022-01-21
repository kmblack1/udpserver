/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	udp头定义
*****************************************************************************/

#ifndef KC_9F4A6A1C_1DC1_3440_8A87_D02E4B0AEC70
#define KC_9F4A6A1C_1DC1_3440_8A87_D02E4B0AEC70

#define IP_VERSION_V4 (4)
#define IP_VERSION_V6 (6)

/*****************************************************************************
*   IP包头大小
*****************************************************************************/
#define KC_IPV4_HEADER_SIZE (20)

/*****************************************************************************
*   UDP FAKE包头大小
*****************************************************************************/
#define KC_UDP_FAKE_HEADER_SIZE (12)

/*****************************************************************************
*   UDP包头大小
*****************************************************************************/
#define KC_UDP_HEADER_SIZE (8)

/*****************************************************************************
*   UDP包头大小
*****************************************************************************/
#define KC_UDP_FAKE_UDP_HEADER_SIZE (KC_UDP_FAKE_HEADER_SIZE+KC_UDP_HEADER_SIZE)

/*****************************************************************************
*   UDP包头大小
*****************************************************************************/
#define KC_IPV4_UDP_HEADER_SIZE (KC_IPV4_HEADER_SIZE + KC_UDP_HEADER_SIZE)


// Define the ICMP header
struct icmp_hdr {
	unsigned char   type;
	unsigned char   code;
	unsigned short checksum;
	unsigned short  id;
	unsigned short  sequence;
	unsigned long   timestamp;
};


/*
* IPv4 header
* 4位版本号（Version）： 指定IP协议的版本号。因为目前仍主要使用IPV4版本，用于标识IP协议版本，IPv4是0100，IPv6是0110，也就是二进制的4和6。
* 4位首部长度（Internet Header Length）： 用于标识首部的长度，单位为4字节（32bit），所以首部长度最大值为60字节，但一般只使用20字节的固定长度。
* 8位服务类型（Type Of Service）： 简称TOS，定义IP封包在传送过程中要求的服务类型。包括一个3bit的优先权子字段，4bit的TOS子字段，1bit未用位但必须置0。4bit的TOS分别代表：最小时延、最大吞吐量、最高可靠性和最小费用，4bit中同时只能置其中1bit为1,。如果所有4bit均为0，那就意味着是一般服务。
*		0,1,2位 优先度
*		3 最低延迟
*		4 最大吞吐
*		5 最大可靠性
*		6 最小代价
*		3~6 最大安全
*		7 未定义
* 16位总长度（Total Length）： 标识IP数据报包的总长度，以字节为单位。利用首部长度字段和总长度字段，就可以知道IP数据报中数据内容的起始位置和长度。由于该字段长16bit，所以IP数据包最长可达2^16 -1 = 65535字节，当数据包被分片时，该字段的值也随着变化。
* 16位标识（Identification）： 用于标识IP数据报的唯一码，如果因为数据链路层帧数据段长度限制（也就是MTU，支持的最大传输单元为1500字节），一个过长的IP数据包需要被拆散进行分片发送，拆分出的每个分片的IP数据包标识都是一致的，当分片要进行重组的时候就是依据了这个标识。
* 3位标识（Flag）： 目前只有两种，即只有后2位有意义；最低位为MF（More Fragment），为1代表后面还有分片的数据包，MF=0代表当前数据包已是最后的数据包。第二位为DF（Don’t Fragment），DF=1代表不能分片，此时若这个包长度大于路由器的长度限制，则直接丢弃了，DF=0代表可以分片。
* 13位片偏移（Fragment Offset）： 代表某个分片在原始IP数据包中的相对位置。通过这个偏移量和16位标识将多个分片数据包进行还原成原IP数据包。
* 8位生存时间（Time To Live）： 简称TTL，表示数据包可以经过的最多路由器数。包在传输中每经过一个路由器TTL便减一，若TTL为0，则丢弃，一般不会经过255这么多，除非有循环路由。
* 8位协议（Protocol）： 代表上层的传输协议类型，一般常见的1代表ICMP，6代表TCP，17代表UDP。
* 16位首部校验和（Header Checksum）： 用于验证数据完整性，用以确保封包（分片）被正确无误的接收到，与UDP和TCP协议包头中的校验和作用是一样的。首先通过校验算法填入16位的值，当目标机器收到后重新进行校验算法并进行比对，若相同代表没有出现问题。
* 32位源IP地址与32为目标IP地址。
*/
struct ip_hdr {
	uint8_t verlen;                // 4-bit IPv4 version 4-bit header length (in 32-bit words)
	uint8_t tos;                    // IP type of service
	uint16_t totallength;        // Total length
	uint16_t id;            // Unique identifier
	uint16_t offset;        // Fragment offset field
	uint8_t ttl;           // Time to live
	uint8_t protocol;      // Protocol(TCP,UDP etc)
	uint16_t checksum;      // IP checksum
	uint32_t srcaddr;       // Source address
	uint32_t destaddr;      // Dest. address
};

//用于计算UDP校验和
struct udp_fake_head {
	uint32_t srcaddr;
	uint32_t destaddr;
	uint8_t filling;
	uint8_t protocol;
	uint16_t length;
};


// Define the UDP header
struct udp_hdr {
	uint16_t src_port;       // Source port no.
	uint16_t dst_port;       // Dest. port no.
	uint16_t length;       // Udp packet length
	uint16_t checksum;     // Udp checksum (optional)
};


#endif	/*KC_9F4A6A1C_1DC1_3440_8A87_D02E4B0AEC70*/

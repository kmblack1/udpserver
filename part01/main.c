/*****************************************************************************
*    昆明畅博科技公司 Copyright (c) 2014-2022
*	 作者：黑哥，2022-01-14
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#ifdef _MSC_VER
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif

#pragma region  结构体

//一个带长度的字符串结构体
struct KC_CUSTOM2 {
	int32_t len;
	uint8_t* bytes;
};


//struct KC_UNION结构体中的item只允许3种类型中的一种，由type字段指明item使用的类型
struct KC_UNION {
	uint8_t type;
	union {		
		//大小为32字节的缓冲区
		uint8_t bytes[32];
		//带长度的字符串
		struct KC_CUSTOM2* u2;
		//无符号64位整数
		uint64_t u3;
	} item;
};
#pragma endregion

//C基本数据类型
int32_t main() {
	#pragma region  整数使用<stdint.h>标准头文件，在任何平台占用的字节大小都一至
	int8_t ival8;				//8bit
	int16_t ival16;			//16bit
	int32_t ival32;			//32bit
	int64_t ival64;			//64bit

	uint8_t uival8;			//8bit
	uint16_t uival16;		//16bit
	uint32_t uival32;		//32bit
	uint64_t uival64;		//64bit
	#pragma endregion

	//浮点数 ，在任何平台占用的字节大小都一至
	float fval32;				//32bit
	double fval64;			//64bit

	//C自身没有bool类型，在C中0表示false,1表示true,如果要使用bool类型引用标准头文件#include <stdbool.h>
	bool bVal = true;

	//日期时间类型，本质上就是int64_t,转换换算法以后讲
	time_t datetime;				//64bit
	//字符类型,char,uint8_t
	char val;									//8bit	
	//汉字“本”UTF8编码为0xE69CAC,二进制编码111001101001110010101100，最高位为111，表示这个字使用3个字节24bit
	//关于UTF-8占用的字节大小以后会讲到
	const char *str1="123本";				//strlen(val),汉字长度取决于编码方式，目前普遍使用UTF8编码，
	//字符串结束标志 \0

	//uint8_t类似C#和java中的byte,在C中也可以表示汉字
	const uint8_t* str2 = "123本";

	//表示任意数据类型
	void* ptr;

	//结构体
	struct KC_UNION un;




	fprintf_s(stdout, "%s\n", str1);
	fprintf_s(stdout, "%s\n", str2);
}

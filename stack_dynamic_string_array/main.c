#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pg_bswap.h>
#include <stringbuffer.h>
#include  <KcStdio.h>

/*
* C语言痛点-动态数组的实现
* 知识点：
*	1、C语言在栈（Stack）上创建的字符串数组且无需释放
*	2、实现动态数组-C语言痛点之一
*/
struct KC_STRING {
	const char* value;
	uint32_t len;
};

/*****************************************************************************
*	2、实现动态数组
*			在C中万物皆指针，根据这一特性使用stringbuffer实现动态数组
*			因为内存是顺序内存，性能更好
*	动态数组格式：
*		4字节数组维数+4字节第x维数组大小+第x维数组第x个元素长度+字符串（为方便输出字符串加结束标志\0）
*	要点：
*		数字（整数、浮点数）以二进制方式存储时，是高位在前低位在后的，而现在大多数cpu是低位在前，高位在后
*		例如十进制数3232235778
*			二进制存储时为0xC0A80102换位0x0102C0A8
*			换算为ip v4 (3232235778>>24)&255 ,(3232235778 >>16)&255,(3232235778 >>8)&255,(3232235778)&255
*			(192<<24)|(168<<16)|(1<<8)|(2)
*****************************************************************************/
int32_t dynamicStringArray(const struct KC_STRING* strObjects, size_t stringCount, StringBuffer str) {
	uint8_t* ptmp, * pfirst, * psecond;
	uint32_t ival32, arrayCout = 0;

	resetStringBuffer(str);
	// 确保内存足够，如果不要字符串结束标志则为4，否则为5
	if (!enlargeStringBuffer(str, 8)) {
		resetStringBuffer(str);
		appendStringBufferStr(str, gettext("out of memory."));
		goto KC_ERROR_CLEAR;
	}

	ptmp = str->data;
	//使用二维数组
	ival32 = 2;
	ival32 = pg_bswap32(ival32);						//换位-高位在前低位在后
	KC_MEMCPY(ptmp, 4, &ival32, 4);
	ptmp += 4;
	//现在还不知道第一维数组中元素的个数，因此暂时设置第一维数组中元素的个数为0
	pfirst = ptmp;
	ival32 = 0;
	ival32 = pg_bswap32(ival32);						//换位-高位在前低位在后
	KC_MEMCPY(ptmp, 4, &ival32, 4);
	ptmp += 4;
	//将下标为偶数的字符串复制到动态数组
	for (size_t i = 0; i < stringCount; ++i) {
		if (0 == (i % 2)) {
			// 确保内存足够，如果不要字符串结束标志则为4，否则为5
			if (!enlargeStringBuffer(str, 5 + strObjects[i].len)) {
				resetStringBuffer(str);
				appendStringBufferStr(str, gettext("out of memory."));
				goto KC_ERROR_CLEAR;
			}
			//第x维数组第x个元素长度
			ival32 = strObjects[i].len;
			ival32 = pg_bswap32(ival32);						//换位-高位在前低位在后
			KC_MEMCPY(ptmp, 4, &ival32, 4);
			ptmp += 4;
			//第x维数组第x个元素字符串.复制字符串
			KC_MEMCPY(ptmp, strObjects[i].len, strObjects[i].value, strObjects[i].len);
			ptmp += strObjects[i].len;
			//字符串结束标志,如果不要可以删除后面二行
			ptmp[0] = '\0';
			++ptmp;

			++arrayCout;
		}
	}
	//设置第二维字符串数组，为简化操作，只设置一个元素
	psecond = ptmp;

	// 确保内存足够，如果不要字符串结束标志则为4，否则为5
	if (!enlargeStringBuffer(str, 12)) {
		resetStringBuffer(str);
		appendStringBufferStr(str, gettext("out of memory."));
		goto KC_ERROR_CLEAR;
	}
	//第二维数组中元素的个数
	KC_VAL32_TO_BYTES(ptmp, 1);
	//第x维数组第x个元素长度
	KC_VAL32_TO_BYTES(ptmp, 3);
	//第x维数组第x个元素字符串.复制字符串
	KC_VAL_ZERO_TO_BYTES(ptmp, "abc\0", 4);

	//现在知道了动态数组的大小，然后我们重新设置动态数数组大小
	KC_VAL32_TO_BYTES(pfirst, arrayCout);


	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

void printDynamicStringArray(StringBuffer str) {
	uint32_t dimension, currentSize, stringLen;
	uint8_t bval32[4];
	uint8_t* ptmp, * parray;

	ptmp = str->data;
	KC_VAL32_FROM_BYTES(ptmp, bval32, dimension);
	fprintf(stdout, "数组大小(%u)\n", dimension);
	for (uint32_t i = 0; i < dimension; ++i) {
		parray = ptmp;
		KC_VAL32_FROM_BYTES(ptmp, bval32, currentSize);
		fprintf(stdout, "第%u维元素个数(%u)\n", i, currentSize);
		for (uint32_t j = 0; j < currentSize; ++j) {
			KC_VAL32_FROM_BYTES(ptmp, bval32, stringLen);
			fprintf(stdout, "第%u维第%u个元素长度：%u，值为：%s\n", i, j, stringLen, ptmp);
			ptmp += stringLen;
			++ptmp;//跳过字符串结束标记
		}
	}
}

int32_t main(int32_t argc, char* argv[]) {
	StringBuffer str = NULL;

	/*****************************************************************************
	*	1、C语言在栈上创建的字符串数组且无需释放
	*****************************************************************************/
	//直接为字符串斌值，但不设置长度
	struct KC_STRING strObjects[] = {
		{"fivefifty",0}, //如果定义为const uint32_t则必须手动设置长度，这里不能使用strlen
		{"内卷时代到来了",0},
			{"定义结构体",0}
	};
	//计算结构体大小
	const size_t STROBJ_ITME_SIZE = sizeof(struct KC_STRING);
	//计算数组大小
	const size_t STRING_COUNT = sizeof(strObjects) / STROBJ_ITME_SIZE;


	str = createStringBuffer();

	//计算项中字符串的长度
	for (size_t i = 0; i < STRING_COUNT; ++i)
		strObjects[i].len = (uint32_t)strlen(strObjects[i].value);

	for (size_t i = 0; i < STRING_COUNT; ++i)
		fprintf(stdout, "%s\n", strObjects[i].value);

	KC_CHECK_RCV2(dynamicStringArray(strObjects, STRING_COUNT, str));
	fprintf(stdout, "\n\n");
	printDynamicStringArray(str);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

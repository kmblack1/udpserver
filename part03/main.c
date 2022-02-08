/*****************************************************************************
*    昆明畅博科技公司 Copyright (c) 2014-2022
*	 作者：黑哥，2022-01-14
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#ifdef _MSC_VER
#	include <io.h>
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#else
#	include <unistd.h>
#endif
#include "KcCore.h"

/*
* 野指针、使用GOTO示例
* 通过定义常用的宏和GOTO简化程序
* 现在的程序简洁明了，容易查找问题所在
*/
int32_t main(int32_t argc, char* argv[]) {
	/*
	* =NULL的指针表示需要释放
	*	遵循谁分配谁释放的原则
	* 在出现内存泄露时我们就很容易排查
	*/
	char* str = NULL;
	FILE* stream = NULL;
	time_t now;
	struct tm t, * ptime = &t;
	const char* fullfile = "CMakeLists.txt";
	int32_t* pval32; //pval32为野指针，值不确定


	time(&now);						//获取系统日期和时间
#ifdef _MSC_VER
	localtime_s(ptime, &now);   //获取当地日期和时间
#else
	ptime = localtime(&now);
#endif
	fprintf(stdout, "%04d%02d%02d%02d%02d%02d\n",
		ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min, ptime->tm_sec);

	str = (char*)malloc(128);
	KC_CHECK_MALLOC(str);  //分配内存后检查，如失败则直接goto至KC_ERROR_CLEAR
	str[0] = 0x41;
	str[1] = '\0';
	fprintf(stdout, "%s", str);
	free(str);
	//不是free以后就没事的，free后str为野指针，应该使用KC_SAFE_FREE(str)释放内存

	/*
	* 读取文件的标准操作
	* 写入文件不需要检查文件是不存在
	* 但是要检查文件是否具有写入权限
	*		文件权限检查，值范围0-7
	*			0	检查文件是否存在
	*			1	可以执行
	*			2	可以写入
	*			3	执行和写入
	*			4	可以读取
	*			5	读取和执行
	*			6  读取和写入
	*			7  读取和写入和执行
	*/
	KC_FILE_READ_ACCESS_CHECK(fullfile);
	KC_FILE_OPEN_AND_CHECK(stream, fullfile, PG_BINARY_R);
	KC_FILE_DESCRIPTORS_CHECK(fullfile);


	KC_FILE_CLOSE(stream);
	/*
	* 因为第53行已经free了str，但是str为野指针
	* KC_SAFE_FREE会再次free一次
	* 虽然程序可以正常运行，但这是一个潜在BUG
	* 说不定那天就挂了，在代码量比较大的时候
	* 类似的问题非常难以排查，所以我们先制定好
	* 规则，编码时遵循这些规则排除潜在的问题
	*/
	KC_SAFE_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	KC_FILE_CLOSE(stream);
	KC_SAFE_FREE(str);
	return EXIT_FAILURE;
}

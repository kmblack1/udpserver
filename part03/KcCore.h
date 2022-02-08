/*****************************************************************************
*    昆明畅博科技公司 Copyright (c) 2014-2022
*	 作者：黑哥，2022-01-14
*
* 头文件注意事项，为防止头文件重覆引用，需要定义头文件宏
* 头文件宏宏命名方式 KC_16位UUID，其中UUID的中划线改成下划线
*
* 常用的宏可以单独拿出来定义，给自己的其它项目使用
*****************************************************************************/

#ifndef KC_043055AA_F295_42FC_BE3A_D4B24B4AAE54
#define KC_043055AA_F295_42FC_BE3A_D4B24B4AAE54

/*****************************************************************************
*	安全释放分配的内容
*****************************************************************************/
#define KC_SAFE_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		free((ptr)); (ptr) = NULL; \
	} \
} while (0)

#define KC_FILE_CLOSE(stream) do {\
	if(NULL !=(stream)  ) { \
		fclose((stream)); (stream) = NULL; \
	} \
} while (0)
/*****************************************************************************
*   检查分配内存是否成功
*****************************************************************************/
#define KC_CHECK_MALLOC(ptr) do{\
	if (NULL == (ptr) ){\
		fprintf(stderr,"out of memory.(%s:%d)\n",__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*	文件夹和文件访问权限及创建目录API
#include <io.h>
00——只检查文件是否存在

02——写权限

04——读权限

06——读写权限
mkdir:创建目录
*****************************************************************************/

#if defined(_MSC_VER) || defined(__CYGWIN__)
#define PG_BINARY	O_BINARY
#define PG_BINARY_A "ab"
#define PG_BINARY_R "rb"
#define PG_BINARY_W "wb"
#else
#define PG_BINARY	0
#define PG_BINARY_A "a"
#define PG_BINARY_R "r"
#define PG_BINARY_W "w"
#endif

#if defined(_MSC_VER)
#define KC_ACCESS(_name,_mode) _access((_name),(_mode))
#define KC_MKDIR(_name,_mode) _mkdir((_name))
#else
#define KC_ACCESS(_name,_mode) access((_name),(_mode))
#define KC_MKDIR(_name,_mode) mkdir((_name),(_mode))
#endif

/*****************************************************************************
*   FILE操作宏
*****************************************************************************/
//读取文件检查文件是否存在和权限
#define KC_FILE_READ_ACCESS_CHECK(fullfile) do {	\
	if (0 != KC_ACCESS((fullfile), 0)) {	\
		fprintf(stderr, "file \"%s\" not found.\n", (fullfile));	\
		goto KC_ERROR_CLEAR;	\
	}	\
	if (0 != KC_ACCESS((fullfile), 4)) {	\
		fprintf(stderr, "file \"%s\" access denied.\n", (fullfile));	\
		goto KC_ERROR_CLEAR;\
	}	\
}  while (0)

//写入文件检查文件是否有写入权限
#define KC_FILE_WRITE_ACCESS_CHECK(fullfile) do {	\
	if (0 != KC_ACCESS((fullfile), 2)) {	\
		fprintf(stderr,"file \"%s\" access denied.\n", (fullfile));	\
		goto KC_ERROR_CLEAR;\
	}	\
}  while (0)


#ifdef _MSC_VER
/*打开文件错误检查*/
#	define KC_FILE_OPEN_AND_CHECK(stream,fullfile,flag) do {\
		errno_t errcode;\
		errcode = fopen_s((&(stream)), (fullfile), (flag));\
		if( (errcode) ) {\
			fprintf(stderr,"open file \"%s\" failure.", (fullfile));\
			goto KC_ERROR_CLEAR;\
		}\
	} while (0)
#else
#	define KC_FILE_OPEN_AND_CHECK(stream,fullfile,flag) do {\
		(stream) = fopen((fullfile), (flag));\
		if( NULL == (stream) ) {\
			fprintf(stderr,"open file \"%s\" failure.", (fullfile));\
			goto KC_ERROR_CLEAR;\
		}\
	} while (0)
#endif // _MSC_VER

/*检查文件描述符
https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-constants?view=msvc-170
EMFILE:打开的文件太多。没有更多的文件描述符可用，因此无法打开更多的文件。
ENFILE:系统中打开的文件过多。
*/
#define KC_FILE_DESCRIPTORS_CHECK(fullfile) do {\
	if (errno == EMFILE || errno == ENFILE){\
		fprintf(stderr, "out of file descriptors: \"%s\"; release and retry", (fullfile));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

#endif	/*KC_043055AA_F295_42FC_BE3A_D4B24B4AAE54*/

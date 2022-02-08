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
*	函数修饰符
*****************************************************************************/
#ifdef _MSC_VER
#	ifdef UKCIMPORT
#		define KC_EXTERNAL __declspec(dllimport)
#		define KCAPI __stdcall
#	elif UKCEXPORTS
#		define KC_EXTERNAL __declspec(dllexport)
#		define KCAPI __stdcall
#	else
#		define KC_EXTERNAL
#		define KCAPI
#	endif
#else
#	define KC_EXTERNAL
#	define KCAPI
#endif

/*****************************************************************************
*	我们自定义标准函数返回值定义
*****************************************************************************/
#ifndef KC_OK
#define KC_OK		((int32_t)0)
#endif

#ifndef KC_FAIL
#define KC_FAIL		((int32_t)1)
#endif


/*****************************************************************************
*   检查我们自定义标准函数返回代码是否正确,如果不正确打印异常后信息后跳转至KC_ERROR_CLEAR
*****************************************************************************/
#define KC_CHECK_RCV1(rc,funname) do{\
	if ((rc)){\
		fprintf(stderr,"call function \"%s\" fail.(%s:%d)",(funname),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*	将x转换为字符串
*****************************************************************************/
#define KC_2STR(x) #x

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
*   内存操作是检查是否越界
*****************************************************************************/
#define KC_CHEKC_MEM_ERANGE(rc) do{\
	if ((rc) ){\
		fprintf(stderr,"memory overflow.(%s:%d)",__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*	copy memory (returns ERANGE on overrun, zero if OK)
*****************************************************************************/
#if _MSC_VER >= 1400
# define KC_memcpy(buf, len, src, num) ((buf) && (size_t)(len) >= (size_t)(num) ? memcpy_s((buf), (len), (src), (num)) : ERANGE)
#else
# define KC_memcpy(buf, len, src, num) ((buf) && (size_t)(len) >= (size_t)(num) ? !memcpy((buf), (src), (num)) : ERANGE)
#endif


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

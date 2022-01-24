/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	编码标准宏定义
*****************************************************************************/

#ifndef KC_043055AA_F295_42FC_BE3A_D4B24B4AAE54
#define KC_043055AA_F295_42FC_BE3A_D4B24B4AAE54

#include <stdio.h>
#ifdef _WIN32
#	include <io.h>
#  include <winsock2.h>
#else
#	include <unistd.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#endif
#include <libintl.h>

#include "KcCore.h"
#include "stringbuffer.h"


/*****************************************************************************
*	自定义函数返回值定义
*****************************************************************************/
#ifndef KC_OK
#define KC_OK		((int32_t)0)
#endif

#ifndef KC_FAIL
#define KC_FAIL		((int32_t)1)
#endif

/*****************************************************************************
*	比较字符串是否为NULL或空
*****************************************************************************/
#define KC_STR_ISNULLOREMPTY(str) (NULL== str || '\0'==str[0] )

/*****************************************************************************
*	UDP读写时的缓冲区大小
*	Internet上的标准MTU值为576字节,UDP的数据缓冲区大小控控制在548字节(576-8-20),20字节的IP首部IP协议格式)和8字节的UDP(UDP协议格式)首部.
*****************************************************************************/
#define KC_UDP_BUFFER_SIZE  (548)

/*****************************************************************************
*	文件或目录最大长度
*****************************************************************************/
#define KC_MAX_PATH (1024)

/*****************************************************************************
*	最大允许分配的内存大小（1GB-1）
*****************************************************************************/
#define MaxAllocSize	((size_t) 0x3fffffff) /* 1 gigabyte - 1 */

/*****************************************************************************
*	UTF8 BOM头
*****************************************************************************/
static const char UTF8BOM[4] = { (char)0xEF ,(char)0xBB ,(char)0xBF,(char)0x00 };

/*****************************************************************************
*	将x转换为字符串
*****************************************************************************/
#define KC_2STR(x) #x

/*****************************************************************************
*	生成指定范围的随机数
*****************************************************************************/
#define RANGE_RANDOM(lmin, lmax)(rand() % (lmax + 1 - lmin) + lmin)

/*****************************************************************************
*	浮点与整型转换		注意只能用指针转换
*****************************************************************************/
#define KC_FLOAT_TO_INT32(x) (*((int32_t*)(&x)))
#define KC_INT32_TO_FLOAT(x) (*((float *)((int32_t *)(&x))))
#define KC_FLOAT_TO_UINT32(x) (*((uint32_t*)(&x)))
#define KC_UINT32_TO_FLOAT(x) (*((float *)((uint32_t *)(&x))))
#define KC_DOUBLE_TO_INT64(x) (*((int64_t*)(&x)))
#define KC_INT64_TO_DOUBLE(x) (*((double *)((int64_t *)(&x))))
#define KC_DOUBLE_TO_UINT64(x) (*((uint64_t*)(&x)))
#define KC_UINT64_TO_DOUBLE(x) (*((double *)((uint64_t *)(&x))))

/*****************************************************************************
*	struct timeval转换为整数毫秒
*****************************************************************************/
#define TIMEVAL2MSEC(x) (NULL ==(x) ? 0 : (((int64_t)((x)->tv_sec)) * 1000) + (((int64_t)((x)->tv_usec)) / 1000))

/*****************************************************************************
*	struct timeval转换为struct timespec结构体
*  millisecond要在struct timespec结构体增加的毫秒数，默认为0L
*****************************************************************************/
#define KC_TIMEVAL2TIMESPEC(tv,abs,millisecond) do{ \
	(tv).tv_usec += ((millisecond) * 1000L); \
	(tv).tv_sec += ((tv).tv_usec / 1000000L); \
	(tv).tv_usec %= 1000000L; \
	(abs).tv_sec = (tv).tv_sec; \
	(abs).tv_nsec = (tv).tv_usec * 1000L; \
}while (0)

/*****************************************************************************
*	SOCKET定义
*****************************************************************************/
#ifdef _WIN32
#	define KC_SOCKET_GETLASTERROR WSAGetLastError()

#	define KC_SOCKET_CLOSE(sock) do {\
	if(INVALID_SOCKET!=(sock)) { \
		closesocket((sock)); \
		(sock)=INVALID_SOCKET; \
	} \
} while (0)
#else
typedef int SOCKET;
#	define INVALID_SOCKET  (SOCKET)(~0)
#	define SOCKET_ERROR            (-1)
#	define KC_SOCKET_GETLASTERROR errno
#	define KC_SOCKET_CLOSE(sock) do {\
	if(INVALID_SOCKET!=(sock)) { \
		close((sock)); \
		(sock)=INVALID_SOCKET; \
	} \
} while (0)
#endif
/*****************************************************************************
*	安全释放分配的内容，只能用于本模块内的资源释放
*****************************************************************************/
#define KC_SAFE_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		free((ptr)); (ptr) = NULL; \
	} \
} while (0)

#define KC_SAFE_DELETE(ptr) do {\
	if (NULL !=  (ptr)) {\
		delete (ptr);  (ptr) = NULL;\
	}\
} while (0)

#define KC_JSON_FREE(ptr) do {\
	if (NULL != (ptr)  ) {\
		json_decref((ptr)); (ptr)  = NULL;\
	}\
} while (0);

#define KC_FILE_CLOSE(stream) do {\
	if(NULL !=(stream)  ) { \
		fclose((stream)); (stream) = NULL; \
	} \
} while (0)

#define KC_SAFE_UUID(ptr) do {\
	if(NULL !=(ptr)  ) { \
		uuid_destroy((ptr)); (ptr) = NULL; \
	} \
} while (0)

/*****************************************************************************
*	json是否为NULL
*****************************************************************************/
#define KC_JSON_ISNULL(json) (NULL== (json) || json_is_null((json)))

/*****************************************************************************
*   检查分配内存是否成功
*****************************************************************************/
#define KC_CHECK_MALLOC(ptr,str) do{\
	if (NULL == (ptr) ){\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("out of memory.(%s:%d)"),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*   内存操作是检查是否越界
*****************************************************************************/
#define KC_CHEKC_MEM_ERANGE(rc,str) do{\
	if (rc ){\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("memory overflow.(%s:%d)"),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*   检查是否为NULL指针,如果是直接跳转至异常
*****************************************************************************/
#define KC_CHECK_NULL_PTR(ptr) do{\
	if (NULL ==(ptr))\
		goto KC_ERROR_CLEAR;\
}while (0)

/*****************************************************************************
*   验证输入参数是否为NULL指针,如果是直接跳转至异常
*****************************************************************************/
#define KC_VALIDATE_PARA_PTR(ptr,str) do{\
	if (NULL ==(ptr)){\
			resetStringBuffer((str));\
			appendStringBuffer((str),gettext("invalid argument.(%s:%d)"),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*   验证输入参数是否为0,如果是直接跳转至异常
*****************************************************************************/
#define KC_PARA_LESS_EQUAL_ZERO(para,str) do{\
		if ((para)<=0){\
			resetStringBuffer((str));\
			appendStringBuffer((str),gettext("invalid argument[val<=0].(%s:%d)"),__FILE__,__LINE__);\
			goto KC_ERROR_CLEAR;\
		}\
	}while (0)

/*****************************************************************************
*	数组索引超出数据范围异常消息`
*****************************************************************************/
#define KC_CHECK_ARRAY_INDEX(array_index,array_size,str) do{\
	if ((array_index)<0 || (array_index)>=(array_size)){\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("array subscript exceeds the range of arrays.(%s:%d)"),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*   检查标准函数返回代码是否正确,如果不正确直接跳转至异常
*****************************************************************************/
#define KC_CHECK_RCV1(rc,funname,str) do{\
	if ((rc)){\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("call function \"%s\" fail.(%s:%d)"),(funname),__FILE__,__LINE__);\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)
/*****************************************************************************
*   检查自定义函数返回代码是否正确,如果不正确直接跳转至异常
*****************************************************************************/
#define KC_CHECK_RCV2(rc) do{\
	if ( KC_OK!=(rc))\
		goto KC_ERROR_CLEAR;\
}while (0)
/*****************************************************************************
*   直接跳转换至异常
*****************************************************************************/
#define KC_GOTOERR(str,format, ...) do{\
		resetStringBuffer((str));\
		appendStringBuffer((str),(format), ##__VA_ARGS__);\
		goto KC_ERROR_CLEAR;\
}while (0)

/*****************************************************************************
*   设置的内容超过缓冲区大小
*****************************************************************************/
#define KC_SET_BUFFER_EXCEED(pBufBegin,pBufEnd,bufSize,str) do{\
		if(((pBufEnd)-(pBufBegin)) > bufSize) { \
			resetStringBuffer((str));\
			appendStringBuffer((str), gettext("the contents of the setting exceed the buffer size.(%s:%d)"), __FILE__, __LINE__); \
			goto KC_ERROR_CLEAR;\
		} \
}while (0)
/*****************************************************************************
*   JSON操作宏
*****************************************************************************/
/*加载json时错误检查*/
#define KC_JSON_LOAD_CHECK(doc,jerror,str) do {\
	if( NULL == (doc) ) {\
		resetStringBuffer((str));\
		appendStringBuffer((str), "%s(%d:%d).", (jerror).text, (jerror).line, (jerror).column);\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

/*检查JSON中的项目是否存在*/
#define KC_JSON_NODE_ISNULL(ptr,nodename,str) do {\
	if( NULL == (ptr) ) {\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("json node \"%s\" does not exist."),(nodename));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

/*检查json_t类型是否为string*/
#define KC_JSON_IS_STRING(ptr,nodename,str) do {\
	if (!json_is_string((ptr))) {\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("json item \"%s\" type must is \"string\"."),(nodename));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

/*检查json_t类型是否为integer*/
#define KC_JSON_IS_INTEGER(ptr,nodename,str) do {\
	if (!json_is_integer((ptr))) {\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("json item \"%s\" type must is \"integer\"."),(nodename));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

/*检查json_t类型是否为bool*/
#define KC_JSON_IS_BOOL(ptr,nodename,str) do {\
	if (!json_is_boolean((ptr))) {\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("json item \"%s\" type must is \"boolean\"."),(nodename));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

/*检查字符串是否为NULL或""*/
#define KC_JSON_VALUE_IS_NULL_OR_EMPTY(ptr,nodename,str) do {\
	if (KC_STR_ISNULLOREMPTY((ptr))) {\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("json item \"%s\" is empty."),(nodename));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)



/*****************************************************************************
*   检查UUID函数返回代码是否正确,如果不正确直接跳转至异常
*****************************************************************************/
#define KC_CHECK_UUID_RC(rc,str) do{\
	if (UUID_RC_OK !=(rc)){\
		resetStringBuffer((str));\
		appendStringBuffer((str),gettext("uuid fail:%s"),uuid_error(rc));\
		goto KC_ERROR_CLEAR;\
	}\
}while (0)

/*****************************************************************************
*   FILE操作宏
*****************************************************************************/
//检查文件或目录是否有指定的权限
#define KC_FILE_ACCESS_CHECK(fullfile,flag,isexists,str) do {	\
	if((isexists)){ \
		if (0 != KC_ACCESS((fullfile), 0)) { \
				resetStringBuffer((str));				\
				appendStringBuffer((str), gettext("file \"%s\" not found.\n"), (fullfile));	\
				goto KC_ERROR_CLEAR;	\
		}	\
	} \
	if (0 != KC_ACCESS((fullfile), (flag))) {	\
		resetStringBuffer((str));	\
		appendStringBuffer((str), gettext("file \"%s\" access denied.\n"), (fullfile));	\
		goto KC_ERROR_CLEAR;\
	}	\
}  while (0)

#ifdef _WIN32
/*打开文件并错误检查*/
#	define KC_FILE_OPEN_AND_CHECK(stream,fullfile,flag,str) do {\
		errno_t errcode;\
		errcode = fopen_s((&(stream)), (fullfile), (flag));\
		if( (errcode) ) {\
			resetStringBuffer((str));\
			appendStringBuffer((str), gettext("open file \"%s\" failure."), (fullfile));\
			goto KC_ERROR_CLEAR;\
		}\
	} while (0)
#else
#	define KC_FILE_OPEN_AND_CHECK(stream,fullfile,flag,str) do {\
		(stream) = fopen((fullfile), (flag));\
		if( NULL == (stream) ) {\
			resetStringBuffer((str));\
			appendStringBuffer((str), gettext("open file \"%s\" failure."), (fullfile));\
			goto KC_ERROR_CLEAR;\
		}\
	} while (0)
#endif // _WIN32

/*检查文件描述符
https://docs.microsoft.com/en-us/cpp/c-runtime-library/errno-constants?view=msvc-170
EMFILE:打开的文件太多。没有更多的文件描述符可用，因此无法打开更多的文件。
ENFILE:系统中打开的文件过多。
*/
#define KC_FILE_DESCRIPTORS_CHECK(fullfile,str) do {\
	if (errno == EMFILE || errno == ENFILE){\
		resetStringBuffer((str));\
		appendStringBuffer((str),  gettext("out of file descriptors: \"%s\"; release and retry"), (fullfile));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)

/*移动文件指针检查*/
#define KC_FILE_SEEK_CHECK(rc,fullfile,str) do {\
	if ((rc)){\
		resetStringBuffer((str));\
		appendStringBuffer((str),  gettext("could not seek in file \"%s\""), (fullfile));\
		goto KC_ERROR_CLEAR;\
	}\
} while (0)


#ifdef _WIN32
#   define fseeko(stream, offset, origin) _fseeki64(stream, offset, origin)
#   define ftello(stream) _ftelli64(stream)
#else
#   ifndef fseeko
#       define fseeko(stream, offset, origin) fseeko64(stream, offset, origin)
#   endif
#   ifndef ftello
#       define ftello(stream) ftello64(stream)
#   endif
#endif
/*****************************************************************************
*    copy string (truncating the result)
*****************************************************************************/
/* copy memory (returns ERANGE on overrun, zero if OK) */
#if _MSC_VER >= 1400
# define KC_MEMCPY(buf, len, src, num) ((buf) && (size_t)(len) >= (size_t)(num) ? memcpy_s((buf), (len), (src), (num)) : ERANGE)
#else
# define KC_MEMCPY(buf, len, src, num) ((buf) && (size_t)(len) >= (size_t)(num) ? !memcpy((buf), (src), (num)) : ERANGE)
#endif


#if defined(_WIN32)	
#	define KC_int642str(buf,len,val) _snprintf_s((buf),(len),"%I64d", (val));
#	define KC_uint642str(buf,len,val) _snprintf_s((buf),(len),"%I64u", (val));
#	define KC_str2int64(str,val) sscanf_s((str),"%I64d", (val));
#	define KC_str2uint64(str,val) sscanf_s((str),"%I64u", (val));
#	define KC_str2double(str,val) sscanf_s((str),"%Lf", (val));
#	define KC_fprintf(stream,format, ...) fprintf_s((stream),(format),##__VA_ARGS__);
#else
#	ifdef __x86_64__
#	define KC_int642str(buf,len,val) snprintf((buf),(len),"%ld", (val));
#	define KC_uint642str(buf,len,val) snprintf((buf),(len),"%lu", (val));
#	define KC_str2int64(str,val) sscanf((str), "%ld", (val));
#	define KC_str2uint64(str,val) sscanf((str), "%lu", (val));
#	elif __i386__
#	define KC_int642str(buf,len,val) snprintf((buf),(len),"%lld", (val));
#	define KC_uint642str(buf,len,val) snprintf((buf),(len),"%llu", (val));
#	define KC_str2int64(str,val) sscanf((str), "%lld", (val));
#	define KC_str2uint64(str,val) sscanf((str), "%llu", (val));
#	endif
#	define KC_str2double(str,val) sscanf((str),"%Lf", (val));
#	define KC_fprintf(stream,format, ...) fprintf((stream),(format),##__VA_ARGS__);
#endif


/*****************************************************************************
*	文件夹和文件访问权限及创建目录API
#include <io.h>
00——只检查文件是否存在

02——写权限

04——读权限

06——读写权限
mkdir:创建目录
getcwd:取得当前的工作目录
*****************************************************************************/

/* ----------------------------------------------------------------
 *				Section 9: system-specific hacks
 *
 *		This should be limited to things that absolutely have to be
 *		included in every source file.  The port-specific header file
 *		is usually a better place for this sort of thing.
 * ----------------------------------------------------------------
 */

 /*
  *	NOTE:  this is also used for opening text files.
  *	WIN32 treats Control-Z as EOF in files opened in text mode.
  *	Therefore, we open files in binary mode on Win32 so we can read
  *	literal control-Z.  The other affect is that we see CRLF, but
  *	that is OK because we can already handle those cleanly.
  */
#if defined(_WIN32) || defined(__CYGWIN__)
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

#if defined(_WIN32)
#define KC_ACCESS(_name,_mode) _access((_name),(_mode))
#define KC_MKDIR(_name,_mode) _mkdir((_name))
#define KC_GETCWD(__buf,__size) _getcwd((__buf),(__size))
#else
#define KC_ACCESS(_name,_mode) access((_name),(_mode))
#define KC_MKDIR(_name,_mode) mkdir((_name),(_mode))
#define KC_GETCWD(__buf,__size) getcwd((__buf),(__size))
#endif

  ///* Check for existence */
  //if ((_access("ACCESS.C", 0)) != -1) {
  //	printf("File ACCESS.C exists ");
  //	/* Check for write permission */
  //	if ((_access("ACCESS.C", 2)) != -1)
  //		printf("File ACCESS.C has write permission ");
  //}

#endif	/*KC_043055AA_F295_42FC_BE3A_D4B24B4AAE54*/

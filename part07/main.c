#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libintl.h>
#include <pthread.h>
#ifdef _WIN32
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif
#include <KcCore.h>
#include <stringbuffer.h>
#include <instr_time.h>
#include <KcUtility.h>
#include <KcShared.h>

#define KC_BACKEND_ITEM_COUNT (1024)

#define KC_FIND_ITEM_FLAG (1022)

//udp server需要用到的结构体
struct  KC_BACKEND_ITEM {
	int32_t flag;

	int32_t recvLen;
	char szRecvBuf[KC_UDP_BUFFER_SIZE];

	int32_t sendLen;
	char szSendBuf[KC_UDP_BUFFER_SIZE];
};

#define KC_SAFE_FREE_TEST1(ptr) do {\
	if (NULL != (ptr)  ) {\
		kcSafeFreeTest1((ptr)); (ptr)  = NULL;\
	}\
} while (0);

void kcSafeFreeTest1(struct  KC_BACKEND_ITEM** ptr) {
	if (NULL == ptr)
		return;
	for (int32_t i = 0; i < KC_BACKEND_ITEM_COUNT; ++i) {
		struct  KC_BACKEND_ITEM* item = ptr[i];
		if (NULL != item)
			free(item);
	}
	free(ptr);
}

//数组常规的内存分配方式,高级语言java、C#数据创建都是用这种方式
int32_t test1(StringBuffer str) {
	struct  KC_BACKEND_ITEM** items = NULL;
	struct  KC_BACKEND_ITEM* findItem;
	instr_time before, after;
	double elapsed_msec = 0;

	/*****************************************************************************
	*   为数组分配内存
	*****************************************************************************/
	INSTR_TIME_SET_CURRENT(before);	
	//String []vals = new String[1024]
	//在C语言中数组一般用calloc分配空间，它会将所分配的空间中的内容全部设置为0，方便更安全的释放内存
	items = (struct  KC_BACKEND_ITEM**)calloc(KC_BACKEND_ITEM_COUNT, sizeof(struct  KC_BACKEND_ITEM*));
	KC_CHECK_MALLOC(items, str);	
	for (int32_t i = 0; i < KC_BACKEND_ITEM_COUNT; ++i) {
		//因为malloc只分配了内存空间，但所分配的空间中的内容是随机的，如果不初始化items[i]中的项，items[i]中的项是野指针
		//有两种方法初始化分配的空间中的内容
		//	1.memset，因为memset对整个struct  KC_BACKEND_ITEM都要初始化一次
		//			同时struct  KC_BACKEND_ITEM较大（548*2+12=1108字节），因此使用memset效率较差
		//	2.自己手动初始化，建议的方法
		//vals[i] = new String();
		items[i] = (struct  KC_BACKEND_ITEM *)malloc(sizeof(struct  KC_BACKEND_ITEM));
		KC_CHECK_MALLOC(items[i], str);
		//memset在这里不建议使用
		//memset(items[i], 0, sizeof(struct  KC_BACKEND_ITEM));
		//手动初始化，只需要操作14字节即可
		items[i]->flag = i;
		items[i]->recvLen = 0;
		items[i]->szRecvBuf[0] = '\0';
		items[i]->sendLen = 0;
		items[i]->szSendBuf[0] = '\0';
	}
	findItem = items[0];
	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "test1 malloc %s\n", str->data);
	/*****************************************************************************
	*   查找数组中的flag==KC_FIND_ITEM_FLAG的项目
	*	此中方法随机读取内存，性能较顺序读取内存差
	*****************************************************************************/
	INSTR_TIME_SET_CURRENT(before);
	for (size_t i = 0; i < KC_BACKEND_ITEM_COUNT; ++i) {
		if (KC_FIND_ITEM_FLAG == items[i]->flag) {
			findItem = items[i];
			break;
		}
	}	
	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "test1 flag %d\n", findItem->flag);
	fprintf(stdout, "test1 find %s\n", str->data);
	KC_SAFE_FREE_TEST1(items);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE_TEST1(items);
	return KC_FAIL;
}


//注意kcSafeFreeTest2的释放方法也不同于kcSafeFreeTest1的
#define KC_SAFE_FREE_TEST2(ptr) do {\
	if (NULL != (ptr)  ) {\
		kcSafeFreeTest2((ptr)); (ptr)  = NULL;\
	}\
} while (0);

void kcSafeFreeTest2(struct  KC_BACKEND_ITEM** ptr) {
	if (NULL == ptr)
		return;
	free(ptr);
}

//数组顺序分配内存
int32_t test2(StringBuffer str) {
	struct  KC_BACKEND_ITEM** items = NULL;
	struct  KC_BACKEND_ITEM* firstAddress;
	instr_time before, after;
	double elapsed_msec = 0;


	/*****************************************************************************
	*   为数组分配内存
	*****************************************************************************/
	INSTR_TIME_SET_CURRENT(before);
	//在C语言中数组一般用calloc分配空间，它会将所分配的空间中的内容全部设置为0，方便更安全的释放内存
	//此种方式的优点是顺序查找时速度较快
	//注意这里一次性分配了至少(KC_BACKEND_ITEM_COUNT*(548*2+12))=1,134,592字节	
	items = (struct  KC_BACKEND_ITEM**)calloc(KC_BACKEND_ITEM_COUNT, sizeof(struct  KC_BACKEND_ITEM));
	KC_CHECK_MALLOC(items, str);
	//获取数组中第一个元素的指针
	firstAddress = (struct  KC_BACKEND_ITEM*)items;
	for (int32_t i = 0; i < KC_BACKEND_ITEM_COUNT; ++i,++firstAddress) {
		//不再需要分配内存和初始化分配的空间中的内容，因为已经calloc一次性分配好并将初始化分配的空间中的内容设置0了
		firstAddress->flag = i;
	}
	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "test2 malloc %s\n", str->data);
	/*****************************************************************************
	*   查找数组中的flag==KC_FIND_ITEM_FLAG的项目
	*	此中方法顺序读取内存查找时速度较快
	*****************************************************************************/
	INSTR_TIME_SET_CURRENT(before);
	//查找前获取数组中第一个元素的指针
	firstAddress = (struct  KC_BACKEND_ITEM*)items;
	for (size_t i = 0; i < KC_BACKEND_ITEM_COUNT; ++i,++firstAddress) {
		if (KC_FIND_ITEM_FLAG == firstAddress->flag)
			break;
	}	
	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "test2 flag %d\n", firstAddress->flag);
	fprintf(stdout, "test2 find %s\n", str->data);
	KC_SAFE_FREE_TEST2(items);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE_TEST2(items);
	return KC_FAIL;
}

int main(int argc, char* argv[]) {	
	StringBuffer str = NULL;

	str = createStringBuffer();
	KC_CHECK_RCV2(test1(str));
	fprintf(stdout, "\n", str->data);
	KC_CHECK_RCV2(test2(str));
	KC_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	KC_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

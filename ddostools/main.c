#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libintl.h>
#include <pthread.h>
#ifdef _WIN32
#	include <winsock2.h>
#	include <conio.h>
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif
#include "pg_bswap.h"
#include "stringbuffer.h"
#include "KcUtility.h"

//WinSocket是否初始化
#ifdef _WIN32
#	define KC_IS_WSA_STARTUP		(1<<0)
#endif

/*
*	服务工作队列大小为服务工作线程数 * 16
*	如果服务和测试程序在一台机器上，那么线程数使用cpu内核数-(服务器工作线程数 * 2)
*	如果服务和测试程序在不同的机器上
*		服务和数据库在一台主机上，服务的工作线程数为内核的数量，
*		服务和数据库不在一台主机上，服务的工作线程数为内核的数量 * 2，
*		测试程序则使用本机的全部cpu
*/
#define KC_THREAD_COUNT		(16)



/*****************************************************************************
*   启动时创建的对象标志
*****************************************************************************/
#define KC_SHARED_MUTEX										(1<<0)
#define KC_SHARED_COND											(1<<1)
#define KC_SHARED_THREAD										(1<<2)


#define KC_SOCKET_CLOSE(sock) do {\
	if(INVALID_SOCKET!=(sock)) { \
		closesocket((sock));\
		(sock)=INVALID_SOCKET; \
	} \
} while (0)

struct KC_SHARED {

	/// <summary>
	/// 初始化标志
	/// </summary>
	int32_t flag;

	/// <summary>
	/// 已经运行的线程数量
	/// </summary>
	uint8_t threadRunCount;

	/// <summary>
	/// 互斥对象
	/// </summary>
	pthread_mutex_t* mutex;

	/// <summary>
	/// 条件变量
	/// </summary>
	pthread_cond_t* cond;

	/// <summary>
	/// 服务是否运行
	/// </summary>
	bool isRuning;

	uint64_t errorCount;

	uint64_t sendCount;

};



void kcFreeThreads(pthread_t** threads, uint8_t threadPool) {
	if (NULL == threads)
		return;
	for (uint8_t i = 0; i < threadPool; ++i) {
		if (NULL != threads[i]) {
			pthread_join(*threads[i], NULL);
			free(threads[i]);
		}
	}
	free(threads);
}

void kcFreeShared(struct KC_SHARED* shared) {
	if (NULL == shared)
		return;

	if (NULL != shared->cond) {
		if (KC_SHARED_COND == (shared->flag & KC_SHARED_COND))
			pthread_cond_destroy(shared->cond);
		free(shared->cond);
		shared->cond = NULL;
	}


	if (NULL != shared->mutex) {
		if (KC_SHARED_MUTEX == (shared->flag & KC_SHARED_MUTEX))
			pthread_mutex_destroy(shared->mutex);
		free(shared->mutex);
		shared->mutex = NULL;
	}

	free(shared);
}

int32_t kcGenerateSocket(StringBuffer error, SOCKET* ptr) {
	int32_t rc, opt = 1;
	SOCKET sock = INVALID_SOCKET;
	int32_t timeout = 1000;
	const uint32_t BUFFER_SIZE = KC_UDP_BUFFER_SIZE;

	//创建套接字
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == sock)
		KC_GOTOERR(error, "socket failed with error %d.", WSAGetLastError());
	//设置发送数据时的缓冲区大小
	rc = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&BUFFER_SIZE, (int32_t)sizeof(uint32_t));
	if (rc)
		KC_GOTOERR(error, "setsockopt SO_SNDBUF failed with error %d\n", WSAGetLastError());
	//设置发送数据时的超时时间
	rc = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, (int32_t)sizeof(int32_t));
	if (rc)
		KC_GOTOERR(error, "set send timeout failed with error %d\n", WSAGetLastError());
	//设置接收数据时的缓冲区大小
	rc = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&BUFFER_SIZE, (int32_t)sizeof(uint32_t));
	if (rc)
		KC_GOTOERR(error, "setsockopt SO_RCVBUF failed with error %d.", WSAGetLastError());
	//设置接收超时时间
	rc = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, (int32_t)sizeof(int32_t));
	if (INVALID_SOCKET == sock)
		KC_GOTOERR(error, "set send timeout failed with error %d.", WSAGetLastError());
	(*ptr) = sock;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SOCKET_CLOSE(sock);
	return KC_FAIL;
}




void kcThreadVoid(void* obj) {
	int32_t rc;
	bool isRuning = true;
	StringBuffer str = NULL;
	struct KC_SHARED* const shared = (struct KC_SHARED* const)obj;
	SOCKET sock = INVALID_SOCKET;

	char sendData[] = {
		0x05,0x51,0x01,0x03,0x08,0x41,0xDC,0x00,0x00,0x42,0x8C,0xCC,0xCD,0x0C,0xE0
	};

	char szRecv[KC_UDP_BUFFER_SIZE];
	struct sockaddr_in sendAdd, recvAdd;
	int32_t addressSize = sizeof(struct sockaddr_in);


	str = createStringBuffer();
	rc = kcGenerateSocket(str, &sock);
	KC_CHECK_RCV2(rc);
	sendAdd.sin_family = AF_INET;  //使用IPv4地址
	sendAdd.sin_addr.s_addr = pg_bswap32(((192 << 24) | (168 << 16) | (1 << 8) | (2 << 0))); //192.168.1.2
	sendAdd.sin_port = pg_bswap16(55431);

	recvAdd.sin_family = AF_INET;

	pthread_mutex_lock(shared->mutex);
	pthread_cond_signal(shared->cond); //通知主线程侦听线程已启动
	pthread_mutex_unlock(shared->mutex);

	while (isRuning) {
		rc = sendto(sock, sendData, sizeof(sendData), 0, (const struct sockaddr*)&sendAdd, sizeof(struct sockaddr_in));
		if (SOCKET_ERROR != rc) {
			rc = recvfrom(sock, szRecv, KC_UDP_BUFFER_SIZE, 0, (struct sockaddr*)(&recvAdd), &addressSize);
			if (SOCKET_ERROR == rc) {
				pthread_mutex_lock(shared->mutex);
				++shared->errorCount;
				printf("recv fail.(%I64d:%I64d:%d)\n", shared->sendCount + 1, shared->errorCount, WSAGetLastError());
			} else {
				pthread_mutex_lock(shared->mutex);
			}
		} else {
			pthread_mutex_lock(shared->mutex);
		}
		++shared->sendCount;
		isRuning = shared->isRuning;
		pthread_mutex_unlock(shared->mutex);
	}
	KC_SOCKET_CLOSE(sock);
	KC_STRINGBUF_FREE(str);
	return;
KC_ERROR_CLEAR:
	KC_SOCKET_CLOSE(sock);
	fprintf(stderr, "%s\n", str->data);
	pthread_mutex_lock(shared->mutex);
	shared->isRuning = false;
	pthread_cond_signal(shared->cond); //通知主线程侦听线程已启动
	pthread_mutex_unlock(shared->mutex);
	KC_STRINGBUF_FREE(str);
}

int32_t kcStartThread(struct KC_SHARED* shared, StringBuffer error, pthread_t*** ptr) {
	int32_t rc;
	pthread_t** threads = NULL;

	threads = (pthread_t**)calloc(KC_THREAD_COUNT, sizeof(pthread_t*));
	KC_CHECK_MALLOC(threads, error);
	for (uint8_t i = 0; i < KC_THREAD_COUNT; ++i) {
		threads[i] = (pthread_t*)malloc(sizeof(pthread_t));
		KC_CHECK_MALLOC(threads[i], error);
		rc = pthread_create(threads[i], NULL, (void* (*)(void*))kcThreadVoid, shared);
		KC_CHECK_RCV1(rc, KC_2STR(pthread_create), error);
		pthread_mutex_lock(shared->mutex);
		pthread_cond_wait(shared->cond, shared->mutex);
		if (!shared->isRuning)
			goto KC_ERROR_CLEAR;
		++shared->threadRunCount;
		pthread_mutex_unlock(shared->mutex);
	}
	//等待所有工作线程启动成功
	pthread_mutex_lock(shared->mutex);
	if (shared->threadRunCount != KC_THREAD_COUNT) {
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("start work thread fail."));
		goto KC_ERROR_CLEAR;
	}
	pthread_mutex_unlock(shared->mutex);
	(*ptr) = threads;
	return KC_OK;
KC_ERROR_CLEAR:
	if (NULL != threads)
		kcFreeThreads(threads, 12);
	return KC_FAIL;
}

void kcStopThread(struct KC_SHARED* shared, pthread_t** threads) {
	if (NULL == shared || NULL == threads)
		return;
	pthread_mutex_lock(shared->mutex);
	shared->isRuning = false;
	pthread_mutex_unlock(shared->mutex);
	kcFreeThreads(threads, KC_THREAD_COUNT);
}

int32_t kcGenerateShared(StringBuffer str, struct KC_SHARED** ptr) {
	int32_t rc;
	struct KC_SHARED* shared = NULL;

	//创建共享对象
	shared = (struct KC_SHARED*)calloc(1, sizeof(struct KC_SHARED));
	KC_CHECK_MALLOC(shared, str);
	shared->isRuning = true;
	//创建互斥对象
	shared->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	KC_CHECK_MALLOC(shared->mutex, str);
	rc = pthread_mutex_init(shared->mutex, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_mutex_init), str);
	shared->flag += KC_SHARED_MUTEX;
	//创建条件变量
	shared->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	KC_CHECK_MALLOC(shared->cond, str);
	rc = pthread_cond_init(shared->cond, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_cond_init), str);
	shared->flag += KC_SHARED_COND;

	(*ptr) = shared;
	return KC_OK;
KC_ERROR_CLEAR:
	if (NULL != shared)
		kcFreeShared(shared);
	return KC_FAIL;
}

int main(int argc, char* argv[]) {
	StringBuffer str = NULL;
	struct KC_SHARED* shared = NULL;
	pthread_t** threads = NULL;
	int ch;
#ifdef _WIN32
	int32_t rc, flag = 0;
	WSADATA w;
#endif

	str = createStringBuffer();
	//初始化Winsock
#ifdef _WIN32
	if ((rc = WSAStartup(MAKEWORD(2, 2), &w))) {
		resetStringBuffer(str);
		appendStringBuffer(str, gettext("WSAStartup failed with error: %s."), rc);
		goto KC_ERROR_CLEAR;
	}
	flag += KC_IS_WSA_STARTUP;
	if (2 != LOBYTE(w.wVersion) || 2 != HIBYTE(w.wVersion)) {
		resetStringBuffer(str);
		appendStringBufferStr(str, gettext("could not find a usable version(2.2) of Winsock.dll"));
		goto KC_ERROR_CLEAR;
	}
#endif
	rc = kcGenerateShared(str, &shared);
	KC_CHECK_RCV2(rc);
	rc = kcStartThread(shared, str, &threads);
	KC_CHECK_RCV2(rc);
	do {
		ch = _getch();
	} while (27 != ch);
	kcStopThread(shared, threads);
	printf("%I64d:%I64d\n", shared->sendCount + 1, shared->errorCount);
	kcFreeShared(shared);
	KC_STRINGBUF_FREE(str);
#ifdef WIN32
	WSACleanup();
#endif
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	printf(str->data);
	if (NULL != shared && NULL != threads)
		kcStopThread(shared, threads);
	if (NULL != shared)
		kcFreeShared(shared);
	KC_STRINGBUF_FREE(str);
#ifdef WIN32
	if (KC_IS_WSA_STARTUP == (flag & KC_IS_WSA_STARTUP))
		WSACleanup();
#endif	
	return EXIT_FAILURE;
}

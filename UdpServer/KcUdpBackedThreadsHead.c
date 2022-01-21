#ifdef _WIN32
#include <WS2tcpip.h>
#endif // _WIN32

#include "pg_bswap.h"
#include "KcUdpBackedThreadsHead.h"
#include "KcUtility.h"
#include "KcUdpBackedThreads.h"
#include "KcPlugins.h"


#define KC_SAFE_QUEUE_FREE(config,ptr) do {\
	if (NULL !=  (ptr)) {\
		kcQueueFree((config),(ptr));  (ptr) = NULL;\
	}\
} while (0)

#define KC_SAFE_THREAD_FREE(ptr) do {\
	if (NULL !=  (ptr)) {\
		pthread_join(*(ptr), NULL); \
		free((ptr)); \
		(ptr) = NULL; \
	}\
} while (0)

#define KC_SAFE_THREAD_WORKS_FREE(ptr) do {\
	if (NULL !=  (ptr)) {\
		kcThreadWorksFree((ptr));  (ptr) = NULL;\
	}\
} while (0)


#define KC_SAFE_SHARED_FREE(ptr) do {\
	if (NULL !=  (ptr)) {\
		kcSharedFree((ptr));  (ptr) = NULL;\
	}\
} while (0)

extern void kcUdpBackendThreadRecv(void* obj);
extern void kcUdpBackendThreadWorks(void* obj);
void kcSharedFree(struct KC_SHARED* shared);
void kcThreadWorksFree(pthread_t** threadWorks);
void kcQueueFree(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM** items);

/*****************************************************************************
*   全局对象,慎用
*****************************************************************************/
struct KC_SHARED* global_shared = NULL;

#pragma region  创建队列和队列中的项

int32_t kcGenerateSendSocketRaw(const struct KC_CONFIG* const config, StringBuffer error, SOCKET* ptr) {
	int32_t rc, hdrincl = 1;
	SOCKET sockRaw = INVALID_SOCKET;
	const uint32_t BUFFER_SIZE = KC_UDP_BUFFER_SIZE;
#ifdef __GNUC__
	struct timeval timeoutSend, timeoutRecv;
#endif
	//创建原始套接字，原始套接字只能以管理员身份运行
	sockRaw = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
	if (INVALID_SOCKET == sockRaw)
		KC_GOTOERR(error, "socket failed with error %d.", KC_SOCKET_GETLASTERROR);

	//设置IP数据包格式,告诉系统内核模块IP数据包由我们自己来填写
	rc = setsockopt(sockRaw, IPPROTO_IP, IP_HDRINCL, (const char*)&hdrincl, (int32_t)sizeof(uint32_t));
	if (rc)
		KC_GOTOERR(error, "setsockopt IP_HDRINCL failed with error %d\n", KC_SOCKET_GETLASTERROR);
	//设置发送数据时的缓冲区大小
	rc = setsockopt(sockRaw, SOL_SOCKET, SO_SNDBUF, (const char*)&BUFFER_SIZE, (int32_t)sizeof(uint32_t));
	if (rc)
		KC_GOTOERR(error, "setsockopt SO_SNDBUF failed with error %d\n", KC_SOCKET_GETLASTERROR);
	//设置发送数据时的超时时间
#ifdef _WIN32
	rc = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, (const char*)&config->sendTimeout, (int32_t)sizeof(int32_t));
	if (rc)
		KC_GOTOERR(error, "set send timeout failed with error %d\n", KC_SOCKET_GETLASTERROR);
#else
	timeoutSend.tv_sec = config->sendTimeout / 1000L;
	timeoutSend.tv_usec = 0;
	rc = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO, &timeoutSend, sizeof(struct timeval));
	if (rc)
		KC_GOTOERR(error, "set send timeout failed with error %d\n", KC_SOCKET_GETLASTERROR);
#endif

	(*ptr) = sockRaw;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SOCKET_CLOSE(sockRaw);
	return KC_FAIL;
}

int32_t kcQueueCreate(struct KC_SHARED* const shared, StringBuffer error) {
	struct KC_BACKEND_ITEM* backendItem;
	shared->items = (struct KC_BACKEND_ITEM**)calloc(shared->config->queue_max, sizeof(struct KC_BACKEND_ITEM));
	KC_CHECK_MALLOC(shared->items, error);
	backendItem = (struct KC_BACKEND_ITEM*)shared->items;
	for (uint16_t i = 0; i < shared->config->queue_max; ++i, ++backendItem) {
		backendItem->flag = KC_BACKEND_ITEM_FLAG_NORMAL;
		backendItem->recv = createStringBuffer();
		if (!enlargeStringBuffer(backendItem->recv, KC_UDP_BUFFER_SIZE)) {
			resetStringBuffer(error);
			appendStringBufferStr(error, gettext("out of memory."));
			goto KC_ERROR_CLEAR;
		}
		KC_CHECK_RCV2(kcGenerateSendSocketRaw(shared->config, error, &backendItem->sockRaw));
	}
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_QUEUE_FREE(shared->config, shared->items);
	return KC_FAIL;
}
#pragma endregion

#pragma region  创建工作线程

int32_t kcThreadWorksCreate(struct KC_SHARED* const shared, StringBuffer error) {
	int32_t rc = KC_OK;

	shared->threadWorks = (pthread_t**)calloc(shared->config->threadPoolSize + 1, sizeof(pthread_t*));
	KC_CHECK_MALLOC(shared->threadWorks, error);
	for (uint8_t i = 0; i < shared->config->threadPoolSize; ++i) {
		shared->threadWorks[i] = (pthread_t*)malloc(sizeof(pthread_t));
		KC_CHECK_MALLOC(shared->threadWorks[i], error);
		rc = pthread_create(shared->threadWorks[i], NULL, (void* (*)(void*))kcUdpBackendThreadWorks, shared);
		KC_CHECK_RCV1(rc, KC_2STR(pthread_create), error);
		pthread_mutex_lock(shared->mutex);
		pthread_cond_wait(shared->condStart, shared->mutex);
		if (!shared->isRuning) {
			pthread_mutex_unlock(shared->mutex);
			resetStringBuffer(error);
			appendStringBufferStr(error, gettext("start work thread listen fial."));
			goto KC_ERROR_CLEAR;
		} else {
			++shared->threadRunCount;
			pthread_mutex_unlock(shared->mutex);
		}
	}
	//等待所有工作线程启动成功
	pthread_mutex_lock(shared->mutex);
	if (shared->threadRunCount != shared->config->threadPoolSize) {
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("start work thread fail."));
		goto KC_ERROR_CLEAR;
	}
	pthread_mutex_unlock(shared->mutex);
	return KC_OK;
KC_ERROR_CLEAR:
	pthread_mutex_lock(shared->mutex);
	shared->isRuning = 0;
	pthread_cond_broadcast(shared->cond);
	pthread_mutex_unlock(shared->mutex);
	KC_SAFE_THREAD_WORKS_FREE(shared->threadWorks);
	return KC_FAIL;
}

#pragma endregion

#pragma region  启动和停止(主线程)

int32_t kcUdpBackedThreadStart(const struct KC_CONFIG* const config, StringBuffer error) {
	int32_t rc = 0;
	struct KC_SHARED* shared = NULL;

	//创建共享对象
	shared = (struct KC_SHARED*)calloc(1, sizeof(struct KC_SHARED));
	KC_CHECK_MALLOC(shared, error);
	global_shared = shared;
	shared->config = config;
	shared->isRuning = 1;
	//创建互斥对象
	shared->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	KC_CHECK_MALLOC(shared->mutex, error);
	rc = pthread_mutex_init(shared->mutex, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_mutex_init), error);
	shared->flag += KC_SHARED_MUTEX;
	//创建条件变量	
	shared->cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	KC_CHECK_MALLOC(shared->cond, error);
	rc = pthread_cond_init(shared->cond, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_cond_init), error);
	shared->flag += KC_SHARED_COND;

	shared->condStart = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	KC_CHECK_MALLOC(shared->condStart, error);
	rc = pthread_cond_init(shared->condStart, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_cond_init), error);
	shared->flag += KC_SHARED_CONDSTART;
	//创建读写锁
	shared->rwlock = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
	KC_CHECK_MALLOC(shared->rwlock, error);
	rc = pthread_rwlock_init(shared->rwlock, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_rwlock_init), error);
	shared->flag += KC_SHARED_RWLOCK;
	//解析数据使用的插件
	rc = kcServerPluginsLoad(config, error, &shared->plugins);
	KC_CHECK_RCV2(rc);
	//创建队列
	rc = kcQueueCreate(shared, error);
	KC_CHECK_RCV2(rc);
	//创建工作线程
	rc = kcThreadWorksCreate(shared, error);
	KC_CHECK_RCV2(rc);
	//启动侦听线程并等待
	shared->threadRecv = (pthread_t*)malloc(sizeof(pthread_t));
	KC_CHECK_MALLOC(shared->threadRecv, error);
	rc = pthread_create(shared->threadRecv, NULL, (void* (*)(void*))kcUdpBackendThreadRecv, shared);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_create), error);
	pthread_mutex_lock(shared->mutex);
	pthread_cond_wait(shared->condStart, shared->mutex);
	if (!shared->isRuning) {
		pthread_mutex_unlock(shared->mutex);
		resetStringBuffer(error);
		appendStringBufferStr(error, gettext("start udp listen fial."));
		goto KC_ERROR_CLEAR;
	} else {
		pthread_mutex_unlock(shared->mutex);
	}
	return KC_OK;
KC_ERROR_CLEAR:
	kcUdpBackedThreadStop();
	return KC_FAIL;
}


void kcUdpBackedThreadStop() {
	SOCKET sock = INVALID_SOCKET;
	const uint32_t BUFFER_SIZE = KC_UDP_BUFFER_SIZE;
	struct sockaddr_in sendAdd;
	struct KC_SHARED* shared = global_shared;

	if (NULL != shared) {
		pthread_mutex_lock(shared->mutex);
		shared->isRuning = 0;
		pthread_cond_broadcast(shared->condStart);
		pthread_cond_broadcast(shared->cond);
		pthread_mutex_unlock(shared->mutex);

		//发送停止信号
		if (0 == shared->config->addres.sin_addr.s_addr) {
			sendAdd.sin_family = AF_INET;  //使用IPv4地址
			sendAdd.sin_addr.s_addr = pg_bswap32(((127 << 24) | (0 << 16) | (0 << 8) | (1 << 0))); //127.0.0.1
			sendAdd.sin_port = shared->config->addres.sin_port;
		} else {
			sendAdd = shared->config->addres;
		}
		sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (INVALID_SOCKET != sock) {
			setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&BUFFER_SIZE, sizeof(int32_t));
			setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&shared->config->sendTimeout, sizeof(int32_t));
			sendto(sock, "\0", 1, 0, (const struct sockaddr*)(&sendAdd), sizeof(struct sockaddr_in));
		}
		KC_SOCKET_CLOSE(sock);
	}

	KC_SAFE_SHARED_FREE(global_shared);
}

#pragma endregion

#pragma region 释放资源

void kcQueueFree(const struct KC_CONFIG* const config, struct KC_BACKEND_ITEM** items) {
	struct KC_BACKEND_ITEM* backendItem;
	if (NULL == items)
		return;
	backendItem = (struct KC_BACKEND_ITEM*)items;
	for (uint16_t i = 0; i < config->queue_max; ++i, ++backendItem) {
		if (NULL != backendItem) {
			KC_SOCKET_CLOSE(backendItem->sockRaw);
			KC_STRINGBUF_FREE(backendItem->recv);
		}
	}
	free(items);
}

void kcThreadWorksFree(pthread_t** threadWorks) {
	uint32_t i;
	pthread_t* pthread;
	if (NULL != threadWorks) {
		i = 0;
		while (NULL != (pthread = threadWorks[i++]))
			KC_SAFE_THREAD_FREE(pthread);
		free(threadWorks);
	}
};

void kcSharedFree(struct KC_SHARED* shared) {
	struct KC_BACKEND_THREAD_ITEM* item = NULL;
	if (NULL == shared)
		return;
	KC_SAFE_THREAD_FREE(shared->threadRecv);
	KC_SAFE_THREAD_WORKS_FREE(shared->threadWorks);
	KC_SAFE_QUEUE_FREE(shared->config, shared->items);
	KC_SAFE_UNLOAD_SERVER_PLUGINS(shared->plugins);

	if (NULL != shared->rwlock) {
		if (KC_SHARED_RWLOCK == (shared->flag & KC_SHARED_RWLOCK))
			pthread_rwlock_destroy(shared->rwlock);
		free(shared->rwlock);
		shared->rwlock = NULL;
	}

	if (NULL != shared->condStart) {
		if (KC_SHARED_CONDSTART == (shared->flag & KC_SHARED_CONDSTART))
			pthread_cond_destroy(shared->condStart);
		free(shared->condStart);
		shared->condStart = NULL;
	}


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

#pragma endregion

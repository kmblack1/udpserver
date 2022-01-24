#include "pg_bswap.h"

#include "KcUtility.h"
#include "KcShared.h"
//#include "KcLibpq.h"
#include "KcUdpBackedThreads.h"
#include "KcPlugins.h"


#pragma region  UDP侦听(侦听线程)

int32_t kcGenerateRevcSocket(const struct KC_CONFIG* const config, StringBuffer error, SOCKET* ptr) {
	int32_t rc, opt = 1;
	SOCKET sock = INVALID_SOCKET;
	const uint32_t BUFFER_SIZE = KC_UDP_BUFFER_SIZE;

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == sock)
		KC_GOTOERR(error, "socket failed with error %d.", KC_SOCKET_GETLASTERROR);
	//设置接收数据时的缓冲区大小
	rc = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&BUFFER_SIZE, (int32_t)sizeof(uint32_t));
	if (rc)
		KC_GOTOERR(error, "setsockopt SO_RCVBUF failed with error %d.", KC_SOCKET_GETLASTERROR);

	//绑定IP和端口
	rc = bind(sock, (const struct sockaddr*)(&config->addres), (int32_t)sizeof(struct sockaddr_in));
	if (rc == SOCKET_ERROR)
		KC_GOTOERR(error, "bind failed with error %d.", KC_SOCKET_GETLASTERROR);
	(*ptr) = sock;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SOCKET_CLOSE(sock);
	return KC_FAIL;
}

struct KC_BACKEND_ITEM* kcUdpEnqueue(struct KC_SHARED* shared) {
	struct KC_BACKEND_ITEM* backendItem;

	pthread_mutex_lock(shared->mutex);
	for (;;) {
		backendItem = (struct KC_BACKEND_ITEM*)shared->items;
		for (uint16_t i = 0; i < shared->config->queue_max; ++i, ++backendItem) {
			if (KC_BACKEND_ITEM_FLAG_NORMAL == backendItem->flag) {
				backendItem->flag |= KC_BACKEND_ITEM_FLAG_RECV_BEGIN;
				pthread_mutex_unlock(shared->mutex);
				return shared->isRuning ? backendItem : NULL;
			}
		}
		pthread_cond_wait(shared->cond, shared->mutex);
		if (!shared->isRuning) {
			pthread_mutex_unlock(shared->mutex);
			return NULL;
		}
	}
}

void kcUdpBackendThreadRecv(void* obj) {
	int32_t rc;
	int32_t send_add_size = sizeof(struct sockaddr_in);
	SOCKET sock = INVALID_SOCKET;
	StringBuffer str = NULL;
	struct KC_BACKEND_ITEM* backendItem;
	struct KC_SHARED* const shared = (struct KC_SHARED* const)obj;



	str = createStringBuffer();
	rc = kcGenerateRevcSocket(shared->config, str, &sock);
	KC_CHECK_RCV2(rc);

	pthread_mutex_lock(shared->mutex);
	pthread_cond_signal(shared->condStart); //通知主线程侦听线程已启动
	pthread_mutex_unlock(shared->mutex);

	for (;;) {
		backendItem = kcUdpEnqueue(shared);
		if (NULL == backendItem)
			break;
		rc = recvfrom(sock, backendItem->recv->data, KC_UDP_BUFFER_SIZE, 0, (struct sockaddr*)(&(backendItem->sender)), &send_add_size);
		if (SOCKET_ERROR == rc) { //接收数据失败，重置队列中的项
			resetStringBuffer(backendItem->recv);
			pthread_mutex_lock(shared->mutex);
			if (shared->isRuning) {
				backendItem->flag = KC_BACKEND_ITEM_FLAG_NORMAL;
				pthread_mutex_unlock(shared->mutex);
			} else {
				pthread_mutex_unlock(shared->mutex);
				break;
			}
		} else {
			backendItem->recv->len = rc;
			pthread_mutex_lock(shared->mutex);
			if (shared->isRuning) {
				backendItem->flag |= KC_BACKEND_ITEM_FLAG_RECV_AFTER;
				pthread_cond_signal(shared->cond);
				pthread_mutex_unlock(shared->mutex);
			} else {
				pthread_mutex_unlock(shared->mutex);
				break;
			}
		}
	}
	KC_SOCKET_CLOSE(sock);
	KC_SAFE_STRINGBUF_FREE(str);
	return;
KC_ERROR_CLEAR:
	KC_SOCKET_CLOSE(sock);
	kcSaveLog(shared->config->log_path, str, NULL);
	KC_SAFE_STRINGBUF_FREE(str);

	pthread_mutex_lock(shared->mutex);
	shared->isRuning = 0;
	pthread_cond_signal(shared->condStart); //通知主线程侦听线程已启动
	pthread_mutex_unlock(shared->mutex);
}

#pragma endregion

#pragma region  工作线程，必须在侦听线程之前启动

struct KC_BACKEND_ITEM* kcUdpDequeue(struct KC_SHARED* const shared) {
	struct KC_BACKEND_ITEM* backendItem;

	pthread_mutex_lock(shared->mutex);
	for (;;) {
		backendItem = (struct KC_BACKEND_ITEM*)shared->items;
		for (uint16_t i = 0; i < shared->config->queue_max; ++i, ++backendItem) {
			if (KC_BACKEND_ITEM_FLAG_RECV_COMPLETE == backendItem->flag) {
				if (shared->isRuning) {
					backendItem->flag |= KC_BACKEND_ITEM_FLAG_PROCESS;
					pthread_mutex_unlock(shared->mutex);
					return backendItem;
				} else {
					pthread_mutex_unlock(shared->mutex);
					return NULL;
				}
			}
		}
		pthread_cond_wait(shared->cond, shared->mutex);
		if (!shared->isRuning) {
			pthread_mutex_unlock(shared->mutex);
			return NULL;
		}
	}
}

void kcUdpBackendThreadWorks(void* obj) {
	int32_t rc;
	StringBuffer str = NULL;
	PGconn* conn = NULL;
	const struct KC_PLUGIN_ITEM* plugin;
	struct KC_BACKEND_ITEM* backendItem;
	struct KC_SHARED* const shared = (struct KC_SHARED* const)obj;


	str = createStringBuffer();
	//rc = kcPqOpenPostgresql(
	//	"192.168.1.2",
	//	"5432",
	//	"drainage",
	//	"drainage",
	//	"!@#123123",
	//	"30",
	//	"UTF-8",
	//	"UdpServer",
	//	str,
	//	&conn);
	//KC_CHECK_RCV2(rc);
	pthread_mutex_lock(shared->mutex);
	pthread_cond_signal(shared->condStart); //通知kcWorksCreate方法线程已启动
	pthread_mutex_unlock(shared->mutex);


	for (;;) {
		backendItem = kcUdpDequeue(shared);
		if (NULL == backendItem)//程序要求退出
			break;
		plugin = kcServerPluginsFind(backendItem->recv, shared->plugins, str);
		if (NULL != plugin) {
			rc = plugin->funProcessData(shared->config, backendItem, plugin->identifierLen, conn, shared->rwlock, str);
			if (KC_OK != rc)
				kcSaveLog(shared->config->log_path, str, NULL);
		}

		//通知继续
		pthread_mutex_lock(shared->mutex);
		backendItem->flag = KC_BACKEND_ITEM_FLAG_NORMAL;
		if (shared->isRuning) {
			pthread_cond_signal(shared->cond);
			pthread_mutex_unlock(shared->mutex);
		} else {
			pthread_mutex_unlock(shared->mutex);
			break;
		}
	}
	//KCPQ_CLOSE(conn);
	KC_SAFE_STRINGBUF_FREE(str);
	return;
KC_ERROR_CLEAR:
	//KCPQ_CLOSE(conn);
	kcSaveLog(shared->config->log_path, str, NULL);
	KC_SAFE_STRINGBUF_FREE(str);

	pthread_mutex_lock(shared->mutex);
	shared->isRuning = 0;
	pthread_cond_signal(shared->condStart); //通知主线程侦听线程已启动，但是启动失败
	pthread_mutex_unlock(shared->mutex);
}
#pragma endregion

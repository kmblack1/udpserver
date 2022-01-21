#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libintl.h>
#include <pthread.h>
#ifdef _WIN32
#	include <conio.h>
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#else
#  include <signal.h>
#	include <curses.h>
#  include <unistd.h>
#endif
#include <KcCore.h>
#include <stringbuffer.h>
#include <KcUtility.h>

/*****************************************************************************
*   线程数量
*****************************************************************************/
#define KC_THREAD_COUNT		(2)

/*****************************************************************************
*   启动时创建的对象标志
*****************************************************************************/
#define KC_SHARED_MUTEX										(1<<0)
#define KC_SHARED_COND											(1<<1)
#define KC_SHARED_CONDSTART								(1<<2)
#define KC_SHARED_THREAD										(1<<3)

/*****************************************************************************
*   安全释放struct KC_SHARED
*****************************************************************************/
#define KC_SAFE_FREE_SHARED(ptr) do {\
	if(NULL !=(ptr)  ) { \
		kcFreeShared((ptr)); (ptr) = NULL; \
	} \
} while (0)


#define KC_SAFE_STOP_THREADS(shared,threads) do {\
	if(NULL !=(shared)  || NULL !=(threads)) { \
		kcStopThread((shared),(threads)); (threads) = NULL; \
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
	pthread_cond_t* condStart; //只在线程启动时使用

	/// <summary>
	/// 服务是否运行
	/// </summary>
	int32_t isRuning;
};

void kcFreeShared(struct KC_SHARED* shared);
int32_t kcGenerateShared(StringBuffer str, struct KC_SHARED** ptr);

void kcThreadVoid(void* obj);
void kcFreeThreads(pthread_t** threads, uint8_t threadPool);
int32_t kcStartThread(struct KC_SHARED* shared, StringBuffer error, pthread_t*** ptr);
void kcStopThread(struct KC_SHARED* shared, pthread_t** threads);


int main(int argc, char* argv[]) {
	int32_t rc, ch;
	StringBuffer str = NULL;
	struct KC_SHARED* shared = NULL;
	pthread_t** threads = NULL;

	str = createStringBuffer();
	rc = kcGenerateShared(str, &shared);
	KC_CHECK_RCV2(rc);
	rc = kcStartThread(shared, str, &threads);
	KC_CHECK_RCV2(rc);
#ifdef __GNUC__
	initscr();
#endif
	do {
#ifdef _WIN32
		ch = _getch();
#else
		ch = getch();
#endif // _WIN32
	} while (27 != ch);
#ifdef __GNUC__
	endwin();
#endif

	KC_SAFE_STOP_THREADS(shared, threads);
	KC_SAFE_FREE_SHARED(shared);
	KC_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	KC_SAFE_STOP_THREADS(shared, threads);
	KC_SAFE_FREE_SHARED(shared);
	fprintf(stderr, "%s\n", str->data);
	KC_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

#pragma region Shared
void kcFreeShared(struct KC_SHARED* shared) {
	if (NULL == shared)
		return;

	if (NULL != shared->cond) {
		if (KC_SHARED_COND == (shared->flag & KC_SHARED_COND))
			pthread_cond_destroy(shared->cond);
		free(shared->cond);
		shared->cond = NULL;
	}

	if (NULL != shared->condStart) {
		if (KC_SHARED_CONDSTART == (shared->flag & KC_SHARED_CONDSTART))
			pthread_cond_destroy(shared->condStart);
		free(shared->condStart);
		shared->condStart = NULL;
	}

	if (NULL != shared->mutex) {
		if (KC_SHARED_MUTEX == (shared->flag & KC_SHARED_MUTEX))
			pthread_mutex_destroy(shared->mutex);
		free(shared->mutex);
		shared->mutex = NULL;
	}

	free(shared);
}
int32_t kcGenerateShared(StringBuffer str, struct KC_SHARED** ptr) {
	int32_t rc;
	struct KC_SHARED* shared = NULL;

	//创建共享对象
	shared = (struct KC_SHARED*)calloc(1, sizeof(struct KC_SHARED));
	KC_CHECK_MALLOC(shared, str);
	shared->isRuning = 1;
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

	shared->condStart = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	KC_CHECK_MALLOC(shared->condStart, str);
	rc = pthread_cond_init(shared->condStart, NULL);
	KC_CHECK_RCV1(rc, KC_2STR(pthread_cond_init), str);
	shared->flag += KC_SHARED_CONDSTART;

	(*ptr) = shared;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE_SHARED(shared);
	return KC_FAIL;
}
#pragma endregion


#pragma region thread

//线程中的工作方法，什么也没做，就是等待线程退出
void kcThreadVoid(void* obj) {
	StringBuffer str = NULL;
	struct KC_SHARED* const shared = (struct KC_SHARED* const)obj;

	str = createStringBuffer();
	pthread_mutex_lock(shared->mutex);
	pthread_cond_signal(shared->condStart); //通知主线程侦听线程已启动
	pthread_mutex_unlock(shared->mutex);

	while (1) {
		pthread_mutex_lock(shared->mutex);
		pthread_cond_wait(shared->cond, shared->mutex);  //等待线程任务
		if (!shared->isRuning) {	//线程停止运行
			pthread_mutex_unlock(shared->mutex);
			break;
		} else {
			pthread_mutex_unlock(shared->mutex);
		}
	}
	KC_STRINGBUF_FREE(str);
	return;
KC_ERROR_CLEAR:
	pthread_mutex_lock(shared->mutex);
	shared->isRuning = 0;
	pthread_mutex_unlock(shared->mutex);
	KC_STRINGBUF_FREE(str);
	return;
}
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
		pthread_cond_wait(shared->condStart, shared->mutex);
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
	KC_SAFE_STOP_THREADS(shared, threads);
	return KC_FAIL;
}
void kcStopThread(struct KC_SHARED* shared, pthread_t** threads) {
	if (NULL == shared || NULL == threads)
		return;
	pthread_mutex_lock(shared->mutex);
	shared->isRuning = 0;
	pthread_cond_broadcast(shared->cond);
	pthread_mutex_unlock(shared->mutex);
	kcFreeThreads(threads, KC_THREAD_COUNT);
}
#pragma endregion

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <gettimeofday.h>
#include <KcCore.h>
#include <stringbuffer.h>
#include <instr_time.h>
#include <KcUtility.h>
#include <KcShared.h>

int32_t main(int32_t argc, char* argv[]) {
	StringBuffer str = NULL;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	struct timeval now;
	struct timespec abstime;
	instr_time before, after;
	double elapsed_msec = 0;

	str = createStringBuffer();
	mutex = PTHREAD_MUTEX_INITIALIZER;
	cond = PTHREAD_COND_INITIALIZER;
	
	//gettimeofday(&now, NULL);
	//暂停10秒
	//KC_TIMEVAL2TIMESPEC(now, abstime, 1000);
	//暂停1毫秒
	//KC_TIMEVAL2TIMESPEC(now, abstime, 10);

	for (int32_t i = 0; i < 10; ++i) {
		//获取当前时间，abstime是绝对值
		gettimeofday(&now, NULL);
		//暂停10秒
		//KC_TIMEVAL2TIMESPEC(now, abstime, 10000);
		//暂停10毫秒
		KC_TIMEVAL2TIMESPEC(now, abstime, 10);

		INSTR_TIME_SET_CURRENT(before);
		pthread_mutex_lock(&mutex);
		pthread_cond_timedwait(&cond, &mutex, &abstime);
		pthread_mutex_unlock(&mutex);
		INSTR_TIME_SET_CURRENT(after);
		INSTR_TIME_SUBTRACT(after, before);
		elapsed_msec = INSTR_TIME_GET_MILLISEC(after);
		kcMilliseconds2String(elapsed_msec, str);
		fprintf(stdout, "%s\n", str->data);
	}

	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	KC_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
}

#ifdef __GNUC__

#include "KcUnix.h"

void signal_handler(int32_t sig) {
	switch (sig) {
	case SIGTERM:
		do {
		} while (0);
		break;
	}
}

int32_t unixServiceRuning(const char* const stdPath, StringBuffer error) {
	int32_t ch, rc;
	void (*prev_handler)(int32_t);
	//程序结束(terminate)信号, 与SIGKILL不同的是该信号可以被阻塞和处理。通常用来要求程序自己正常退出，shell命令kill缺省产生这个信号。如果进程终止不了，我们才会尝试SIGKILL。
	prev_handler = signal(SIGTERM, signal_handler);
	if (SIG_ERR == prev_handler)
		return EXIT_FAILURE;
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

#endif //__GNUC__

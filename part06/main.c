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
#include <instr_time.h>
#include <KcUtility.h>
#include <KcShared.h>

#define KC_FOR_COUNT (1024)

int32_t test1(StringBuffer str) {
	char* buffer = NULL;
	instr_time before, after;
	double elapsed_msec = 0;

	INSTR_TIME_SET_CURRENT(before);
	for (int32_t i = 0; i < KC_FOR_COUNT; ++i) {
		//尽量不要在循环中malloc和free
		buffer = malloc(2);
		KC_CHECK_MALLOC(buffer, str);
		buffer[0] = RANGE_RANDOM(0x41, 0x5A); //随机大写A-Z
		buffer[1] = '\0';
		KC_SAFE_FREE(buffer);
	}
	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "test1 %s\n", str->data);
	KC_SAFE_FREE(buffer);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(buffer);
	return KC_FAIL;
}

int32_t test2(StringBuffer str) {
	char* buffer = NULL;
	instr_time before, after;
	double elapsed_msec = 0;

	buffer = malloc(2);
	KC_CHECK_MALLOC(buffer, str);
	buffer[1] = '\0';

	INSTR_TIME_SET_CURRENT(before);
	for (int32_t i = 0; i < KC_FOR_COUNT; ++i) {
		//正确的使用方式
		buffer[0] = RANGE_RANDOM(0x41,0x5A);//随机大写A-Z
	}
	INSTR_TIME_SET_CURRENT(after);
	INSTR_TIME_SUBTRACT(after, before);
	elapsed_msec += INSTR_TIME_GET_MILLISEC(after);
	kcMilliseconds2String(elapsed_msec, str);
	fprintf(stdout, "test2 %s\n", str->data);
	KC_SAFE_FREE(buffer);
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(buffer);
	return KC_FAIL;
}

int32_t main(int argc, char* argv[]) {
	StringBuffer str = NULL;

	str= createStringBuffer();
	//功能一样，但是test2的效率是test1的几十倍
	KC_CHECK_RCV2(test1(str));
	fprintf(stdout, "\n", str->data);
	KC_CHECK_RCV2(test2(str));
	KC_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	fprintf(stderr, "%s\n", str->data);
	KC_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

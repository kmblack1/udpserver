/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	自定义日志
*****************************************************************************/
#include <libintl.h>
#include <time.h>
#include "KcShared.h"
#include "KcUtility.h"


void kcSaveLog(const char* const logPath, const StringBuffer context, const char* const fileName) {
	StringBuffer str = NULL;
	FILE* stream = NULL;
	time_t now;
	struct tm t, * ptr = &t;
	char* fullfile = NULL;
	size_t rc, binLen, count = 1;

	str = createStringBuffer();
	time(&now);						//获取系统日期和时间
#ifdef _WIN32
	localtime_s(ptr, &now);   //获取当地日期和时间
#else
	localtime_r(&now, ptr);
#endif
	if (NULL == fileName) {
		appendStringBuffer(str, "%s/%04d%02d%02d.log", logPath, 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday);
	} else {
		appendStringBuffer(str, "%s/%s", logPath, fileName);
	}
	KC_CHECK_RCV2(kcStringCopyV2(str->data, str->len, str, &fullfile));


	KC_FILE_OPEN_AND_CHECK(stream, fullfile, PG_BINARY_A, str);
	KC_FILE_DESCRIPTORS_CHECK(fullfile, str);
	KC_FILE_SEEK_CHECK(fseeko(stream, 0, SEEK_END), fullfile, str);

	resetStringBuffer(str);
	appendStringBuffer(str, "%04d-%02d-%02d %02d:%02d:%02d:", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	fwrite(str->data, str->len, 1, stream);

	binLen = context->len;
	while (binLen > 0 && !ferror(stream)) {
		rc = fwrite(context->data, sizeof(char), binLen > KC_UDP_BUFFER_SIZE ? KC_UDP_BUFFER_SIZE : binLen, stream);
		if (rc > 0) {
			binLen -= rc;
			if (0 == (count % 16)) { //每8k左右刷一次磁盘
				fflush(stream);
				count = 1;
			} else {
				++count;
			}
		} else {
			goto KC_ERROR_CLEAR;
		}
	}
	fwrite("\n\n", 1, 2, stream);
	fflush(stream);
	KC_SAFE_FREE(fullfile);
	KC_FILE_CLOSE(stream);
	KC_SAFE_STRINGBUF_FREE(str);
	return;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(fullfile);
	KC_FILE_CLOSE(stream);
	KC_SAFE_STRINGBUF_FREE(str);
	return;
}

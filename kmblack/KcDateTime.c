/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	日期时间函数实现
*****************************************************************************/

#include <math.h>
#include "KcUtility.h"
#include "KcShared.h"

void kcMilliseconds2String(double elapsed_msec, StringBuffer str) {
	double		seconds;
	double		minutes;
	double		hours;
	double		days;


	resetStringBuffer(str);
	if (elapsed_msec < 1000.0) {
		/* This is the traditional (pre-v10) output format */
		appendStringBuffer(str, gettext("Time: %.3f ms"), elapsed_msec);
		return;
	}

	/*
	 * Note: we could print just seconds, in a format like %06.3f, when the
	 * total is less than 1min.  But that's hard to interpret unless we tack
	 * on "s" or otherwise annotate it.  Forcing the display to include
	 * minutes seems like a better solution.
	 */
	seconds = elapsed_msec / 1000.0;
	minutes = floor(seconds / 60.0);
	seconds -= 60.0 * minutes;
	if (minutes < 60.0) {
		appendStringBuffer(str, gettext("Time: %.3f ms (%02d:%06.3f)"),
			elapsed_msec, (int)minutes, seconds);
		return;
	}

	hours = floor(minutes / 60.0);
	minutes -= 60.0 * hours;
	if (hours < 24.0) {
		appendStringBuffer(str, gettext("Time: %.3f ms (%02d:%02d:%06.3f)"),
			elapsed_msec, (int)hours, (int)minutes, seconds);
		return;
	}

	days = floor(hours / 24.0);
	hours -= 24.0 * days;
	appendStringBuffer(str, gettext("Time: %.3f ms (%.0f d %02d:%02d:%06.3f)"),
		elapsed_msec, days, (int)hours, (int)minutes, seconds);
}

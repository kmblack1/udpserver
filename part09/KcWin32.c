#ifdef _WIN32
#include <stdio.h>
#include <time.h>
#ifdef _DEBUG
#	include <vld.h>
#endif
#include "KcWin32.h"

LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException) {
	time_t current;
	HANDLE hDumpFile = NULL;
	struct tm now, * ptm = &now;
	char szBuf[MAX_PATH + 1];

	current = time(NULL);
	localtime_s(ptm, &current);
	sprintf_s(szBuf, MAX_PATH, "%04d%02d%02d%02d%02d%02d.dmp",
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	hDumpFile = CreateFile(szBuf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE) {
		//Dump信息
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = pException;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;
		//写入Dump文件内容
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
		CloseHandle(hDumpFile);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

#endif //_WIN32

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libintl.h>
#include "argparse.h"
#include "KcUtility.h"
#include "stringbuffer.h"
#include "KcShared.h"
#include "KcConfig.h"
#include "KcUdpBackedThreads.h"
#include "KcWin32.h"
#include "KcUnix.h"


static const char* const USAGE[] = {
		"udpservice [options]",
		NULL,
};

int32_t consoleRuning(const char* const stdPath, StringBuffer error);

int32_t main(int32_t argc, char* argv[]) {
	int32_t run = 0, start = 0;
	struct argparse argparse;
	StringBuffer str = NULL;
	char* stdPath = NULL;
	char path[KC_MAX_PATH + sizeof(char)];
#ifdef _WIN32
	char* depend = NULL, * desc = NULL;
	int32_t install = 0, uninstall = 0, stop = 0;
	struct argparse_option options[] = {
			OPT_HELP(),
			OPT_GROUP("windows service install options"),
			OPT_BOOLEAN('i', "install", &install, "install an udpservice service."),
			OPT_STRING('d', "depend", &depend, "windows service depends."),
			OPT_STRING('e', "description", &desc, "windows service description."),
			OPT_GROUP("windows service uninstall options"),
			OPT_BOOLEAN('u', "uninstall", &uninstall, "uninstall an udpservice service."),
			OPT_GROUP("windows service options"),
			OPT_BOOLEAN('s', "start", &start, "tell udpservice to start."),
			OPT_BOOLEAN('k', "stop", &stop, "tell running udpservice to shutdown."),
			OPT_GROUP("windows console options"),
			OPT_BOOLEAN('r', "run", &run, "console running udpservice."),
			OPT_END(),
	};

	int flag = 0;
	WSADATA w;

	//注冊异常捕获函数
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
	//创建字符缓冲区
	str = createStringBuffer();
	//初始化WinSocket
	if (WSAStartup(MAKEWORD(2, 2), &w)) {
		resetStringBuffer(str);
		appendStringBuffer(str, gettext("call function \"%s\" fail(%d)."), KC_2STR(WSAStartup), WSAGetLastError());
		goto KC_ERROR_CLEAR;
	}
	flag += KC_IS_WSA_STARTUP;
	if (2 != LOBYTE(w.wVersion) || 2 != HIBYTE(w.wVersion)) {
		resetStringBuffer(str);
		appendStringBuffer(str, "%s", gettext("WinSocket version is not 2.2"));
		goto KC_ERROR_CLEAR;
	}
	//获取当前工作目录
	if (!KC_GETCWD(path, KC_MAX_PATH)) {
		resetStringBuffer(str);
		appendStringBuffer(str, "%s", gettext("gets the current working directory fail."));
		goto KC_ERROR_CLEAR;
	}
	KC_CHECK_RCV2(kcStringReplace(path, "\\", "/", str, &stdPath));



	argparse_init(&argparse, options, USAGE, 0);
	//argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
	argc = argparse_parse(&argparse, argc, (const char**)argv);
	if (install) {
		//ServiceInstall(depend, desc);
	} else if (uninstall) {
		//ServiceUninstall();
	} else if (start) {
		/*SERVICE_TABLE_ENTRY entrytable[2] = {
				{(char*)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
				{ NULL, NULL }
		};
		if (!StartServiceCtrlDispatcher(entrytable))
			ServiceReportEvent("start service fail.");*/
	} else if (stop) {
		//ServiceStop();
	} else if (run) {
		KC_CHECK_RCV2(consoleRuning(stdPath, str));
	} else {
		argparse_usage(&argparse);
	}
#else
	struct argparse_option options[] = {
			OPT_HELP(),
			OPT_GROUP("linux service options"),
			OPT_BOOLEAN('s', "start", &start, "service running udpservice."),
			OPT_GROUP("linux console options"),
			OPT_BOOLEAN('r', "run", &run, "console running udpservice."),
			OPT_END(),
	};
	//创建字符缓冲区
	str = createStringBuffer();
	//获取当前工作目录
	if (!KC_GETCWD(path, KC_MAX_PATH)) {
		resetStringBuffer(str);
		appendStringBuffer(str, "%s", gettext("gets the current working directory fail."));
		goto KC_ERROR_CLEAR;
	}
	KC_CHECK_RCV2(kcStringReplace(path, "\\", "/", str, &stdPath));

	argparse_init(&argparse, options, USAGE, 0);
	//argparse_describe(&argparse, "\nA brief description of what the program does and how it works.", "\nAdditional description of the program after the description of the arguments.");
	argc = argparse_parse(&argparse, argc, (const char**)argv);
	if (run) {
		KC_CHECK_RCV2(consoleRuning(stdPath, str));
	} else if (start) {
		KC_CHECK_RCV2(unixServiceRuning(stdPath, str));
	} else {
		argparse_usage(&argparse);
	}
#endif
#ifdef WIN32
	WSACleanup();
#endif
	KC_SAFE_MODULE_FREE(stdPath);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
#ifdef WIN32
	if (KC_IS_WSA_STARTUP == (flag & KC_IS_WSA_STARTUP))
		WSACleanup();
#endif
	KC_SAFE_MODULE_FREE(stdPath);
	fprintf(stderr, "%s\n", str->data);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

int32_t consoleRuning(const char* const stdPath, StringBuffer str) {
	int32_t ch;
	struct KC_CONFIG* config = NULL;
	KC_CHECK_RCV2(kcConfigCreate(stdPath, str, &config));
	KC_CHECK_RCV2(kcUdpBackedThreadStart(config, str));
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
	kcUdpBackedThreadStop();
	KC_SAFE_FEEE_CONFIG(config);
	return KC_OK;
KC_ERROR_CLEAR:
	kcUdpBackedThreadStop();
	KC_SAFE_FEEE_CONFIG(config);
	return KC_FAIL;
}

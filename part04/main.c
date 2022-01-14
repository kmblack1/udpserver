/*****************************************************************************
*    昆明畅博科技公司 Copyright (c) 2014-2022
*	 作者：黑哥，2022-01-14
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <KcCore.h>
#ifdef _WIN32
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif
#include <dllKc.h>


//模块分配内存和释放内存演示
int32_t main(int32_t argc, char* argv[]) {
	char* str = NULL;

	KC_CHECK_RCV1(kcStringCopy("hostaddr", &str), KC_2STR(kcStringCopy));
	fprintf(stdout, "%s\n", str);
	KC_SAFE_MODULE_FREE(str);
KC_ERROR_CLEAR:
	KC_SAFE_MODULE_FREE(str);
	return EXIT_FAILURE;
}

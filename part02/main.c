/*****************************************************************************
*    昆明畅博科技公司 Copyright (c) 2014-2022
*	 作者：黑哥，2022-01-14
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef _MSC_VER
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif

//对象生命周期
int32_t main(int32_t argc, char* argv[]) {
    char szBuf[] = {0x41,0x42,0x00}; //AB

    fprintf(stdout,"%s\n", szBuf);
    do {
        char szBuf[] = { 0x41,0x43,0x00 }; //AC
        fprintf(stdout, "%s\n", szBuf);
    } while (0);
    fprintf(stdout, "%s\n", szBuf);

	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	return EXIT_FAILURE;
}

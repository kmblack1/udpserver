#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <libintl.h>
#include <pthread.h>
#ifdef _WIN32
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif
#include <KcCore.h>
#include <stringbuffer.h>
#include <instr_time.h>
#include <KcUtility.h>
#include <KcShared.h>


int main(int argc, char* argv[]) {	
	char* data = NULL;
	char* buffer = NULL;
	StringBuffer str = NULL;


	str = createStringBuffer();
	//在本模块内分配的内存
	data = (char*)malloc(2);
	KC_CHECK_MALLOC(data, str);
	//在kmblack模块中分配的内存
	KC_CHECK_RCV2(kcStringCopyV2("aaa", 3, str, &buffer));

	KC_SAFE_FREE(data);
	KC_SAFE_MODULE_FREE(buffer);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(data);
	KC_SAFE_MODULE_FREE(buffer);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

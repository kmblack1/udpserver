#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#endif
#include "dllKc.h"





void kcSafeFree(char* ptr) {
	if (NULL == ptr)
		return;
	free(ptr);
}

int32_t kcStringCopy(const char* source, char** ptr) {
	//执行成功的返回值，执行成功不需要释放
	//外部使用完成后调用KC_SAFE_MODULE_FREE宏释放
	char* str = NULL;
#ifdef _WIN32
	errno_t errcode;
#endif

	size_t srcLen;

	if (NULL == source)
		return  KC_FAIL;//invalid argument
	srcLen = strlen(source);
	str = (char*)malloc(srcLen + sizeof(char));
	if (NULL == str)
		return  KC_FAIL;//out of memory

	errcode = KC_memcpy(str, srcLen, source, srcLen);
	KC_CHEKC_MEM_ERANGE(errcode);

	str[srcLen] = '\0';
	(*ptr) = str;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(str);
	return KC_FAIL;
}

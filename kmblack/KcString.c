/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	字符串操作
*****************************************************************************/
#include <ctype.h>
#include "KcUtility.h"
#include "KcShared.h"

void kcSafeFree(char* ptr) {
	if (NULL == ptr)
		return;
	free(ptr);
}

static int32_t kcStringCopy(const char* source, size_t srcLen, StringBuffer error, char** ptr) {
	char* str = NULL;

	str = (char*)malloc(srcLen + sizeof(char));
	KC_CHECK_NULL_PTR(str);
	KC_MEMCPY(str, srcLen, source, srcLen);
	str[srcLen] = '\0';
	(*ptr) = str;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(str);
	return KC_FAIL;
}

int32_t kcStringCopyV1(const char* source, StringBuffer error, char** ptr, size_t* ptrLen) {
	size_t srcLen;

	KC_VALIDATE_PARA_PTR(source, error);

	srcLen = strlen(source);
	KC_CHECK_RCV2(kcStringCopy(source, srcLen, error, ptr));

	(*ptrLen) = srcLen;
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcStringCopyV2(const char* source, size_t srcLen, StringBuffer error, char** ptr) {
	KC_VALIDATE_PARA_PTR(source, error);
	KC_PARA_LESS_EQUAL_ZERO(srcLen, error);

	KC_CHECK_RCV2(kcStringCopy(source, srcLen, error, ptr));
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcStringCopyV3(const char* source, StringBuffer error, char** ptr) {
	size_t srcLen;

	KC_VALIDATE_PARA_PTR(source, error);

	srcLen = strlen(source);
	KC_CHECK_RCV2(kcStringCopy(source, srcLen, error, ptr));
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}



int32_t kcStringReplace(const char* source, const char* substr, const char* dest, StringBuffer error, char** ptr) {
	const char* position = source, * ptmp = source;
	size_t substrLen = strlen(substr);

	KC_VALIDATE_PARA_PTR(source, error);
	KC_VALIDATE_PARA_PTR(substr, error);
	KC_VALIDATE_PARA_PTR(dest, error);


	resetStringBuffer(error);
	while ((position = strstr(ptmp, substr))) {
		appendStringBufferBinary(error, ptmp, (position - ptmp));
		appendStringBufferStr(error, dest);
		ptmp = (position + substrLen);
	}
	appendStringBufferStr(error, ptmp);
	KC_CHECK_RCV2(kcStringCopyV2(error->data, error->len, error, ptr));
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}


int32_t kcHexStringToBytes(const char* hexString, size_t hexLen, StringBuffer error, uint8_t** ptr, size_t* ptrLen) {
	const char* src = hexString;
	char buffer[3];
	uint8_t* data = NULL;
	size_t dataLen;

	KC_VALIDATE_PARA_PTR(hexString, error);
	KC_PARA_LESS_EQUAL_ZERO(hexLen, error);

	if (0x30 == src[0] && 0x78 == src[1]) {  //舍去字符串0x
		src += 2;
		hexLen -= 2;
	}
	if (0 != (hexLen % 2))							//16进制字符串为二个一组
		goto KC_ERROR_CLEAR;

	dataLen = hexLen / 2;
	data = (uint8_t*)malloc(dataLen);
	KC_CHECK_MALLOC(data, error);
	buffer[2] = '\0';
	for (size_t i = 0; i < dataLen; ++i, src += 2) {
		if (src[0] == '\0' || src[1] == '\0')
			goto KC_ERROR_CLEAR;
		memcpy(buffer, src, 2);
		if (!isxdigit((uint8_t)buffer[0]) || !isxdigit((uint8_t)buffer[1]))
			goto KC_ERROR_CLEAR;

		data[i] = (uint8_t)strtoul(buffer, NULL, 16);
	}
	(*ptr) = data;
	(*ptrLen) = dataLen;
	return KC_OK;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(data);
	resetStringBuffer(error);
	appendStringBuffer(error, "%s", gettext("is not a valid hexadecimal string"));
	return KC_FAIL;
}

int32_t kcHexStringFromBytes(const uint8_t* bytes, size_t bytesLen, StringBuffer buffer) {
	static const char HEX_CHARS[] = "0123456789ABCDEF";
	uint8_t			hi, lo;
	const uint8_t* src = bytes;

	KC_VALIDATE_PARA_PTR(bytes, buffer);
	KC_PARA_LESS_EQUAL_ZERO(bytesLen, buffer);


	resetStringBuffer(buffer);
	appendStringBufferStr(buffer, "0x");
	for (size_t i = 0; i < bytesLen; ++i) {
		hi = src[i] >> 4;
		lo = src[i] & 0x0F;
		appendStringBufferChar(buffer, HEX_CHARS[hi]);
		appendStringBufferChar(buffer, HEX_CHARS[lo]);
	}
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

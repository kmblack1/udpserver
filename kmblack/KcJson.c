/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	JSON操作
*****************************************************************************/
#include "KcUtility.h"
#include "KcShared.h"


int32_t kcJsonGetString(const char* const key, const json_t* const value, char** ptr, StringBuffer error) {
	const char* val;

	KC_JSON_IS_STRING(value, key, error);
	val = json_string_value(value);
	KC_JSON_VALUE_IS_NULL_OR_EMPTY(val, key, error);

	KC_CHECK_RCV2(kcStringCopyV3(val, error, ptr));

	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcJsonGetStringN(const char* const key, const json_t* const value, char** ptr, size_t* ptrLen, StringBuffer error) {
	const char* val;

	KC_JSON_IS_STRING(value, key, error);
	val = json_string_value(value);
	KC_JSON_VALUE_IS_NULL_OR_EMPTY(val, key, error);
	KC_CHECK_RCV2(kcStringCopyV1(val, error, ptr, ptrLen));
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcJsonGetBool(const char* const key, const json_t* const value, bool* ptr, StringBuffer error) {
	bool val;

	KC_JSON_IS_BOOL(value, key, error);
	val = json_boolean_value(value);
	(*ptr) = val;
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcJsonGetInt32(const char* const key, const json_t* const value, int32_t* ptr, StringBuffer error) {
	int32_t val;

	KC_JSON_IS_INTEGER(value, key, error);
	val = (int32_t)json_integer_value(value);
	(*ptr) = val;
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

int32_t kcJsonGetInt64(const char* const key, const json_t* const value, int64_t* ptr, StringBuffer error) {
	int64_t val;

	KC_JSON_IS_INTEGER(value, key, error);
	val = json_integer_value(value);
	(*ptr) = val;
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

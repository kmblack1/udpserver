/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	regex正则表达式
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef _MSC_VER
#	include <tre/regex.h>
#	ifdef _DEBUG
#		include <vld.h>
#	endif
#else
#	include <regex.h>
#endif
#include <stringbuffer.h>
#include <KcUtility.h>

#define KC_SAFE_REGEX_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		regfree((ptr)); \
		free((ptr)); \
		(ptr) = NULL; \
	} \
} while (0)


reg_errcode_t kcCheckGetMobile(const char* values, StringBuffer str) {
	reg_errcode_t rc;
	int32_t len;
	size_t nsub;
	regex_t* reg = NULL;
	regmatch_t *matchs = NULL;
	/* 判断是否为手机号,接收以下7种格式
	* +861xxxxxxxxxx
	* 861xxxxxxxxxx
	* (+86)1xxxxxxxxxx
	* (86)1xxxxxxxxxx
	* （+86）1xxxxxxxxxx，这里的括号是中文的括号
	* （86）1xxxxxxxxxx，这里的括号是中文的括号
	* 1xxxxxxxxxx
	*/
	static const char* PATTERN = "^((\\+86)|(86)|(\\(\\+86\\))|(\\(86\\))|(（\\+86）)|(（86）))?(1[0-9]{10})$";


	
	enlargeStringBuffer(str, 256);
	reg = (regex_t*)malloc(sizeof(regex_t));
	KC_CHECK_MALLOC(reg, str);
	rc = regcomp(reg, PATTERN, REG_EXTENDED);
	if (rc) {
		resetStringBuffer(str);
		appendStringBufferStr(str, "error:");
		regerror(rc, reg, str->data, str->len);
		appendStringBufferChar(str, 0x10);
		goto KC_ERROR_CLEAR;
	}
	nsub = reg->re_nsub + 1;
	matchs = (regmatch_t *)malloc(nsub * sizeof(regmatch_t));
	KC_CHECK_MALLOC(matchs, str);
	rc = regexec(reg, values, nsub,matchs, 0);
	switch (rc) {
	case 0:do {
		//i==0匹配字符串自身，所以从1开始
		for (int32_t i = 0; i < nsub; ++i) {
			if ( -1 != matchs[i].rm_so ) {
				resetStringBuffer(str);
				len = matchs[i].rm_eo - matchs[i].rm_so;  // 匹配长度	
				appendStringBufferBinary(str, (values + matchs[i].rm_so), len);
				fprintf(stdout, "%s\n", str->data);				
			} else {
				fprintf(stdout, "null\n");
			}
		}
	} while (0);
	break;
	case REG_NOMATCH:do {
		resetStringBuffer(str);
		appendStringBufferStr(str, "no match");
	} while (0);
	break;
	default:do {
		resetStringBuffer(str);
		appendStringBufferStr(str, "error:");
		regerror(rc, reg, str->data, str->len);
		appendStringBufferChar(str, 0x10);
	} while (0);
	break;
	}
	KC_SAFE_FREE(matchs);
	KC_SAFE_REGEX_FREE(reg);
	return rc;
KC_ERROR_CLEAR:
	KC_SAFE_FREE(matchs);
	KC_SAFE_REGEX_FREE(reg);
	return rc;
}

int32_t main(int32_t argc, char* argv[]) {
	reg_errcode_t ret;
	StringBuffer str = NULL;

	str = createStringBuffer();

	ret = kcCheckGetMobile("(+86)13912345678", str);
	if (0 != ret)
		goto KC_ERROR_CLEAR;

	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_SUCCESS;
KC_ERROR_CLEAR:
	fprintf(stderr, "%s\n", str->data);
	KC_SAFE_STRINGBUF_FREE(str);
	return EXIT_FAILURE;
}

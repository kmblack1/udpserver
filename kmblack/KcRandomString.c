#include <time.h>
#include <pthread.h>
#include "KcUtility.h"
#include "KcShared.h"

#define KC_UTF8_MAX_SIZE (4)

void kcNumAbcZhRandom(uint8_t flag, uint32_t characterCount, StringBuffer random);

int32_t kcRandomString( uint16_t min, uint16_t max, uint8_t flag, StringBuffer random, StringBuffer error) {
	uint32_t characterCount;

	KC_VALIDATE_PARA_PTR(random, error);

	/*生成随机长度的字符串*/
	if ((min < 1 || max < 1)) {
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("invalid argument[val<1].(%s:%d)"), __FILE__, __LINE__);
		goto KC_ERROR_CLEAR;
	}
	if ((min > 0x8000 || max > 0x8000)) {
		resetStringBuffer(error);
		appendStringBuffer(error, gettext("invalid argument[val>32768].(%s:%d)"), __FILE__, __LINE__);
		goto KC_ERROR_CLEAR;
	}

	if (min > max) {
		min = min ^ max;
		max = min ^ max;
		min = min ^ max;
	}	
	
	characterCount = KC_RANDOM(min, max);
	KC_SAFE_ENLARGE_STRINGBUFFER(random, characterCount * 4 + 1, error);
	kcNumAbcZhRandom(flag, characterCount, random);
	
	return KC_OK;
KC_ERROR_CLEAR:
	return KC_FAIL;
}

uint8_t kcCkeckFlag(uint8_t flag) {
	switch (flag) {
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
		return KC_RANDOM(1, 2);
	case 4:
		return 3;
	case 5:
		return KC_RANDOM(0, 1) ? 1 : 3;
	case 6:
		return KC_RANDOM(2, 3);
	case 7:
		return KC_RANDOM(1, 3);
	case 8:
		return 4;
	case 9:
		return KC_RANDOM(0, 1) ? 1 : 4;
	case 10:
		return KC_RANDOM(0, 1) ? 2 : 4;
	case 11:
		return KC_RANDOM(0, 1) ? KC_RANDOM(1, 2) : 4;
	case 12:
		return KC_RANDOM(3, 4);
	case 13:
		return KC_RANDOM(0, 1) ? 1 : KC_RANDOM(3, 4);
	case 14:
		return KC_RANDOM(2, 4);
	default:
		return 0;
	}
}

int32_t kcUtf8ZhRandom(uint32_t code, char* buffer) {
	if (code <= 0x7F) {
		buffer[0] = (uint8_t)code;
		return 1;
	} else if (code <= 0x7FF) {
		buffer[0] = (code >> 6) + 192;
		buffer[1] = (code & 63) + 128;
		return 2;
	} else if (0xd800 <= code && code <= 0xdfff) {
		return 0;
	} //invalid block of utf8
	else if (code <= 0xFFFF) {
		buffer[0] = (code >> 12) + 224;
		buffer[1] = ((code >> 6) & 63) + 128;
		buffer[2] = (code & 63) + 128;
		return 3;
	} else if (code <= 0x10FFFF) {
		buffer[0] = (code >> 18) + 240;
		buffer[1] = ((code >> 12) & 63) + 128;
		buffer[2] = ((code >> 6) & 63) + 128;
		buffer[3] = (code & 63) + 128;
		return 4;
	}
	return 0;
}

int32_t kcNumAbcZhRandomV1(uint8_t flag, char* buffer) {
	uint8_t flagUpper;
	int32_t isValid;

	switch (flag) {
	case 1:
		buffer[0] = KC_RANDOM(48, 57);	/*生成数字*/
		return 1;
	case 2:
		buffer[0] = KC_RANDOM(65, 90);	/*生成大写字母*/
		return 1;
	case 3:
		buffer[0] = KC_RANDOM(97, 122);	/*生成小写字母*/
		return 1;
	case 4:/*生成汉字*/
		do {
			isValid = kcUtf8ZhRandom(KC_RANDOM(19968, 20901), buffer);
		} while (!isValid);
		return isValid;
	default:	/*随机生成数字大小写汉字混合字符串*/
		flagUpper = KC_RANDOM(0, 3);
		switch (flagUpper) {
		case 1:
			buffer[0] = KC_RANDOM(65, 90);  /*生成大写字母*/
			return 1;
		case 2:
			buffer[0] = KC_RANDOM(97, 122);	/*生成小写字母*/
			return 1;
		case 3:/*生成汉字*/
			do {
				isValid = kcUtf8ZhRandom(KC_RANDOM(19968, 20901), buffer);
			} while (!isValid);
			return isValid;
		default:
			buffer[0] = KC_RANDOM(48, 57);	/*生成数字*/
			return 1;
		}
		break;
	}
}

void kcNumAbcZhRandom(uint8_t flag, uint32_t characterCount, StringBuffer random) {
	int32_t chrMemSize = 0;
	char* ptmp;
	
	ptmp = random->data;
	for (uint32_t i = 0; i < characterCount; ++i) {
		chrMemSize = kcNumAbcZhRandomV1(kcCkeckFlag(flag), ptmp);
		ptmp += chrMemSize;
		random->len += chrMemSize;
	}
	random->data[random->len] = '\0';
}

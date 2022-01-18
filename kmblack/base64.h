/*
 * base64.h
 *	  Encoding and decoding routines for base64 without whitespace
 *	  support.
 *
 * Portions Copyright (c) 2001-2021, PostgreSQL Global Development Group
 *
 * src/include/common/base64.h
 */
#ifndef BASE64_H
#define BASE64_H

#include "KcCore.h"

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/


/* base 64 */
extern KC_EXTERNAL int	KCAPI pg_b64_encode(const char *src, int len, char *dst, int dstlen);
extern KC_EXTERNAL int	KCAPI pg_b64_decode(const char *src, int len, char *dst, int dstlen);
extern KC_EXTERNAL int	KCAPI pg_b64_enc_len(int srclen);
extern KC_EXTERNAL int	KCAPI pg_b64_dec_len(int srclen);


#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif							/* BASE64_H */

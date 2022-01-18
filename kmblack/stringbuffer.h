/*-------------------------------------------------------------------------
 *
 * pqexpbuffer.h
 *	  Declarations/definitions for "StringBuffer" functions.
 *
 * StringBuffer provides an indefinitely-extensible string data type.
 * It can be used to buffer either ordinary C strings (null-terminated text)
 * or arbitrary binary data.  All storage is allocated with malloc().
 *
 * This module is essentially the same as the backend's StringInfo data type,
 * but it is intended for use in frontend libpq and client applications.
 * Thus, it does not rely on palloc() nor elog().
 *
 * It does rely on vsnprintf(); if configure finds that libc doesn't provide
 * a usable vsnprintf(), then a copy of our own implementation of it will
 * be linked into libpq.
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/interfaces/libpq/pqexpbuffer.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef STRINGBUFFER_H
#define STRINGBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include "KcCore.h"
 /*
  * Hints to the compiler about the likelihood of a branch. Both likely() and
  * unlikely() return the boolean value of the contained expression.
  *
  * These should only be used sparingly, in very hot code paths. It's very easy
  * to mis-estimate likelihoods.
  */
#if __GNUC__ >= 3
#define likely(x)	__builtin_expect((x) != 0, 1)
#define unlikely(x) __builtin_expect((x) != 0, 0)
#else
#define likely(x)	((x) != 0)
#define unlikely(x) ((x) != 0)
#endif

  /*
   * Macro that allows to cast constness and volatile away from an expression, but doesn't
   * allow changing the underlying type.  Enforcement of the latter
   * currently only works for gcc like compilers.
   *
   * Please note IT IS NOT SAFE to cast constness away if the result will ever
   * be modified (it would be undefined behaviour). Doing so anyway can cause
   * compiler misoptimizations or runtime crashes (modifying readonly memory).
   * It is only safe to use when the result will not be modified, but API
   * design or language restrictions prevent you from declaring that
   * (e.g. because a function returns both const and non-const variables).
   *
   * Note that this only works in function scope, not for global variables (it'd
   * be nice, but not trivial, to improve that).
   */
#if defined(HAVE__BUILTIN_TYPES_COMPATIBLE_P)
#define unconstify(underlying_type, expr) \
	(StaticAssertExpr(__builtin_types_compatible_p(__typeof(expr), const underlying_type), \
					  "wrong cast"), \
	 (underlying_type) (expr))
#define unvolatize(underlying_type, expr) \
	(StaticAssertExpr(__builtin_types_compatible_p(__typeof(expr), volatile underlying_type), \
					  "wrong cast"), \
	 (underlying_type) (expr))
#else
#define unconstify(underlying_type, expr) \
	((underlying_type) (expr))
#define unvolatize(underlying_type, expr) \
	((underlying_type) (expr))
#endif


   /* GCC and XLC support format attributes */
#if defined(__GNUC__) || defined(__IBMC__)
#define PG_PRINTF_ATTRIBUTE   printf
#define pg_attribute_format_arg(a) __attribute__((format_arg(a)))
#define pg_attribute_printf(f,a) __attribute__((format(PG_PRINTF_ATTRIBUTE, f, a)))
#else
#define pg_attribute_format_arg(a)
#define pg_attribute_printf(f,a)
#endif

/*-------------------------
 * StringBufferData holds information about an extensible string.
 *		data	is the current buffer for the string (allocated with malloc).
 *		len		is the current string length.  There is guaranteed to be
 *				a terminating '\0' at data[len], although this is not very
 *				useful when the string holds binary data rather than text.
 *		maxlen	is the allocated size in bytes of 'data', i.e. the maximum
 *				string size (including the terminating '\0' char) that we can
 *				currently store in 'data' without having to reallocate
 *				more space.  We must always have maxlen > len.
 *
 * An exception occurs if we failed to allocate enough memory for the string
 * buffer.  In that case data points to a statically allocated empty string,
 * and len = maxlen = 0.
 *-------------------------
 */
typedef struct StringBufferData {
	char* data;
	size_t		len;
	size_t		maxlen;
} StringBufferData;

typedef StringBufferData* StringBuffer;

/*------------------------
 * Test for a broken (out of memory) StringBuffer.
 * When a buffer is "broken", all operations except resetting or deleting it
 * are no-ops.
 *------------------------
 */
#define StringBufferBroken(str)	\
	((str) == NULL || (str)->maxlen == 0)

 /*------------------------
  * Same, but for use when using a static or local StringBufferData struct.
  * For that, a null-pointer test is useless and may draw compiler warnings.
  *------------------------
  */
#define StringBufferDataBroken(buf)	\
	((buf).maxlen == 0)

  /*------------------------
   * Initial size of the data buffer in a StringBuffer.
   * NB: this must be large enough to hold error messages that might
   * be returned by PQrequestCancel().
   *------------------------
   */
#define INITIAL_EXPBUFFER_SIZE	256

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/

	/*------------------------
	 * There are two ways to create a StringBuffer object initially:
	 *
	 * StringBuffer stringptr = createStringBuffer();
	 *		Both the StringBufferData and the data buffer are malloc'd.
	 *
	 * StringBufferData string;
	 * initStringBuffer(&string);
	 *		The data buffer is malloc'd but the StringBufferData is presupplied.
	 *		This is appropriate if the StringBufferData is a field of another
	 *		struct.
	 *-------------------------
	 */

	 /*------------------------
	  * createStringBuffer
	  * Create an empty 'StringBufferData' & return a pointer to it.
	  */
	extern KC_EXTERNAL StringBuffer KCAPI createStringBuffer(void);

	/*------------------------
	 * initStringBuffer
	 * Initialize a StringBufferData struct (with previously undefined contents)
	 * to describe an empty string.
	 */
	extern KC_EXTERNAL void KCAPI initStringBuffer(StringBuffer str);

	/*------------------------
	 * To destroy a StringBuffer, use either:
	 *
	 * destroyStringBuffer(str);
	 *		free()s both the data buffer and the StringBufferData.
	 *		This is the inverse of createStringBuffer().
	 *
	 * termStringBuffer(str)
	 *		free()s the data buffer but not the StringBufferData itself.
	 *		This is the inverse of initStringBuffer().
	 *
	 * NOTE: some routines build up a string using StringBuffer, and then
	 * release the StringBufferData but return the data string itself to their
	 * caller.  At that point the data string looks like a plain malloc'd
	 * string.
	 */
	extern KC_EXTERNAL void KCAPI destroyStringBuffer(StringBuffer str);
	extern KC_EXTERNAL void KCAPI termStringBuffer(StringBuffer str);

	/*------------------------
	 * resetStringBuffer
	 *		Reset a StringBuffer to empty
	 *
	 * Note: if possible, a "broken" StringBuffer is returned to normal.
	 */
	extern KC_EXTERNAL void KCAPI resetStringBuffer(StringBuffer str);

	/*------------------------
	 * enlargeStringBuffer
	 * Make sure there is enough space for 'needed' more bytes in the buffer
	 * ('needed' does not include the terminating null).
	 *
	 * Returns 1 if OK, 0 if failed to enlarge buffer.  (In the latter case
	 * the buffer is left in "broken" state.)
	 */
	extern KC_EXTERNAL int	KCAPI enlargeStringBuffer(StringBuffer str, size_t needed);

	/*------------------------
	 * printfStringBuffer
	 * Format text data under the control of fmt (an sprintf-like format string)
	 * and insert it into str.  More space is allocated to str if necessary.
	 * This is a convenience routine that does the same thing as
	 * resetStringBuffer() followed by appendStringBuffer().
	 */
	extern KC_EXTERNAL void KCAPI printfStringBuffer(StringBuffer str, const char* fmt, ...) pg_attribute_printf(2, 3);

	/*------------------------
	 * appendStringBuffer
	 * Format text data under the control of fmt (an sprintf-like format string)
	 * and append it to whatever is already in str.  More space is allocated
	 * to str if necessary.  This is sort of like a combination of sprintf and
	 * strcat.
	 */
	extern KC_EXTERNAL void KCAPI appendStringBuffer(StringBuffer str, const char* fmt, ...) pg_attribute_printf(2, 3);

	/*------------------------
	 * appendStringBufferStr
	 * Append the given string to a StringBuffer, allocating more space
	 * if necessary.
	 */
	extern KC_EXTERNAL void KCAPI appendStringBufferStr(StringBuffer str, const char* data);

	/*------------------------
	 * appendStringBufferChar
	 * Append a single byte to str.
	 * Like appendStringBuffer(str, "%c", ch) but much faster.
	 */
	extern KC_EXTERNAL void KCAPI appendStringBufferChar(StringBuffer str, char ch);

	/*------------------------
	 * appendBinaryStringBuffer
	 * Append arbitrary binary data to a StringBuffer, allocating more space
	 * if necessary.
	 */
	extern KC_EXTERNAL void KCAPI appendBinaryStringBuffer(StringBuffer str,
		const char* data, size_t datalen);

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/

#endif							/* STRINGBUFFER_H */

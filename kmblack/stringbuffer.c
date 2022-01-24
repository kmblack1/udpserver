/*-------------------------------------------------------------------------
 *
 * pqexpbuffer.c
 *
 * StringBuffer provides an indefinitely-extensible string data type.
 * It can be used to buffer either ordinary C strings (null-terminated text)
 * or arbitrary binary data.  All storage is allocated with malloc().
 *
 * This module is essentially the same as the backend's StringInfo data type,
 * but it is intended for use in frontend libpq and client applications.
 * Thus, it does not rely on palloc() nor elog(), nor psprintf.c which
 * will exit() on error.
 *
 * It does rely on vsnprintf(); if configure finds that libc doesn't provide
 * a usable vsnprintf(), then a copy of our own implementation of it will
 * be linked into libpq.
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/interfaces/libpq/pqexpbuffer.c
 *
 *-------------------------------------------------------------------------
 */

//#include "postgres_fe.h"

#include <limits.h>

#include "stringbuffer.h"
/*#ifdef WIN32
#include "win32.h"
#endif*/


/* All "broken" StringBuffers point to this string. */
static const char oom_buffer[1] = "";

/* Need a char * for unconstify() compatibility */
static const char *oom_buffer_ptr = oom_buffer;

static bool appendStringBufferVA(StringBuffer str, const char *fmt, va_list args) pg_attribute_printf(2, 0);


/*
 * markStringBufferBroken
 *
 * Put a StringBuffer in "broken" state if it isn't already.
 */
static void
markStringBufferBroken(StringBuffer str)
{
	if (str->data != oom_buffer)
		free(str->data);

	/*
	 * Casting away const here is a bit ugly, but it seems preferable to not
	 * marking oom_buffer const.  We want to do that to encourage the compiler
	 * to put oom_buffer in read-only storage, so that anyone who tries to
	 * scribble on a broken StringBuffer will get a failure.
	 */
	str->data = unconstify(char *, oom_buffer_ptr);
	str->len = 0;
	str->maxlen = 0;
}

/*
 * createStringBuffer
 *
 * Create an empty 'StringBufferData' & return a pointer to it.
 */
StringBuffer
createStringBuffer(void)
{
	StringBuffer res;

	res = (StringBuffer) malloc(sizeof(StringBufferData));
	if (res != NULL)
		initStringBuffer(res);

	return res;
}

/*
 * initStringBuffer
 *
 * Initialize a StringBufferData struct (with previously undefined contents)
 * to describe an empty string.
 */
void
initStringBuffer(StringBuffer str)
{
	str->data = (char *) malloc(INITIAL_EXPBUFFER_SIZE);
	if (str->data == NULL)
	{
		str->data = unconstify(char *, oom_buffer_ptr); /* see comment above */
		str->maxlen = 0;
		str->len = 0;
	}
	else
	{
		str->maxlen = INITIAL_EXPBUFFER_SIZE;
		str->len = 0;
		str->data[0] = '\0';
	}
}

/*
 * destroyStringBuffer(str);
 *
 *		free()s both the data buffer and the StringBufferData.
 *		This is the inverse of createStringBuffer().
 */
void
destroyStringBuffer(StringBuffer str)
{
	if (str)
	{
		termStringBuffer(str);
		free(str);
	}
}

/*
 * termStringBuffer(str)
 *		free()s the data buffer but not the StringBufferData itself.
 *		This is the inverse of initStringBuffer().
 */
void
termStringBuffer(StringBuffer str)
{
	if (str->data != oom_buffer)
		free(str->data);
	/* just for luck, make the buffer validly empty. */
	str->data = unconstify(char *, oom_buffer_ptr); /* see comment above */
	str->maxlen = 0;
	str->len = 0;
}

/*
 * resetStringBuffer
 *		Reset a StringBuffer to empty
 *
 * Note: if possible, a "broken" StringBuffer is returned to normal.
 */
void
resetStringBuffer(StringBuffer str)
{
	if (str)
	{
		if (str->data != oom_buffer)
		{
			str->len = 0;
			str->data[0] = '\0';
		}
		else
		{
			/* try to reinitialize to valid state */
			initStringBuffer(str);
		}
	}
}

/*
 * enlargeStringBuffer
 * Make sure there is enough space for 'needed' more bytes in the buffer
 * ('needed' does not include the terminating null).
 *
 * Returns 1 if OK, 0 if failed to enlarge buffer.  (In the latter case
 * the buffer is left in "broken" state.)
 */
int
enlargeStringBuffer(StringBuffer str, size_t needed)
{
	size_t		newlen;
	char	   *newdata;

	if (StringBufferBroken(str))
		return 0;				/* already failed */

	/*
	 * Guard against ridiculous "needed" values, which can occur if we're fed
	 * bogus data.  Without this, we can get an overflow or infinite loop in
	 * the following.
	 */
	if (needed >= ((size_t) INT_MAX - str->len))
	{
		markStringBufferBroken(str);
		return 0;
	}

	needed += str->len + 1;		/* total space required now */

	/* Because of the above test, we now have needed <= INT_MAX */

	if (needed <= str->maxlen)
		return 1;				/* got enough space already */

	/*
	 * We don't want to allocate just a little more space with each append;
	 * for efficiency, double the buffer size each time it overflows.
	 * Actually, we might need to more than double it if 'needed' is big...
	 */
	newlen = (str->maxlen > 0) ? (2 * str->maxlen) : 64;
	while (needed > newlen)
		newlen = 2 * newlen;

	/*
	 * Clamp to INT_MAX in case we went past it.  Note we are assuming here
	 * that INT_MAX <= UINT_MAX/2, else the above loop could overflow.  We
	 * will still have newlen >= needed.
	 */
	if (newlen > (size_t) INT_MAX)
		newlen = (size_t) INT_MAX;

	newdata = (char *) realloc(str->data, newlen);
	if (newdata != NULL)
	{
		str->data = newdata;
		str->maxlen = newlen;
		return 1;
	}

	markStringBufferBroken(str);
	return 0;
}

/*
 * printfStringBuffer
 * Format text data under the control of fmt (an sprintf-like format string)
 * and insert it into str.  More space is allocated to str if necessary.
 * This is a convenience routine that does the same thing as
 * resetStringBuffer() followed by appendStringBuffer().
 */
void
printfStringBuffer(StringBuffer str, const char *fmt,...)
{
	int			save_errno = errno;
	va_list		args;
	bool		done;

	resetStringBuffer(str);

	if (StringBufferBroken(str))
		return;					/* already failed */

	/* Loop in case we have to retry after enlarging the buffer. */
	do
	{
		errno = save_errno;
		va_start(args, fmt);
		done = appendStringBufferVA(str, fmt, args);
		va_end(args);
	} while (!done);
}

/*
 * appendStringBuffer
 *
 * Format text data under the control of fmt (an sprintf-like format string)
 * and append it to whatever is already in str.  More space is allocated
 * to str if necessary.  This is sort of like a combination of sprintf and
 * strcat.
 */
void
appendStringBuffer(StringBuffer str, const char *fmt,...)
{
	int			save_errno = errno;
	va_list		args;
	bool		done;

	if (StringBufferBroken(str))
		return;					/* already failed */

	/* Loop in case we have to retry after enlarging the buffer. */
	do
	{
		errno = save_errno;
		va_start(args, fmt);
		done = appendStringBufferVA(str, fmt, args);
		va_end(args);
	} while (!done);
}

/*
 * appendStringBufferVA
 * Shared guts of printfStringBuffer/appendStringBuffer.
 * Attempt to format data and append it to str.  Returns true if done
 * (either successful or hard failure), false if need to retry.
 *
 * Caution: callers must be sure to preserve their entry-time errno
 * when looping, in case the fmt contains "%m".
 */
static bool
appendStringBufferVA(StringBuffer str, const char *fmt, va_list args)
{
	size_t		avail;
	size_t		needed;
	int			nprinted;

	/*
	 * Try to format the given string into the available space; but if there's
	 * hardly any space, don't bother trying, just enlarge the buffer first.
	 */
	if (str->maxlen > str->len + 16)
	{
		avail = str->maxlen - str->len;

		nprinted = vsnprintf(str->data + str->len, avail, fmt, args);

		/*
		 * If vsnprintf reports an error, fail (we assume this means there's
		 * something wrong with the format string).
		 */
		if (unlikely(nprinted < 0))
		{
			markStringBufferBroken(str);
			return true;
		}

		if ((size_t) nprinted < avail)
		{
			/* Success.  Note nprinted does not include trailing null. */
			str->len += nprinted;
			return true;
		}

		/*
		 * We assume a C99-compliant vsnprintf, so believe its estimate of the
		 * required space, and add one for the trailing null.  (If it's wrong,
		 * the logic will still work, but we may loop multiple times.)
		 *
		 * Choke if the required space would exceed INT_MAX, since str->maxlen
		 * can't represent more than that.
		 */
		if (unlikely(nprinted > INT_MAX - 1))
		{
			markStringBufferBroken(str);
			return true;
		}
		needed = nprinted + 1;
	}
	else
	{
		/*
		 * We have to guess at how much to enlarge, since we're skipping the
		 * formatting work.  Fortunately, because of enlargeStringBuffer's
		 * preference for power-of-2 sizes, this number isn't very sensitive;
		 * the net effect is that we'll double the buffer size before trying
		 * to run vsnprintf, which seems sensible.
		 */
		needed = 32;
	}

	/* Increase the buffer size and try again. */
	if (!enlargeStringBuffer(str, needed))
		return true;			/* oops, out of memory */

	return false;
}

/*
 * appendStringBufferStr
 * Append the given string to a StringBuffer, allocating more space
 * if necessary.
 */
void
appendStringBufferStr(StringBuffer str, const char *data)
{
	appendStringBufferBinary(str, data, strlen(data));
}

/*
 * appendStringBufferChar
 * Append a single byte to str.
 * Like appendStringBuffer(str, "%c", ch) but much faster.
 */
void
appendStringBufferChar(StringBuffer str, char ch)
{
	/* Make more room if needed */
	if (!enlargeStringBuffer(str, 1))
		return;

	/* OK, append the character */
	str->data[str->len] = ch;
	str->len++;
	str->data[str->len] = '\0';
}

/*
 * appendBinaryStringBuffer
 *
 * Append arbitrary binary data to a StringBuffer, allocating more space
 * if necessary.
 */
void
appendStringBufferBinary(StringBuffer str, const char *data, size_t datalen)
{
	/* Make more room if needed */
	if (!enlargeStringBuffer(str, datalen))
		return;

	/* OK, append the data */
	memcpy(str->data + str->len, data, datalen);
	str->len += datalen;

	/*
	 * Keep a trailing null in place, even though it's probably useless for
	 * binary data...
	 */
	str->data[str->len] = '\0';
}

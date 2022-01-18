
#ifndef KC_8C7A2455_694D_419A_869F_422FA562E8CF
#define KC_8C7A2455_694D_419A_869F_422FA562E8CF
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>


/*****************************************************************************
*	函数修饰符
*****************************************************************************/
#ifdef _WIN32
#	ifdef UKCIMPORT
#		define KC_EXTERNAL __declspec(dllimport)
#		define KCAPI __stdcall
#	elif UKCEXPORTS
#		define KC_EXTERNAL __declspec(dllexport)
#		define KCAPI __stdcall
#	else
#		define KC_EXTERNAL
#		define KCAPI
#	endif
#else
#	define KC_EXTERNAL
#	define KCAPI
#endif

#define INT64CONST(x)  (x##LL)
#define UINT64CONST(x) (x##ULL)



/*
 * UKCASSERT_CHECKING, if defined, turns on all the assertions.
 * - plai  9/5/90
 *
 * It should _NOT_ be defined in releases or in benchmark copies
 */

 /*
  * Assert() can be used in both frontend and backend code. In frontend code it
  * just calls the standard assert, if it's available. If use of assertions is
  * not configured, it does nothing.
  */
#ifndef UKCASSERT_CHECKING

#define Assert(condition)	((void)true)
#define AssertMacro(condition)	((void)true)
#define AssertArg(condition)	((void)true)
#define AssertState(condition)	((void)true)
#define AssertPointerAlignment(ptr, bndr)	((void)true)
#define Trap(condition, errorType)	((void)true)
#define TrapMacro(condition, errorType) (true)

#elif defined(FRONTEND)

#include <assert.h>
#define Assert(p) assert(p)
#define AssertMacro(p)	((void) assert(p))
#define AssertArg(condition) assert(condition)
#define AssertState(condition) assert(condition)
#define AssertPointerAlignment(ptr, bndr)	((void)true)

#else							/* UKCASSERT_CHECKING && !FRONTEND */

  /*
   * Trap
   *		Generates an exception if the given condition is true.
   */
#define Trap(condition, errorType) \
	do { \
		if (condition) \
			ExceptionalCondition(#condition, (errorType), \
								 __FILE__, __LINE__); \
	} while (0)

   /*
	*	TrapMacro is the same as Trap but it's intended for use in macros:
	*
	*		#define foo(x) (AssertMacro(x != 0), bar(x))
	*
	*	Isn't CPP fun?
	*/
#define TrapMacro(condition, errorType) \
	((bool) (! (condition) || \
			 (ExceptionalCondition(#condition, (errorType), \
								   __FILE__, __LINE__), 0)))

#define Assert(condition) \
	do { \
		if (!(condition)) \
			ExceptionalCondition(#condition, "FailedAssertion", \
								 __FILE__, __LINE__); \
	} while (0)

#define AssertMacro(condition) \
	((void) ((condition) || \
			 (ExceptionalCondition(#condition, "FailedAssertion", \
								   __FILE__, __LINE__), 0)))

#define AssertArg(condition) \
	do { \
		if (!(condition)) \
			ExceptionalCondition(#condition, "BadArgument", \
								 __FILE__, __LINE__); \
	} while (0)

#define AssertState(condition) \
	do { \
		if (!(condition)) \
			ExceptionalCondition(#condition, "BadState", \
								 __FILE__, __LINE__); \
	} while (0)

	/*
	 * Check that `ptr' is `bndr' aligned.
	 */
#define AssertPointerAlignment(ptr, bndr) \
	Trap(TYPEALIGN(bndr, (uintptr_t)(ptr)) != (uintptr_t)(ptr), \
		 "UnalignedPointer")

#endif							/* UKCASSERT_CHECKING && !FRONTEND */


#endif	/* KC_8C7A2455_694D_419A_869F_422FA562E8CF */

/*****************************************************************************
*    昆明畅博科技公司 Copyright (c) 2014-2022
*	 作者：黑哥，2022-01-14
*****************************************************************************/

#ifndef KC_BE4BF34D_F216_4F88_BB1F_74488F92DB63
#define KC_BE4BF34D_F216_4F88_BB1F_74488F92DB63

#include <stdint.h>
#include <KcCore.h>


#define KC_SAFE_MODULE_FREE(ptr) do {\
	if(NULL !=(ptr)  ) { \
		kcSafeFree((ptr)); (ptr) = NULL; \
	} \
} while (0)

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus 1*/

	extern KC_EXTERNAL  void KCAPI  kcSafeFree(char* ptr);

	extern KC_EXTERNAL  int32_t KCAPI  kcStringCopy(const char* source, char** ptr);

#ifdef __cplusplus
};
#endif  /*__cplusplus 2*/



/*
dumpbin /EXPORTS D:\MyWork\2022\01\x64\Debug\dllKc.dll
dumpbin /DEPENDENTS D:\MyWork\2022\01\x64\Debug\dllKc.dll
*/
#endif	/*KC_BE4BF34D_F216_4F88_BB1F_74488F92DB63*/

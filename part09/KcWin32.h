/*****************************************************************************
*	昆明畅博科技公司 Copyright (c) 2014-2022
*	Copyright (c) 2014-2022, Kunming Changbo Technology Co., Ltd.
*	www.kuncb.cn
*	作者：黑哥
*
*	WINDOWS 以服务方式运行
*****************************************************************************/

#ifndef KC_5A076818_04C4_B24C_B324_06C14CE245A3
#define KC_5A076818_04C4_B24C_B324_06C14CE245A3
#ifdef _WIN32

#include <Windows.h>
#include <DbgHelp.h>
#include <conio.h>
#include <direct.h>


//	WinSocket是否初始化
#define KC_IS_WSA_STARTUP		(1<<0)

/// <summary>
/// 程序崩溃时生成dmp文件
/// </summary>
/// <param name="pException"></param>
/// <returns></returns>
LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException);

#endif //_WIN32
#endif	/*KC_5A076818_04C4_B24C_B324_06C14CE245A3 */

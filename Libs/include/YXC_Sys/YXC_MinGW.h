/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_MINGW_H__
#define __INC_YXC_BASE_MINGW_H__

#if __LP64__
#define YXC_IS_64BIT 1
#else
#define YXC_IS_64BIT 0
#endif /* __LP64__ */

#define YXC_PLATFORM_WIN 1
#define YXC_PLATFORM_LINUX 0
#define YXC_PLATFORM_APPLE 0
#define YXC_PLATFORM_MINGW 1
#define YXC_GNU_C 1
#define STACK_SIZE_PARAM_IS_A_RESERVATION 0x00010000

#if YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL
	#define YXC_API(retType) __declspec(dllexport) retType __cdecl
	#define YXC_CLASS __declspec(dllexport)
#else
	#define YXC_API(retType) retType __cdecl
	#define YXC_CLASS
#endif /* YXC_EXPORTS */

#include <Windows.h>

#define YXC_KOBJATTR_ALL_ACCESS NULL

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef HANDLE YXC_KObject;
	typedef SECURITY_ATTRIBUTES YXC_KObjectAttr;
	typedef DWORD YXC_KObjectFlags;
	typedef DWORD etid_t;
	typedef HANDLE ethread_t;

	YXC_API(const OSVERSIONINFOEXW*) YXC_GetOSVersionInfo();

	typedef GUID YXC_Guid;
	typedef HWND YXC_Window;

#if YXC_IS_64BIT
	typedef long long ssize_t;
#else
	typedef int ssize_t;
#endif /* YXC_IS_64BIT */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_BASE_MINGW_H__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_WIN32_H__
#define __INC_YXC_SYS_BASE_WIN32_H__

#ifndef _WINSOCKAPI_ // not include socket header for winsock2 extension
#define _WINSOCKAPI_
#include <windows.h>
#undef _WINSOCKAPI_ /* _WINSOCKAPI_ */
#endif /* _WINSOCKAPI_ */

#if YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL
	#define YXC_API(retType) __declspec(dllexport) retType __cdecl
	#define YXC_CLASS __declspec(dllexport)
#else
	#define YXC_API(retType) retType __cdecl
	#define YXC_CLASS
#endif /* YXC_EXPORTS */

#ifdef _WIN64
#define YXC_IS_64BIT 1
#define YXC_IS_32BIT 0
#else
#define YXC_IS_64BIT 0
#define YXC_IS_32BIT 1
#endif /* _WIN64 */

#define YXC_PLATFORM_WIN 1
#define YXC_PLATFORM_UNIX 0
#define YXC_PLATFORM_APPLE 0

#define YXC_GNU_C 0

#define va_copy(va_dest, va_src) (va_dest) = (va_src)
#define YXC_KOBJATTR_ALL_ACCESS NULL

#if !defined(__BCOPT__)
#define vsnwprintf vswprintf
#endif /* __BCOPT__ */

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
	typedef RECT YXC_Rect;

#if YXC_IS_64BIT
	typedef long long ssize_t;
#else
	typedef int ssize_t;
#endif /* YXC_IS_64BIT */

#ifdef _DEBUG
#define YXC_DEBUG 1
#else
#define YXC_DEBUG 0
#endif /* YXC_DEBUG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_WIN32_H__ */

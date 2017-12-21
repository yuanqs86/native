
/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_C_EXTEND_H__
#define __INC_YXC_BASE_C_EXTEND_H__

#include <YXC_Sys/YXC_Platform.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#if YXC_PLATFORM_WIN /* Include timeval structure. */

#ifndef _WINSOCKAPI_
#include <WinSock2.h>
#define _WINSOCKAPI_
#endif /* _WINSOCKAPI_ */

#endif /* YXC_PLATFORM_WIN */

#if YXC_PLATFORM_UNIX
#define stricmp(p1, p2) strcasecmp(p1, p2)
#define wcsicmp(p1, p2) wcscasecmp(p1, p2)
#define yxcwrap_wcstok(str, delim, p) wcstok(str, delim, p)
#define yxcwrap_strtok(str, delim, p) strtok_r(str, delim, p)
#else
#if !EJ_PLATFORM_MINGW
#include <wchar.h>
#endif
#define yxcwrap_wcstok(str, delim, p) wcstok_s(str, delim, p)
#define yxcwrap_strtok(str, delim, p) strtok_s(str, delim, p)
#endif /* YXC_PLATFORM_UNIX */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    YXC_API(ssize_t) yxcwrap_printf(const char* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_wprintf(const wchar_t* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_vprintf(const char* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_vwprintf(const wchar_t* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_sprintf(char* szDst, const char* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_swprintf(wchar_t* szDst, const wchar_t* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_snprintf(char* szDst, ssize_t sz, const char* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_snwprintf(wchar_t* szDst, ssize_t sz, const wchar_t* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_vsprintf(char* szDst, const char* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_vswprintf(wchar_t* szDst, const wchar_t* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_vsnprintf(char* szDst, ssize_t sz, const char* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_vsnwprintf(wchar_t* szDst, ssize_t sz, const wchar_t* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_fprintf(FILE* fp, const char* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_fwprintf(FILE* fp, const wchar_t* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_vfprintf(FILE* fp, const char* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_vfwprintf(FILE* fp, const wchar_t* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_scprintf(const char* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_scwprintf(const wchar_t* pszFmt, ...);

    YXC_API(ssize_t) yxcwrap_vscprintf(const char* pszFmt, va_list ls);

    YXC_API(ssize_t) yxcwrap_vscwprintf(const wchar_t* pszFmt, va_list ls);

    YXC_API(int) yxcwrap_gettimeofday(struct timeval* tm_of_day);

    YXC_API(long long) yxcwrap_getelapsed();

	YXC_API(long long) yxcwrap_getcpu_clock();

	YXC_API(uintptr_t) yxcwrap_getstack_top();

	YXC_API(int) yxcwrap_setenv(const char* key, const char* val);

	YXC_API(int) yxcwrap_wsetenv(const wchar_t* key, const wchar_t* val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_BASE_C_EXTEND_H__ */

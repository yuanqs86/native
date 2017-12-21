/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_CHARACTERS_H__
#define __INC_YXC_SYS_BASE_CHARACTERS_H__

#include <YXC_Sys/YXC_Platform.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if YXC_PLATFORM_WIN
#define YCHAR_WCHAR_T 1
#elif YXC_PLATFORM_UNIX
#define YCHAR_WCHAR_T 0
#endif /* YXC_PLATFORM_WIN */

#if YCHAR_WCHAR_T
    typedef wchar_t ychar;
#else
    typedef char ychar;
#endif /* YCHAR_WCHAR_T */

#if YCHAR_WCHAR_T
#define YC(x) L##x
#define yh_strchr(str, ch) wcschr(str, ch)
#define yh_strrchr(str, ch) wcsrchr(str, ch)
#define yh_strcpy(dst, src) wcscpy(dst, src)
#define yh_strncpy(dst, src, n) wcsncpy(dst, src, n)
#define yh_strcmp(src1, src2) wcscmp(src1, src2)
#define yh_stricmp(src1, src2) wcsicmp(src1, src2)
#define yh_strncmp(src1, src2, n) wcsncmp(src1, src2, n)
#define yh_strcat(dst, src) wcscat(dst, src)
#define yh_strncat(dst, src, n) wcsncat(dst, src, n)
#define yh_strstr(dst, src) wcsstr(dst, src)
#define yh_strlen(str) wcslen(str)
#define yh_strdup(str) wcsdup(str)
#define yh_setenv(key, val) yxcwrap_wsetenv(key, val)

#define yh_strtok(str, delim, ptr) yxcwrap_wcstok(str, delim, ptr)

#if YXC_GNU_C

#define yh_scanf(format, args...) wscanf(format, ##args)
#define yh_sscanf(str, format, args...) swscanf(str, format, ##args)
#define yh_printf(format, ...) yxcwrap_wprintf(format, ##args)
#define yh_sprintf(dst, format, ...) yxcwrap_swprintf(dst, format, ##args)
#define yh_snprintf(str, size, format, ...) yxcwrap_snwprintf(str, size, format, ##args)
#define yh_scprintf(format, ...) yxcwrap_scwprintf(format, ##args)

#else

#define yh_scanf(format, ...) wscanf(format, ##__VA_ARGS__)
#define yh_sscanf(str, format, ...) swscanf(str, format, ##__VA_ARGS__)
#define yh_printf(format, ...) yxcwrap_wprintf(format, ##__VA_ARGS__)
#define yh_sprintf(dst, format, ...) yxcwrap_swprintf(dst, format, ##__VA_ARGS__)
#define yh_snprintf(str, size, format, ...) yxcwrap_snwprintf(str, size, format, ##__VA_ARGS__)
#define yh_scprintf(format, ...) yxcwrap_scwprintf(format, ##__VA_ARGS__)

#endif /* YXC_GNU_C */

#define yh_vprintf(format, va_list) yxcwrap_vwprintf(format, va_list)
#define yh_vsprintf(str, format, ls) yxcwrap_vswprintf(str, format, ls)
#define yh_vsnprintf(str, size, format, ls) yxcwrap_vsnwprintf(str, size, format, ls)
#define yh_vscprintf(format, ls) yxcwrap_vscwprintf(format, ls)

#define yh_fopen(path, mode) _wfopen(path, mode)
#define yh_fputs(str, fp) fputws(str, fp)

#else
#define YC(x) x
#define yh_strchr(str, ch) strchr(str, ch)
#define yh_strrchr(str, ch) strrchr(str, ch)
#define yh_strcpy(dst, src) strcpy(dst, src)
#define yh_strncpy(dst, src, n) strncpy(dst, src, n)
#define yh_strcmp(src1, src2) strcmp(src1, src2)
#define yh_stricmp(src1, src2) stricmp(src1, src2)
#define yh_strncmp(src1, src2, n) strncmp(src1, src2, n)
#define yh_strcat(dst, src) strcat(dst, src)
#define yh_strncat(dst, src, n) strncat(dst, src, n)
#define yh_strstr(str, substr) strstr(str, substr)
#define yh_strlen(str) strlen(str)
#define yh_strdup(str) strdup(str)
#define yh_setenv(key, val) yxcwrap_setenv(key, val)

#define yh_strtok(str, delim, ptr) yxcwrap_strtok(str, delim, ptr)

#if YXC_GNU_C

#define yh_scanf(format, args...) scanf(format, ##args)
#define yh_sscanf(str, format, args...) sscanf(str, format, ##args)
#define yh_printf(format, ...) yxcwrap_printf(format, ##args)
#define yh_sprintf(dst, format, ...) yxcwrap_sprintf(dst, format, ##args)
#define yh_snprintf(str, size, format, ...) yxcwrap_snprintf(str, size, format, ##args)
#define yh_scprintf(format, ...) yxcwrap_scprintf(format, ##args)

#else

#define yh_scanf(format, ...) scanf(format, ##__VA_ARGS__)
#define yh_sscanf(str, format, ...) sscanf(str, format, ##__VA_ARGS__)
#define yh_printf(format, ...) yxcwrap_printf(format, ##__VA_ARGS__)
#define yh_sprintf(dst, format, ...) yxcwrap_sprintf(dst, format, ##__VA_ARGS__)
#define yh_snprintf(str, size, format, ...) yxcwrap_snprintf(str, size, format, ##__VA_ARGS__)
#define yh_scprintf(format, ...) yxcwrap_scprintf(format, ##__VA_ARGS__)

#endif /* YXC_GNU_C */

#define yh_vprintf(format, va_list) vprintf(format, va_list)
#define yh_vsprintf(str, format, ls) yxcwrap_vsprintf(str, format, ls)
#define yh_vsnprintf(str, size, format, ls) yxcwrap_vsnprintf(str, size, format, ls)
#define yh_vscprintf(format, ls) yxcwrap_vscprintf(format, ls)

#define yh_fopen(path, mode) fopen(path, mode)
#define yh_fputs(str, fp) fputs(str, fp)
#endif /* YCHAR_WCHAR_T */

	YXC_API(wchar_t*) __efunction(const char* func);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus

#include <string>
#if YCHAR_WCHAR_T
    typedef std::wstring estring_t;
#else
    typedef std::string estring_t;
#endif /* YCHAR_WCHAR_T */

#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_CHARACTERS_H__ */

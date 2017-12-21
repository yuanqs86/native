#include <stdio.h>
#include <YXC_Sys/YXC_Sys.h>
#include <limits.h>
#include <time.h>

#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#define isnumber isdigit
#endif /* YXC_PLATFORM_WIN */

#if YXC_PLATFORM_UNIX
#include <sys/time.h>
#endif /* YXC_PLATFORM_UNIX */

#if YXC_PLATFORM_APPLE
#include <mach/mach_time.h>

static mach_timebase_info_data_t g_timebase_info = {0, 0};
static kern_return_t gs_info_init = mach_timebase_info(&g_timebase_info);
static yint64_t gs_epoch = mach_absolute_time();

#define CLOCK_MONOTONIC 192996728
static int clock_gettime(int which, struct timespec* tp)
{
    yint64_t now = mach_absolute_time();

    yint64_t nanoelapsed = (now - gs_epoch) * (g_timebase_info.numer / g_timebase_info.denom);

    tp->tv_sec = nanoelapsed / 1000000000;
    tp->tv_nsec = nanoelapsed % 1000000000;

    return 0;
}
#elif YXC_PLATFORM_UNIX
#define isnumber isdigit
#endif /* YXC_PLATFORM_APPLE */

#if YXC_PLATFORM_ANDROID
static int isdigit(char num)
{
	return num >= '0' && num <= '9';
}
#endif /* YXC_PLATFORM_ANDROID */

namespace _CImpl
{
#if YXC_PLATFORM_WIN
    static int gettimeofday(struct timeval *tp, void *tzp)
    {
        time_t clock;
        struct tm tm;
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        tm.tm_year     = wtm.wYear - 1900;
        tm.tm_mon     = wtm.wMonth - 1;
        tm.tm_mday     = wtm.wDay;
        tm.tm_hour     = wtm.wHour;
        tm.tm_min     = wtm.wMinute;
        tm.tm_sec     = wtm.wSecond;
        tm. tm_isdst    = -1;
        clock = mktime(&tm);
        tp->tv_sec = clock;
        tp->tv_usec = wtm.wMilliseconds * 1000;
        return 0;
    }

	static DWORD gs_tickLoops = 0;
	static DWORD gs_lastTick = 0;
	static YXX_Crit gs_tickCrit(4000);

    static long long getelapsed()
    {
		DWORD dwLoops = 0;

		gs_tickCrit.Lock();
		DWORD dwCurrentTick = timeGetTime();
		if (dwCurrentTick < gs_lastTick) /* Loop back tick. */
		{
			gs_tickLoops++;
		}
		gs_lastTick = dwCurrentTick;
		dwLoops = gs_tickLoops;
		gs_tickCrit.Unlock();

		yuint64_t all_tick = (yuint64_t)dwLoops << 32;
		all_tick += dwCurrentTick;
		return (yint64_t)all_tick;
    }
#elif YXC_PLATFORM_UNIX
    using ::gettimeofday;

    static long long getelapsed()
    {
        struct timespec tp;
        clock_gettime(CLOCK_MONOTONIC, &tp);

        return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    }
#endif /* YXC_PLATFORM_WIN */

    static void _change_format(const char* pszSrcFmt, char* pszDstFmt, size_t len)
    {
        const char* s = pszSrcFmt;
        char* d = pszDstFmt;

        int in_fmt = FALSE;

        while (s < pszSrcFmt + len)
        {
            char c = *s++;
            if (in_fmt)
            {
                if (isnumber(c) || c == '-' || c == '[' || c == ']') /* Is format. */
                {
                    *d++ = c;
                }
                else
                {
                    in_fmt = FALSE;
                    if (c == '@')
                    {
                        *d++ = 's';
                    }
                    else
                    {
                        *d++ = c;
                    }
                }
            }
            else
            {
                *d++ = c;
                if (c == '%') /* Is a format. */
                {
                    in_fmt = TRUE;
                }
            }
        }

		pszDstFmt[len] = 0;
    }

    static void _change_format(const wchar_t* pszSrcFmt, wchar_t* pszDstFmt, size_t len)
    {
        const wchar_t* s = pszSrcFmt;
        wchar_t* d = pszDstFmt;

        int in_fmt = FALSE;

        while (s < pszSrcFmt + len)
        {
            wchar_t c = *s++;
            if (in_fmt)
            {
                if (isnumber((int)c) || c == '-' || c == '[' || c == ']') /* Is format. */
                {
                    *d++ = c;
                }
                else
                {
                    in_fmt = FALSE;
                    if (c == '@') /* switch this to '%s' in linux and '%S' on windows. */
                    {
#if YXC_PLATFORM_WIN
                        *d++ = 'S';
#else
                        *d++ = 's';
#endif /* YXC_PLATFORM_WIN */
                    }
                    else
                    {
                        *d++ = c;
                    }
                }
            }
            else
            {
                *d++ = c;
                if (c == '%') /* Is a format. */
                {
                    in_fmt = TRUE;
                }
            }
		}
		pszDstFmt[len] = 0;
    }
}

extern "C"
{
    ssize_t yxcwrap_printf(const char* pszFmt, ...)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
        ssize_t iRet = vprintf(szFmt2, vaList);
        va_end(vaList);

        return iRet;
    }

    ssize_t yxcwrap_wprintf(const wchar_t* pszFmt, ...)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
        ssize_t iRet = vwprintf(szFmt2, vaList);
        va_end(vaList);

        return iRet;
    }

    ssize_t yxcwrap_vprintf(const char* pszFmt, va_list ls)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
        ssize_t iRet = vprintf(szFmt2, vaList);
        return iRet;
    }

    ssize_t yxcwrap_vwprintf(const wchar_t* pszFmt, va_list ls)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
        ssize_t iRet = vwprintf(szFmt2, vaList);
        return iRet;
    }

    ssize_t yxcwrap_sprintf(char* szDst, const char* pszFmt, ...)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
        ssize_t iRet = vsprintf(szDst, szFmt2, vaList);
        va_end(vaList);

        return iRet;
    }

    ssize_t yxcwrap_swprintf(wchar_t* szDst, const wchar_t* pszFmt, ...)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
#if YXC_PLATFORM_WIN
        int ret = vswprintf(szDst, szFmt2, vaList);
#else
        ssize_t ret = vswprintf(szDst, YXC_SSIZE_MAX, szFmt2, vaList);
#endif /* YXC_PLATFORM_WIN */
        va_end(vaList);

        return ret;
    }

    ssize_t yxcwrap_snprintf(char* szDst, ssize_t sz, const char* pszFmt, ...)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
        ssize_t iRet = vsnprintf(szDst, sz, szFmt2, vaList);
        va_end(vaList);

        return iRet;
    }

    ssize_t yxcwrap_snwprintf(wchar_t* szDst, ssize_t sz, const wchar_t* pszFmt, ...)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
#if YXC_PLATFORM_WIN
        ssize_t iRet = vsnwprintf(szDst, sz, szFmt2, vaList);
        va_end(vaList);
#else
	ssize_t iRet = vswprintf(szDst, sz, szFmt2, vaList);
        va_end(vaList);
#endif /* YXC_PLATFORM_WIN */

        return iRet;
    }

    ssize_t yxcwrap_vsprintf(char* szDst, const char* pszFmt, va_list ls)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
        ssize_t iRet = vsprintf(szDst, szFmt2, vaList);
        return iRet;
    }

    ssize_t yxcwrap_vswprintf(wchar_t* szDst, const wchar_t* pszFmt, va_list ls)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
#if YXC_PLATFORM_WIN
        int ret = vswprintf(szDst, szFmt2, vaList);
#else
        ssize_t ret = vswprintf(szDst, YXC_SSIZE_MAX, szFmt2, vaList);
#endif /* YXC_PLATFORM_WIN */
        return ret;
    }

    ssize_t yxcwrap_vsnprintf(char* szDst, ssize_t sz, const char* pszFmt, va_list ls)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
        ssize_t iRet = vsnprintf(szDst, sz, szFmt2, vaList);
        return iRet;
    }

    ssize_t yxcwrap_vsnwprintf(wchar_t* szDst, ssize_t sz, const wchar_t* pszFmt, va_list ls)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
#if YXC_PLATFORM_WIN
        ssize_t ret = vsnwprintf(szDst, sz, szFmt2, vaList);
#else
	ssize_t ret = vswprintf(szDst, sz, szFmt2, vaList);
#endif /* YXC_PLATFORM_WIN */
        return ret;
    }

    ssize_t yxcwrap_fprintf(FILE* fp, const char* pszFmt, ...)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
        ssize_t iRet = vfprintf(fp, szFmt2, vaList);
        va_end(vaList);

        return iRet;
    }

    ssize_t yxcwrap_fwprintf(FILE* fp, const wchar_t* pszFmt, ...)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
        ssize_t ret = vfwprintf(fp, szFmt2, vaList);
        va_end(vaList);

        return ret;
    }

    ssize_t yxcwrap_vfprintf(FILE* fp, const char* pszFmt, va_list ls)
    {
        size_t len = strlen(pszFmt);

        char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
        ssize_t iRet = vfprintf(fp, szFmt2, vaList);
        return iRet;
    }

    ssize_t yxcwrap_vfwprintf(FILE* fp, const wchar_t* pszFmt, va_list ls)
    {
        size_t len = wcslen(pszFmt);

        wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
        if (szFmt2 == NULL) return -1;

        YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);

        _CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_copy(vaList, ls);
        ssize_t ret = vfwprintf(fp, szFmt2, vaList);
        return ret;
    }

    ssize_t yxcwrap_scprintf(const char* pszFmt, ...)
	{
		size_t len = strlen(pszFmt);

		char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
		if (szFmt2 == NULL) return -1;

		YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);
		_CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);

#if YXC_PLATFORM_WIN
        int ret = _vscprintf(szFmt2, vaList);
        va_end(vaList);
        return ret;
#else
        ssize_t ret = vsnprintf(NULL, 0, szFmt2, vaList);
        va_end(vaList);
        return ret;
#endif /* YXC_PLATFORM_WIN */
    }

    ssize_t yxcwrap_scwprintf(const wchar_t* pszFmt, ...)
	{
		size_t len = wcslen(pszFmt);

		wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
		if (szFmt2 == NULL) return -1;

		YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);
		_CImpl::_change_format(pszFmt, szFmt2, len);

        va_list vaList;
        va_start(vaList, pszFmt);
#if YXC_PLATFORM_WIN
        int ret = _vscwprintf(szFmt2, vaList);
        va_end(vaList);
        return ret;
#else
        ssize_t ret = vswprintf(NULL, YXC_SSIZE_MAX, szFmt2, vaList);
        va_end(vaList);
        return ret;
#endif /* YXC_PLATFORM_WIN */
    }

    ssize_t yxcwrap_vscprintf(const char* pszFmt, va_list ls)
	{
		size_t len = strlen(pszFmt);

		char* szFmt2 = (char*)malloc((len + 1) * sizeof(char));
		if (szFmt2 == NULL) return -1;

		YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);
		_CImpl::_change_format(pszFmt, szFmt2, len);

        va_list list2;
        va_copy(list2, ls);
#if YXC_PLATFORM_WIN
        int ret = _vscprintf(szFmt2, list2);
        return ret;
#else
        ssize_t ret = vsnprintf(NULL, 0, szFmt2, list2);
        return ret;
#endif /* YXC_PLATFORM_WIN */
    }

    ssize_t yxcwrap_vscwprintf(const wchar_t* pszFmt, va_list ls)
	{
		size_t len = wcslen(pszFmt);

		wchar_t* szFmt2 = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
		if (szFmt2 == NULL) return -1;

		YXCLib::HandleRef<void*> szFmt2_res(szFmt2, free);
		_CImpl::_change_format(pszFmt, szFmt2, len);

        va_list list2;
        va_copy(list2, ls);
#if YXC_PLATFORM_WIN
        int ret = _vscwprintf(szFmt2, list2);
        return ret;
#else
        ssize_t ret = vswprintf(NULL, YXC_SSIZE_MAX, szFmt2, list2);
        return ret;
#endif /* YXC_PLATFORM_WIN */
    }

	int yxcwrap_setenv(const char* str, const char* val)
	{
#if YXC_PLATFORM_WIN
		BOOL bRet = ::SetEnvironmentVariableA(str, val);
		if (!bRet) return ::GetLastError();
		return 0;
#else
		return 0;
#endif /* YXC_PLATFORM_WIN */
	}

	int yxcwrap_wsetenv(const wchar_t* str, const wchar_t* val)
	{
#if YXC_PLATFORM_WIN
		BOOL bRet = ::SetEnvironmentVariableW(str, val);
		if (!bRet) return ::GetLastError();
		return 0;
#else
		return 0;
#endif /* YXC_PLATFORM_WIN */
	}

    int yxcwrap_gettimeofday(struct timeval* tm_of_day)
    {
        return _CImpl::gettimeofday(tm_of_day, 0);
    }

    long long yxcwrap_getelapsed()
    {
        return _CImpl::getelapsed();
    }

#if YXC_PLATFORM_UNIX
	int get_rdtsc() {
		asm("rdtsc");
	}
#endif /* YXC_PLATFORM_UNIX */

	long long yxcwrap_getcpu_clock()
	{
#if YXC_PLATFORM_WIN
		return __rdtsc();
#else
		return get_rdtsc();
#endif /* YXC_PLATFORM_WIN */
	}

	yuintptr_t _getstack_top();

#if YXC_PLATFORM_WIN
	yuintptr_t yxcwrap_getstack_top()
	{
#if YXC_IS_64BIT
		return _getstack_top();
#else
		yuintptr_t stackTop;
		__asm
		{
			mov eax, fs:[4]
			mov stackTop, eax;
		}

		return stackTop;
#endif /* YXC_IS_64BIT */
	}
#else
	uintptr_t yxcwrap_getstack_top() { return 0; }
#endif /* YXC_PLATFORM_WIN */
}

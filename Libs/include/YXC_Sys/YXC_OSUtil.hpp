/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_OS_UTIL_HPP__
#define __INC_YXC_SYS_BASE_OS_UTIL_HPP__

#include <YXC_Sys/YXC_Platform.h>
#include <YXC_Sys/YXC_Types.h>
#include <YXC_Sys/YXC_ErrorCode.h>
#include <YXC_Sys/YXC_Locker.hpp>

#if YXC_PLATFORM_UNIX
#include <sys/types.h>
#include <dirent.h>
#endif /* YXC_PLATFORM_UNIX */

namespace YXCLib
{
	inline long long OSGetLastError()
	{
#if YXC_PLATFORM_WIN
		return ::GetLastError();
#else
		return errno;
#endif /* YXC_PLATFORM_WIN */
	}

	inline void OSSleep(yuint32_t uMilliSeconds)
	{
#if YXC_PLATFORM_WIN
		Sleep(uMilliSeconds);
#elif YXC_PLATFORM_UNIX
        usleep(uMilliSeconds * 1000);
#else
#error Not support OSSleep
#endif /* YXC_PLATFORM_WIN */
	}

	inline etid_t OSGetCurrentThreadId()
	{
#if YXC_PLATFORM_WIN
		return GetCurrentThreadId();
#elif YXC_PLATFORM_APPLE
        pthread_t thr = pthread_self();
        etid_t ret;
        pthread_threadid_np(thr, (unsigned long long*)&ret);
        return ret;
#elif YXC_PLATFORM_UNIX
		pthread_t thr = pthread_self();
		return thr;
#else
#error Not support OSGetCurrentThreadId
#endif /* YXC_PLATFORM_WIN */
	}

	inline yuint32_t OSGetCurrentProcessId()
	{
#if YXC_PLATFORM_WIN
		return GetCurrentProcessId();
#elif YXC_PLATFORM_UNIX
        return getpid();
#else
#error Not support OSGetCurrentProcessId
#endif /* YXC_PLATFORM_WIN */
	}

    inline void OSCloseThreadHandle(ethread_t upThread)
	{
#if YXC_PLATFORM_WIN
		CloseHandle((HANDLE)upThread);
#elif YXC_PLATFORM_UNIX
        pthread_detach(upThread);
#else
#error Not support OSCloseThreadHandle
#endif /* YXC_PLATFORM_WIN */
	}

	inline void OSWaitThread(ethread_t upThread)
	{
#if YXC_PLATFORM_WIN
		WaitForSingleObject((HANDLE)upThread, INFINITE);
#elif YXC_PLATFORM_UNIX
        pthread_join(upThread, NULL);
#else
#error Not support OSWaitThread
#endif /* YXC_PLATFORM_WIN */
	}

	typedef yuint32_t (__stdcall *ThreadStartAddr)(void* pParam);

#if YXC_PLATFORM_UNIX
    struct __ux_thread_struct
    {
        ThreadStartAddr func;
        void* param;
    };

    static inline void* __ux_thread_proc(void* param)
    {
        __ux_thread_struct* ptr_struct = (__ux_thread_struct*)param;

        ThreadStartAddr func = ptr_struct->func;
        void* param_in = ptr_struct->param;

        free(ptr_struct);

        return reinterpret_cast<void*>(func(param_in));
    }

    typedef int (*__ux_thread_create_proc)(pthread_t*, const pthread_attr_t*, void* (*func)(void*), void* help);

    static inline pthread_t __ux_create_thread(__ux_thread_create_proc proc, ThreadStartAddr pfnThread, void* pParam, etid_t* puThreadId)
    {
        __ux_thread_struct* pts = (__ux_thread_struct*)malloc(sizeof(__ux_thread_struct));
        if (pts == NULL) return 0;

        pts->func = pfnThread;
        pts->param = pParam;

        pthread_t thr;
        int nc = proc(&thr, NULL, __ux_thread_proc, pts);
        if (nc != 0)
        {
            free(pts);
            return 0;
        }

	if (puThreadId != NULL)
	{
#if YXC_PLATFORM_APPLE
        	pthread_threadid_np(thr, (__uint64_t*)puThreadId);
#else
		*puThreadId = thr;
#endif /* YXC_PLATFORM_APPLE */
	}

        return thr;
    }
#endif /* YXC_PLATFORM_UNIX */

	inline ethread_t OSCreateThread(ThreadStartAddr pfnThread, void* pParam, etid_t* puThreadId)
	{
#if YXC_PLATFORM_WIN
		ethread_t upRet = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pfnThread, pParam, 0, (DWORD*)puThreadId);
        return upRet;
#elif YXC_PLATFORM_UNIX
        ethread_t upRet = (ethread_t)__ux_create_thread(pthread_create, pfnThread, pParam, puThreadId);
        return upRet;
#else
#error Not support OSCreateThread
#endif /* YXC_PLATFORM_WIN */
	}

#if YXC_PLATFORM_WIN || YXC_PLATFORM_APPLE
	inline ethread_t OSCreateThreadEx(ThreadStartAddr pfnThread, ysize_t stStackSize, ybool_t bIsSuspended,
		void* pParam, etid_t* puThreadId)
	{
#if YXC_PLATFORM_WIN
		DWORD dwCreationFlags = STACK_SIZE_PARAM_IS_A_RESERVATION;
		if (bIsSuspended)
		{
			dwCreationFlags |= CREATE_SUSPENDED;
		}
		return CreateThread(NULL, stStackSize, (LPTHREAD_START_ROUTINE)pfnThread, pParam,
			dwCreationFlags, (DWORD*)puThreadId);
#elif YXC_PLATFORM_APPLE
        pthread_attr_t attr;
        int rc = pthread_attr_init(&attr);
        if (rc != 0) return 0;

		if (stStackSize != 0)
		{
			pthread_attr_setstacksize(&attr, stStackSize);
		}

        __ux_thread_create_proc proc = bIsSuspended ? pthread_create_suspended_np : pthread_create;
        ethread_t thr = __ux_create_thread(proc, pfnThread, pParam, puThreadId);

        pthread_attr_destroy(&attr);
        return thr;
#endif /* YXC_PLATFORM_WIN */
	}

	inline YXC_Status OSResumeThread(ethread_t upThread)
	{
#if YXC_PLATFORM_WIN
		DWORD dwRet = ResumeThread(upThread);
		return dwRet != (DWORD)-1 ? YXC_ERC_SUCCESS : YXC_ERC_OS;
#elif YXC_PLATFORM_APPLE
        mach_port_t mp = pthread_mach_thread_np(upThread);
        thread_resume(mp);
        return YXC_ERC_SUCCESS;
#else
		return YXC_ERC_OS;
#endif /* YXC_PLATFORM_WIN */
	}

	inline YXC_Status OSSuspendThread(ethread_t upThread)
	{
#if YXC_PLATFORM_WIN
		DWORD dwRet = SuspendThread((HANDLE)upThread);
		return dwRet != (DWORD)-1 ? YXC_ERC_SUCCESS : YXC_ERC_OS;
#elif YXC_PLATFORM_APPLE
        mach_port_t mp = pthread_mach_thread_np((pthread_t)upThread);
        thread_suspend(mp);
        return YXC_ERC_SUCCESS;
#else
		return YXC_ERC_OS;
#endif /* YXC_PLATFORM_WIN */
	}

#endif /* YXC_PLATFORM_WIN || YXC_PLATFORM_APPLE */

	inline ethread_t OSCreateThreadStack(ThreadStartAddr pfnThread, ysize_t stStackSize, void* pParam, etid_t* puThreadId)
	{
#if YXC_PLATFORM_WIN || YXC_PLATFORM_APPLE
		return OSCreateThreadEx(pfnThread, stStackSize, FALSE, pParam, puThreadId);
#elif YXC_PLATFORM_UNIX
		pthread_attr_t attr;
		int rc = pthread_attr_init(&attr);
		if (rc != 0) return 0;

		if (stStackSize != 0)
		{
			pthread_attr_setstacksize(&attr, stStackSize);
		}

		__ux_thread_create_proc proc = pthread_create;
		ethread_t thr = __ux_create_thread(proc, pfnThread, pParam, puThreadId);

		pthread_attr_destroy(&attr);
		return thr;
#else
#error Not support OSCreateThread
#endif /* YXC_PLATFORM_WIN */
	}

	inline ybool_t _IsLittleEndianCPU()
	{
		ybyte_t randomNumber = (ybyte_t)rand();
		ybyte_t randomNumber2 = randomNumber + 1;

		yuint16_t s = (randomNumber << 8) | randomNumber2;
		return *(ybyte_t*)&s == randomNumber2;
	}

    struct OSFileAttr
    {
        YXC_FPath osFilePath;
        ybool_t bIsDir;
        yuint64_t fileSize;
        //euint32_t uFAttr;
        yuint64_t accessTime;
        yuint64_t createTime;
        yuint64_t modifyTime;
    };

    class OSFileFinder
    {
    public:
        OSFileFinder();

        ~OSFileFinder();

    public:
        YXC_Status Open(const ychar* fPath);

        void Close();

        YXC_Status Next(OSFileAttr* nextAttr);

        YXC_Status SelfAttr(OSFileAttr* pSelfAttr);

    public:
        inline ybool_t IsDir() const { return this->_bIsDir; }

    private:
        YXC_FPath _fPath;

        ybool_t _bIsDir;
#if YXC_PLATFORM_WIN
        HANDLE _hFinder;
#else
        DIR* _pDir;
#endif /* YXC_PLATFORM_WIN */
    };

	class OSSharedMemory
	{
    public:
        OSSharedMemory();

        ~OSSharedMemory();

	public:
		YXC_Status CreateA(const char* pszName, yuint64_t u64ShmSize, ybool_t* pbCreatedNew);

		YXC_Status CreateW(const ychar* pszName, yuint64_t u64ShmSize, ybool_t* pbCreatedNew);

		YXC_Status OpenW(const ychar* pszName);

		YXC_Status Map(yuint64_t u64FileOffset, ysize_t stMapSize, void** ppRetMapped);

		void Unmap(void* pMappedPtr);

		YXC_Status Close();

	private:
#if YXC_PLATFORM_WIN
		HANDLE _hFileMapping;
#elif YXC_PLATFORM_UNIX
        int _fd;
#else
#error Not support OSSharedMemory
#endif /* YXC_PLATFORM_WIN */
	};

	enum ThreadStatus
	{
		THREAD_STATUS_UNKNOWN = 0,
		THREAD_STATUS_RUNNING = 1,
		THREAD_STATUS_PERIOD_RUNNING = 2,
		THREAD_STATUS_PAUSED = 3,
		THREAD_STATUS_STOPPED = 4,
		THREAD_STATUS_COMPLETED = 5,
	};

	inline ybool_t CVThreadMatchFunc(const void* pCond, void* pVal)
	{
		const ThreadStatus* pStatus = (const ThreadStatus*)pCond;
		return *pStatus != THREAD_STATUS_PAUSED && *pStatus != THREAD_STATUS_PERIOD_RUNNING;
	}

	inline void CVThreadChangeStatusFunc(void* pData, void* pExt)
	{
		ThreadStatus* pStatus = (ThreadStatus*)pData;
		*pStatus = *(ThreadStatus*)pExt;
	}

#if YXC_PLATFORM_WIN
	inline yuint64_t _FileTimeToGTime(const FILETIME& fTime)
	{
		yuint64_t u64Time = ((yuint64_t)fTime.dwHighDateTime << 32) | fTime.dwLowDateTime;
		return (u64Time - 116444736000000000LL) / 10000000;
	}

	inline FILETIME _GTimeToFileTime(yuint64_t gTime)
	{
		yuint64_t u64Time = gTime * 10000000 + 116444736000000000LL;
		FILETIME fTime;

		fTime.dwHighDateTime = YXC_HI_32BITS(u64Time);
		fTime.dwLowDateTime = YXC_LO_32BITS(u64Time);

		return fTime;
	}
#endif /* YXC_PLATFORM_WIN */

    template <typename T>
    static inline void TDelete(T* object)
    {
        object->~T();
        free(object);
    }

	template <typename T>
	static inline void TDeleteNP(T* object)
	{
		delete object;
	}

    template <typename T>
    static inline void TFree(T* object)
    {
        free(object);
    }
}

#endif /* __INC_YXC_SYS_BASE_OS_UTIL_HPP__ */

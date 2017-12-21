/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_LOCKER_H__
#define __INC_YXC_SYS_BASE_LOCKER_H__

#include <YXC_Sys/YXC_SysBase.h>
#include <YXC_Sys/YXC_ErrorCode.h>

#define YXC_SEMAPHORE_MAX_COUNT (1 << 16)

#if YXC_PLATFORM_UNIX

#include <pthread.h>
#include <fcntl.h>
#include <limits.h>

#endif /* YXC_PLATFORM_UNIX */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if YXC_PLATFORM_WIN
	typedef HANDLE YXC_Mutex;
	typedef HANDLE YXC_Sem;
	typedef HANDLE YXC_Event;
	typedef CRITICAL_SECTION YXC_Crit;

	YXC_INLINE YXC_Status YXC_NamedMutexCreateW(YXC_KObjectAttr* pAttr, ybool_t bInitOwned, const ychar* pszName,
		YXC_Mutex* pNamedMutex, ybool_t* pbCreatedNew)
	{
		HANDLE hMutex = ::CreateMutexW(pAttr, bInitOwned, pszName);
		if (hMutex == NULL) return YXC_ERC_OS;

		*pNamedMutex = hMutex;
		*pbCreatedNew = GetLastError() != ERROR_ALREADY_EXISTS;
		return YXC_ERC_SUCCESS;
	}

	YXC_INLINE YXC_Status YXC_MutexOpenW(YXC_KObjectFlags uFlags, const ychar* pszName, YXC_Mutex* pNamedMutex)
	{
		DWORD dwFlags = uFlags == 0 ? MUTEX_ALL_ACCESS : uFlags;
		HANDLE hMutex = ::OpenMutexW(dwFlags, FALSE, pszName);
		if (hMutex == NULL) return YXC_ERC_OS;

		*pNamedMutex = hMutex;
		return YXC_ERC_SUCCESS;
	}

	YXC_INLINE YXC_Status YXC_MutexCreate(ybool_t bInitOwned, YXC_Mutex* pNamedMutex)
	{
		ybool_t bCreatedNew;
		return YXC_NamedMutexCreateW(NULL, bInitOwned, NULL, pNamedMutex, &bCreatedNew);
	}

	YXC_INLINE HANDLE YXC_HandleDuplicate(HANDLE obj)
	{
		HANDLE hRet;
		BOOL bRet = ::DuplicateHandle(GetCurrentProcess(), obj, GetCurrentProcess(), &hRet, 0, FALSE, DUPLICATE_SAME_ACCESS);

		if (!bRet) return NULL;
		return hRet;
	}

	YXC_INLINE YXC_Status YXC_WaitSingleKObjectTimeout(HANDLE hObject, yuint32_t stmsTimeout)
	{
		DWORD dwRet = ::WaitForSingleObject(hObject, stmsTimeout);
		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			return YXC_ERC_SUCCESS;
		case WAIT_TIMEOUT:
			return YXC_ERC_TIMEOUT;
		case WAIT_ABANDONED:
			return YXC_ERC_WAIT_ABANDONED;
		default:
			return YXC_ERC_OS;
		}
	}

	YXC_INLINE YXC_Status YXC_WaitSingleKObject(HANDLE hObject)
	{
		DWORD dwRet = ::WaitForSingleObject(hObject, INFINITE);
		switch (dwRet)
		{
		case WAIT_OBJECT_0:
			return YXC_ERC_SUCCESS;
		case WAIT_ABANDONED:
			return YXC_ERC_WAIT_ABANDONED;
		default:
			return YXC_ERC_OS;
		}
	}

	YXC_INLINE YXC_Status YXC_MutexLock(YXC_Mutex mutex)
	{
		return YXC_WaitSingleKObject(mutex);
	}

	YXC_INLINE YXC_Status YXC_MutexLockTimeout(YXC_Mutex mutex, yuint32_t stmsTimeout)
	{
		return YXC_WaitSingleKObjectTimeout(mutex, stmsTimeout);
	}

	YXC_INLINE YXC_Status YXC_MutexUnlock(YXC_Mutex mutex)
	{
		BOOL bRet = ::ReleaseMutex(mutex);
		return bRet ? YXC_ERC_SUCCESS : YXC_ERC_OS;
	}

	YXC_INLINE void YXC_MutexDestroy(YXC_Mutex mutex)
	{
		::CloseHandle(mutex);
	}

	YXC_INLINE YXC_Status YXC_NamedSemCreateW(YXC_KObjectAttr* pAttr, yuint32_t uNumResources, const ychar* pszName, YXC_Sem* pSem, ybool_t* pbCreatedNew)
	{
		HANDLE hSem = ::CreateSemaphoreW(NULL, uNumResources, YXC_SEMAPHORE_MAX_COUNT, pszName);
		if (hSem == NULL) return YXC_ERC_OS;

		*pSem = hSem;
		*pbCreatedNew = GetLastError() != ERROR_ALREADY_EXISTS;
		return YXC_ERC_SUCCESS;
	}

	YXC_INLINE YXC_Status YXC_SemOpenW(YXC_KObjectFlags flags, const ychar* pszName, YXC_Sem* pSem)
	{
		DWORD dwFlags = flags == 0 ? SEMAPHORE_ALL_ACCESS : flags;
		HANDLE hSem = ::OpenSemaphoreW(dwFlags, FALSE, pszName);
		if (hSem == NULL) return YXC_ERC_OS;

		*pSem = hSem;
		return YXC_ERC_SUCCESS;
	}

	YXC_INLINE YXC_Status YXC_SemCreate(yuint32_t uNumResources, YXC_Sem* pSem)
	{
		ybool_t bCreatedNew;
		return YXC_NamedSemCreateW(NULL, uNumResources, NULL, pSem, &bCreatedNew);
	}

	YXC_INLINE YXC_Status YXC_SemLock(YXC_Sem sem)
	{
		return YXC_WaitSingleKObject(sem);
	}

	YXC_INLINE YXC_Status YXC_SemLockTimeout(YXC_Sem sem, yuint32_t stmsTimeout)
	{
		return YXC_WaitSingleKObjectTimeout(sem, stmsTimeout);
	}

	YXC_INLINE YXC_Status YXC_SemUnlock(YXC_Sem sem)
	{
		BOOL bRet = ::ReleaseSemaphore(sem, 1, NULL);
		return bRet ? YXC_ERC_SUCCESS : YXC_ERC_OS;
	}

	YXC_INLINE void YXC_SemDestroy(YXC_Sem sem)
	{
		::CloseHandle(sem);
	}

	YXC_INLINE YXC_Status YXC_NamedEventCreateW(YXC_KObjectAttr* pAttr, ybool_t bManualReset, ybool_t bInitState, const ychar* pszName,
		YXC_Event* pEvent, ybool_t* pbCreatedNew)
	{
		HANDLE hEve = ::CreateEventW(pAttr, bManualReset, bInitState, pszName);
		if (hEve == NULL) return YXC_ERC_OS;

		*pEvent = hEve;
		*pbCreatedNew = GetLastError() != ERROR_ALREADY_EXISTS;
		return YXC_ERC_SUCCESS;
	}

	YXC_INLINE YXC_Status YXC_EventOpenW(YXC_KObjectFlags flags, const ychar* pszName, YXC_Event* pEvent)
	{
		DWORD dwFlags = flags == 0 ? EVENT_ALL_ACCESS : flags;
		HANDLE hEve = ::OpenEventW(dwFlags, FALSE, pszName);
		if (hEve == NULL) return YXC_ERC_OS;

		*pEvent = hEve;
		return YXC_ERC_SUCCESS;
	}

	YXC_INLINE YXC_Status YXC_EventCreate(ybool_t bManualReset, ybool_t bInitState, YXC_Event* pEvent)
	{
		ybool_t bCreatedNew;
		return YXC_NamedEventCreateW(NULL, bManualReset, bInitState, NULL, pEvent, &bCreatedNew);
	}

	YXC_INLINE YXC_Status YXC_EventLock(YXC_Event event)
	{
		return YXC_WaitSingleKObject(event);
	}

	YXC_INLINE YXC_Status YXC_EventLockTimeout(YXC_Event event, yuint32_t stmsTimeout)
	{
		return YXC_WaitSingleKObjectTimeout(event, stmsTimeout);
	}

	YXC_INLINE YXC_Status YXC_EventSet(YXC_Event event)
	{
		BOOL bRet = ::SetEvent(event);
		return bRet ? YXC_ERC_SUCCESS : YXC_ERC_OS;
	}

	YXC_INLINE YXC_Status YXC_EventReset(YXC_Event event)
	{
		BOOL bRet = ::ResetEvent(event);
		return bRet ? YXC_ERC_SUCCESS : YXC_ERC_OS;
	}

	YXC_INLINE void YXC_EventDestroy(YXC_Event event)
	{
		::CloseHandle(event);
	}

	YXC_INLINE void YXC_CritInit(YXC_Crit* pCrit)
	{
		InitializeCriticalSection(pCrit);
	}

	YXC_INLINE void YXC_CritInitSpin(YXC_Crit* pCrit, yuint32_t uSpinCount)
	{
		InitializeCriticalSectionAndSpinCount(pCrit, uSpinCount);
	}

	YXC_INLINE void YXC_CritDelete(YXC_Crit* pCrit)
	{
		DeleteCriticalSection(pCrit);
	}

	YXC_INLINE void YXC_CritLock(YXC_Crit* pCrit)
	{
		EnterCriticalSection(pCrit);
	}

	YXC_INLINE ybool_t YXC_CritTryLock(YXC_Crit* pCrit)
	{
#if _MSC_VER > 1400
		return TryEnterCriticalSection(pCrit);
#else
		EnterCriticalSection(pCrit);
		return TRUE;
#endif /* _MSC_VER */
	}

	YXC_INLINE void YXC_CritUnlock(YXC_Crit* pCrit)
	{
		LeaveCriticalSection(pCrit);
	}
#elif YXC_PLATFORM_UNIX

	typedef pthread_mutex_t YXC_Crit;

    YXC_DECLARE_STRUCTURE_HANDLE(YXC_PKObject);

	typedef YXC_PKObject YXC_Sem;
	typedef YXC_PKObject YXC_Event;
    typedef YXC_PKObject YXC_Mutex;

    YXC_API(YXC_Status) YXC_WaitSingleKObject(YXC_PKObject obj);

    YXC_API(YXC_Status) YXC_WaitSingleKObjectTimeout(YXC_PKObject obj, ysize_t stmsTimeout);

    YXC_API(YXC_PKObject) YXC_HandleDuplicate(YXC_PKObject obj);

    YXC_API(YXC_Status) YXC_MutexOpenW(YXC_KObjectFlags uFlags, const ychar* pszName, YXC_Mutex* pNamedMutex);

	YXC_API(YXC_Status) YXC_NamedMutexCreateW(YXC_KObjectAttr* pAttr, ybool_t bInitOwned, const ychar* pszName,
                                          YXC_Mutex* pNamedMutex, ybool_t* pbCreatedNew);

	YXC_API(YXC_Status) YXC_MutexCreate(ybool_t bInitOwned, YXC_Mutex* pNamedMutex);

	YXC_API(YXC_Status) YXC_MutexLock(YXC_Mutex mutex);

	YXC_API(YXC_Status) YXC_MutexLockTimeout(YXC_Mutex mutex, ysize_t stmsTimeout);

	YXC_API(YXC_Status) YXC_MutexUnlock(YXC_Mutex mutex);

	YXC_API(void) YXC_MutexDestroy(YXC_Mutex mutex);

    YXC_API(YXC_Status) YXC_NamedSemCreateW(YXC_KObjectAttr* pAttr, yuint32_t uNumResources, const ychar* pszName, YXC_Sem* pSem, ybool_t* pbCreatedNew);

    YXC_API(YXC_Status) YXC_SemOpenW(YXC_KObjectFlags flags, const ychar* pszName, YXC_Sem* pSem);

    YXC_API(YXC_Status) YXC_SemCreate(yuint32_t uNumResources, YXC_Sem* pSem);

    YXC_API(YXC_Status) YXC_SemLock(YXC_Sem sem);

	YXC_API(YXC_Status) YXC_SemLockTimeout(YXC_Sem sem, ysize_t stmsTimeout);

	YXC_API(YXC_Status) YXC_SemUnlock(YXC_Sem sem);

	YXC_API(void) YXC_SemDestroy(YXC_Sem sem);

	YXC_API(YXC_Status) YXC_NamedEventCreateW(YXC_KObjectAttr* pAttr, ybool_t bManualReset, ybool_t bInitState, const ychar* pszName,
                                   YXC_Event* pEvent, ybool_t* pbCreatedNew);

	YXC_API(YXC_Status) YXC_EventOpenW(YXC_KObjectFlags flags, const ychar* pszName, YXC_Event* pEvent);

	YXC_API(YXC_Status) YXC_EventCreate(ybool_t bManualReset, ybool_t bInitState, YXC_Event* pEvent);

	YXC_API(YXC_Status) YXC_EventLock(YXC_Event event);

	YXC_API(YXC_Status) YXC_EventLockTimeout(YXC_Event event, ysize_t stmsTimeout);

	YXC_API(YXC_Status) YXC_EventSet(YXC_Event event);

	YXC_API(YXC_Status) YXC_EventReset(YXC_Event event);

	YXC_API(void) YXC_EventDestroy(YXC_Event event);

	YXC_INLINE void YXC_CritInit(YXC_Crit* pCrit)
	{
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(pCrit, &attr);

        pthread_mutexattr_destroy(&attr);
    }

	YXC_INLINE void YXC_CritInitSpin(YXC_Crit* pCrit, yuint32_t uSpinCount)
	{
		pthread_mutex_init(pCrit, NULL);
	}

	YXC_INLINE void YXC_CritDelete(YXC_Crit* pCrit)
	{
		pthread_mutex_destroy(pCrit);
	}

	YXC_INLINE void YXC_CritLock(YXC_Crit* pCrit)
	{
		pthread_mutex_lock(pCrit);
	}

	YXC_INLINE ybool_t YXC_CritTryLock(YXC_Crit* pCrit)
	{
        int iRet = pthread_mutex_trylock(pCrit);
        return iRet == 0;
	}

	YXC_INLINE void YXC_CritUnlock(YXC_Crit* pCrit)
	{
		pthread_mutex_unlock(pCrit);
	}

#else
#error Not support other platforms
#endif /* YXC_PLATFORM_WIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_LOCKER_H__ */

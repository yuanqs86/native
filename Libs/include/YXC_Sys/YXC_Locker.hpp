/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_LOCKER_HPP__
#define __INC_YXC_SYS_BASE_LOCKER_HPP__

#include <YXC_Sys/YXC_Locker.h>

namespace YXCLib
{
    struct CriticalSec
	{
		YXC_Crit sec;
		inline CriticalSec() { this->Init(); }
		inline CriticalSec(yuint32_t dwSpinCount) { this->Init(dwSpinCount); }
		inline ~CriticalSec() { this->Close(); }
		inline void Init() { YXC_CritInit(&sec); }
#if YXC_PLATFORM_WIN && _MSC_VER > 1400
		inline void Init(yuint32_t dwSpinCount) { YXC_CritInitSpin(&sec, dwSpinCount); }
#else
		inline void Init(yuint32_t dwSpinCount) { Init(); }
#endif /* _MSC_VER */
		inline void Lock() { YXC_CritLock(&sec); }
		inline void Unlock() { YXC_CritUnlock(&sec); }
		inline ybool_t TryLock() { return YXC_CritTryLock(&sec); }

		inline void Close() { YXC_CritDelete(&sec); }
    private:
		CriticalSec(const CriticalSec& other);

		CriticalSec& operator =(const CriticalSec& other);
	};
}

#if YXC_PLATFORM_WIN

#if _MSC_VER > 1400
#include <intrin.h>
#else
#define _InterlockedAdd InterlockedAdd
#define _InterlockedIncrement InterlockedIncrement
#define _InterlockedExchange InterlockedExchange
#define _InterlockedDecrement InterlockedDecrement
#define _InterlockedExchangeAdd InterlockedExchangeAdd
#define _InterlockedExchangeAdd64 InterlockedExchangeAdd64
#define _InterlockedCompareExchange InterlockedCompareExchange
#endif /* _MSV_VER */

namespace YXCLib
{
#if 0
	struct RWLock
	{
		SRWLOCK srwLock;

		inline RWLock() { this->Init(); }
		inline ~RWLock() {  }

		inline void Init() { InitializeSRWLock(&srwLock); }

		inline void Lock() { AcquireSRWLockExclusive(&srwLock); }
		inline void Unlock() { ReleaseSRWLockExclusive(&srwLock); }
		inline void SharedLock() { AcquireSRWLockShared(&srwLock); }
		inline void SharedUnlock() { ReleaseSRWLockShared(&srwLock); }
	};
#else
	struct RWLock : public CriticalSec
	{
		inline void SharedLock() { this->Lock(); }
		inline void SharedUnlock() { this->Unlock(); }
	};
#endif /* 0 */

	struct Interlocked
	{
		static yuint32_t Increment(volatile yuint32_t* pPtr)
		{
			return ::_InterlockedIncrement((volatile long*)pPtr);
		}

		static yuint32_t Decrement(volatile yuint32_t* pPtr)
		{
			return ::_InterlockedDecrement((volatile long*)pPtr);
		}

		static yuint32_t Exchange(volatile yuint32_t* pPtr, yuint32_t uValue)
		{
			return ::_InterlockedExchange((volatile long*)pPtr, uValue);
		}

		static yuint32_t CompareExchange(volatile yuint32_t* pPtr, yuint32_t uValue, yuint32_t compare)
		{
			return ::_InterlockedCompareExchange((volatile long*)pPtr, uValue, compare);
		}

#if YXC_IS_64BIT
		static yuint64_t Exchange64(volatile yuint64_t* pPtr, yuint64_t u64Value)
		{
			return ::_InterlockedExchange64((volatile long long*)pPtr, u64Value);
		}

		static yuint64_t CompareExchange64(volatile yuint64_t* pPtr, yuint64_t u64Value, yuint64_t u64Compare)
		{
			return ::_InterlockedCompareExchange64((volatile long long*)pPtr, u64Value, u64Compare);
		}
#endif /* YXC_IS_64BIT */

		template <typename T>
		static T* ExchangePtr(T* volatile* pPtr, T* pValue)
		{
#if YXC_IS_64BIT
			return (T*)Exchange64((volatile yuint64_t*)pPtr, (yuint64_t)pValue);
#else
			return (T*)Exchange((volatile yuint32_t*)pPtr, (yuint32_t)pValue);
#endif /* YXC_IS_64BIT */
		}

		template <typename T>
		static T* CompareExchangePtr(T* volatile* pPtr, T* pValue, T* pCompare)
		{
#if YXC_IS_64BIT
			return (T*)CompareExchange64((volatile yuint64_t*)pPtr, (yuint64_t)pValue, (yuint64_t)pCompare);
#else
			return (T*)CompareExchange((volatile yuint32_t*)pPtr, (yuint32_t)pValue, (yuint32_t)pCompare);
#endif /* YXC_IS_64BIT */
		}

		static yuint32_t ExchangeAdd(volatile yuint32_t* pPtr, yuint32_t uAdd)
		{
			return ::_InterlockedExchangeAdd((volatile long*)pPtr, uAdd);
		}

#if YXC_IS_64BIT
		static yuint64_t ExchangeAdd64(volatile yuint64_t* pPtr, yuint64_t u64Add)
		{
			return ::_InterlockedExchangeAdd64((volatile long long*)pPtr, u64Add);
		}
#endif /* YXC_IS_64BIT */

		template <typename T>
		static T* ExchangeAddPtr(T* volatile* pPtr, yintptr_t ipDistance)
		{
#if YXC_IS_64BIT
			return (T*)ExchangeAdd64((volatile yuint64_t*)pPtr, (yuint64_t)ipDistance);
#else
			return (T*)ExchangeAdd((volatile yuint32_t*)pPtr, (yuint32_t)ipDistance);
#endif /* YXC_IS_64BIT */
		}

#if _MSC_VER > 1400
		static yuint32_t ExchangeAnd(volatile yuint32_t* pPtr, yuint32_t uAnd)
		{
			return ::_InterlockedAnd((volatile long*)pPtr, uAnd);
		}

		static yuint32_t ExchangeOr(volatile yuint32_t* pPtr, yuint32_t uOr)
		{
			return ::_InterlockedOr((volatile long*)pPtr, uOr);
		}

		static yuint32_t ExchangeSubtract(volatile yuint32_t* pPtr, yuint32_t uSub)
		{
			return ::_InterlockedExchangeAdd((volatile long*)pPtr, -(yint32_t)uSub);
		}
#endif /* _MSC_VER */
	};
}
#elif YXC_PLATFORM_APPLE
#include <libkern/OSAtomic.h>

namespace YXCLib
{
    struct Interlocked
    {
		static yuint32_t Increment(volatile yuint32_t* pPtr)
		{
			return ::OSAtomicIncrement32((volatile int32_t*)pPtr);
		}

		static yuint32_t Decrement(volatile yuint32_t* pPtr)
		{
			return ::OSAtomicDecrement32((volatile int32_t*)pPtr);
		}

		static yuint32_t Exchange(volatile yuint32_t* pPtr, yuint32_t uValue)
		{
            while (TRUE)
            {
                yuint32_t uDetectedVal = *pPtr;
                bool ret = ::OSAtomicCompareAndSwapInt(uDetectedVal, uValue, (volatile int*)pPtr);
                if (ret)
                {
                    return uDetectedVal;
                }
            }
		}

#if YXC_IS_64BIT
		static yuint64_t Exchange64(volatile yuint64_t* pPtr, yuint64_t u64Value)
		{
            while (TRUE)
            {
                yuint64_t uDetectedVal = *pPtr;
                bool ret = ::OSAtomicCompareAndSwap64(uDetectedVal, u64Value, (volatile long long*)pPtr);
                if (ret)
                {
                    return uDetectedVal;
                }
            }
		}
#endif /* YXC_IS_64BIT */
		static yuint32_t ExchangeAdd(volatile yuint32_t* pPtr, yuint32_t uAdd)
		{
			yuint32_t uNewValue = OSAtomicAdd32(uAdd, (volatile int*)pPtr);
            return uNewValue - uAdd;
		}

#if YXC_IS_64BIT
		static yuint64_t ExchangeAdd64(volatile yuint64_t* pPtr, yuint64_t u64Add)
		{
			yuint64_t uNewValue = OSAtomicAdd64(u64Add, (volatile long long*)pPtr);
            return uNewValue - u64Add;
		}
#endif /* YXC_IS_64BIT */

		template <typename T>
		static T* ExchangeAddPtr(T* volatile* pPtr, yintptr_t ipDistance)
		{
#if YXC_IS_64BIT
			return (T*)ExchangeAdd64((volatile yuint64_t*)pPtr, (yuint64_t)ipDistance);
#else
			return (T*)ExchangeAdd((volatile yuint32_t*)pPtr, (yuint32_t)ipDistance);
#endif /* YXC_IS_64BIT */
		}

		template <typename T>
		static T* ExchangePtr(T* volatile* pPtr, T* pValue)
		{
#if YXC_IS_64BIT
			return (T*)Exchange64((volatile yuint64_t*)pPtr, (yuint64_t)pValue);
#else
			return (T*)Exchange((volatile yuint32_t*)pPtr, (yuint32_t)pValue);
#endif /* YXC_IS_64BIT */
		}

		static yuint32_t ExchangeAnd(volatile yuint32_t* pPtr, yuint32_t uAnd)
		{
			return ::OSAtomicAnd32Orig(uAnd, pPtr);
		}

		static yuint32_t ExchangeOr(volatile yuint32_t* pPtr, yuint32_t uOr)
		{
			return ::OSAtomicOr32Orig(uOr, pPtr);
		}

		static yuint32_t ExchangeSubtract(volatile yuint32_t* pPtr, yuint32_t uSub)
		{
			return ExchangeAdd(pPtr, -(yint32_t)uSub);
		}
    };

	struct RWLock : public CriticalSec
	{
		inline void SharedLock() { this->Lock(); }
		inline void SharedUnlock() { this->Unlock(); }
	};
}

#elif YXC_PLATFORM_UNIX

namespace YXCLib
{
	struct RWLock : public CriticalSec
	{
		inline void SharedLock() { this->Lock(); }
		inline void SharedUnlock() { this->Unlock(); }
	};

	struct Interlocked
	{
		static yuint32_t Increment(volatile yuint32_t* pPtr)
		{
			yuint32_t uRet = __sync_fetch_and_add(pPtr, 1);
			return uRet + 1;
		}

		static yuint32_t Decrement(volatile yuint32_t* pPtr)
		{
			yuint32_t uRet = __sync_fetch_and_sub(pPtr, 1);
			return uRet - 1;
		}

		static yuint32_t Exchange(volatile yuint32_t* pPtr, yuint32_t uValue)
		{
			while (TRUE)
			{
				yuint32_t uDetectedVal = *pPtr;
				bool ret = __sync_bool_compare_and_swap((volatile int*)pPtr, uDetectedVal, uValue);
				if (ret)
				{
					return uDetectedVal;
				}
			}
		}

#if YXC_IS_64BIT
		static yuint64_t Exchange64(volatile yuint64_t* pPtr, yuint64_t u64Value)
		{
			while (TRUE)
			{
				yuint64_t uDetectedVal = *pPtr;
				bool ret = __sync_bool_compare_and_swap((volatile long long*)pPtr, uDetectedVal, u64Value);
				if (ret)
				{
					return uDetectedVal;
				}
			}
		}
#endif /* YXC_IS_64BIT */
		static yuint32_t ExchangeAdd(volatile yuint32_t* pPtr, yuint32_t uAdd)
		{
			yuint32_t uNewValue = __sync_fetch_and_add((volatile int*)pPtr, uAdd);
			return uNewValue;
		}

#if YXC_IS_64BIT
		static yuint64_t ExchangeAdd64(volatile yuint64_t* pPtr, yuint64_t u64Add)
		{
			yuint64_t uNewValue = __sync_fetch_and_add((volatile long long*)pPtr, u64Add);
			return uNewValue;
		}
#endif /* YXC_IS_64BIT */

		template <typename T>
		static T* ExchangeAddPtr(T* volatile* pPtr, yintptr_t ipDistance)
		{
#if YXC_IS_64BIT
			return (T*)ExchangeAdd64((volatile yuint64_t*)pPtr, (yuint64_t)ipDistance);
#else
			return (T*)ExchangeAdd((volatile yuint32_t*)pPtr, (yuint32_t)ipDistance);
#endif /* YXC_IS_64BIT */
		}

		template <typename T>
		static T* ExchangePtr(T* volatile* pPtr, T* pValue)
		{
#if YXC_IS_64BIT
			return (T*)Exchange64((volatile yuint64_t*)pPtr, (yuint64_t)pValue);
#else
			return (T*)Exchange((volatile yuint32_t*)pPtr, (yuint32_t)pValue);
#endif /* YXC_IS_64BIT */
		}

		static yuint32_t ExchangeAnd(volatile yuint32_t* pPtr, yuint32_t uAnd)
		{
			return __sync_and_and_fetch(pPtr, uAnd);
		}

		static yuint32_t ExchangeOr(volatile yuint32_t* pPtr, yuint32_t uOr)
		{
			return __sync_or_and_fetch(pPtr, uOr);
		}

		static yuint32_t ExchangeSubtract(volatile yuint32_t* pPtr, yuint32_t uSub)
		{
			return ExchangeAdd(pPtr, -(yint32_t)uSub);
		}
	};
}

#else
#error Platform not support atomic operation.
#endif /* YXC_PLATFORM_WIN */

namespace YXCLib
{
	struct Mutex
	{
	protected:
		YXC_Mutex mutex;

	public:
		inline Mutex() : mutex(NULL)
		{

		}

		inline ~Mutex() { this->Close(); }

		inline YXC_Status NamedCreateW(YXC_KObjectAttr* pAttr, const ychar* pszName, ybool_t bInitOwned, ybool_t* pbCreatedNew)
		{
			return YXC_NamedMutexCreateW(pAttr, bInitOwned, pszName, &this->mutex, pbCreatedNew);
		}

		inline YXC_Status OpenW(YXC_KObjectFlags flags, const ychar* pszName)
		{
			return YXC_MutexOpenW(flags, pszName, &this->mutex);
		}

		inline YXC_Status Create(ybool_t bInitOwned)
		{
			return YXC_MutexCreate(bInitOwned, &this->mutex);
		}

		inline YXC_Status Lock()
		{
			return YXC_MutexLock(this->mutex);
		}

		inline YXC_Status Lock(yuint32_t stmsTimeout)
		{
			return YXC_MutexLockTimeout(this->mutex, stmsTimeout);
		}

		inline YXC_Status Unlock()
		{
			return YXC_MutexUnlock(this->mutex);
		}

		inline void Destroy()
		{
            YXC_MutexDestroy(this->mutex);
		}

		inline void Close()
		{
			if (this->mutex != NULL)
			{
				this->Destroy();
                this->mutex = NULL;
			}
		}

		operator YXC_Mutex() { return this->mutex; }

		void SetHandle(YXC_Mutex mutex_val) { this->mutex = mutex_val; }

		YXC_Mutex Detach()
		{
			YXC_Mutex ret = this->mutex;
			this->mutex = NULL;
			return ret;
		}

	private :
		Mutex(const Mutex& other);
		Mutex& operator =(const Mutex& other);
	};

	struct Semaphore
	{
	private:
		YXC_Sem sem;

	public:
		inline Semaphore()
		{
			this->sem = NULL;
		}

		inline ~Semaphore() { this->Close(); }

		inline YXC_Status NamedCreate(YXC_KObjectAttr* pAttr, yuint32_t uInitCount, const ychar* pszName, ybool_t* pbCreatedNew)
		{
			return YXC_NamedSemCreateW(pAttr, uInitCount, pszName, &this->sem, pbCreatedNew);
		}

		inline YXC_Status OpenW(YXC_KObjectFlags flags, const ychar* pszName)
		{
			return YXC_SemOpenW(flags, pszName, &this->sem);
		}

		inline YXC_Status Create(yuint32_t uInitCount)
		{
			return YXC_SemCreate(uInitCount, &this->sem);
		}

		inline YXC_Status Lock()
		{
			return YXC_SemLock(this->sem);
		}

		inline YXC_Status Lock(yuint32_t stmsTimeout)
		{
			return YXC_SemLockTimeout(this->sem, stmsTimeout);
		}

		inline YXC_Status Unlock()
		{
			return YXC_SemUnlock(sem);
		}

		inline void Destroy()
		{
            YXC_SemDestroy(this->sem);
		}

		inline void Close()
		{
			if (this->sem != NULL)
			{
				this->Destroy();
                this->sem = NULL;
			}
		}

		inline void SetHandle(YXC_Sem sem_val) { this->sem = sem_val; }

		inline operator YXC_Sem() { return this->sem; }

		inline YXC_Sem Detach()
		{
			YXC_Sem ret = this->sem;
			this->sem = NULL;
			return ret;
		}

	private :
		Semaphore(const Semaphore& other);
		Semaphore& operator =(const Semaphore& other);
	};

	struct Event
	{
	private:
		YXC_Event event;

	public:
		inline Event()
		{
			this->event = NULL;
		}

		inline ~Event() { this->Close(); }

		inline YXC_Status NamedCreate(YXC_KObjectAttr* pAttr, ybool_t bManualReset, ybool_t bInitState, const ychar* pszName, ybool_t* pbCreatedNew)
		{
			return YXC_NamedEventCreateW(pAttr, bManualReset, bInitState, pszName, &this->event, pbCreatedNew);
		}

		inline YXC_Status OpenW(YXC_KObjectFlags flags, const ychar* pszName)
		{
			return YXC_EventOpenW(flags, pszName, &this->event);
		}

		inline YXC_Status Create(ybool_t bManualReset, ybool_t bInitState)
		{
			return YXC_EventCreate(bManualReset, bInitState, &this->event);
		}

		inline YXC_Status Lock()
		{
			return YXC_EventLock(this->event);
		}

		inline YXC_Status Lock(yuint32_t stmsTimeout)
		{
			return YXC_EventLockTimeout(this->event, stmsTimeout);
		}

		inline YXC_Status Set()
		{
			return YXC_EventSet(this->event);
		}

		inline YXC_Status Reset()
		{
			return YXC_EventReset(this->event);
		}

		inline void Destroy()
		{
            YXC_EventDestroy(this->event);
		}

		inline void Close()
		{
			if (this->event != NULL)
			{
				this->Destroy();
                this->event = NULL;
			}
		}

		inline void SetHandle(YXC_Event event) { this->event = event; }

		inline operator YXC_Event() { return this->event; }

		inline YXC_Event Detach()
		{
			YXC_Event ret = this->event;
			this->event = NULL;
			return ret;
		}

	private :
		Event(const Event& other);
		Event& operator =(const Event& other);
	};

	template <typename Lock>
	struct Locker
	{
		Lock& _lock;
		inline Locker(Lock& lock) : _lock(lock) { _lock.Lock(); }
		// inline Locker(Lock& lock, ysize_t stTimeout) : _lock(lock) { _lock.Lock(stTimeout); }
		inline ~Locker() { _lock.Unlock(); }
	};

	template <typename Lock>
	struct SharedLocker
	{
		Lock& _lock;
		inline SharedLocker(Lock& lock) : _lock(lock) { _lock.SharedLock(); }
		// inline Locker(Lock& lock, ysize_t stTimeout) : _lock(lock) { _lock.Lock(stTimeout); }
		inline ~SharedLocker() { _lock.SharedUnlock(); }
	};

	template <typename Lock>
	struct Locker2
	{
		Lock& _lock;
		YXC_Status rc;
		inline Locker2(Lock& lock) : _lock(lock)
		{
			rc = lock.Lock();
		}
		inline ~Locker2()
		{
			if (rc == YXC_ERC_SUCCESS) _lock.Unlock();
		}
	};
}

typedef YXCLib::Mutex YXX_Mutex;
typedef YXCLib::CriticalSec YXX_Crit;
typedef YXCLib::Semaphore YXX_Sem;
typedef YXCLib::Event YXX_Event;
typedef YXCLib::RWLock YXX_RWLock;

typedef YXCLib::Locker<YXX_Crit> YXX_CritLocker;
typedef YXCLib::Locker<YXX_RWLock> YXX_ExlusiveLocker;
typedef YXCLib::Locker<YXX_RWLock> YXX_SharedLocker;

#endif /* __INC_YXC_SYS_LOCKER_HPP__ */

#include <YXC_Sys/YXC_Locker.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>
#include <algorithm>

#if YXC_PLATFORM_UNIX
#define __YXC_POSIX_MUTEX_PREFIX "__yxc_posix_mutex__"
#define __YXC_POSIX_SEM_PREFIX "__yxc_posix_sem__"
#define __YXC_POSIX_EVE_PREFIX "__yxc_posix_event__"

struct EventObject
{
    sem_t* sem;
    int manual_reset;
};

static sem_t* sem_open2(const char* name, int pshared, int mode, int count)
{
#if YXC_PLATFORM_ANDROID
		sem_t* sem = (sem_t*)malloc(sizeof(sem_t));
		if (sem == NULL) return NULL;

		sem_init(sem, pshared, count);
		return sem;
#else
	  return sem_open(name, pshared, mode, count);
#endif /* YXC_PLATFORM_ANDROID */
}

static void sem_destroy2(sem_t* sem)
{
#if YXC_PLATFORM_ANDORID
    sem_destroy(sem);
    free(sem);
#else
    sem_destroy(sem);
#endif /* YXC_PLATFORM_ANDORID */
}

#include <YXC_Sys/YXC_TextEncoding.h>
#include <sys/mman.h>
//#ifndef _POSIX_THREAD_PROCESS_SHARED
//#error "This platform does not support process shared mutex"
//#endif

static bool _check_obj_inited(volatile yuint32_t* p_init_var)
{
    yuint32_t uMaxLoops = 10000 * 10, uNumLoops = 0;
    while (uNumLoops++ < uMaxLoops)
    {
        yuint32_t uResultVal = YXCLib::Interlocked::ExchangeAdd(p_init_var, 0);
        if (uResultVal != 0)
        {
            return true;
        }
    }

    return false;
}

#if YXC_PLATFORM_APPLE || YXC_PLATFORM_ANDROID

static int pthread_mutex_timedlock1(pthread_mutex_t *mutex, const timespec* abs_timeout)
{
    struct timespec remaining, ts;

    remaining = *abs_timeout;
    while (TRUE) {
        int pthread_rc = pthread_mutex_trylock(mutex);
        if (pthread_rc == 0) return 0;

        if (errno != EBUSY) return errno;

        ts.tv_sec = 0;
        ts.tv_nsec = (remaining.tv_sec > 0 ? 1000000 : std::min(remaining.tv_nsec, (long)1000000));
        nanosleep(&ts, &ts);

        if (ts.tv_nsec <= remaining.tv_nsec) {
            remaining.tv_nsec -= ts.tv_nsec;
        }
        else {
            remaining.tv_sec--;
            remaining.tv_nsec = (1000000000 - (ts.tv_nsec - remaining.tv_nsec));
        }
        if (remaining.tv_sec < 0 || (!remaining.tv_sec && remaining.tv_nsec <= 0)) {
            return ETIMEDOUT;
        }
    }
}

static int sem_timedwait2(sem_t *sem, const timespec* rel_timeout)
{
    struct timespec remaining, ts;
    remaining = *abs_timeout;
    while (TRUE) {
        int pthread_rc = sem_trywait(sem);
        if (pthread_rc == 0) return 0;

        if (errno != EAGAIN) return errno;

        ts.tv_sec = 0;
        ts.tv_nsec = (remaining.tv_sec > 0 ? 2000000 : std::min(remaining.tv_nsec, (long)2000000));
        nanosleep(&ts, &ts);
        if (ts.tv_nsec <= remaining.tv_nsec) {
            remaining.tv_nsec -= ts.tv_nsec;
        }
        else {
            remaining.tv_sec--;
            remaining.tv_nsec = (1000000000 - (ts.tv_nsec - remaining.tv_nsec));
        }
        if (remaining.tv_sec < 0 || (!remaining.tv_sec && remaining.tv_nsec <= 0)) {
            return ETIMEDOUT;
        }
    }
}

#else
#define pthread_mutex_timedlock1 pthread_mutex_timedlock
static int sem_timedwait2(sem_t *sem, const timespec* rel_timeout)
{
    struct timespec abs_timeout;

    clock_gettime(CLOCK_REALTIME, &abs_timeout);

    abs_timeout.tv_sec += rel_timeout->tv_sec;
    abs_timeout.tv_nsec += rel_timeout->tv_nsec;
    if (abs_timeout.tv_nsec >= 1000000000)
    {
        abs_timeout.tv_nsec -= 1000000000;
        abs_timeout.tv_sec++;
    }
    int ret = sem_timedwait(sem, &abs_timeout);
    return ret;
}

#endif /* YXC_PLATFORM_APPLE || YXC_PLATFORM_ANDROID */

#define _MUTEX_SHARED_FLAG 0x1
#define _MUTEX_OWN_FLAG 0x2
#define _EVENT_MANUAL_FLAG 0x1
#define _EVENT_INIT_FLAG 0x2
#define _EVENT_SHARED_FLAG 0x4

static int pthread_mutex_init_2(pthread_mutex_t* mutex, yuint32_t flags)
{
    pthread_mutexattr_t attr;
    int ret = pthread_mutexattr_init(&attr);
    if (ret != 0) return ret;

    if (flags & _MUTEX_SHARED_FLAG)
    {
        ret = pthread_mutexattr_setpshared(&attr, TRUE);
        if (ret != 0) return ret;
    }

    ret = pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    if (ret != 0) return ret;

    if (flags & _MUTEX_OWN_FLAG)
    {
        ret = pthread_mutex_lock(mutex);
        if (ret != 0)
        {
            pthread_mutex_destroy(mutex);
            return ret;
        }
    }

    return 0;
}

//static int sem_init_2(sem_t* sem, yuint32_t flags)
//{
//    yuint32_t uNumResources = flags & 0x7fffffff;
//    yuint32_t uShared = (flags & 0x80000000) != 0;
//
//    int ret = sem_init(sem, uShared, uNumResources);
//    return ret;
//}
//
//static int event_init_2(sem_t* sem, yuint32_t flags)
//{
//    int pt_init = sem_init(sem, flags & _EVENT_SHARED_FLAG ? 1 : 0, flags & _EVENT_INIT_FLAG ? 1 : 0);
//    if (pt_init == 0)
//    {
//        int* manual_reset = (int*)(sem + 1);
//        *manual_reset = (flags & _EVENT_MANUAL_FLAG) ? 1 : 0;
//    }
//
//    return pt_init;
//}

static int event_wait(EventObject* eve)
{
    int ret = sem_wait(eve->sem);
    if (ret == 0)
    {
        if (eve->manual_reset)
        {
            sem_post(eve->sem);
        }
    }
    return ret;
}

static int event_timedwait(EventObject* eve, const timespec* abs_timeout)
{
    int ret = sem_timedwait2(eve->sem, abs_timeout);
    if (ret == 0)
    {
        if (eve->manual_reset)
        {
            sem_post(eve->sem);
        }
    }
    return ret;
}

static void event_destroy(EventObject* eve)
{
    sem_close(eve->sem);
}

namespace
{
    typedef void (*YXC_PKObjectDestroyFunc)(void*);
    typedef int (*YXC_PKObjectWaitFunc)(void*);
    typedef int (*YXC_PKObjectTimedWaitFunc)(void*, const timespec*);
    typedef int (*YXC_PKObjectInitFunc)(void*, yuint32_t flags);

    typedef struct __YXC_POSIX_KERNEL_REF_OBJECT
    {
        int ref;
        int created_or_opened;

        YXC_PKObjectDestroyFunc fn_destroy;
        YXC_PKObjectWaitFunc fn_wait;
        YXC_PKObjectTimedWaitFunc fn_timedwait;
    }YXC_PKObjectHead;

    typedef struct __YXC_NAMED_KERNEL_OBJECT
    {
        YXC_PKObjectHead obj_head;

        int shm_fd;
        char name[YXC_MAX_KOBJ_NAME];

        yuint32_t cb_obj;
        void* ptr_obj;
    }_PKObject;

    void _init_pk_object(_PKObject* kernel_obj)
    {
        kernel_obj->obj_head.ref = 1;
        kernel_obj->obj_head.fn_wait = NULL;
        kernel_obj->obj_head.fn_timedwait = NULL;
        kernel_obj->obj_head.fn_destroy = NULL;
        kernel_obj->ptr_obj = NULL;
        kernel_obj->cb_obj = 0;
        kernel_obj->shm_fd = -1;
        kernel_obj->obj_head.created_or_opened = 0;
    }

    void _free_pk_object(_PKObject* kernel_obj)
    {
        yuint32_t uRemain = YXCLib::Interlocked::ExchangeSubtract((volatile yuint32_t*)&kernel_obj->obj_head.ref, 1);

        if (uRemain == 1) /* Last decrement place!. */
        {
            if (kernel_obj->shm_fd == -1)
            {
                if (kernel_obj->ptr_obj)
                {
                    kernel_obj->obj_head.fn_destroy(kernel_obj->ptr_obj);
                }
            }
            else
            {
                if (kernel_obj->ptr_obj)
                {
                    munmap(kernel_obj->ptr_obj, kernel_obj->cb_obj + sizeof(yuint32_t));
                }
                close(kernel_obj->shm_fd);
#if !YXC_PLATFORM_ANDROID
                if (kernel_obj->obj_head.created_or_opened)
                {
                    shm_unlink(kernel_obj->name);
                }
#endif /* YXC_PLATFORM_ANDROID */
            }
            free(kernel_obj);
        }
    }

    YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_PKObject, _PKObject, _PKPtr, _PKHdl);

    YXC_Status _init_object_open(const char* pszPrefix, int len, const ychar* pszName, yuint32_t obj_size, _PKObject** ppObject)
    {
#if !YXC_PLATFORM_ANDROID
        _YCHK_MAL_R1(kernel_obj, _PKObject);
        YXCLib::HandleRef<_PKObject*> resHandler(kernel_obj, _free_pk_object);

        strcpy(kernel_obj->name, pszPrefix);

        strncpy(kernel_obj->name + len, pszName, YXC_MAX_KOBJ_NAME - len - 1);
        kernel_obj->name[YXC_MAX_KOBJ_NAME - 1] = 0;

        int fd = shm_open(kernel_obj->name, O_RDWR, S_IRUSR | S_IWUSR);
        _YXC_CHECK_OS_RET(fd >= 0, YC("Failed to open shm %s"), pszName);

        kernel_obj->shm_fd = fd;

        void* obj = (pthread_mutex_t*)mmap(NULL, obj_size + sizeof(yuint32_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        _YXC_CHECK_OS_RET(obj != NULL, YC("Failed to map memory %d"), obj_size + sizeof(yuint32_t));

        kernel_obj->ptr_obj = obj;
        kernel_obj->cb_obj = obj_size;

        bool bInit = _check_obj_inited((volatile yuint32_t*)((ybyte_t*)obj + obj_size));
        _YXC_CHECK_OS_RET(bInit, YC("Failed to init object status"));

        *ppObject = resHandler.Detach();
        return YXC_ERC_SUCCESS;
#else
	_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Android doesn't support named mutex."));
#endif /* YXC_PLATFORM_ANDROID */
    }

    YXC_Status _init_object_create(const char* pszPrefix, int len, const ychar* pszName, yuint32_t obj_size, YXC_PKObjectInitFunc fn_init, yuint32_t flags,
                                  _PKObject** ppObject, ybool_t* pbCreatedNew)
    {
        _YCHK_MAL_R1(kernel_obj, _PKObject);
        _init_pk_object(kernel_obj);

        YXCLib::HandleRef<_PKObject*> kernel_res(kernel_obj, _free_pk_object);

        if (pszName != NULL)
        {
#if !YXC_PLATFORM_ANDROID
            strcpy(kernel_obj->name, pszPrefix);
            strncpy(kernel_obj->name + len, pszName, YXC_MAX_KOBJ_NAME - len - 1);
            kernel_obj->name[YXC_MAX_KOBJ_NAME - 1] = 0;

            int fd = shm_open(kernel_obj->name, O_CREAT | O_EXCL | O_RDWR, 0x0644);
            if (fd == EEXIST)
            {
                YXC_Status rc = _init_object_open(pszPrefix, len, pszName, obj_size, ppObject);
                if (rc == YXC_ERC_SUCCESS)
                {
                    *pbCreatedNew = FALSE;
                }
                return rc;
            }
            kernel_obj->shm_fd = fd;

            int ret = ftruncate(fd, sizeof(yuint32_t) + obj_size);
            _YXC_CHECK_OS_RET(ret == 0, YC("Set object(%s) to size(%d) failed"), kernel_obj->name, sizeof(yuint32_t) + obj_size);

            void* obj = (pthread_mutex_t*)mmap(NULL, obj_size + sizeof(yuint32_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            _YXC_CHECK_OS_RET(obj != NULL, YC(""));

            kernel_obj->ptr_obj = obj;
            kernel_obj->cb_obj = obj_size;

            ret = fn_init(obj, flags);
            _YXC_CHECK_OS_RET(ret == 0, YC("Failed to init shared object"));

            *(yuint32_t*)((ybyte_t*)obj + obj_size) = TRUE; /* Mark that the object has been initialized. */
#else
	    _YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Android doesn't support named mutex."));
#endif /* YXC_PLATFORM_ANDROID */
        }
        else
        {
            _YCHK_MAL_ARR_R1(ptr_obj, ybyte_t, sizeof(yuint32_t) + obj_size);
            YXCLib::HandleRef<void*> ptr_res(ptr_obj, free);

            int ret = fn_init(ptr_obj, flags);
            _YXC_CHECK_OS_RET(ret == 0, YC("Failed to init exclusive object"));

            kernel_obj->ptr_obj = ptr_res.Detach();
        }

        *ppObject = kernel_res.Detach();
        *pbCreatedNew = TRUE;
        return YXC_ERC_SUCCESS;
    }
}

extern "C"
{
    YXC_PKObject YXC_HandleDuplicate(YXC_PKObject obj)
    {
        _PKObject* kernel_obj = _PKPtr(obj);
        YXCLib::Interlocked::ExchangeAdd((volatile yuint32_t*)&kernel_obj->obj_head.ref, 1);

        return obj;
    }

    YXC_Status YXC_WaitSingleKObject(YXC_PKObject obj)
    {
        _PKObject* kernel_obj = _PKPtr(obj);

        int ret = kernel_obj->obj_head.fn_wait(kernel_obj->ptr_obj);

        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to wait for object"));
        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_WaitSingleKObjectTimeout(YXC_PKObject obj, ysize_t stms_timeout)
    {
        if (stms_timeout == YXC_INFINITE)
        {
            return YXC_WaitSingleKObject(obj);
        }

        struct timespec tm;
        tm.tv_sec = stms_timeout / 1000;
        tm.tv_nsec = stms_timeout % 1000 * 1000 * 1000;

        _PKObject* kernel_obj = _PKPtr(obj);
        int ret = kernel_obj->obj_head.fn_timedwait(kernel_obj->ptr_obj, &tm);

        _YXC_CHECK_OS_RET(ret == 0, YC("Wait TO(%d)"), (int)stms_timeout);

        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_NamedMutexCreateW(YXC_KObjectAttr* pAttr, ybool_t bInitOwned, const ychar* pszName,
                                   YXC_Mutex* pNamedMutex, ybool_t* pbCreatedNew)
	{
        yuint32_t flags = 0;
        if (bInitOwned) flags |= _MUTEX_OWN_FLAG;
        if (pszName != NULL) flags |= _MUTEX_SHARED_FLAG;

        _PKObject* obj;
        YXC_Status rc = _init_object_create(__YXC_POSIX_MUTEX_PREFIX, YXC_STRING_ARR_LEN(__YXC_POSIX_MUTEX_PREFIX), pszName, sizeof(pthread_mutex_t),
                                           (YXC_PKObjectInitFunc)pthread_mutex_init_2, flags, &obj, pbCreatedNew);
        _YXC_CHECK_RC_RET(rc);

        obj->obj_head.fn_destroy = (YXC_PKObjectDestroyFunc)pthread_mutex_destroy;
        obj->obj_head.fn_wait = (YXC_PKObjectWaitFunc)pthread_mutex_lock;
        obj->obj_head.fn_timedwait = (YXC_PKObjectTimedWaitFunc)pthread_mutex_timedlock1;

        *pNamedMutex = _PKHdl(obj);
        return YXC_ERC_SUCCESS;
	}

    YXC_Status YXC_MutexOpenW(YXC_KObjectFlags uFlags, const ychar* pszName, YXC_Mutex* pNamedMutex)
    {
        _PKObject* obj = NULL;
        YXC_Status rc = _init_object_open(__YXC_POSIX_MUTEX_PREFIX, YXC_STRING_ARR_LEN(__YXC_POSIX_MUTEX_PREFIX), pszName, sizeof(pthread_mutex_t), &obj);
        if (rc != YXC_ERC_SUCCESS)
        {
            return rc;
        }

        obj->obj_head.fn_destroy = (YXC_PKObjectDestroyFunc)pthread_mutex_destroy;
        obj->obj_head.fn_wait = (YXC_PKObjectWaitFunc)pthread_mutex_lock;
        obj->obj_head.fn_timedwait = (YXC_PKObjectTimedWaitFunc)pthread_mutex_timedlock1;

        *pNamedMutex = _PKHdl(obj);
        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_MutexCreate(ybool_t bInitOwned, YXC_Mutex* pNamedMutex)
    {
        ybool_t bCreatedNew;
        return YXC_NamedMutexCreateW(NULL, bInitOwned, NULL, pNamedMutex, &bCreatedNew);
    }

    YXC_Status YXC_MutexLock(YXC_Mutex mutex)
	{
        return YXC_WaitSingleKObject(mutex);
	}

    YXC_Status YXC_MutexLockTimeout(YXC_Mutex mutex, ysize_t stmsTimeout)
    {
        return YXC_WaitSingleKObjectTimeout(mutex, stmsTimeout);
    }

	YXC_Status YXC_MutexUnlock(YXC_Mutex mutex)
	{
        _PKObject* kernel_obj = _PKPtr(mutex);

        int ret = pthread_mutex_unlock((pthread_mutex_t*)kernel_obj->ptr_obj);
        _YXC_CHECK_OS_RET(ret == 0, YC(""));

        return YXC_ERC_SUCCESS;
    }

    void YXC_MutexDestroy(YXC_Mutex mutex)
	{
        _free_pk_object(_PKPtr(mutex));
    }

    static yuint32_t gst_pk_id = 0;
    YXC_Status YXC_NamedSemCreateW(YXC_KObjectAttr* pAttr, yuint32_t uNumResources, const ychar* pszName,
                                   YXC_Sem* pNamedSem, ybool_t* pbCreatedNew)
	{
        _YCHK_MAL_R1(obj, _PKObject);
        _init_pk_object(obj);

        YXCLib::HandleRef<_PKObject*> obj_ref(obj, _free_pk_object);

        yuint32_t new_id = YXCLib::Interlocked::ExchangeAdd(&gst_pk_id, 1);
        time_t tm_now = time(NULL);

        if (pszName == NULL)
        {
            yh_sprintf(obj->name, YC("__yxc_sem__%d_%d_%d"), YXCLib::OSGetCurrentProcessId(), new_id, (yuint32_t)tm_now);
        }
        else
        {
            yh_sprintf(obj->name, YC("%s%s"), __YXC_POSIX_SEM_PREFIX, pszName);
        }

        sem_t* sem_obj = sem_open2(obj->name, O_CREAT | O_EXCL, 0777, uNumResources);
        _YXC_CHECK_OS_RET(sem_obj != SEM_FAILED, YC("Failed to create sem('%s')"), obj->name);

        obj->ptr_obj = sem_obj;
#if !YXC_PLATFORM_ANDROID
        sem_unlink(obj->name);
#endif /* YXC_PLATFORM_ANDROID */

        obj->obj_head.fn_destroy = (YXC_PKObjectDestroyFunc)sem_close;
        obj->obj_head.fn_wait = (YXC_PKObjectWaitFunc)sem_wait;
        obj->obj_head.fn_timedwait = (YXC_PKObjectTimedWaitFunc)sem_timedwait2;
        obj->obj_head.created_or_opened = 1;

        *pNamedSem = _PKHdl(obj_ref.Detach());
        return YXC_ERC_SUCCESS;
	}

    YXC_Status YXC_SemOpenW(YXC_KObjectFlags uFlags, const ychar* pszName, YXC_Sem* pNamedSem)
    {
        _YCHK_MAL_R1(obj, _PKObject);
        _init_pk_object(obj);

        YXCLib::HandleRef<_PKObject*> obj_ref(obj, _free_pk_object);

        yh_sprintf(obj->name, YC("%s%s"), __YXC_POSIX_SEM_PREFIX, pszName);
        sem_t* sem_obj = sem_open(obj->name, 0);
        _YXC_CHECK_OS_RET(sem_obj != SEM_FAILED, YC("Failed to OPEN sem('%s')"), obj->name);

        obj->ptr_obj = sem_obj;
        obj->obj_head.fn_destroy = (YXC_PKObjectDestroyFunc)sem_close;
        obj->obj_head.fn_wait = (YXC_PKObjectWaitFunc)sem_wait;
        obj->obj_head.fn_timedwait = (YXC_PKObjectTimedWaitFunc)sem_timedwait;

        *pNamedSem = _PKHdl(obj_ref.Detach());
        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_SemCreate(yuint32_t uNumResources, YXC_Sem* pSem)
	{
		ybool_t bCreatedNew;
		return YXC_NamedSemCreateW(NULL, uNumResources, NULL, pSem, &bCreatedNew);
	}

    YXC_Status YXC_SemLock(YXC_Sem sem)
	{
        return YXC_WaitSingleKObject(sem);
	}

    YXC_Status YXC_SemLockTimeout(YXC_Sem sem, ysize_t stms_timeout)
    {
        return YXC_WaitSingleKObjectTimeout(sem, stms_timeout);
    }

    YXC_Status YXC_SemUnlock(YXC_Sem sem)
	{
        _PKObject* kernel_obj = _PKPtr(sem);

		int ret = sem_post((sem_t*)kernel_obj->ptr_obj);
        _YXC_CHECK_OS_RET(ret == 0, YC(""));

        return YXC_ERC_SUCCESS;
    }

    void YXC_SemDestroy(YXC_Sem sem)
	{
        _free_pk_object(_PKPtr(sem));
    }

    YXC_Status YXC_NamedEventCreateW(YXC_KObjectAttr* pAttr, ybool_t bManualReset, ybool_t bInitState, const ychar* pszName,
                                          YXC_Event* pEvent, ybool_t* pbCreatedNew)
    {
        _YCHK_MAL_R1(obj, _PKObject);
        _init_pk_object(obj);
        YXCLib::HandleRef<_PKObject*> obj_ref(obj, _free_pk_object);

        yuint32_t new_id = YXCLib::Interlocked::ExchangeAdd(&gst_pk_id, 1);
        time_t tm_now = time(NULL);

        if (pszName == NULL)
        {
            yh_sprintf(obj->name, YC("_ev_%d_%d_%d"), YXCLib::OSGetCurrentProcessId(), new_id, (yuint32_t)tm_now);
        }
        else
        {
            yh_sprintf(obj->name, YC("%s%s"), __YXC_POSIX_EVE_PREFIX, pszName);
        }

        sem_t* sem_obj = sem_open2(obj->name, O_CREAT | O_EXCL, 0777, bInitState ? 1 : 0);
        _YXC_CHECK_OS_RET(sem_obj != SEM_FAILED, YC("Failed to create event('%s')"), obj->name);

#if !YXC_PLATFORM_ANDROID
        sem_unlink(obj->name);
#endif /* YXC_PLATFORM_ANDROID */
        YXCLib::HandleRef<sem_t*> sem_res(sem_obj, (YXCLib::HandleRef<sem_t*>::DestroyFunc)sem_destroy);

        _YCHK_MAL_R1(eve_ptr, EventObject);
        obj->ptr_obj = eve_ptr;
        eve_ptr->sem = sem_res.Detach();
        eve_ptr->manual_reset = bManualReset;

        obj->obj_head.fn_destroy = (YXC_PKObjectDestroyFunc)event_destroy;
        obj->obj_head.fn_wait = (YXC_PKObjectWaitFunc)event_wait;
        obj->obj_head.fn_timedwait = (YXC_PKObjectTimedWaitFunc)event_timedwait;
        obj->obj_head.created_or_opened = 1;

        *pEvent = _PKHdl(obj_ref.Detach());
        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_EventOpenW(YXC_KObjectFlags flags, const ychar* pszName, YXC_Event* pEvent)
    {
        _YCHK_MAL_R1(obj, _PKObject);
        _init_pk_object(obj);
        YXCLib::HandleRef<_PKObject*> obj_ref(obj, _free_pk_object);
        yh_sprintf(obj->name, YC("%s%s"), __YXC_POSIX_EVE_PREFIX, pszName);

        sem_t* sem_obj = sem_open2(obj->name, 0, 0644, 0);
        _YXC_CHECK_OS_RET(sem_obj != SEM_FAILED, YC("Failed to open event('%s')"), obj->name);

        YXCLib::HandleRef<sem_t*> sem_res(sem_obj, (YXCLib::HandleRef<sem_t*>::DestroyFunc)sem_destroy);

        _YCHK_MAL_R1(eve_ptr, EventObject);
        obj->ptr_obj = eve_ptr;
        eve_ptr->sem = sem_res.Detach();
        eve_ptr->manual_reset = FALSE;

        obj->obj_head.fn_destroy = (YXC_PKObjectDestroyFunc)event_destroy;
        obj->obj_head.fn_wait = (YXC_PKObjectWaitFunc)event_wait;
        obj->obj_head.fn_timedwait = (YXC_PKObjectTimedWaitFunc)event_timedwait;

        *pEvent = _PKHdl(obj);
        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_EventCreate(ybool_t bManualReset, ybool_t bInitState, YXC_Event* pEvent)
    {
        ybool_t bCreatedNew;
        YXC_Status rc = YXC_NamedEventCreateW(NULL, bManualReset, bInitState, NULL, pEvent, &bCreatedNew);
        return rc;
    }

    YXC_Status YXC_EventLock(YXC_Event event)
    {
        return YXC_WaitSingleKObject(event);
    }

    YXC_Status YXC_EventLockTimeout(YXC_Event event, ysize_t stmsTimeout)
    {
        return YXC_WaitSingleKObjectTimeout(event, stmsTimeout);
    }

    YXC_Status YXC_EventSet(YXC_Event event)
    {
        _PKObject* kernel_obj = _PKPtr(event);

		int ret = sem_post(*(sem_t**)kernel_obj->ptr_obj);
        _YXC_CHECK_OS_RET(ret == 0, YC(""));

        return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_EventReset(YXC_Event event)
    {
        _PKObject* kernel_obj = _PKPtr(event);

        int ret = sem_trywait(*(sem_t**)kernel_obj->ptr_obj);
        _YXC_CHECK_OS_RET(ret == EBUSY || ret == 0, YC(""));

        return YXC_ERC_SUCCESS;
    }

    void YXC_EventDestroy(YXC_Event event)
    {
        _free_pk_object(_PKPtr(event));
    }
}

#endif /* YXC_PLATFORM_UNIX */

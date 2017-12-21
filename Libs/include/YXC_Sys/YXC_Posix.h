
/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_POSIX_H__
#define __INC_YXC_BASE_POSIX_H__

#if YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL
#define YXC_API(retType) __attribute__((visibility("default"))) retType
#define YXC_CLASS __attribute__((visibility("default")))
#else
#define YXC_API(retType) retType
#define YXC_CLASS
#endif /* YXC_EXPORTS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <uuid/uuid.h>

#ifdef DEBUG
#define YXC_DEBUG 1
#else
#define YXC_DEBUG 0
#endif /* YXC_DEBUG */

#define YXC_PLATFORM_UNIX 1
#define YXC_MAX_KOBJ_NAME PATH_MAX
#define YXC_GNU_C 0
#define __stdcall

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    typedef unsigned int YXC_KObjectFlags;
    typedef mode_t YXC_KObjectAttr;
	typedef void* YXC_KObject;
	typedef pthread_t ethread_t;
	typedef struct __YXC_POSIX_GUID {
	    unsigned int Data1;
	    unsigned short Data2;
	    unsigned short Data3;
	    unsigned char Data4[8];
	}YXC_Guid;

#define YXC_KOBJATTR_ALL_ACCESS (YXC_KObjectAttr*)0777
#define YXC_KOBJFLAG_ALL_ACCESS (YXC_KObjectFlags)0777

    typedef struct __YXC_POSIX_KERNEL_SHARED_MEMORY
    {
        int shm_fd;
        char name[YXC_MAX_KOBJ_NAME];
    }YXC_PKShm;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_BASE_POSIX_H__ */

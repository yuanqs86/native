/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_APPLE_H__
#define __INC_YXC_BASE_APPLE_H__

#if __LP64__
#define YXC_IS_64BIT 1
#else
#define YXC_IS_64BIT 0
#endif /* __LP64__ */

#define YXC_PLATFORM_WIN 0
#define YXC_PLATFORM_LINUX 0

#include <YXC_Sys/YXC_Posix.h>
#define __stdcall /* No stdcall types on not windows platform. */
#define YXC_PLATFORM_APPLE 1

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#define YXC_PLATFORM_IOS 1
#endif /* TARGET_OS_IPHONE */

typedef unsigned long long etid_t;
#include <mach/thread_status.h>
#include <mach/mach.h>
#include <sys/mman.h>

#endif /* __INC_YXC_BASE_APPLE_H__ */

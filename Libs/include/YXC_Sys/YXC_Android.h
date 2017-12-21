/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_ANDROID_H__
#define __INC_YXC_BASE_ANDROID_H__

#if __LP64__
#define YXC_IS_64BIT 1
#else
#define YXC_IS_64BIT 0
#endif /* __LP64__ */

#define YXC_PLATFORM_WIN 0
#define YXC_PLATFORM_LINUX 0
#define YXC_PLATFORM_ANDROID 1
#define YXC_PLATFORM_APPLE 0
#define YXC_PLATFORM_UNIX 1

#include <YXC_Sys/YXC_Posix.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <wchar.h>

#define __stdcall

typedef pthread_t etid_t;

#endif /* __INC_YXC_BASE_ANDROID_H__ */

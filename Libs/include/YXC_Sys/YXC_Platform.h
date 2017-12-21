/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_PLATFORM_H__
#define __INC_YXC_BASE_PLATFORM_H__

#define YXC_EXPORTS_FLAG_DLL 0x01
#define YXC_EXPORTS_FLAG_DISPATCH 0x02

#define YXC_EXPORTS_WHOLE_DLL (YXC_EXPORTS_FLAG_DLL)
#define YXC_EXPORTS_DISPATCH_DLL (YXC_EXPORTS_FLAG_DLL | YXC_EXPORTS_FLAG_DISPATCH)
#define YXC_EXPORTS_STATIC_LIB 0

#ifndef YXC_EXPORTS_FLAG
#define YXC_EXPORTS_FLAG YXC_EXPORTS_STATIC_LIB
#endif /* YXC_EXPORTS_LEVEL */

#if defined(__MINGW32__)
#include <YXC_Sys/YXC_MinGW.h>
#elif defined(WIN32)
#include <YXC_Sys/YXC_Win32.h>
#elif defined(__APPLE__)
#include <YXC_Sys/YXC_Apple.h>
#elif defined(__ANDROID__)
#include <YXC_Sys/YXC_Android.h>
#elif defined(linux)
#include <YXC_Sys/YXC_Linux.h>
#else
#error Platform unsupported
#endif /* WIN32 */

#ifndef YES
#define YES 1
#endif /* YES */

#ifndef NO
#define NO 0
#endif /* NO */

#endif /* __INC_YXC_BASE_PLATFORM_H__ */

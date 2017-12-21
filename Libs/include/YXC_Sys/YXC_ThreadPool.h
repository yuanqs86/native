/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_THREAD_POOL_H__
#define __INC_YXC_SYS_BASE_THREAD_POOL_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_TPool);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_TPoolThread);

	typedef yuint32_t (__stdcall *YXC_TPoolThreadProc)(void* lParam);

	YXC_API(YXC_Status) YXC_TPoolCreate(yuint32_t uNumMaxThreads, YXC_TPool* pPool);

	YXC_API(YXC_Status) YXC_TPoolGetThread(YXC_TPool pool, yuint32_t stTimeout, YXC_TPoolThread* pThread);

	YXC_API(YXC_Status) YXC_TPoolThreadWait(YXC_TPoolThread th, yuint32_t stTimeoutMS);

	YXC_API(YXC_Status) YXC_TPoolThreadStart(YXC_TPoolThread th);

	YXC_API(void) YXC_TPoolThreadSetParam(YXC_TPoolThread th, YXC_TPoolThreadProc proc, void* pParam);

	YXC_API(YXC_Status) YXC_TPoolThreadReturn(YXC_TPool pool, YXC_TPoolThread thReturn);

	YXC_API(YXC_Status) YXC_TPoolThreadSuspend(YXC_TPoolThread th);

	YXC_API(YXC_Status) YXC_TPoolThreadResume(YXC_TPoolThread th);

	YXC_API(void) YXC_TPoolDestroy(YXC_TPool pool);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_THREAD_POOL_H__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_CONDITION_VARIABLE_H__
#define __INC_YXC_SYS_BASE_CONDITION_VARIABLE_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_CondVar);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_CondVarSV);

	/* Supervisor can change this match condition. */
	typedef ybool_t (*YXC_CondVarSVMatchFunc)(const void* pCond, void* pExternal);

	typedef void (*YXC_CondVarChangeFunc)(void* pCond, void* pExternal);

	YXC_API(YXC_Status) YXC_CondVarCreate(ysize_t stMaxNumClients, ysize_t stCondSize, const void* pInitCond, YXC_CondVar* pOutVar);

	YXC_API(YXC_Status) YXC_CondVarCreateSupervisor(YXC_CondVar cVar, YXC_CondVarSVMatchFunc pfnMatch, void* pSuperPtr,
		YXC_CondVarSV* pSupervisor);

	YXC_API(YXC_Status) YXC_CondVarControlSupervisor(YXC_CondVar cVar, YXC_CondVarSV supervisor, YXC_CondVarSVMatchFunc pfnNewMatch, void* pSuper);

	YXC_API(YXC_Status) YXC_CondVarSupervisorWait(YXC_CondVar cVar, YXC_CondVarSV supervisor, yuint32_t stmsTimeout, void* pCurrentCond);

	YXC_API(YXC_Status) YXC_CondVarWake(YXC_CondVar cVar, YXC_CondVarChangeFunc pfnCondChange, void* pCtrl);

    YXC_API(void) YXC_CondVarCopyCond(YXC_CondVar cVar, void* pData);

	YXC_API(void) YXC_CondVarNotifyClose(YXC_CondVar cVar);

	YXC_API(void) YXC_CondVarForceDestroy(YXC_CondVar cVar);

	YXC_API(void) YXC_CondVarSupervisorClose(YXC_CondVar cVar, YXC_CondVarSV supervisor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_CONDITION_VARIABLE_H__ */

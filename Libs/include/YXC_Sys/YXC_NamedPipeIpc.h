/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_NAMED_PIPE_IPC_H__
#define __INC_YXC_NAMED_PIPE_IPC_H__

#include <YXC_Sys/YXC_Sys.h>

#define YXC_NAMED_PIPE_MAX_INSTANCES 255
#define YXC_NAMED_PIPE_MAX_NAME 256

#define YXC_NPIPE_FLAGS_NO_THREAD_WAITING 1
#define YXC_NPIPE_FLAGS_RAW_INPUT 2

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_NPipeSrv);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_NPipeCli);

	typedef YXC_Status (*YXC_NPipeSrvProc)(const void* pInBuffer, ysize_t stCbIn, void** ppOutBuffer,
		ysize_t* pstCbOut, void* pSrvData);

	YXC_API(YXC_Status) YXC_NPipeServerCreate(
		const wchar_t* cpszPipeName,
		yuint32_t uNumInstances,
		yuint32_t uCtrlTimeout,
		void* const* ppSrvDatas,
		YXC_NPipeSrvProc pServiceProc,
		YXC_NPipeSrv* pPipeSrv
	);

	YXC_API(YXC_Status) YXC_NPipeServerCreateEx(
		const wchar_t* cpszPipeName,
		YXC_KObjectAttr* pSecurity,
		yuint32_t uNumInstances,
		yuint32_t uCtrlTimeout,
		yuint32_t uCbBufferRead,
		yuint32_t uCbBufferWrite,
		void* const* ppSrvDatas,
		YXC_NPipeSrvProc pServiceProc,
		yuint32_t uPipeFlags,
		YXC_NPipeSrv* pPipeSrv
	);

	YXC_API(void) YXC_NPipeServerClose(YXC_NPipeSrv pipeSrv);

	YXC_API(YXC_Status) YXC_NPipeClientConnect(
		const wchar_t* cpszPipeName,
		YXC_KObjectAttr* pSecurity,
		yuint32_t umsTimeout,
		YXC_NPipeCli* pClient
	);

	YXC_API(YXC_Status) YXC_NPipeClientCall(
		YXC_NPipeCli client,
		const void* pInBuffer,
		ysize_t stCbInBuffer,
		void** ppOutBuffer,
		ysize_t* pstCbOut
	);

	YXC_API(void) YXC_NPipeClientQueryHandle(
		YXC_NPipeCli client,
		ybool_t bDuplicate,
		YXC_KObject* pObject
	);

	YXC_API(void) YXC_NPipeClientClose(YXC_NPipeCli client);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_NAMED_PIPE_IPC_H__ */

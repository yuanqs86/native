/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_PSCMP_H__
#define __INC_YXC_SYS_BASE_PSCMP_H__

#include <YXC_Sys/YXC_MMInterface.h>
#include <YXC_Sys/YXC_PNCMP.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	// PNCMP : N producers with n consumers memory pool.

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_PSCMP);

	YXC_API YXC_Status YXC_PSCMPCreate(
		ysize_t stMaxNumBlocks,
		YXC_PNCMPMMParam mmParam,
		YXC_PSCMP* pOutMemPool
	);

	YXC_API YXC_Status YXC_PSCMPCreateEx(
		ysize_t stMaxNumBlocks,
		YXC_PNCMPMMParam mmParam,
		ybool_t bAllowDiskSwapWhenFull,  // reserved, not used
		ybool_t bEnableBlockDrop, // enable check & drop thread
		YXC_PSCMP* pOutMemPool
	);

	YXC_API YXC_Status YXC_PSCMPChangeConumserOpt(
		YXC_PSCMP memPool,
		yuint32_t uBlockCheckPeriod,
		ysize_t stMaxBlocksOnQueue,
		ybool_t bForceInsertNew,
		YXC_PNCMPConsumer* pConsumer
	);

	YXC_API void YXC_PSCMPDestroy(
		YXC_PSCMP memPool
	);

	YXC_API void YXC_PSCMPClearPool(
		YXC_PSCMP memPool
	);

	YXC_API YXC_Status YXC_PSCMPSetProducerOption(
		YXC_PSCMP producer,
		YXC_PNCMPProducerOpt producerOpt,
		const YXC_PNCMPProducerOptVal* pOptValue
	);

	YXC_API YXC_Status YXC_PSCMPPushBlock(
		YXC_PSCMP buffer,
		const void* pData,
		ysize_t stDataSize,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(сю╬ц)
	);

	YXC_API YXC_Status YXC_PSCMPPushBlockEx(
		YXC_PSCMP buffer,
		const void* pData,
		ysize_t stDataSize,
		const void* pExtData,
		ysize_t stExtDataSize,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(сю╬ц)
	);

	YXC_API YXC_Status YXC_PSCMPPullBlock(
		YXC_PSCMP buffer,
		const void** ppOutBuffer,
		ysize_t* pstBufSize,
		ysize_t stTimeoutInMilliSeconds
	);

	YXC_API YXC_Status YXC_PSCMPPullBlockEx(
		YXC_PSCMP buffer,
		const void** ppOutBuffer,
		ysize_t* pstBufSize,
		const void** ppOutExtBuffer,
		ysize_t* pstExtBufSize,
		ysize_t stTimeoutInMilliSeconds
	);

	YXC_API void YXC_PSCMPGetConsumerInfo(
		YXC_PSCMP buffer,
		YXC_PNCMPConsumerInfo* pConsumerInfo
	);

	YXC_API void YXC_PSCMPUnreferenceBlock(
		YXC_PSCMP buffer,
		const void* pBuffer
	);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_PNCMP_H__ */

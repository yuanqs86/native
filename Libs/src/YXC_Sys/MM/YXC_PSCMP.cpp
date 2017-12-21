//#define __MODULE__ "EK.PSCMP"
//
//#include <YXC_Sys/YXC_PSCMP.h>
//#include <YXC_Sys/YXC_ErrMacros.hpp>
//#include <YXC_Sys/YXC_UtilMacros.hpp>
//
//namespace
//{
//	struct _PSCMP
//	{
//		YXC_PNCMP memPool;
//		YXC_PNCMPProducer producer;
//		YXC_PNCMPConsumer consumer;
//	};
//
//	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_PSCMP, _PSCMP, _PSCMPPtr, _PSCMPHdl)
//}
//
//extern "C"
//{
//	YXC_Status YXC_PSCMPCreate(
//		ysize_t stMaxNumBlocks,
//		YXC_PNCMPMMParam mmParam,
//		YXC_PSCMP* pOutMemPool
//	)
//	{
//		return YXC_PSCMPCreateEx(stMaxNumBlocks, mmParam, FALSE, FALSE, pOutMemPool);
//	}
//
//	YXC_Status YXC_PSCMPCreateEx(
//		ysize_t stMaxNumBlocks,
//		YXC_PNCMPMMParam mmParam,
//		ybool_t bAllowDiskSwapWhenFull,  // reserved, not used
//		ybool_t bEnableBlockDrop, // enable check & drop thread
//		YXC_PSCMP* pOutMemPool
//	)
//	{
//		_YCHK_MAL_R1(pscmp, _PSCMP);
//		YXCLib::HandleRef<void*> pscmp_res(pscmp, free);
//
//		YXC_PNCMP memPool;
//		YXC_Status rc = YXC_PNCMPCreateEx(stMaxNumBlocks, 1, mmParam, bAllowDiskSwapWhenFull, bEnableBlockDrop, NULL, &memPool);
//		_YXC_CHECK_RC_RET(rc);
//
//		YXCLib::HandleRef<YXC_PNCMP> memPool_res(memPool, YXC_PNCMPForceDestroy);
//
//		rc = YXC_PNCMPProducerAttach(memPool, &pscmp->producer);
//		_YXC_CHECK_RC_RET(rc);
//
//		rc = YXC_PNCMPConsumerAttach(memPool, 0, 0, &pscmp->consumer);
//		_YXC_CHECK_RC_RET(rc);
//
//		pscmp->memPool = memPool_res.Detach();
//		pscmp_res.Detach();
//		*pOutMemPool = _PSCMPHdl(pscmp);
//		return YXC_ERC_SUCCESS;
//	}
//
//	YXC_API void YXC_PSCMPDestroy(
//		YXC_PSCMP memPool
//	)
//	{
//		_PSCMP* pool = _PSCMPPtr(memPool);
//
//		YXC_PNCMPProducerDetach(pool->memPool, pool->producer);
//		YXC_PNCMPConsumerDetach(pool->memPool, pool->consumer);
//		YXC_PNCMPReleaseWhenLastConsumerDetach(pool->memPool);
//	}
//
//	YXC_API void YXC_PSCMPClearPool(
//		YXC_PSCMP memPool
//	)
//	{
//		_PSCMP* pool = _PSCMPPtr(memPool);
//
//		YXC_PNCMPClearPool(pool->memPool);
//	}
//
//	YXC_API YXC_Status YXC_PSCMPSetProducerOption(
//		YXC_PSCMP memPool,
//		YXC_PNCMPProducerOpt producerOpt,
//		const YXC_PNCMPProducerOptVal* pOptValue
//	)
//	{
//		_PSCMP* pool = _PSCMPPtr(memPool);
//
//		YXC_PNCMPProducer pool->memPool
//
//	}
//
//	YXC_API YXC_Status YXC_PSCMPPushBlock(
//		YXC_PSCMP buffer,
//		const void* pData,
//		ysize_t stDataSize,
//		yuint32_t stBlockMemoryLifeTime // 0 == permenant(永久)
//	);
//
//	YXC_API YXC_Status YXC_PSCMPPushBlockEx(
//		YXC_PSCMP buffer,
//		const void* pData,
//		ysize_t stDataSize,
//		const void* pExtData,
//		ysize_t stExtDataSize,
//		yuint32_t stBlockMemoryLifeTime // 0 == permenant(永久)
//	);
//
//	YXC_API YXC_Status YXC_PSCMPPullBlock(
//		YXC_PSCMP buffer,
//		const void** ppOutBuffer,
//		ysize_t* pstBufSize,
//		ysize_t stTimeoutInMilliSeconds
//	);
//
//	YXC_API YXC_Status YXC_PSCMPPullBlockEx(
//		YXC_PSCMP buffer,
//		const void** ppOutBuffer,
//		ysize_t* pstBufSize,
//		const void** ppOutExtBuffer,
//		ysize_t* pstExtBufSize,
//		ysize_t stTimeoutInMilliSeconds
//	);
//
//	YXC_API void YXC_PSCMPGetConsumerInfo(
//		YXC_PSCMP buffer,
//		YXC_PNCMPConsumerInfo* pConsumerInfo
//	);
//
//	YXC_API void YXC_PSCMPUnreferenceBlock(
//		YXC_PSCMP buffer,
//		const void* pBuffer
//	);
//}

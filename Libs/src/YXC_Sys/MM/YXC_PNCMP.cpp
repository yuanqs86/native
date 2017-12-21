#define __MODULE__ "EK.PNCMP"

#include <YXC_Sys/YXC_PNCMP.h>
#include <YXC_Sys/MM/_YXC_PNCMPConsumer.hpp>
#include <YXC_Sys/MM/_YXC_PNCMP.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

using namespace YXC_Inner;

extern "C"
{
	YXC_Status YXC_PNCMPCreate(yuint32_t stMaxNumBlocks, yuint32_t stMaxNumConsumers, YXC_PNCMPMMParam mmParam,
		const YXC_PNCMPWarningParam* pWarningParam, YXC_PNCMP* pOutMemPool)
	{
		return YXC_PNCMPCreateEx(stMaxNumBlocks, stMaxNumConsumers, mmParam, FALSE, FALSE, NULL, pOutMemPool);
	}

	YXC_Status YXC_PNCMPCreateEx(
		yuint32_t stMaxNumBlocks,
		yuint32_t stMaxNumConsumers,
		YXC_PNCMPMMParam mmParam,
		ybool_t bAllowDiskSwapWhenFull,
		ybool_t bEnableCheckThread,
		const YXC_PNCMPWarningParam* pWarningParam,
		YXC_PNCMP* pOutMemPool
	)
	{
        _YCHK_MAL_R1(pPool, YXC_Inner::_PNCMP);
		YXC_Status rc;
		if (pWarningParam != NULL)
		{
			rc = pPool->Create(stMaxNumBlocks, stMaxNumConsumers, mmParam.mmAllocType, mmParam.u1.stSizeInfo, bAllowDiskSwapWhenFull, bEnableCheckThread,
				pWarningParam->stNumBlocksOnWarning, pWarningParam->stMemSpaceOnWarning, pWarningParam->pWarningCallback, pWarningParam->pWarningExtData);
		}
		else
		{
			rc = pPool->Create(stMaxNumBlocks, stMaxNumConsumers, mmParam.mmAllocType, mmParam.u1.stSizeInfo, bAllowDiskSwapWhenFull, bEnableCheckThread,
				0, 0, NULL, NULL);
		}

		if (rc != YXC_ERC_SUCCESS)
		{
			free(pPool);
			return rc;
		}

		*pOutMemPool = _PNCMPHdl(pPool);
		return YXC_ERC_SUCCESS;
	}

	void YXC_PNCMPForceDestroy(YXC_PNCMP memPool)
	{
		_PNCMPPtr(memPool)->Destroy();
		free(memPool);
	}

	void YXC_PNCMPReleaseWhenLastConsumerDetach(YXC_PNCMP memPool)
	{
		ybool_t bDestroy;
		_PNCMPPtr(memPool)->ReleaseWhenLastConsumerDetach(&bDestroy);

		if (bDestroy)
		{
			YXC_PNCMPForceDestroy(memPool);
		}
	}

	YXC_Status YXC_PNCMPProducerAttach(
		YXC_PNCMP memPool,
		YXC_PNCMPProducer* pProducer
	)
	{
		return YXC_PNCMPProducerAttachEx(memPool, YXC_INFINITE, FALSE, FALSE, pProducer);
	}

	YXC_Status YXC_PNCMPProducerAttachEx(
		YXC_PNCMP memPool,
		yuint32_t stTimeoutInMilliSeconds,
		ybool_t bWaitForConsumer,
		ybool_t bWaitForAllConsumers,
		YXC_PNCMPProducer* pProducer)
	{
		return _PNCMPPtr(memPool)->CreateProducer(stTimeoutInMilliSeconds, bWaitForConsumer, bWaitForAllConsumers, (_PNCMPProducer**)pProducer);
	}

	YXC_Status YXC_PNCMPProducerSetOption(
		YXC_PNCMPProducer producer,
		YXC_PNCMPProducerOpt producerOpt,
		const YXC_PNCMPProducerOptVal* pOptValue
	)
	{
		_PNCMPProducer* pProducer = _PNCMPPtr_P(producer);

		return pProducer->pPool->SetProducerOption(producerOpt, pOptValue);
	}

	void YXC_PNCMPProducerDetach(
		YXC_PNCMP memPool,
		YXC_PNCMPProducer producer
	)
	{
		_PNCMP* pMemPool = _PNCMPPtr(memPool);
		_PNCMPProducer* pProducer = _PNCMPPtr_P(producer);

		pMemPool->DestroyProducer(pProducer);
	}

	YXC_API(YXC_Status) YXC_PNCMPGetConsumersInfo(
		YXC_PNCMP memPool,
		yuint32_t uMaxConsumers,
		yuint32_t* puNumConsumers,
		YXC_PNCMPConsumerInfo* pConsumersInfos
	)
	{
		_PNCMP* pMemPool = _PNCMPPtr(memPool);
		return pMemPool->QueryConsumersInfo(uMaxConsumers, puNumConsumers, pConsumersInfos);
	}

	YXC_API(void) YXC_PNCMPGetConsumerInfo(
		YXC_PNCMPConsumer consumer,
		YXC_PNCMPConsumerInfo* pConsumerInfo
	)
	{
		_PNCMPConsumer* pConsumer = _PNCMPPtr_C(consumer);
		pConsumer->QueryInfo(pConsumerInfo);
	}

	YXC_Status YXC_PNCMPConsumerAttach(
		YXC_PNCMP memPool,
		yuint32_t uBlockCheckPeriod,
		yuint32_t stMaxBlocksOnQueue, // max to pool max blocks
		YXC_PNCMPConsumer* pConsumer
	)
	{
		YXC_Status rc = _PNCMPPtr(memPool)->CreateConsumer(uBlockCheckPeriod, stMaxBlocksOnQueue,
			NULL, FALSE, NULL, (_PNCMPConsumer**)pConsumer);
		return rc;
	}

	YXC_Status YXC_PNCMPConsumerAttachEx(
		YXC_PNCMP memPool,
		yuint32_t uBlockCheckPeriod,
		yuint32_t stMaxBlocksOnQueue, // max to pool max blocks
		YXC_PNCMPConsumerBlockCheckFunc pfnCheckBlock,
		ybool_t bForceInsertNew,
		void* pConsumerCtrl,
		YXC_PNCMPConsumer* pConsumer
	)
	{
		YXC_Status rc = _PNCMPPtr(memPool)->CreateConsumer(uBlockCheckPeriod, stMaxBlocksOnQueue,
			pfnCheckBlock, bForceInsertNew, pConsumerCtrl, (_PNCMPConsumer**)pConsumer);
		return rc;
	}

	void YXC_PNCMPConsumerSetBlockCheck(YXC_PNCMPConsumer consumer, YXC_PNCMPConsumerBlockCheckFunc pfnCheckBlock, void* pControl)
	{
		_PNCMPConsumer* pConsumer = (_PNCMPConsumer*)consumer;
		return pConsumer->SetBlockCheck(pfnCheckBlock, pControl);
	}


	void YXC_PNCMPConsumerDetach(
		YXC_PNCMP memPool,
		YXC_PNCMPConsumer consumer
	)
	{
		ybool_t bDestroy;

		_PNCMPPtr(memPool)->RemoveConsumer(_PNCMPPtr_C(consumer), &bDestroy);
		if (bDestroy)
		{
			YXC_PNCMPForceDestroy(memPool);
		}
	}

	YXC_Status YXC_PNCMPProducerMMPush(
		YXC_PNCMPProducer producer,
		ybool_t bFreeWhenNoRef,
		const void* pAllocatedData,
		const void* pBlockDesc,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(”¿æ√)
	)
	{
		_PNCMPProducer* pProducer = _PNCMPPtr_P(producer);
		return pProducer->pPool->MMPush(bFreeWhenNoRef, pAllocatedData, pBlockDesc, stBlockMemoryLifeTime);
	}

	YXC_API(YXC_Status) YXC_PNCMPMMAlloc(
		YXC_PNCMP memPool,
		ysize_t stCbData,
		ysize_t stCbExData,
		void** ppAllocated
	)
	{
		_PNCMP* pMemPool = _PNCMPPtr(memPool);
		return pMemPool->MMAlloc(stCbData, stCbExData, ppAllocated);
	}

	YXC_API(void) YXC_PNCMPMMFree(
		YXC_PNCMP memPool,
		void* pAllocated
	)
	{
		_PNCMPBlock* pBlock =(_PNCMPBlock*)((ybyte_t*)pAllocated - sizeof(_PNCMPBlock));
		_PNCMP* pMemPool = _PNCMPPtr(memPool);
		pMemPool->FreeBlock(pBlock);
	}

	YXC_Status YXC_PNCMPProducerPushBlock(
		YXC_PNCMPProducer producer,
		const void* pData,
		ysize_t stDataSize,
		yuint32_t uBlockMemoryLifeTime
	)
	{
		_PNCMPProducer* pProducer = _PNCMPPtr_P(producer);
		return pProducer->pPool->AddBlock(pData, stDataSize, uBlockMemoryLifeTime);
	}

	YXC_Status YXC_PNCMPProducerPushBlockEx(
		YXC_PNCMPProducer producer,
		const void* pData,
		ysize_t stDataSize,
		const void* pExtData,
		ysize_t stExtDataSize,
		yuint32_t uBlockMemoryLifeTime
	)
	{
		_PNCMPProducer* pProducer = _PNCMPPtr_P(producer);
		return pProducer->pPool->AddBlockEx(pData, stDataSize, pExtData, stExtDataSize, NULL, uBlockMemoryLifeTime);
	}

	YXC_Status YXC_PNCMPProducerPushBlockCond(
		YXC_PNCMPProducer producer,
		const void* pData,
		ysize_t stDataSize,
		const void* pExtData,
		ysize_t stExtDataSize,
		const void* pBlockDesc,
		yuint32_t uBlockMemoryLifeTime
	)
	{
		_PNCMPProducer* pProducer = _PNCMPPtr_P(producer);
		return pProducer->pPool->AddBlockEx(pData, stDataSize, pExtData, stExtDataSize, pBlockDesc, uBlockMemoryLifeTime);
	}

	YXC_Status YXC_PNCMPConsumerPullBlock(
		YXC_PNCMPConsumer consumer,
		const void** ppOutBuffer,
		ysize_t* pstBufSize,
		yuint32_t stTimeoutInMilliSeconds
	)
	{
		_PNCMPConsumer* pConsumer = _PNCMPPtr_C(consumer);

		_PNCMPBlock* pBlock = pConsumer->WaitForBlock(stTimeoutInMilliSeconds);
		_YXC_CHECK_REPORT_NEW_RET(pBlock != NULL, YXC_ERC_TIMEOUT, YC("No block available now"));

		*ppOutBuffer = (const ybyte_t*)(pBlock + 1);
		*pstBufSize = pBlock->stBlockSize + pBlock->stExtSize;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_PNCMPConsumerPullBlockEx(
		YXC_PNCMPConsumer consumer,
		const void** ppOutBuffer,
		ysize_t* pstBufSize,
		const void** ppOutExtBuffer,
		ysize_t* pstExtBufSize,
		yuint32_t stTimeoutInMilliSeconds
	)
	{
		_PNCMPConsumer* pConsumer = _PNCMPPtr_C(consumer);

		_PNCMPBlock* pBlock = pConsumer->WaitForBlock(stTimeoutInMilliSeconds);
		_YXC_CHECK_REPORT_NEW_RET(pBlock != NULL, YXC_ERC_TIMEOUT, YC("No block available now"));

		const ybyte_t* pBuffer = (const ybyte_t*)(pBlock + 1);

		*ppOutBuffer = pBuffer;
		*pstBufSize = pBlock->stBlockSize;

		*ppOutExtBuffer = pBuffer + pBlock->stBlockSize;
		*pstExtBufSize = pBlock->stExtSize;

		return YXC_ERC_SUCCESS;
	}

	YXC_API(void) YXC_PNCMPConsumerDropBlocks(
		YXC_PNCMPConsumer consumer,
		YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl,
		yuint32_t* pstNumBlocksDropped
	)
	{
		_PNCMPConsumer* pConsumer = _PNCMPPtr_C(consumer);
		pConsumer->DropBlocks(pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
	}

	void YXC_PNCMPConsumerUnreferenceBlock(
		YXC_PNCMPConsumer consumer,
		const void* pBuffer
	)
	{
		_PNCMPBlock* pBlock =(_PNCMPBlock*)((const ybyte_t*)pBuffer - sizeof(_PNCMPBlock));
		_PNCMPConsumer* pConsumer = _PNCMPPtr_C(consumer);
		pConsumer->ReleaseBlock(pBlock);
	}

	void YXC_PNCMPClearPool(YXC_PNCMP memPool)
	{
		_PNCMP* pPool = _PNCMPPtr(memPool);

		return pPool->Clear();
	}
}

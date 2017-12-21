#define __MODULE__ "EK.PNCMP"

#include <YXC_Sys/MM/_YXC_PNCMP.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

#if _MT
#include <process.h>
#endif /* _MT */

namespace YXC_Inner
{
	YXC_Status _PNCMP::Create(yuint32_t stMaxNumBlocks, yuint32_t stMaxNumConsumers, YXC_MMAllocatorType mmAllocType, ysize_t stMMSizeInfo,
		ybool_t bAllowDiskSwapWhenFull, ybool_t bEnableCheckThread, ysize_t stNumBlocksOnWarning, ysize_t stMemSpaceOnWarning,
		YXC_PNCMPWarningCallback pWarningCallback, void* pCallbackData)
	{
		this->_csPool.Init(4000);
		this->_csPoolMem.Init(4000);
		this->_stNumBlocksOnWarning = stNumBlocksOnWarning;
		this->_stMemSpaceOnWarning = stMemSpaceOnWarning;
		this->_stMaxNumBlocks = stMaxNumBlocks;
		this->_stNumBlocks = 0;
		this->_stMemSpace = 0;
		this->_stMaxNumConsumers = stMaxNumConsumers;
		this->_bHasProducer = FALSE;
		this->_pWarningCallback = pWarningCallback;
		this->_pCallbackData = pCallbackData;
		this->_bSwapToDiskWhenFull = bAllowDiskSwapWhenFull;
		this->_bEnableCheckThread = bEnableCheckThread;
		this->_spcWarningLvl = YXC_PNCMP_WARNING_LEVEL_NONE;
		this->_blkWarningLvl = YXC_PNCMP_WARNING_LEVEL_NONE;
		this->_bReleased = FALSE;
		this->_bSignaled = FALSE;
		this->_uNextBlockId = 0;
		this->_teProducer.SetHandle(NULL);

		StlAlloc<SysMemoryPool, _PNCMPConsumer*> al(&this->_mpInternal, stMaxNumConsumers);

		YXC_Status rcRet = YXC_ERC_SUCCESS;
		YXC_Status rcMM = YXC_ERC_SUCCESS;

		switch (mmAllocType)
		{
		case YXC_MM_ALLOCATOR_TYPE_FIXED:
			rcMM = YXC_MMCreateFixed(stMaxNumBlocks, stMMSizeInfo + sizeof(_PNCMPBlock), &this->_mmAllocator);
			_YXC_CHECK_REPORT_NEW_RET(rcMM == YXC_ERC_SUCCESS, rcMM, YC("Create fixed memory manager failed"));
			break;
		case YXC_MM_ALLOCATOR_TYPE_FLAT:
			rcMM = YXC_MMCreateFlat(stMMSizeInfo + stMaxNumBlocks * sizeof(_PNCMPBlock), &this->_mmAllocator);
			_YXC_CHECK_REPORT_NEW_RET(rcMM == YXC_ERC_SUCCESS, rcMM, YC("Create flat memory manager failed"));
			break;
		default:
			YXC_MMCreateCRunTime(&this->_mmAllocator);
			break;
		}

		ysize_t stBlockQueueCapa = sizeof(_PNCMPBlock*) * stMaxNumConsumers * stMaxNumBlocks;
		ysize_t stBlocksCapa = sizeof(_PNCMPBlock) * stMaxNumBlocks;
		ysize_t stConsumersCapa = (sizeof(_PNCMPConsumer*) + sizeof(_PNCMPConsumer)) * stMaxNumConsumers;

		ysize_t stSureSize = (stBlockQueueCapa + stBlocksCapa + stConsumersCapa) * 4; // temp

		YXC_Status rc = this->_mpInternal.Create(TRUE, stSureSize, 1 << 10);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create internal buffer for block queue failed"));

		new (&this->_vConsumers) ConsumerVec(al);
		this->_vConsumers.reserve(stMaxNumConsumers);

		if (bEnableCheckThread)
		{
			rc = this->_tsBlocks.Create(0);
			_YXC_CHECK_OS_GOTO(rc == YXC_ERC_SUCCESS, YC("Create Semaphore for check blocks failed"));

			this->_upCheckThread = YXCLib::OSCreateThread(_CheckQueueThread, this, NULL);
			_YXC_CHECK_OS_GOTO(this->_upCheckThread != 0, YC("Create background thread failed"));
		}

		return YXC_ERC_SUCCESS;
err_ret:
		this->_mpInternal.Destroy();
		if (rcMM == YXC_ERC_SUCCESS) YXC_MMDestroy(&this->_mmAllocator);
		return rcRet;
	}

	void _PNCMP::Destroy()
	{
		if (this->_bEnableCheckThread)
		{
			YXC_SemDestroy(this->_tsBlocks);
#if YXC_PLATFORM_WIN
			CloseHandle((HANDLE)this->_upCheckThread);
#endif /* YXC_PLATFORM_WIN */
		}

		if (this->_bHasProducer && this->_producer.bWaitForConsumer)
		{
			YXC_EventDestroy(this->_teProducer);
		}
		for (ConsumerVecIter pci = this->_vConsumers.begin(); pci != this->_vConsumers.end(); ++pci)
		{
			_PNCMPConsumer* pConsumer = *pci;
			pConsumer->ClearQueue();
			pConsumer->Destroy();
			this->_mpInternal.Free(pConsumer->blockQueue.pQueueBlocks);
			this->_mpInternal.Free(pConsumer);
		}

		this->_csPool.Close();
		this->_csPoolMem.Close();

		this->_vConsumers.~vector();
		::YXC_MMDestroy(&this->_mmAllocator);
		this->_mpInternal.Destroy();
	}

	void _PNCMP::Clear()
	{
		YXCLib::Locker<YXX_Crit> locker(this->_csPool);

		for (ConsumerVecIter pci = this->_vConsumers.begin(); pci != this->_vConsumers.end(); ++pci)
		{
			_PNCMPConsumer* pConsumer = *pci;
			pConsumer->ClearQueue();
		}

		this->_CheckAndSetSignal();
	}

	void _PNCMP::ReleaseWhenLastConsumerDetach(ybool_t* pbDestroy)
	{
		YXCLib::Locker<YXX_Crit> locker(this->_csPool);
		YXCLib::Locker<YXX_Crit> locker2(this->_csPoolMem);

		*pbDestroy = this->_vConsumers.size() == 0;

		this->_bReleased = TRUE;
	}

	YXC_Status _PNCMP::_AllocBlock(ysize_t stSize, ysize_t stExtSize, yuint32_t uBlockLifeTime,
		yuint32_t uUseCount, _PNCMPBlock** pOutBlock)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		_PNCMPMemory* pMemory = NULL;
		_YXC_CHECK_REPORT_NEW_RET(this->_stNumBlocks < this->_stMaxNumBlocks, YXC_ERC_PNCMP_POOL_FULL, YC("The memory pool is full"));

        _PNCMPBlock* pBlock = NULL;
		pMemory = (_PNCMPMemory*)this->_mmAllocator.pAlloc(sizeof(_PNCMPBlock) + stSize + stExtSize, this->_mmAllocator.pAllocator);
		_YXC_CHECK_REPORT_NEW_GOTO(pMemory != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc block memory from memory allocator failed"));

		pBlock = &pMemory->block;
		pBlock->uBlockId = this->_uNextBlockId++;
		pBlock->uBlockLifeTime = uBlockLifeTime;
		pBlock->uUseCount = uUseCount;
		pBlock->uDiskPushNum = 0;
		pBlock->stBlockSize = stSize;
		pBlock->stExtSize = stExtSize;

		++this->_stNumBlocks;
		this->_stMemSpace += stSize;

		this->_HandleWarning(FALSE);
		*pOutBlock = &pMemory->block;
		return YXC_ERC_SUCCESS;

err_ret:
		if (pMemory != NULL) this->_mmAllocator.pFree(pMemory, this->_mmAllocator.pAllocator);

		return rcRet;
	}

	void _PNCMP::FreeBlock(_PNCMPBlock* pBlock)
	{
		YXCLib::Locker<YXX_Crit> locker(this->_csPoolMem);

		this->_FreeBlock(pBlock);
	}

	void _PNCMP::_FreeBlock(_PNCMPBlock* pBlock)
	{
		this->_stMemSpace -= pBlock->stBlockSize;
		--this->_stNumBlocks;

		if (this->_bEnableCheckThread) this->_tsBlocks.Unlock();

		this->_mmAllocator.pFree(pBlock, this->_mmAllocator.pAllocator);

		// YXCLib::Interlocked::ExchangeSubtract(&this->_stMemSpace, pBlock->stBlockSize);
		// YXCLib::Interlocked::Decrement(&this->_stNumBlocks);

		this->_HandleWarning(TRUE);
	}

	YXC_Status _PNCMP::MMAlloc(ysize_t stCbData, ysize_t stCbExt, void** pAllocated)
	{
		YXCLib::Locker<YXX_Crit> locker(this->_csPoolMem);

		_PNCMPBlock* pBlock;
		YXC_Status rc = this->_AllocBlock(stCbData, stCbExt, 0, 0, &pBlock);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Alloc MM block failed, size(%ld)"), stCbData);

		*pAllocated = pBlock + 1;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _PNCMP::AddBlock(const void* pData, ysize_t stSize, yuint32_t uBlockLifeTime)
	{
		return this->AddBlockEx(pData, stSize, NULL, 0, NULL, uBlockLifeTime);
	}

	YXC_Status _PNCMP::_AddAllocatedBlock(_PNCMPBlock* pBlock, ysize_t stNumConsumers, ybool_t bWaitForClientSignal,
		const void* pBlockDesc, ybool_t bFreeWhenNoRef)
	{
		ysize_t stUseCount = 0;
		ysize_t stPassiveDropCount = 0;
		for (ConsumerVecIter pci = this->_vConsumers.begin(); pci != this->_vConsumers.end(); ++pci)
		{
			_PNCMPConsumer* pConsumer = *pci;

			yint32_t iInsert = pConsumer->InsertBlock(pBlock, pBlockDesc);
			if (iInsert > 0)
			{
				++stUseCount;
			}
			else if (iInsert == 0)
			{
				++stPassiveDropCount;
			}
		}

		if (stUseCount == 0)
		{
			if (bFreeWhenNoRef) this->FreeBlock(pBlock);
		}
		else
		{
			yuint32_t uDecreaseCount = (yuint32_t)(stNumConsumers - stUseCount);
			if (uDecreaseCount > 0) /* Otherwise don't operate this buffer. prevent access violation. */
			{
				yuint32_t uRemainCount = YXCLib::Interlocked::ExchangeSubtract(&pBlock->uUseCount, uDecreaseCount);
				if (uRemainCount == uDecreaseCount && uDecreaseCount != 0) // The last decrement place
				{
					this->FreeBlock(pBlock);
				}
			}
			if (bWaitForClientSignal)
			{
				this->_CheckAndSetSignal();
			}
			if (this->_bEnableCheckThread) this->_tsBlocks.Unlock();
		}

		_YXC_CHECK_REPORT_NEW_RET(stPassiveDropCount != stNumConsumers, YXC_ERC_PNCMP_CONSUMER_ALL_FULL, YC("All of consumer queues are full"));
		_YXC_CHECK_REPORT_NEW_RET(stPassiveDropCount == 0, YXC_ERC_PNCMP_CONSUMER_FULL, YC("One or more consumer queues are full"));
		return YXC_ERC_SUCCESS;
	}

	void _PNCMP::_WaitCondBeforeBlockAdd(ybool_t* pbWaitClientSignal)
	{
		yuint32_t stTimeout;
		ysize_t stNumConsumers;
		{
			YXX_CritLocker locker(this->_csPool);
			*pbWaitClientSignal = this->_producer.bWaitForConsumer;
			stTimeout = this->_producer.stTimeout;
			stNumConsumers = this->_vConsumers.size();
		}

		if (*pbWaitClientSignal)
		{
			this->_teProducer.Lock(stTimeout);
		}
	}

	YXC_Status _PNCMP::MMPush(ybool_t bFreeWhenNoRef, const void* pData, const void* pBlockDesc, yuint32_t uBlockLifeTime)
	{
		ybool_t bWaitClientSignal = FALSE;
		this->_WaitCondBeforeBlockAdd(&bWaitClientSignal);

		_PNCMPBlock* pBlock = (_PNCMPBlock*)pData - 1;

		YXX_CritLocker locker(this->_csPool);
		ysize_t stNumConsumers = this->_vConsumers.size();
		_YXC_CHECK_REPORT_NEW_RET(stNumConsumers > 0, YXC_ERC_PNCMP_NO_CONSUMER, YC("No consumer exists now"));
		pBlock->uUseCount = (yuint32_t)stNumConsumers;
		pBlock->uBlockLifeTime = uBlockLifeTime;

		YXC_Status rc = this->_AddAllocatedBlock(pBlock, stNumConsumers, bWaitClientSignal, pBlockDesc, bFreeWhenNoRef);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Add allocated block failed."));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _PNCMP::AddBlockEx(const void* pData, ysize_t stSize, const void* pExtData, ysize_t stExtSize,
		const void* pBlockDesc, yuint32_t uBlockLifeTime)
	{
		ybool_t bWaitClientSignal;
		this->_WaitCondBeforeBlockAdd(&bWaitClientSignal);

		YXX_CritLocker locker(this->_csPool);
		ysize_t stNumConsumers = this->_vConsumers.size();
		_YXC_CHECK_REPORT_NEW_RET(stNumConsumers > 0, YXC_ERC_PNCMP_NO_CONSUMER, YC("No consumer exists now"));
		_PNCMPBlock* pBlock = NULL;
		{
			YXX_CritLocker locker(this->_csPoolMem);
			YXC_Status rc = this->_AllocBlock(stSize, stExtSize, uBlockLifeTime, (yuint32_t)stNumConsumers, &pBlock);
			if (rc != YXC_ERC_SUCCESS) return rc;
		}

		ybyte_t* pBuffer = (ybyte_t*)(pBlock + 1);
		memcpy(pBuffer, pData, stSize);
		memcpy(pBuffer + stSize, pExtData, stExtSize);

		YXC_Status rc = this->_AddAllocatedBlock(pBlock, stNumConsumers, bWaitClientSignal, pBlockDesc, TRUE);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Add allocated block failed."));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _PNCMP::CreateProducer(yuint32_t stTimeoutInMilliSeconds, ybool_t bWaitForConsumer,
		ybool_t bWaitForAllConsumers, _PNCMPProducer** ppProducer)
	{
		*ppProducer = NULL;

		YXX_CritLocker locker2(this->_csPool);
		_YXC_CHECK_REPORT_NEW_RET(!this->_bHasProducer, YXC_ERC_PNCMP_PRODUCER_ALIVE, YC("Producer is still alive"));

		this->_producer.pPool = this;
		this->_producer.bWaitForAllConsumers = bWaitForAllConsumers;
		this->_producer.bWaitForConsumer = bWaitForConsumer;
		this->_producer.stTimeout = stTimeoutInMilliSeconds;

		if (bWaitForConsumer)
		{
			this->_teProducer.Create(TRUE, FALSE);
			this->_bSignaled = FALSE;
			this->_CheckAndSetSignal();
		}
		*ppProducer = &this->_producer;
		this->_bHasProducer = TRUE;
		return YXC_ERC_SUCCESS;
	}

	void _PNCMP::DestroyProducer(_PNCMPProducer* pProducer)
	{
		YXX_CritLocker locker2(this->_csPool);
		this->_bHasProducer = FALSE;

		this->_teProducer.Close();
		this->_bSignaled = FALSE;
	}

	YXC_Status _PNCMP::SetProducerOption(YXC_PNCMPProducerOpt producerOpt, const YXC_PNCMPProducerOptVal* pOptValue)
	{
		YXX_CritLocker locker(this->_csPool);

		switch (producerOpt)
		{
		case YXC_PNCMP_PRODUCER_OPTION_WAIT_CONSUMER_TIMEOUT:
			this->_producer.stTimeout = pOptValue->stTimeoutInMilliSeconds;
			break;
		case YXC_PNCMP_PRODUCER_OPTION_WAIT_FOR_ALL_CONSUMERS:
			if (this->_producer.bWaitForAllConsumers != pOptValue->bWaitForAllConsumers)
			{
				this->_producer.bWaitForAllConsumers = pOptValue->bWaitForAllConsumers;
				if (this->_producer.bWaitForConsumer)
				{
					 this->_CheckAndSetSignal();
				}
			}
			break;
		case YXC_PNCMP_PRODUCER_OPTION_WAIT_FOR_CONSUMER:
			if (this->_producer.bWaitForConsumer != pOptValue->bWaitForConsumer)
			{
				ybool_t bWaitForConsumer = pOptValue->bWaitForConsumer;
				if (bWaitForConsumer)
				{
					YXC_Status rc = this->_teProducer.Create(TRUE, FALSE);
					if (rc != YXC_ERC_SUCCESS) return rc;

					this->_producer.bWaitForConsumer = bWaitForConsumer;
					this->_CheckAndSetSignal();
				}
				else
				{
					this->_producer.bWaitForConsumer = bWaitForConsumer;
					this->_CheckAndSetSignal();
					// this->_teProducer.Close();
				}
			}
			break;
		default:
			_YXC_REPORT_NEW_ERR(YXC_ERC_INVALID_PARAMETER, YC("Invalid PNCMP producer option %d"), producerOpt);
			return YXC_ERC_INVALID_PARAMETER;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _PNCMP::CreateConsumer(yuint32_t uBlockCheckPeriod, yuint32_t stMaxBlocksOnQueue,
		YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck, ybool_t bForceInsertNew, void* pCtrl,
		_PNCMPConsumer** ppConsumer)
	{
		_PNCMPConsumer* pConsumer = NULL;
		_PNCMPBlock** ppBlockArr = NULL;
		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;

		YXX_CritLocker locker(this->_csPool);
		YXX_CritLocker locker2(this->_csPoolMem);

		if (stMaxBlocksOnQueue == 0 || stMaxBlocksOnQueue >= this->_stMaxNumBlocks) stMaxBlocksOnQueue = this->_stMaxNumBlocks;

		_YXC_CHECK_REPORT_NEW_GOTO(this->_vConsumers.size() < this->_stMaxNumConsumers, YXC_ERC_PNCMP_EXCEED_MAX_CONSUMERS,
			YC("The pool reach the limit of number consumers"));

		ppBlockArr = (_PNCMPBlock**)this->_mpInternal.Alloc(stMaxBlocksOnQueue * sizeof(_PNCMPBlock*));
		_YXC_CHECK_REPORT_NEW_GOTO(ppBlockArr != NULL, YXC_ERC_OUT_OF_MEMORY, YC("No memory space left in internal buffer for block pointer array"));

		pConsumer = (_PNCMPConsumer*)this->_mpInternal.Alloc(sizeof(_PNCMPConsumer));
		_YXC_CHECK_REPORT_NEW_GOTO(pConsumer != NULL, YXC_ERC_OUT_OF_MEMORY, YC("No memory space left in internal buffer for new consumer"));

        rc = pConsumer->Create(this, uBlockCheckPeriod, stMaxBlocksOnQueue, ppBlockArr, pfnBlockCheck, bForceInsertNew, pCtrl);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create consumer failed"));

		this->_vConsumers.push_back(pConsumer);

		*ppConsumer = pConsumer;

		this->_CheckAndSetSignal();

		return YXC_ERC_SUCCESS;
err_ret:
		if (pConsumer != NULL)
		{
			pConsumer->Destroy();
			this->_mpInternal.Free(pConsumer);
		}

		if (ppBlockArr != NULL) this->_mpInternal.Free(ppBlockArr);
		*ppConsumer = NULL;
		return rcRet;
	}

	void _PNCMP::RemoveConsumer(_PNCMPConsumer* pConsumer, ybool_t* pbDestroy)
	{
		pConsumer->ClearQueue();

		YXX_CritLocker locker(this->_csPool);
		YXX_CritLocker locker2(this->_csPoolMem);

		pConsumer->Destroy();
		this->_mpInternal.Free(pConsumer->blockQueue.pQueueBlocks);
		this->_mpInternal.Free(pConsumer);

		for (ConsumerVecIter pci = this->_vConsumers.begin(); pci != this->_vConsumers.end(); ++pci)
		{
			if (pConsumer == *pci)
			{
				this->_vConsumers.erase(pci);
				break;
			}
		}

		if (this->_vConsumers.size() == 0 && this->_bReleased)
		{
			*pbDestroy = TRUE;
		}
		else
		{
			*pbDestroy = FALSE;
			this->_CheckAndSetSignal();
		}
	}

	void _PNCMP::CheckAndSetSignal()
	{
		YXX_CritLocker locker(this->_csPool);

		this->_CheckAndSetSignal();
	}

	void _PNCMP::_CheckAndSetSignal()
	{
		if (this->_teProducer)
		{
			if (!this->_producer.bWaitForConsumer)
			{
				if (!this->_bSignaled)
				{
					this->_teProducer.Set();
					this->_bSignaled = TRUE;
				}
			}
			if (this->_vConsumers.empty())
			{
				if (this->_bSignaled)
				{
					this->_teProducer.Reset();
					this->_bSignaled = FALSE;
				}
				return;
			}

			ybool_t bWaitForAll = this->_producer.bWaitForAllConsumers;
			if (bWaitForAll)
			{
				for (ConsumerVecIter pci = this->_vConsumers.begin(); pci != this->_vConsumers.end(); ++pci)
				{
					_PNCMPConsumer* pConsumer = *pci;

					if (!pConsumer->IsSignaled())
					{
						if (this->_bSignaled)
						{
							this->_teProducer.Reset();
							this->_bSignaled = FALSE;
						}
						return;
					}
				}

				if (!this->_bSignaled)
				{
					this->_teProducer.Set();
					this->_bSignaled = TRUE;
				}
			}
			else
			{
				for (ConsumerVecIter pci = this->_vConsumers.begin(); pci != this->_vConsumers.end(); ++pci)
				{
					_PNCMPConsumer* pConsumer = *pci;

					if (pConsumer->IsSignaled())
					{
						if (!this->_bSignaled)
						{
							this->_teProducer.Set();
							this->_bSignaled = TRUE;
						}
						return;
					}
				}

				if (this->_bSignaled)
				{
					this->_teProducer.Reset();
					this->_bSignaled = FALSE;
				}
			}
		}
	}

	void _PNCMP::_HandleWarning(ybool_t bRemoveWarning)
	{
		if (this->_pWarningCallback != NULL)
		{
			this->_HandleBlockWarning(bRemoveWarning);
			this->_HandleSpaceWarning(bRemoveWarning);
		}
	}

	void _PNCMP::_HandleBlockWarning(ybool_t bRemove)
	{
		ysize_t stNumBlocks = this->_stNumBlocks;
		if (bRemove)
		{
			switch (this->_blkWarningLvl)
			{
			case YXC_PNCMP_WARNING_LEVEL_LIMIT:
				if (stNumBlocks < this->_stMaxNumBlocks * 2 / 3)
				{
					this->_blkWarningLvl = YXC_PNCMP_WARNING_LEVEL_USER;
					this->_pWarningCallback(YXC_PNCMP_WARNING_TYPE_BLOCK, YXC_PNCMP_WARNING_LCHANGE_LIMIT_TO_USER,
						&stNumBlocks, this->_pCallbackData);
				}
				break;
			case YXC_PNCMP_WARNING_LEVEL_USER:
				if (stNumBlocks < this->_stNumBlocksOnWarning * 3 / 4)
				{
					this->_blkWarningLvl = YXC_PNCMP_WARNING_LEVEL_NONE;
					this->_pWarningCallback(YXC_PNCMP_WARNING_TYPE_BLOCK, YXC_PNCMP_WARNING_LCHANGE_USER_TO_NONE,
						&stNumBlocks, this->_pCallbackData);
				}
				break;
            default:
                break;
			}
		}
		else
		{
			switch (this->_blkWarningLvl)
			{
			case YXC_PNCMP_WARNING_LEVEL_NONE:
				if (stNumBlocks >= this->_stNumBlocksOnWarning)
				{
					this->_blkWarningLvl = YXC_PNCMP_WARNING_LEVEL_USER;
					this->_pWarningCallback(YXC_PNCMP_WARNING_TYPE_BLOCK, YXC_PNCMP_WARNING_LCHANGE_NONE_TO_USER,
						&stNumBlocks, this->_pCallbackData);
				}
				break;
			case YXC_PNCMP_WARNING_LEVEL_USER:
				if (stNumBlocks >= this->_stMaxNumBlocks * 9 / 10)
				{
					this->_blkWarningLvl = YXC_PNCMP_WARNING_LEVEL_LIMIT;
					this->_pWarningCallback(YXC_PNCMP_WARNING_TYPE_BLOCK, YXC_PNCMP_WARNING_LCHANGE_USER_TO_LIMIT,
						&stNumBlocks, this->_pCallbackData);
				}
				break;
            default:
                break;
			}
		}
	}

	void _PNCMP::_HandleSpaceWarning(ybool_t bRemove)
	{
		ysize_t stMemSpace = this->_stMemSpace;
		if (bRemove)
		{
			switch (this->_spcWarningLvl)
			{
			case YXC_PNCMP_WARNING_LEVEL_USER:
				if (stMemSpace < this->_stMemSpaceOnWarning * 3 / 4)
				{
					this->_spcWarningLvl = YXC_PNCMP_WARNING_LEVEL_NONE;
					this->_pWarningCallback(YXC_PNCMP_WARNING_TYPE_SPACE, YXC_PNCMP_WARNING_LCHANGE_USER_TO_NONE,
						&stMemSpace, this->_pCallbackData);
				}
				break;
            default:
                break;
			}
		}
		else
		{
			switch (this->_spcWarningLvl)
			{
			case YXC_PNCMP_WARNING_LEVEL_NONE:
				if (stMemSpace >= this->_stMemSpaceOnWarning)
				{
					this->_spcWarningLvl = YXC_PNCMP_WARNING_LEVEL_USER;
					this->_pWarningCallback(YXC_PNCMP_WARNING_TYPE_SPACE, YXC_PNCMP_WARNING_LCHANGE_NONE_TO_USER,
						&stMemSpace, this->_pCallbackData);
				}
				break;
            default:
                break;
			}
		}
	}

	unsigned int _PNCMP::_CheckQueueThread(void* pAddr)
	{
		_PNCMP* pPool = (_PNCMP*)pAddr;

		yuint32_t uFrameTime = 0;

		while (pPool->_tsBlocks.Lock())
		{
			StlAlloc<SysMemoryPool, _PNCMPConsumer*> al(&pPool->_mpInternal, pPool->_stMaxNumConsumers);
			ConsumerVec vConsumers(al);

			{
				YXX_CritLocker locker(pPool->_csPool);
				vConsumers.assign(pPool->_vConsumers.begin(), pPool->_vConsumers.end());
			}

			for (ConsumerVecIter pci = vConsumers.begin(); pci != vConsumers.end(); ++pci)
			{
				_PNCMPConsumer* pConsumer = *pci;

				pConsumer->ExactBlockQueue(uFrameTime);
			}

			++uFrameTime;
		}
		return 0;
	}

	YXC_Status _PNCMP::QueryConsumersInfo(yuint32_t uMaxConsumers, yuint32_t* puNumConsumers,
		YXC_PNCMPConsumerInfo* pConsumersInfos)
	{
		YXX_CritLocker locker(this->_csPool);

		*puNumConsumers = (yuint32_t)this->_vConsumers.size();
		if (this->_vConsumers.size() > uMaxConsumers)
		{
			return YXC_ERC_BUFFER_NOT_ENOUGH;
		}
		else
		{
			for (ysize_t i = 0; i < this->_vConsumers.size(); ++i)
			{
				YXC_PNCMPConsumerInfo* pInfo = pConsumersInfos + i;
				this->_vConsumers[i]->QueryInfo(pInfo);
			}

			return YXC_ERC_SUCCESS;
		}
	}
}

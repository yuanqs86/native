#include <YXC_Sys/MM/_YXC_PNCMP.hpp>
#include <YXC_Sys/MM/_YXC_PNCMPConsumer.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

namespace YXC_Inner
{
	YXC_Status _PNCMPConsumer::Create(_PNCMP* pPool, yuint32_t uTimePeriodToCheck, ysize_t stMaxBlocksOnQueue,
		_PNCMPBlock** pBlockBuffer, YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck, ybool_t bForceInsertNew, void* pCheckCtrl)
	{
		this->pPool = pPool;
		this->uTimePeriodToCheck = uTimePeriodToCheck;
		this->bHasDiskStorage = FALSE;
		this->uNumBytesSwapped = 0;
		this->uNumBytesDropped = 0;

		YXC_Status rc = this->tsResourceCount.Create(0);
		_YXC_CHECK_OS_RET(rc == YXC_ERC_SUCCESS, YC("Create semaphore for consumer failed"));

		this->csQueueLock.Init(4000);
		this->blockQueue.Create(pBlockBuffer, (yuint32_t)stMaxBlocksOnQueue);
		this->uThreadId = YXCLib::OSGetCurrentThreadId();
		this->bSignaled = TRUE;
		this->bForceInsertNew = bForceInsertNew;
		this->pfnBlockCheck = pfnBlockCheck;
		this->pCheckCtrl = pCheckCtrl;

		return YXC_ERC_SUCCESS;
	}

	void _PNCMPConsumer::SetBlockCheck(YXC_PNCMPConsumerBlockCheckFunc func, void* pCtrl)
	{
		YXX_CritLocker locker(this->pPool->GetCSPool());
		this->pfnBlockCheck = func;
		this->pCheckCtrl = pCtrl;
	}

	void _PNCMPConsumer::Destroy()
	{
		this->csQueueLock.Close();
		YXC_SemDestroy(this->tsResourceCount);
	}

	void _PNCMPConsumer::ClearQueue()
	{
		YXCLib::Locker<YXX_Crit> locker(this->csQueueLock);
		for (yuint32_t i = 0; i < this->blockQueue.uQueueCount; ++i)
		{
			yuint32_t index = this->blockQueue.uQueueIndex + i;
			if (index >= this->blockQueue.uQueueBufSize) index = index % this->blockQueue.uQueueBufSize;
			_PNCMPBlock* block = this->blockQueue.pQueueBlocks[index];

			yuint32_t uCount = YXCLib::Interlocked::Decrement(&block->uUseCount);
			if (uCount == 0)
			{
				this->pPool->FreeBlock(block);
			}
		}
		this->blockQueue.uQueueCount = 0;
	}

	yint32_t _PNCMPConsumer::InsertBlock(_PNCMPBlock* pBlock, const void* pBlockDesc)
	{
		if (this->pfnBlockCheck)
		{
			YXC_PNCMPConsumerInfo cInfo;
			this->QueryInfo(&cInfo);
			YXC_Status rc = this->pfnBlockCheck(_PNCMPHdl_C(this), pBlockDesc, &cInfo, this->pCheckCtrl);
			if (rc == YXC_ERC_PNCMP_DROP_BLOCK) /* Drop block actively. */
			{
				this->uNumBytesDropped += pBlock->stBlockSize;
				return -1;
			}
			else if (rc != YXC_ERC_SUCCESS)
			{
				this->uNumBytesDropped += pBlock->stBlockSize;
				return 0;
			}
		}

		YXCLib::Locker<YXX_Crit> locker(this->csQueueLock);
		ybool_t bInsert = this->blockQueue.InsertBlock(pBlock);
		this->Signal(this->blockQueue.uQueueCount < this->blockQueue.uQueueBufSize);

		if (bInsert)
		{
			this->tsResourceCount.Unlock(); // YXC_SemUnlock(this->tsResourceCount);
			return 1;
		}
		else
		{
			if (this->bForceInsertNew && this->blockQueue.uQueueCount > 0) /* Remove first block and push new block into the queue. */
			{
				_PNCMPBlock* pBlock2 = this->blockQueue.PopBlock();
				this->uNumBytesDropped += pBlock2->stBlockSize;
				this->ReleaseBlock(pBlock2);

				bInsert = this->blockQueue.InsertBlock(pBlock);
				if (bInsert)
				{
					return 1;
				}
			}
		}

		this->uNumBytesDropped += pBlock->stBlockSize;
		return 0;
	}

	ybool_t _PNCMPConsumer::IsQueueFull()
	{
		YXCLib::Locker<YXX_Crit> locker(this->csQueueLock);

		return this->blockQueue.uQueueCount == this->blockQueue.uQueueBufSize;
	}

	void _PNCMPConsumer::Signal(ybool_t bSignaled)
	{
		yuint32_t uValue = bSignaled ? 1 : 0;
		YXCLib::Interlocked::Exchange(&this->bSignaled, uValue);
	}

	ybool_t _PNCMPConsumer::IsSignaled()
	{
		yuint32_t uValue = YXCLib::Interlocked::ExchangeAdd(&this->bSignaled, 0);
		return uValue != 0 ? TRUE : FALSE;
	}

	_PNCMPBlock* _PNCMPConsumer::WaitForBlock(yuint32_t stTimeoutInMilliSeconds)
	{
		YXC_Status rc = this->tsResourceCount.Lock(stTimeoutInMilliSeconds);

		if (rc != YXC_ERC_SUCCESS)
		{
			return NULL;
		}

		this->csQueueLock.Lock();
		_PNCMPBlock* pBlock = this->blockQueue.PopBlock();
		if (pBlock != NULL)
		{
			this->uNumBytesSwapped += pBlock->stBlockSize;
		}
		this->csQueueLock.Unlock();

		if (pBlock != NULL)
		{
			ybool_t bSignal = this->IsSignaled();
			if (!bSignal)
			{
				this->Signal(TRUE);
				this->pPool->CheckAndSetSignal();
			}
		}

		return pBlock;
	}

	_PNCMPBlock* _PNCMPConsumer::PeekBlock(yuint32_t stTimeoutInMilliSeconds)
	{
		YXC_Status rc = this->tsResourceCount.Lock(stTimeoutInMilliSeconds);

		if (rc != YXC_ERC_SUCCESS)
		{
			return NULL;
		}

		this->csQueueLock.Lock();
		_PNCMPBlock* pBlock = this->blockQueue.GetFirstBlock();
		this->csQueueLock.Unlock();

		YXCLib::Interlocked::Increment(&pBlock->uUseCount);
		return pBlock;
	}

	void _PNCMPConsumer::ReleaseBlock(_PNCMPBlock* pBlock)
	{
		yuint32_t uCount = YXCLib::Interlocked::Decrement(&pBlock->uUseCount);
		if (uCount == 0)
		{
			this->pPool->FreeBlock(pBlock);
		}
	}

	void _PNCMPConsumer::QueryInfo(YXC_PNCMPConsumerInfo* pInfo)
	{
		// YXX_CritLocker locker(this->csQueueLock);
		pInfo->hConsumer = _PNCMPHdl_C(this);
		pInfo->stMaxNumBlocksOnQueue = this->blockQueue.uQueueBufSize;
		pInfo->pConsumerCtrl = this->pCheckCtrl;

		// No lock here, read only, no need to lock since query info.
		YXCLib::Locker<YXX_Crit> locker2(this->csQueueLock);
		pInfo->stNumBlocksOnQueue = this->blockQueue.uQueueCount;
		pInfo->u64CbTotalSwapped = this->uNumBytesSwapped;
		pInfo->u64CbTotalDropped += this->uNumBytesDropped;
		// pInfo->stTotalSizeOnQueue = this->blockQueue.GetTotalSize();
	}

	ybool_t _DropByIndex(const void* pData, ysize_t stCbData, const void* pExt,
		ysize_t stCbExt, yuint32_t uBlockIndex, void* dropCtrl)
	{
		DropIndexCtrl* ixCtrl = (DropIndexCtrl*)dropCtrl;

		ysize_t stRelativePos = uBlockIndex - ixCtrl->stStart;

		if (stRelativePos % ixCtrl->stStride == 0 && stRelativePos / ixCtrl->stStride < ixCtrl->stCount)
		{
			return TRUE;
		}

		return FALSE;
	}

	void _PNCMPConsumer::DropBlocks(YXC_PNCMPSpecBlockFunc pfnSpec, void* dropCtrl, yuint32_t* pstNumBlocksDropped)
	{
		YXCLib::Locker<YXX_Crit> locker2(this->csQueueLock);

		yuint32_t uQueueStartIndex = this->blockQueue.uQueueIndex;
		yuint32_t uQueueIterIndex = this->blockQueue.uQueueIndex;
		yuint32_t uFinalQueueCount = 0;
		for (yuint32_t i = 0; i < this->blockQueue.uQueueCount; ++i) /* Go though the block queue to remove blocks. */
		{
			_PNCMPBlock* pBlock = this->blockQueue.pQueueBlocks[uQueueIterIndex];
			ybyte_t* pData = (ybyte_t*)(pBlock + 1);

			ybool_t bDropBlock = pfnSpec(pData, pBlock->stBlockSize, pData + pBlock->stBlockSize, pBlock->stExtSize, i, dropCtrl);
			if (bDropBlock) /* Remove this block!. */
			{
				this->uNumBytesDropped += pBlock->stBlockSize;
				yuint32_t uUseCount = YXCLib::Interlocked::Decrement(&pBlock->uUseCount);
				if (uUseCount == 0)
				{
					this->pPool->FreeBlock(pBlock);
				}
				this->tsResourceCount.Lock();
				goto loop_1;
			}

			this->blockQueue.pQueueBlocks[uQueueStartIndex] = this->blockQueue.pQueueBlocks[uQueueIterIndex];
			if (++uQueueStartIndex == this->blockQueue.uQueueBufSize)
			{
				uQueueStartIndex = 0;
			}
			++uFinalQueueCount;
loop_1:
			if (++uQueueIterIndex == this->blockQueue.uQueueBufSize)
			{
				uQueueIterIndex = 0;
			}
		}
		*pstNumBlocksDropped = this->blockQueue.uQueueCount - uFinalQueueCount;
		this->blockQueue.uQueueCount = uFinalQueueCount;
	}

	void _PNCMPConsumer::ExactBlockQueue(yuint32_t uBlockTime)
	{
		if (this->uTimePeriodToCheck != 0 && uBlockTime % this->uTimePeriodToCheck == 0)
		{
			YXCLib::Locker<YXX_Crit> locker2(this->csQueueLock);

			// ybool_t* pRemoveArr = (ybool_t*)_alloca(sizeof(ybool_t) * this->blockQueue.uQueueCount);

			yuint32_t uQueueCount = this->blockQueue.uQueueCount;
			yuint32_t uNumDropped = 0, uRemainCount = uQueueCount;
			yuint32_t uFirstIndex = this->blockQueue.uQueueCount / 2 + 1;
			yuint32_t uNewIndex = this->blockQueue.uQueueIndex + uFirstIndex; // never drop previous blocks
			if (uNewIndex >= this->blockQueue.uQueueBufSize) uNewIndex -= this->blockQueue.uQueueBufSize;
			for (yuint32_t i = uFirstIndex; i < this->blockQueue.uQueueCount; ++i)
			{
				yuint32_t uIndex = i + this->blockQueue.uQueueIndex;
				if (uIndex >= this->blockQueue.uQueueBufSize) uIndex -= this->blockQueue.uQueueBufSize;
				_PNCMPBlock* pBlock = this->blockQueue.pQueueBlocks[uIndex];

				if (uRemainCount < this->pPool->GetWarningBlocks() ||
					(pBlock->uBlockLifeTime == 0 || pBlock->uBlockLifeTime + pBlock->uBlockId >= uBlockTime)) // reserved frame
				{
					this->blockQueue.pQueueBlocks[uNewIndex++] = this->blockQueue.pQueueBlocks[uIndex];
					if (uNewIndex >= this->blockQueue.uQueueBufSize) uNewIndex -= this->blockQueue.uQueueBufSize;
				}
				else
				{
					this->uNumBytesDropped += pBlock->stBlockSize;
					yuint32_t uUseCount = YXCLib::Interlocked::Decrement(&pBlock->uUseCount);
					if (uUseCount == 0)
					{
						this->pPool->FreeBlock(pBlock);
					}
					++uNumDropped;
					--uRemainCount;
				}
			}

			this->blockQueue.uQueueCount -= uNumDropped;
			for (yuint32_t i = 0; i < uNumDropped; ++i)
			{
				this->tsResourceCount.Lock();
			}
		}
	}
}

#include <YXC_Sys/MM/_YXC_PNCMP.hpp>
#include <YXC_Sys/MM/_YXC_PNCMPBlockQueue.hpp>

namespace YXC_Inner
{
	void _PNCMPBlockQueue::Create(_PNCMPBlock** pBlockBuffer, yuint32_t uMaxCount)
	{
		this->pQueueBlocks = pBlockBuffer;
		this->uQueueBufSize = uMaxCount;
		this->uQueueCount = 0;
		this->uQueueIndex = 0;
	}

	ybool_t _PNCMPBlockQueue::InsertBlock(_PNCMPBlock* pBlock)
	{
		if (this->uQueueCount >= this->uQueueBufSize) return FALSE;

		this->InsertBlock2(pBlock);
		return TRUE;
	}

	void _PNCMPBlockQueue::InsertBlock2(_PNCMPBlock* pBlock)
	{
		yuint32_t uLastIndex = this->uQueueIndex + this->uQueueCount;

		uLastIndex %= this->uQueueBufSize;

		this->pQueueBlocks[uLastIndex] = pBlock;
		++this->uQueueCount;
	}

	_PNCMPBlock* _PNCMPBlockQueue::PopBlock()
	{
		if (this->uQueueCount == 0) return NULL;

		yuint32_t uFirstIndex = this->uQueueIndex;
		_PNCMPBlock* pBlock = this->pQueueBlocks[uFirstIndex];

		if (uFirstIndex == this->uQueueBufSize - 1) this->uQueueIndex = 0;
		else ++this->uQueueIndex;
		--this->uQueueCount;

		return pBlock;
	}

	_PNCMPBlock* _PNCMPBlockQueue::GetFirstBlock()
	{
		if (this->uQueueCount == 0) return NULL;

		yuint32_t uFirstIndex = this->uQueueIndex;
		_PNCMPBlock* pBlock = this->pQueueBlocks[uFirstIndex];
		return pBlock;
	}

	ysize_t _PNCMPBlockQueue::GetTotalSize()
	{
		ysize_t stTotalSize = 0;

		yuint32_t uQueueIndex = this->uQueueIndex;
		for (yuint32_t i = 0; i < this->uQueueCount; ++i)
		{
			_PNCMPBlock* pBlock = this->pQueueBlocks[uQueueIndex];
			stTotalSize += pBlock->stBlockSize;
			if (++uQueueIndex == this->uQueueBufSize) uQueueIndex = 0;
		}

		return stTotalSize;
	}
}

#ifndef __INNER_INC_YXC_SYS_BASE_PNCMP_CONSUMER_HPP__
#define __INNER_INC_YXC_SYS_BASE_PNCMP_CONSUMER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

#include <YXC_Sys/MM/_YXC_PNCMPBlockQueue.hpp>
#include <YXC_Sys/YXC_PNCMP.h>

namespace YXC_Inner
{
	struct DropIndexCtrl
	{
		ysize_t stStart;
		ysize_t stCount;
		ysize_t stStride;
	};

	class _PNCMP;
	struct _PNCMPConsumer
	{
		_PNCMP* pPool;
		etid_t uThreadId;
		yuint64_t uNumBytesSwapped;
		yuint64_t uNumBytesDropped;
		YXX_Crit csQueueLock;
		YXX_Sem tsResourceCount;
		yuint32_t uTimePeriodToCheck;
		ybool_t bHasDiskStorage;
		ybool_t bForceInsertNew;
		volatile yuint32_t bSignaled; // will sync with pool
		YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck;
		void* pCheckCtrl;

		_PNCMPBlockQueue blockQueue;

		YXC_Status Create(_PNCMP* pPool, yuint32_t uTimePeriodToCheck, ysize_t stMaxBlocksOnQueue, _PNCMPBlock** pBlockBuffer,
			YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck, ybool_t bForceInsertNew, void* pCtrl);

		void SetBlockCheck(YXC_PNCMPConsumerBlockCheckFunc func, void* pCtrl);

		void QueryInfo(YXC_PNCMPConsumerInfo* pInfo);

		void Destroy();

		void Signal(ybool_t bSignaled);

		ybool_t IsSignaled();

		ybool_t IsQueueFull();

		void ClearQueue();

		yint32_t InsertBlock(_PNCMPBlock* pBlock, const void* pBlockDesc);

		void ExactBlockQueue(yuint32_t uBlockTime);

		void DropBlocks(YXC_PNCMPSpecBlockFunc pfnSpec, void* dropCtrl, yuint32_t* pstNumBlocksDropped);

		_PNCMPBlock* WaitForBlock(yuint32_t stTimeoutInMilliSeconds);

		_PNCMPBlock* PeekBlock(yuint32_t stTimeoutInMilliSeconds);

		void ReleaseBlock(_PNCMPBlock* pBlock);
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_PNCMPConsumer, _PNCMPConsumer, _PNCMPPtr_C, _PNCMPHdl_C);
}

#endif /* __INNER_INC_YXC_SYS_BASE_PNCMP_CONSUMER_HPP__ */

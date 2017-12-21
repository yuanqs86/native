#ifndef __INNER_INC_YXC_SYS_BASE_PNCMP_HPP__
#define __INNER_INC_YXC_SYS_BASE_PNCMP_HPP__

#define __MODULE__ "EK.PNCMP"

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_PNCMP.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_StlAlloc.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/MM/_YXC_PNCMPConsumer.hpp>
#include <YXC_Sys/MM/_YXC_PNCMPProducer.hpp>

#include <vector>
#include <map>

using YXCLib::SysMemoryPool;
using YXCLib::StlAlloc;

namespace YXC_Inner
{
	class _PNCMP
	{
	public:
		YXC_Status Create(yuint32_t stMaxNumBlocks, yuint32_t stMaxNumConsumers, YXC_MMAllocatorType mmAllocType, ysize_t stMMSizeInfo,
			ybool_t bAllowDiskSwapWhenFull, ybool_t bEnableCheckThread, ysize_t stNumBlocksOnWarning, ysize_t stMemSpaceOnWarning,
			YXC_PNCMPWarningCallback pWarningCallback, void* pCallbackData);

		void Destroy();

		void Clear();

		YXC_Status CreateProducer(yuint32_t stTimeoutInMilliSeconds, ybool_t bWaitForConsumer,
			ybool_t bWaitForAllConsumers,_PNCMPProducer** ppProducer);

		YXC_Status CreateConsumer(yuint32_t uBlockCheckPeriod, yuint32_t uNumBlocksOnCache, YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck,
			ybool_t bForceInsertNew, void* pCtrl, _PNCMPConsumer** ppConsumer);

		void DestroyProducer(_PNCMPProducer* pProducer);

		void RemoveConsumer(_PNCMPConsumer* pConsumer, ybool_t* pbDestroy);

		YXC_Status SetProducerOption(YXC_PNCMPProducerOpt producerOpt, const YXC_PNCMPProducerOptVal* pOptValue);

		YXC_Status AddBlock(const void* pData, ysize_t stSize, yuint32_t uBlockLifeTime);

		YXC_Status AddBlockEx(const void* pData, ysize_t stSize, const void* pExtData,
			ysize_t stExtSize, const void* pBlockDesc, yuint32_t uBlockLifeTime);

		YXC_Status MMPush(ybool_t bFreeWhenNoRef, const void* pData, const void* pBlockDesc, yuint32_t uBlockLifeTime);

		YXC_Status MMAlloc(ysize_t stCbData, ysize_t stCbExtData, void** pAllocated);

		void FreeBlock(_PNCMPBlock* pBlock);

		void ReleaseWhenLastConsumerDetach(ybool_t* pbDestroy);

		void CheckAndSetSignal();

		YXC_Status QueryConsumersInfo(yuint32_t uMaxConsumers, yuint32_t* puNumConsumers,
			YXC_PNCMPConsumerInfo* pConsumersInfos);

	public:
		inline ybool_t CanSwapToDisk() const { return this->_bSwapToDiskWhenFull; }

		inline ysize_t GetWarningBlocks() const { return this->_stNumBlocksOnWarning; }

		inline YXX_Crit& GetCSPool() { return this->_csPool; }

	private:
		_PNCMP(_PNCMP&);

		_PNCMP& operator =(_PNCMP&);

	private:
		YXC_Status _AllocBlock(ysize_t stSize, ysize_t stExtSize, yuint32_t uBlockLifeTime, yuint32_t uUseCount,
			_PNCMPBlock** pOutBlock);

		YXC_Status _AddAllocatedBlock(_PNCMPBlock* pBlock, ysize_t stNumConsumers, ybool_t bWaitForClientSignal,
			const void* pBlockDesc, ybool_t bFreeWhenNoRef);

		void _WaitCondBeforeBlockAdd(ybool_t* pbWaitClientSignal);

		void _FreeBlock(_PNCMPBlock* pBlock);

		void _CheckAndSetSignal();

		void _HandleWarning(ybool_t bRemoveWarning);

		void _HandleBlockWarning(ybool_t bRemoveWarning);

		void _HandleSpaceWarning(ybool_t bRemoveWarning);

		static unsigned int __stdcall _CheckQueueThread(void* pAddr);

	private:
		yuint32_t _stNumBlocks;
		yuint32_t _stMaxNumBlocks;

		ysize_t _stNumBlocksOnWarning;
		ysize_t _stMemSpaceOnWarning;
		ysize_t _stMemSpace;
		yuint32_t _uNextBlockId;

		ybool_t _bReleased;
		ybool_t _bEnableCheckThread;
		ybool_t _bSwapToDiskWhenFull;

		YXC_MMAllocator _mmAllocator;

		SysMemoryPool _mpInternal;

		ybool_t _bHasProducer;
		_PNCMPProducer _producer;

		ysize_t _stMaxNumConsumers;
		std::vector< _PNCMPConsumer*, StlAlloc<SysMemoryPool, _PNCMPConsumer*> > _vConsumers;

		typedef std::vector< _PNCMPConsumer*, StlAlloc<SysMemoryPool, _PNCMPConsumer*> > ConsumerVec;
		typedef std::vector< _PNCMPConsumer*, StlAlloc<SysMemoryPool, _PNCMPConsumer*> >::iterator ConsumerVecIter;

		typedef _VecWrap< ConsumerVec, StlAlloc<SysMemoryPool, _PNCMPConsumer*> > VecWrap;

		YXC_PNCMPWarningCallback _pWarningCallback;
		void* _pCallbackData;

		YXX_Sem _tsBlocks;
		YXX_Event _teProducer;
		ybool_t _bSignaled;

		YXC_PNCMPWarningLevel _spcWarningLvl;
		YXC_PNCMPWarningLevel _blkWarningLvl;

		ethread_t _upCheckThread;

		YXX_Crit _csPool;
		YXX_Crit _csPoolMem;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_PNCMP, _PNCMP, _PNCMPPtr, _PNCMPHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_PNCMP_HPP__ */

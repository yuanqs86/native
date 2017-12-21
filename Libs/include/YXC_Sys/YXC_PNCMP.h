/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_PNCMP_H__
#define __INC_YXC_SYS_BASE_PNCMP_H__

#include <YXC_Sys/YXC_MMInterface.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	// PNCMP : N producers with n consumers memory pool.

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_PNCMP);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_PNCMPProducer);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_PNCMPConsumer);

	typedef enum _YXC_PNCMP_WARNING_TYPE
	{
		YXC_PNCMP_WARNING_TYPE_NONE,
		YXC_PNCMP_WARNING_TYPE_BLOCK,
		YXC_PNCMP_WARNING_TYPE_SPACE,
	}YXC_PNCMPWarningType;

	typedef enum _YXC_PNCMP_WARNING_LEVEL
	{
		YXC_PNCMP_WARNING_LEVEL_NONE,
		YXC_PNCMP_WARNING_LEVEL_USER,  // warning 1(reach the user defined warning threshold)
		YXC_PNCMP_WARNING_LEVEL_LIMIT, // warning 2(almost reach the limit)
	}YXC_PNCMPWarningLevel;

	typedef enum _YXC_PNCMP_WARNING_LEVEL_CHANGE
	{
		YXC_PNCMP_WARNING_LCHANGE_NONE,
		YXC_PNCMP_WARNING_LCHANGE_NONE_TO_USER, // level up
		YXC_PNCMP_WARNING_LCHANGE_USER_TO_LIMIT, // level up
		YXC_PNCMP_WARNING_LCHANGE_LIMIT_TO_USER,  // level down
		YXC_PNCMP_WARNING_LCHANGE_USER_TO_NONE, // level down
	}YXC_PNCMPWarningLevelChange;

	//typedef struct __YXC_PNCMP_CONSUMER_INFOS
	//{
	//	YXC_PNCMPConsumer consumer;
	//	ybool_t bFull;
	//};

	typedef struct __YXC_PNCMP_CONSUMER_INFO
	{
		YXC_PNCMPConsumer hConsumer;
		void* pConsumerCtrl;
		yuint32_t stNumBlocksOnQueue;
		yuint32_t stMaxNumBlocksOnQueue;
		ysize_t stTotalSizeOnQueue;
		yuint64_t u64CbTotalSwapped;
		yuint64_t u64CbTotalDropped;
	}YXC_PNCMPConsumerInfo;

	typedef struct __YXC_PNCMP_BLOCK_INFO
	{
		ysize_t stDataSize;
		const void* pData;
		ysize_t stExtDataSize;
		const void* pExtData;
	}YXC_PNCMPBlockInfoEx;

	typedef enum _YXC_PNCMP_PRODUCER_OPTION
	{
		YXC_PNCMP_PRODUCER_OPTION_NONE,
		YXC_PNCMP_PRODUCER_OPTION_WAIT_CONSUMER_TIMEOUT,
		YXC_PNCMP_PRODUCER_OPTION_WAIT_FOR_CONSUMER,
		YXC_PNCMP_PRODUCER_OPTION_WAIT_FOR_ALL_CONSUMERS,
	}YXC_PNCMPProducerOpt;

	typedef union _YXC_PNCMP_PRODUCER_OPTION_VALUE
	{
		yuint32_t stTimeoutInMilliSeconds;
		ybool_t bWaitForConsumer;
		ybool_t bWaitForAllConsumers;
	}YXC_PNCMPProducerOptVal;

	typedef struct __YXC_PNCMP_MM_PARAM
	{
		YXC_MMAllocatorType mmAllocType;
		union
		{
			ysize_t stMaxBlockSize; // used by YXC_MM_ALLOCATOR_TYPE_FIXED
			ysize_t stSizeInfo; // used by YXC_MM_ALLOCATORY_TYPE_FLAT
		}u1;
	}YXC_PNCMPMMParam;

	YXC_INLINE YXC_PNCMPMMParam _YXC_PNCMPMMParam(YXC_MMAllocatorType mmAllocType, ysize_t stVal)
	{
		YXC_PNCMPMMParam mmParam;
		mmParam.mmAllocType = mmAllocType;
		mmParam.u1.stSizeInfo = stVal;
		return mmParam;
	}

	typedef void (*YXC_PNCMPWarningCallback)(YXC_PNCMPWarningType warningType, YXC_PNCMPWarningLevelChange lvlChange,
		void* pvData, void* pExtData);

	typedef YXC_Status (*YXC_PNCMPConsumerBlockCheckFunc)(YXC_PNCMPConsumer consumer, const void* pBlockDesc,
		YXC_PNCMPConsumerInfo* pConsumerInfo, void* pConsumerCtrl);

	typedef struct __YXC_PNCMP_WARNING_PARAM
	{
		ysize_t stNumBlocksOnWarning;
		ysize_t stMemSpaceOnWarning;
		YXC_PNCMPWarningCallback pWarningCallback;
		void * pWarningExtData;
	}YXC_PNCMPWarningParam;

	YXC_API(YXC_Status) YXC_PNCMPCreate(
		yuint32_t stMaxNumBlocks,
		yuint32_t stMaxNumConsumers,
		YXC_PNCMPMMParam mmParam,
		const YXC_PNCMPWarningParam* pWarningParam,
		YXC_PNCMP* pOutMemPool
	);

	YXC_API(YXC_Status) YXC_PNCMPCreateEx(
		yuint32_t stMaxNumBlocks,
		yuint32_t stMaxNumConsumers,
		YXC_PNCMPMMParam mmParam,
		ybool_t bAllowDiskSwapWhenFull,  // reserved, not used
		ybool_t bEnableBlockDrop, // enable check & drop thread
		const YXC_PNCMPWarningParam* pWarningParam,
		YXC_PNCMP* pOutMemPool
	);

	YXC_API(void) YXC_PNCMPForceDestroy(
		YXC_PNCMP memPool
	);

	YXC_API(void) YXC_PNCMPClearPool(
		YXC_PNCMP memPool
	);

	YXC_API(void) YXC_PNCMPReleaseWhenLastConsumerDetach(
		YXC_PNCMP memPool
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerAttach(
		YXC_PNCMP memPool,
		YXC_PNCMPProducer* pProducer
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerAttachEx(
		YXC_PNCMP memPool,
		yuint32_t stTimeoutInMilliSeconds,
		ybool_t bWaitForConsumer,
		ybool_t bWaitForAllConsumers,
		YXC_PNCMPProducer* pProducer
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerSetOption(
		YXC_PNCMPProducer producer,
		YXC_PNCMPProducerOpt producerOpt,
		const YXC_PNCMPProducerOptVal* pOptValue
	);

	YXC_API(void) YXC_PNCMPProducerDetach(
		YXC_PNCMP memPool,
		YXC_PNCMPProducer producer
	);

	YXC_API(YXC_Status) YXC_PNCMPMMAlloc(
		YXC_PNCMP memPool,
		ysize_t stCbData,
		ysize_t stCbExData,
		void** ppAllocated
	);

	YXC_API(void) YXC_PNCMPMMFree(
		YXC_PNCMP memPool,
		void* pAllocated
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerMMPush(
		YXC_PNCMPProducer producer,
		ybool_t bFreeWhenNoRef,
		const void* pAllocatedData, // will be filled
		const void* pDataDesc,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(сю╬ц)
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerPushBlock(
		YXC_PNCMPProducer producer,
		const void* pData,
		ysize_t stDataSize,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(сю╬ц)
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerPushBlockEx(
		YXC_PNCMPProducer producer,
		const void* pData,
		ysize_t stDataSize,
		const void* pExtData,
		ysize_t stExtDataSize,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(сю╬ц)
	);

	YXC_API(YXC_Status) YXC_PNCMPProducerPushBlockCond(
		YXC_PNCMPProducer producer,
		const void* pData,
		ysize_t stDataSize,
		const void* pExtData,
		ysize_t stExtDataSize,
		const void* pBlockDesc,
		yuint32_t stBlockMemoryLifeTime // 0 == permenant(сю╬ц)
	);

	YXC_API(YXC_Status) YXC_PNCMPGetConsumersInfo(
		YXC_PNCMP memPool,
		yuint32_t uMaxConsumers,
		yuint32_t* puNumConsumers,
		YXC_PNCMPConsumerInfo* pConsumersInfos
	);

	YXC_API(void) YXC_PNCMPGetConsumerInfo(
		YXC_PNCMPConsumer consumer,
		YXC_PNCMPConsumerInfo* pConsumerInfo
	);

	YXC_API(YXC_Status) YXC_PNCMPConsumerAttach(
		YXC_PNCMP memPool,
		yuint32_t uBlockCheckPeriod, // 0
		yuint32_t stMaxBlocksOnQueue, // max to pool max blocks
		YXC_PNCMPConsumer* pConsumer
	);

	YXC_API(YXC_Status) YXC_PNCMPConsumerAttachEx(
		YXC_PNCMP memPool,
		yuint32_t uBlockCheckPeriod,
		yuint32_t stMaxBlocksOnQueue,
		YXC_PNCMPConsumerBlockCheckFunc pfnCheckBlock,
		ybool_t bForceInsertNew,
		void* pConsumerCtrl,
		YXC_PNCMPConsumer* pConsumer
	);

	YXC_API(void) YXC_PNCMPConsumerDetach(
		YXC_PNCMP memPool,
		YXC_PNCMPConsumer consumer
	);

	YXC_API(void) YXC_PNCMPConsumerSetBlockCheck(YXC_PNCMPConsumer consumer, YXC_PNCMPConsumerBlockCheckFunc pfnCheckBlock, void* pControl);

	YXC_API(YXC_Status) YXC_PNCMPConsumerPullBlock(
		YXC_PNCMPConsumer consumer,
		const void** ppOutBuffer,
		ysize_t* pstBufSize,
		yuint32_t stTimeoutInMilliSeconds
	);

	YXC_API(YXC_Status) YXC_PNCMPConsumerPullBlockEx(
		YXC_PNCMPConsumer consumer,
		const void** ppOutBuffer,
		ysize_t* pstBufSize,
		const void** ppOutExtBuffer,
		ysize_t* pstExtBufSize,
		yuint32_t stTimeoutInMilliSeconds
	);

	typedef ybool_t (*YXC_PNCMPSpecBlockFunc)(const void* pData, ysize_t stCbData, const void* pExt,
		ysize_t stCbExt, yuint32_t uBlockIndex, void* dropCtrl);

	YXC_API(void) YXC_PNCMPConsumerDropBlocks(
		YXC_PNCMPConsumer consumer,
		YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl,
		yuint32_t* pstNumBlocksDropped
	);

	//YXC_API(YXC_Status) YXC_PNCMPConsumerPeekBlock(
	//	YXC_PNCMPConsumer consumer,
	//	const void** ppOutBuffer,
	//	ysize_t* pstBufSize,
	//	ysize_t stTimeoutInMilliSeconds
	//);

	//YXC_API(YXC_Status) YXC_PNCMPConsumerPeekBlockEx(
	//	YXC_PNCMPConsumer consumer,
	//	const void** ppOutBuffer,
	//	ysize_t* pstBufSize,
	//	const void** ppOutExtBuffer,
	//	ysize_t* pstExtBufSize,
	//	ysize_t stTimeoutInMilliSeconds
	//);

	YXC_API(void) YXC_PNCMPConsumerUnreferenceBlock(
		YXC_PNCMPConsumer consumer,
		const void* pBuffer
	);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_PNCMP_H__ */

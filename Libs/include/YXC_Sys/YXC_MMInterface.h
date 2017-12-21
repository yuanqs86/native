/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_MM_INTERFACE_H__
#define __INC_YXC_SYS_BASE_MM_INTERFACE_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef enum _YXC_MM_ALLOCATOR_TYPE
	{
		YXC_MM_ALLOCATOR_TYPE_UNKNOWN = 0,
		YXC_MM_ALLOCATOR_TYPE_FLAT = 1, // flat, continue memory
		YXC_MM_ALLOCATOR_TYPE_FIXED = 2, //
		YXC_MM_ALLOCATOR_TYPE_C_RUNTIME = 3,
	}YXC_MMAllocatorType;

	typedef void* (*YXC_MMAllocFunc)(ysize_t size, void* pAllocator);
	typedef void (*YXC_MMFreeFunc)(void* ptr, void* pAllocator);

	typedef struct __YXC_MM_ALLOCATOR
	{
		void* pAllocator;
		YXC_MMAllocFunc pAlloc;
		YXC_MMFreeFunc pFree;

		YXC_MMAllocatorType allocType;
	}YXC_MMAllocator;

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_EMPool);

	YXC_API(YXC_Status) YXC_MMCreateFixed(ysize_t stNumBlocks, ysize_t stMaxBlkSize, YXC_MMAllocator* pAllocator);

	YXC_API(YXC_Status) YXC_MMCreateFlat(ysize_t stPoolSize, YXC_MMAllocator* pAllocator);

	YXC_API(void) YXC_MMCreateCRunTime(YXC_MMAllocator* pAllocator);

	YXC_API(void) YXC_MMDestroy(YXC_MMAllocator* pAllocator);

	YXC_API(YXC_Status) YXC_MMCExpandBuffer(void** ppBuffer, ysize_t* pstBufferSize, ysize_t cbRequired);

	YXC_API(YXC_Status) YXC_MMCMakeSureBufferOrFree(void** ppBuffer, ysize_t* pstBufferSize, ysize_t cbRequired);

	YXC_API(void) YXC_MMCFreeData(void* buffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_MM_INTERFACE_H__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_HEAP_DETECT_H__
#define __INC_YXC_SYS_BASE_HEAP_DETECT_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_ResDetBase.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if YXC_PLATFORM_WIN

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_HeapAInfo);

	typedef struct __YXC_HEAP_ALLOC_RECORD
	{
		YXC_ResourceAlloc rBase;
		yssize_t stAllocationSize;
	}YXC_HeapAllocRecord;

	YXC_API(YXC_Status) YXC_EnableHeapDetect();

	YXC_API(void) YXC_DisableHeapDetect();

	YXC_API(YXC_Status) YXC_QueryHeapAllocs(YXC_HeapAInfo* pHeapAllocInfo);

	YXC_API(YXC_Status) YXC_GetProcessHeapAllocs(yuint32_t uProcessId, YXC_HeapAInfo* pHeapAllocInfo);

	YXC_API(YXC_Status) YXC_DumpProcessHeapAllocs(HANDLE hProcess, const YXC_HeapAInfo allocInfo, const wchar_t* cpszFilename);

	YXC_API(YXC_Status) YXC_WriteHeapAllocs(const wchar_t* cpszFilename, yuint32_t uProcessId, const YXC_HeapAInfo heapInfo);

	YXC_API(YXC_Status) YXC_ReadHeapAllocs(const wchar_t* cpszFilename, yuint32_t* puProcessId, YXC_HeapAInfo* pHeapInfo);

	YXC_API(void) YXC_FreeHeapAllocs(YXC_HeapAInfo allocInfo);

	YXC_API(YXC_Status) YXC_DiffHeapAllocs(const YXC_HeapAInfo alloc1, const YXC_HeapAInfo alloc2, YXC_HeapAInfo* pHeapDiff);

#endif /* YXC_PLATFORM_WIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_HEAP_DETECT_H__ */

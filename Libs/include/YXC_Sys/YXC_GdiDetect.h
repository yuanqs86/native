/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_GDI_DETECT_H__
#define __INC_YXC_SYS_BASE_GDI_DETECT_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_ResDetBase.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if YXC_PLATFORM_WIN

	typedef enum __YXC_GDI_ALLOC_TYPE
	{
		YXC_GDI_ALLOC_TYPE_UNKNOWN = 0,
		YXC_GDI_ALLOC_TYPE_PEN,
		YXC_GDI_ALLOC_TYPE_DC,
		YXC_GDI_ALLOC_TYPE_BRUSH,
		YXC_GDI_ALLOC_TYPE_BITMAP,
		YXC_GDI_ALLOC_TYPE_FONT,
		YXC_GDI_ALLOC_TYPE_REGION,
		YXC_GDI_ALLOC_TYPE_PALETTE,
	}YXC_GdiAllocType;

	typedef struct __YXC_GDI_ALLOC_RECORD
	{
		YXC_ResourceAlloc recAlloc;
		YXC_GdiAllocType allocType;
	}YXC_GdiAllocRecord;

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_GdiAInfo);

	YXC_API(YXC_Status) YXC_EnableGdiDetect();

	YXC_API(void) YXC_DisableGdiDetect();

	YXC_API(YXC_Status) YXC_QueryGdiAllocs(YXC_GdiAInfo* pGdiAllocInfo);

	YXC_API(YXC_Status) YXC_GetProcessGdiAllocs(yuint32_t uProcessId, YXC_GdiAInfo* pGdiAllocInfo);

	YXC_API(YXC_Status) YXC_DumpProcessGdiAllocs(HANDLE hProcess, const YXC_GdiAInfo allocInfo, const wchar_t* cpszFilename);

	YXC_API(YXC_Status) YXC_WriteGdiAllocs(const wchar_t* cpszFilename, yuint32_t uProcessId, const YXC_GdiAInfo gdiInfo);

	YXC_API(YXC_Status) YXC_ReadGdiAllocs(const wchar_t* cpszFilename, yuint32_t* puProcessId, YXC_GdiAInfo* pGdiAllocInfo);

	YXC_API(void) YXC_FreeGdiAllocs(YXC_GdiAInfo allocInfo);

	YXC_API(YXC_Status) YXC_DiffGdiAllocs(const YXC_GdiAInfo alloc1, const YXC_GdiAInfo alloc2, YXC_GdiAInfo* pGdiDiff);

#endif /* YXC_PLATFORM_WIN */

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_GDI_DETECT_H__ */

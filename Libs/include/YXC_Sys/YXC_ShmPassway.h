/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_SHARE_MEMORY_PASSWAY_H__
#define __INC_YXC_SYS_BASE_SHARE_MEMORY_PASSWAY_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_ShmPassway);

	YXC_API(YXC_Status) YXC_ShmPWCreate(const wchar_t* pszShmName, yuint32_t uInitSize, yuint32_t uPWMaxSize, YXC_ShmPassway* pPassway);

	YXC_API(YXC_Status) YXC_ShmPWOpen(const wchar_t* pszShmName, YXC_ShmPassway* pPassway, yuint32_t* puPWMaxSize);

	YXC_API(YXC_Status) YXC_ShmPWLock(YXC_ShmPassway passway, yuint32_t stmsTimeout);

	YXC_API(YXC_Status) YXC_ShmPWUnlock(YXC_ShmPassway passway);

	YXC_API(YXC_Status) YXC_ShmPWMap(YXC_ShmPassway passway, yuint32_t uNumBytesToMap, void** ppAddr);

	YXC_API(void) YXC_ShmPWUnmap(YXC_ShmPassway passway, void* pMappedAddr);

	YXC_API(void) YXC_ShmPWGetSize(YXC_ShmPassway passway, yuint32_t* puCurrentSize);

	YXC_API(YXC_Status) YXC_ShmPWSetSize(YXC_ShmPassway passway, yuint32_t uNewSize);

	YXC_API(void) YXC_ShmPWClose(YXC_ShmPassway passway);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_SHARE_MEMORY_PASSWAY_H__ */

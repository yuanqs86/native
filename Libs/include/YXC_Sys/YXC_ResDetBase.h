/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_RESOURCE_DETECT_BASE_H__
#define __INC_YXC_SYS_BASE_RESOURCE_DETECT_BASE_H__

#define YXC_RESOURCE_MAX_STACK 64

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef struct __YXC_RESOURCE_ALLOC_INFO
	{
		yuint32_t uNumStacks;
		yint32_t iCallCount;
		yuintptr_t upAllocStacks[YXC_RESOURCE_MAX_STACK];
	}YXC_ResourceAlloc;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_RESOURCE_DETECT_BASE_H__ */

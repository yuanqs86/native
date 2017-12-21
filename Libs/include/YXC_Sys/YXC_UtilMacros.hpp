/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_UTIL_MACROS_HPP__
#define __INC_YXC_SYS_BASE_UTIL_MACROS_HPP__

#include <YXC_Sys/YXC_Sys.h>

#define YXC_DECLARE_HANDLE_HP_TRANSFER(HdlType, PtrType, FuncHdlToPtr, FuncPtrToHdl)	\
	inline PtrType* FuncHdlToPtr(HdlType hdl) { return (PtrType*)hdl; } \
	inline HdlType FuncPtrToHdl(PtrType* ptr) { return (HdlType)ptr; }

#define YXC_SWAP_STRUCTURES(s1, s2)	\
	do {	\
		ybyte_t byStructBytes[sizeof(s1)];			\
		memcpy(byStructBytes, &s1, sizeof(s1));		\
		memcpy(&s1, &s2, sizeof(s1));				\
		memcpy(&s2, &byStructBytes, sizeof(s1));	\
	} while (0);

#endif /* __INC_YXC_SYS_BASE_UTIL_MACROS_HPP__ */

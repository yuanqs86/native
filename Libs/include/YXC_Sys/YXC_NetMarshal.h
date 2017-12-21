/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_MARSHAL_H__
#define __INC_YXC_SYS_BASE_NET_MARSHAL_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NPBase.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_API(ybool_t) YXC_NetNeedMarshalNumber();

	YXC_API(yuint16_t) YXC_NetMarshalUInt16(yuint16_t u16Val);

	YXC_API(yuint16_t) YXC_NetUnmarshalUInt16(yuint16_t u16Marshalled);

	YXC_API(yuint32_t) YXC_NetMarshalUInt32(yuint32_t u32Val);

	YXC_API(yuint32_t) YXC_NetUnmarshalUInt32(yuint32_t u32Marshalled);

	YXC_API(yuint64_t) YXC_NetMarshalUInt64(yuint64_t u64Val);

	YXC_API(yuint64_t) YXC_NetUnmarshalUInt64(yuint64_t u64Marshalled);

	YXC_API(yuint64_t) YXC_NetMarshalUInt64Bits(yuint64_t u64Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen);

	YXC_API(yuint64_t) YXC_NetUnmarshalUInt64Bits(yuint64_t u64Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen);

	YXC_API(yuint32_t) YXC_NetMarshalUInt32Bits(yuint32_t u32Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen);

	YXC_API(yuint32_t) YXC_NetUnmarshalUInt32Bits(yuint32_t u32Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen);

	YXC_API(void) YXC_NetMarshalStringW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch);

	YXC_API(void) YXC_NetMarshalStringSelfW(wchar_t* pszStr, ysize_t stCch);

	YXC_API(void) YXC_NetUnmarshalStringW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch);

	YXC_API(void) YXC_NetUnmarshalStringSelfW(wchar_t* pszStr, ysize_t stCch);

	YXC_API(void) YXC_NetMarshalGuid(YXC_Guid* guid);

	YXC_API(void) YXC_NetUnmarshalGuid(YXC_Guid* guid);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_NET_MARSHAL_H__ */

#ifndef __INNER_INC_YXC_SYS_BASE_NET_STREAM_UTILITY_H__
#define __INNER_INC_YXC_SYS_BASE_NET_STREAM_UTILITY_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_TypeDealer.hpp>
#include <YXC_Sys/YXC_IMemoryStream.hpp>
#include <YXC_Sys/YXC_OMemoryStream.hpp>
#include <exception>

namespace YXCLib
{
	YXC_API(void) NSReadStringW(IMemoryStream& is, yuint32_t stCcStr, wchar_t* pszStr, yuint32_t* puCcStr,
		const char* pszName);

	YXC_API(void) NSReadGuid(IMemoryStream& is, YXC_Guid* pGuid, const char* pszName);

	YXC_API(void) NSReadStringA(IMemoryStream& is, yuint32_t stCcStr, char* pszStr, yuint32_t* puCcStr,
                              const char* pszName);

	YXC_API(YXC_Status) NSReadEmpStringW(IMemoryStream& is, YMAlloc& emp, wchar_t** ppszStr, yuint32_t* puCcStr, const char* pszName);

	YXC_API(YXC_Status) NSReadEmpStringA(IMemoryStream& is, YMAlloc& emp, char** ppszStr, yuint32_t* puCcStr, const char* pszName);

	YXC_API(YXC_Status) NSReadEmpBinary(IMemoryStream& is, YMAlloc& emp, void** ppBinData,
		yuint32_t* puCbData, const char* pszName);

	YXC_API(void) NSWriteBinary(OMemoryStream& os, const void* pBinData, yuint32_t uCbBin, const char* pszName);

	YXC_API(void) NSWriteGuid(OMemoryStream& os, const YXC_Guid* guid, const char* pszName);

	YXC_API(void) NSWriteStringW(OMemoryStream& os, const wchar_t* pszStr, const char* pszName);

	YXC_API(void) NSWriteStringA(OMemoryStream& os, const char* pszStr, const char* pszName);

#if YCHAR_WCHAR_T
#define NSReadString NSReadStringW
#define NSReadEmpString NSReadEmpStringW
#define NSWriteString NSWriteStringW
#else
#define NSReadString NSReadStringA
#define NSReadEmpString NSReadEmpStringA
#define NSWriteString NSWriteStringA

#endif /* YCHAR_WCHAR_T */

}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_STREAM_UTILITY_H__ */

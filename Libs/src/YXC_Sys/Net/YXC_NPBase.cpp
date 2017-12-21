#define __MODULE__ "EK.Net.Protocol"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <stdio.h>
#include <assert.h>

#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_NetMarshal.h>

#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#else
#include <sys/socket.h>
#if YXC_PLATFORM_UNIX
#include <netdb.h>
#include <arpa/inet.h>
#endif /* YXC_PLATFORM_APPLE */
#endif /* YXC_PLATFORM_WIN */

namespace
{
	const ychar* _http_protocol_name(void* pCtrl)
	{
		return YC("http net protocol");
	}
	//---------------------------------------------------------------------------
	yuint32_t _http_header_size(void* pCtrl)
	{
		return 0;
	}
	//---------------------------------------------------------------------------
	YXC_Status _http_check_header(void* pHeader, yint32_t* pCbPakData, void* pCtrl)
	{
		*pCbPakData = -1; // Unknown size, will control by application.
		return YXC_ERC_SUCCESS;
	}
	//---------------------------------------------------------------------------
	YXC_Status _http_check_data(const void* pHeader, const void* pData, yuint32_t uCbData, void* pCtrl)
	{
		return YXC_ERC_SUCCESS;
	}
	//---------------------------------------------------------------------------
	void _http_create_package(const void* pData, yuint32_t uCbData, ybyte_t byProtocol, ybool_t bIsFromServer,
		const YXC_NetPackageExArgs* pArgs, YXC_NetPackage* pPackage, void* pCtrl)
	{
		ybyte_t* pByte = pPackage->byHdrBuffer;

		ysize_t stNumExtHeaders = pArgs->stNumExtHeaders;
		const YXC_Buffer* pExtBuffers = pArgs->pExtHeaders;

		ysize_t stOffset = 0;
		for (ysize_t i = 0; i < stNumExtHeaders; ++i)
		{
			memcpy(pByte + stOffset, pExtBuffers[i].pBuffer, pExtBuffers[i].stBufSize);
			stOffset += pPackage->extHeaders[i].stBufSize;
		}

		pPackage->stNumExtHeaders = stNumExtHeaders;
		pPackage->stNumExtContents = 0;
		memcpy(pPackage->extHeaders, pExtBuffers, sizeof(YXC_Buffer) * stNumExtHeaders);

		pPackage->uDataLen = stOffset + uCbData;//(0xff);
		pPackage->pvData = (ybyte_t*)pData;
	}
}

namespace YXC_Inner
{
	ybyte_t gs_packageSrc = 0;

	yuint32_t _NPGetHeaderCheckSum(const YXC_NetHeader* pHeader)
	{
		ybyte_t* pByte = (ybyte_t*)pHeader;
		yuint32_t uHCheckSum = 0;

		yuint32_t uOldCheckSum = pHeader->u12HeaderChkSum;
		((YXC_NetHeader*)pHeader)->u12HeaderChkSum = 0;

		for (ysize_t i = 0; i < sizeof(YXC_NetHeader); ++i)
		{
			uHCheckSum += pByte[i];
		}

		((YXC_NetHeader*)pHeader)->u12HeaderChkSum = uOldCheckSum; // restore the old check sum
		uHCheckSum -= 'Y' + 'X';
		return uHCheckSum;
	}

	yuint32_t _NPGetCheckSum(const ybyte_t* pData, ysize_t stNumBytes, ysize_t stCheckOffset)
	{
		const ysize_t CHECK_PERIOD = 8;

		ysize_t stFirstCheck = (CHECK_PERIOD - stCheckOffset) % CHECK_PERIOD;
		if (stNumBytes <= stFirstCheck) return 0;

		yuint32_t uRet = 0;
		ysize_t stNumChecks = (stNumBytes - stFirstCheck - 1) / CHECK_PERIOD + 1;
		ysize_t stChkIdx = stFirstCheck;

		for (ysize_t i = 0; i < stNumChecks; ++i)
		{
			uRet += pData[stChkIdx];
			stChkIdx += CHECK_PERIOD;
		}

		return uRet;
	}

	yuint32_t _NPGetDataCheckSum(ysize_t stNumExtHeaders, const YXC_Buffer* pExtHeaders, ysize_t stNumExtContents,
		const YXC_Buffer* pExtContents, ysize_t stDataSize, const void* pData)
	{
		yuint32_t uRet = 0;
		ysize_t stOffset = 0;
		for (ysize_t i = 0; i < stNumExtHeaders; ++i)
		{
			uRet += _NPGetCheckSum((ybyte_t*)pExtHeaders[i].pBuffer, pExtHeaders[i].stBufSize, stOffset);
			stOffset += pExtHeaders[i].stBufSize;
		}

		for (ysize_t i = 0; i < stNumExtContents; ++i)
		{
			uRet += _NPGetCheckSum((ybyte_t*)pExtContents[i].pBuffer, pExtContents[i].stBufSize, stOffset);
			stOffset += pExtContents[i].stBufSize;
		}

		uRet += _NPGetCheckSum((const ybyte_t*)pData, stDataSize, stOffset);
		return uRet;
	}

	void _NPCaclPackage(YXC_NetPackage* pPackage, yuint32_t uDataLen)
	{
		YXC_NetHeader* pHeader = (YXC_NetHeader*)pPackage->byHdrBuffer;
		pHeader->u16DataChkSum = _NPGetDataCheckSum(pPackage->stNumExtHeaders, pPackage->extHeaders, pPackage->stNumExtContents,
			pPackage->extContents, uDataLen, pPackage->pvData);

		pHeader->u12HeaderChkSum = _NPGetHeaderCheckSum(pHeader);

		pPackage->uDataLen = pHeader->u27DataLen;
	}

	void _NPAppendPackage(YXC_NetPackage* pPackage, const void* pExtHeader, yuint32_t uExtHeaderLen)
	{
		ysize_t stOffset = 0;
		for (ysize_t i = 0; i < pPackage->stNumExtHeaders; ++i)
		{
			stOffset += pPackage->extHeaders[i].stBufSize;
		}
		memcpy(pPackage->byHdrBuffer + sizeof(YXC_NetHeader) + stOffset, pExtHeader, uExtHeaderLen);

		pPackage->extHeaders[pPackage->stNumExtHeaders].pBuffer = pPackage->byHdrBuffer + sizeof(YXC_NetHeader) + stOffset;
		pPackage->extHeaders[pPackage->stNumExtHeaders].stBufSize = uExtHeaderLen;

		++pPackage->stNumExtHeaders;

		YXC_NetHeader* pNetHeader = (YXC_NetHeader*)pPackage->byHdrBuffer;
		pNetHeader->u27DataLen += uExtHeaderLen;
	}

	const ybyte_t bySegsInfo[] = { 1, 12, 16, 8, 27 };
	const ybyte_t bySegsVer[] = { 4, 4 };

	void _NPMarshalHeader(YXC_NetHeader* pHeader)
	{
		if (pHeader->byVersionHigh > 1) /* Support old unmarshaled net protocols. */
		{
			ybyte_t* pbyVer = (ybyte_t*)pHeader + 2;
			*pbyVer = YXC_NetMarshalUInt32Bits(*pbyVer, YXC_ARR_COUNT(bySegsVer), bySegsVer);

			yuint64_t* pu64Desc = (yuint64_t*)((ybyte_t*)pHeader + 4);
			*pu64Desc = YXC_NetMarshalUInt64Bits(*pu64Desc, YXC_ARR_COUNT(bySegsInfo), bySegsInfo);
		}
		else
		{

		}
	}

	void _NPUnmarshalHeader(YXC_NetHeader* pHeader)
	{
		if (pHeader->byVersionHigh <= 1 && pHeader->byVersionLow <= 1) /* This protocol is lower than 1.0. */
		{

		}
		else
		{
			ybyte_t* pbyVer = (ybyte_t*)pHeader + 2;
			*pbyVer = YXC_NetUnmarshalUInt32Bits(*pbyVer, YXC_ARR_COUNT(bySegsVer), bySegsVer);

			yuint64_t* pu64Desc = (yuint64_t*)((ybyte_t*)pHeader + 4);
			*pu64Desc = YXC_NetUnmarshalUInt64Bits(*pu64Desc, YXC_ARR_COUNT(bySegsInfo), bySegsInfo);
		}
	}

	template <typename Ch>
	static void _YXC_Str2IpAddr(const Ch* pszIpAddr, YXC_SockAddr* pAddrOut)
	{
		yuint32_t uDots = 0, uColons = 0, uStrLen = 0;

		char addrX[YXC_MAX_IP], *pAddr = addrX;

		const Ch* pszTemp = pszIpAddr;
		while (TRUE)
		{
			char c = (char)pszTemp[uStrLen];
			if (uStrLen == YXC_MAX_IP - 1 || c == 0) break;
			++uStrLen;
			*pAddr++ = c;
			if (c == '.') ++uDots;
			else if (c == ':') ++uColons;
		}
		addrX[uStrLen] = 0;

		struct addrinfo* addrInfo = NULL;
		getaddrinfo(addrX, NULL, NULL, &addrInfo);

		if (addrInfo != NULL)
		{
			pAddrOut->un.afType = addrInfo->ai_family;
			if (addrInfo->ai_family == AF_INET)
			{
				memcpy(&pAddrOut->un.v4Addr, addrInfo->ai_addr, sizeof(sockaddr_in));
			}
			else if (addrInfo->ai_family == AF_INET6)
			{
				memcpy(&pAddrOut->un.v6Addr, addrInfo->ai_addr, sizeof(sockaddr_in6));
			}
			freeaddrinfo(addrInfo);
		}
		else
		{
			pAddrOut->un.afType = AF_UNSPEC;
		}
	}
}

using namespace YXC_Inner;

extern "C"
{
	void YXC_NPCreatePackage(ybool_t bIsFromServer, ybyte_t byProtocolType, yuint32_t uDataLen, const void* pData, YXC_NetPackage* pPackage)
	{
		YXC_NetPackageExArgs args = { 0 };
		YXC_NPCreatePackageArgs(bIsFromServer, byProtocolType, uDataLen, pData, &args, pPackage);
	}

	void YXC_NPCreatePackageEx(ybool_t bIsFromServer, ybyte_t byProtocolType, yuint32_t uDataLen, const void* pData,
		ysize_t stNumExtHeaders, const YXC_Buffer* pExtBuffers, YXC_NetPackage* pPackage)
	{
		YXC_NetPackageExArgs args = { stNumExtHeaders, pExtBuffers, 0, NULL };
		YXC_NPCreatePackageArgs(bIsFromServer, byProtocolType, uDataLen, pData, &args, pPackage);
	}

	void YXC_NPCreatePackageArgs(ybool_t bIsFromServer, ybyte_t byProtocolType, yuint32_t uDataLen, const void* pData,
		const YXC_NetPackageExArgs* pExArgs, YXC_NetPackage* pPackage)
	{
		YXC_NetHeader* pHeader = (YXC_NetHeader*)pPackage->byHdrBuffer;
		pHeader->byHeader[0] = 'Y';
		pHeader->byHeader[1] = 'X';
		pHeader->byVersionHigh = 2;
		pHeader->byVersionLow = 1;
		pHeader->u1IsFromServer = bIsFromServer;
		pHeader->byProtocolType = byProtocolType;
		pHeader->u8PackageSrc = gs_packageSrc;
		pHeader->u27DataLen = uDataLen;
		pPackage->pvData = (ybyte_t*)pData;
		pPackage->stNumExtHeaders = 0;
		pPackage->stNumExtContents = pExArgs->stNumExtContents;

		assert(pExArgs->stNumExtHeaders < YXC_NP_MAX_ADDTIONAL_HEADERS);
		for (ysize_t i = 0; i < pExArgs->stNumExtHeaders; ++i)
		{
			assert(pExArgs->pExtHeaders[i].stBufSize < YXC_NP_MAX_ADDTIONAL_HEADER_SIZE);
			_NPAppendPackage(pPackage, pExArgs->pExtHeaders[i].pBuffer, (yuint32_t)pExArgs->pExtHeaders[i].stBufSize);
		}

		for (ysize_t i = 0; i < pExArgs->stNumExtContents; ++i)
		{
			pPackage->netHeader.u27DataLen += pExArgs->pExtContents[i].stBufSize;
			pPackage->extContents[i] = pExArgs->pExtContents[i];
		}

		_NPCaclPackage(pPackage, uDataLen);
		_NPMarshalHeader((YXC_NetHeader*)pPackage->byHdrBuffer);
	}

	void YXC_NPInitTransferInfo(const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		pTransferInfo->stTotalHeaderSize = sizeof(YXC_NetHeader);
		if (pPackage != NULL)
		{
			for (ysize_t i = 0; i < pPackage->stNumExtHeaders; ++i)
			{
				pTransferInfo->stTotalHeaderSize += pPackage->extHeaders[i].stBufSize;
			}
		}
		pTransferInfo->stExtHeaderSize = pTransferInfo->stTotalHeaderSize - sizeof(YXC_NetHeader);
		pTransferInfo->stHeaderTransferred = 0;
		pTransferInfo->stDataTransferred = 0;
		pTransferInfo->stExtContentIdx = 0;
		pTransferInfo->stExtContentTransferred = 0;
	}

	yuint32_t YXC_NPGetHeaderChkSum(const YXC_NetHeader* pHeader)
	{
		return _NPGetHeaderCheckSum(pHeader);
	}

	yuint32_t YXC_NPGetDataChkSum(yuint32_t uCbData, const void* pBuffer)
	{
		yuint32_t uDataChkSum = _NPGetDataCheckSum(0, NULL, 0, NULL, uCbData, pBuffer);
		return uDataChkSum & 0x0000FFFF; // 16 bit
	}

	void YXC_NPSetPackageSrc(ybyte_t byPackageSrc)
	{
		gs_packageSrc = byPackageSrc;
	}

	yuint32_t YXC_NPBaseGetHeaderSize(void* pCtrl)
	{
		return sizeof(YXC_NetHeader);
	}

	YXC_Status YXC_NPBaseCheckHeader(void* pHeader, yint32_t* pCbPakData, void* pCtrl)
	{
		YXC_NetHeader* pNetHeader = (YXC_NetHeader*)pHeader;
		_YXC_CHECK_REPORT_NEW_RET(pNetHeader->byHeader[0] == 'Y' && pNetHeader->byHeader[1] == 'X', YXC_ERC_NET_INVALID_HEADER,
			YC("Net header is not started with 'YX'"));

		YXC_NetHeader header2 = *pNetHeader;
		_NPUnmarshalHeader(&header2);

		yuint32_t uExpectedHChkSum = YXC_NPGetHeaderChkSum(&header2);
		yuint32_t uActualHChkSum = header2.u12HeaderChkSum;

		_YXC_CHECK_REPORT_NEW_RET(uExpectedHChkSum == uActualHChkSum, YXC_ERC_NET_CHECKSUM_FAILED,
			YC("Package header check sum validation failed, expected = %u, actual = %u"),
			uExpectedHChkSum, uActualHChkSum);

		*pNetHeader = header2;
		*pCbPakData = pNetHeader->u27DataLen;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_NPBaseCheckData(const void* pHeader, const void* pData, yuint32_t uCbData, void* pCtrl)
	{
		const YXC_NetHeader* pNetHeader = (const YXC_NetHeader*)pHeader;

		yuint32_t uExpectDChkSum = YXC_NPGetDataChkSum(uCbData, pData);
		yuint32_t uActualDChkSum = pNetHeader->u16DataChkSum;

		_YXC_CHECK_REPORT_NEW_RET(uExpectDChkSum == uActualDChkSum, YXC_ERC_NET_CHECKSUM_FAILED,
			YC("Package data check sum validation failed, expected = %u, actual = %u"), uExpectDChkSum, uActualDChkSum);

		return YXC_ERC_SUCCESS;
	}

	void YXC_NPBaseCreatePackage(const void* pData, yuint32_t uCbData, ybyte_t byProtocol, ybool_t bIsFromServer,
		const YXC_NetPackageExArgs* pArgs, YXC_NetPackage* pPackage, void* pCtrl)
	{
		YXC_NPCreatePackageArgs(bIsFromServer, byProtocol, uCbData, pData, pArgs, pPackage);
	}

	const ychar* YXC_NPBaseGetName(void* pCtrl)
	{
		return YC("yxc basic net protocol");
	}

	void YXC_IPV4ToStrW(YXC_IPV4STRW* pszAddr, yuint32_t uAddrV4)
	{
		YXC_IPV4STRA strA;
		YXC_IPV4ToStrA(&strA, uAddrV4);

		wchar_t (&szIp)[YXC_MAX_IPV4] = *pszAddr;
		for (yuint32_t i = 0; i < YXC_MAX_IPV4; ++i)
		{
			szIp[i] = strA[i];
		}
	}

	void YXC_IPV4ToStrA(YXC_IPV4STRA* pszAddr, yuint32_t uAddrV4)
	{
		in_addr addr;
		addr.s_addr = uAddrV4;
		strcpy(*pszAddr, inet_ntoa(addr));
	}

	void YXC_Str2IpAddrW(const wchar_t* pszIpAddr, YXC_SockAddr* pAddrOut)
	{
		_YXC_Str2IpAddr<wchar_t>(pszIpAddr, pAddrOut);
	}

	void YXC_Str2IpAddrA(const char* pszIpAddr, YXC_SockAddr* pAddrOut)
	{
		_YXC_Str2IpAddr<char>(pszIpAddr, pAddrOut);
	}

	YXC_NetProtocol YXC_NPGetHttpProtocol()
	{
		YXC_NetProtocol transportation;
		transportation.pCtrl = NULL;
		transportation.pfnCheckHeader = _http_check_header;
		transportation.pfnCheckPackage = _http_check_data;
		transportation.pfnCreatePackage = _http_create_package;
		transportation.pfnGetHeaderSize = _http_header_size;
		transportation.pfnGetProtocolName = _http_protocol_name;

		return transportation;
	}
}

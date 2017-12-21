/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_PROTOCOL_H__
#define __INC_YXC_SYS_BASE_NET_PROTOCOL_H__

#include <YXC_Sys/YXC_Sys.h>
#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#include <ws2tcpip.h>
#elif YXC_PLATFORM_UNIX
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif /* YXC_PLATFORM_WIN */

#define YXC_NP_MAX_PACKAGE_SRC ((1 << 12) - 1)
#define YXC_NP_MAX_PACKAGE_TYPE ((1 << 12) - 1)
#define YXC_NP_MAX_PACKAGE_SIZE (1 << 27) // 128M

#define YXC_NP_PROTOCOL_TYPE_NONE 0
#define YXC_NP_PROTOCOL_TYPE_MULTICAST 1
#define YXC_NP_PROTOCOL_TYPE_AV_DATA 2
#define YXC_NP_PROTOCOL_TYPE_AV_CTRL 3
#define YXC_NP_PROTOCOL_TYPE_P2P 4
#define YXC_NP_PROTOCOL_TYPE_AV_OLD 5
#define YXC_NP_PROTOCOL_TYPE_DISK_SRV 6
#define YXC_NP_PROTOCOL_TYPE_RSHELL 7
#define YXC_NP_PROTOCOL_TYPE_RP_CONF 8

#define YXC_NP_MAX_ADDTIONAL_HEADERS 4
#define YXC_NP_MAX_ADDTIONAL_CONTENTS 4
#define YXC_NP_MAX_ADDTIONAL_HEADER_SIZE 64

#define YXC_NP_MAX_ADDTIONAL_HEADER_BUF (YXC_NP_MAX_ADDTIONAL_HEADERS * YXC_NP_MAX_ADDTIONAL_HEADER_SIZE)

#define YXC_NP_MAX_NET_HDR (200)
#define YXC_NP_MAX_ALL_HEADERS_SIZE (YXC_NP_MAX_NET_HDR + YXC_NP_MAX_ADDTIONAL_HEADER_BUF)

#define YXC_NP_FPACKAGE_NUM_IDSETS (1 << 20)
#define YXC_MAX_IPV4 16
#define YXC_MAX_IPV6 46
#define YXC_MAX_IPV6_SCOPE 10
#define YXC_MAX_IP (YXC_MAX_IPV6 + YXC_MAX_IPV6_SCOPE - 1) /* Include terminating character. */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    typedef wchar_t YXC_IPSTRW[YXC_MAX_IP];
    typedef char YXC_IPSTRA[YXC_MAX_IP];
    typedef wchar_t YXC_IPV4STRW[YXC_MAX_IPV4];
    typedef char YXC_IPV4STRA[YXC_MAX_IPV4];

#if YCHAR_WCHAR_T
    typedef YXC_IPSTRW YXC_IPSTR;
    typedef YXC_IPV4STRW YXC_IPV4STR;
#else
    typedef YXC_IPSTRA YXC_IPSTR;
    typedef YXC_IPV4STRA YXC_IPV4STR;
#endif /* YCHAR_WCHAR_T */


#pragma pack(push, 1)
	typedef struct __YXC_NET_PROTOCOL_HEADER // version 1, 12 bytes
	{
		ybyte_t byHeader[2]; /* YX */
		ybyte_t byVersionHigh:4;
		ybyte_t byVersionLow:4;
		ybyte_t byProtocolType;
		yuint64_t u1IsFromServer:1;
		yuint64_t u12HeaderChkSum:12;
		yuint64_t u16DataChkSum:16;
		yuint64_t u8PackageSrc:8;
		yuint64_t u27DataLen:27;
	}YXC_NetHeader; /* Easy crc, use protocol crc */

	typedef struct __YXC_NET_FRAGMENT_HEADER
	{
		yuint16_t uSubPacketId; // max to 65536 sub packets, support at least 16M package
		yuint16_t uTotalPackages;
		yuint32_t uUseIntranetMTU : 1;
		yuint32_t uLastPackageSize : 11; // max to 2048 bytes
		yuint32_t uPackageId : 20; // main package id
	}YXC_NetHeaderF;
#pragma pack(pop)

	typedef struct __YXC_NET_TRANSFER_PACKAGE
	{
		union
		{
			ybyte_t byHdrBuffer[YXC_NP_MAX_ALL_HEADERS_SIZE];
			YXC_NetHeader netHeader; // for compiling.
		};

		/* Following members only use in directly send, for dispatch data pointers send, to avoid copying memory here. */
		ysize_t stNumExtHeaders;
		ysize_t stNumExtContents;
		YXC_Buffer extHeaders[YXC_NP_MAX_ADDTIONAL_HEADERS];
		YXC_Buffer extContents[YXC_NP_MAX_ADDTIONAL_CONTENTS];

		yuint32_t uDataLen;
		ybyte_t* pvData;
	}YXC_NetPackage;

	typedef struct __YXC_NET_TRANSFER_INFO
	{
		ysize_t stHeaderTransferred;
		ysize_t stTotalHeaderSize;
		ysize_t stExtHeaderSize;
		ysize_t stExtContentIdx;
		ysize_t stExtContentTransferred;
		ysize_t stDataTransferred;
	}YXC_NetTransferInfo;

	typedef struct __YXC_NET_PACKAGE_CREATE_ARGS_EX
	{
		ysize_t stNumExtHeaders;
		const YXC_Buffer* pExtHeaders;
		ysize_t stNumExtContents;
		const YXC_Buffer* pExtContents;
	}YXC_NetPackageExArgs;

	typedef yuint32_t (*YXC_NPGetHeaderSizeFunc)(void* pCtrl);
	/* return -1 if unknown data size, then use default buffer size */
	typedef YXC_Status (*YXC_NPCheckHeaderFunc)(void* pHeader, yint32_t* pCbPakData, void* pCtrl);
	typedef YXC_Status (*YXC_NPCheckPackageFunc)(const void* pHeader, const void* pData, yuint32_t uCbData, void* pCtrl);
	typedef void (*YXC_NPCreatePackageFunc)(const void* pData, yuint32_t uCbData, ybyte_t byProtocol, ybool_t bIsFromServer,
		const YXC_NetPackageExArgs* pExArgs, YXC_NetPackage* pPackage, void* pCtrl);
	typedef const ychar* (*YXC_NPGetNameFunc)(void* pCtrl);

	typedef struct __YXC_NET_PROTOCOL
	{
		YXC_NPGetHeaderSizeFunc pfnGetHeaderSize;
		YXC_NPCheckHeaderFunc pfnCheckHeader;
		YXC_NPCheckPackageFunc pfnCheckPackage;
		YXC_NPGetNameFunc pfnGetProtocolName;
		YXC_NPCreatePackageFunc pfnCreatePackage;

		void* pCtrl;
	}YXC_NetProtocol;

	YXC_API(void) YXC_NPCreatePackage(ybool_t bIsFromServer, ybyte_t byProtocolType, yuint32_t uDataLen, const void* pData, YXC_NetPackage* pPackage);

	YXC_API(void) YXC_NPCreatePackageEx(ybool_t bIsFromServer, ybyte_t byProtocolType, yuint32_t uDataLen, const void* pData,
		ysize_t stNumExtHeaders, const YXC_Buffer* pExtBuffers, YXC_NetPackage* pPackage);

	YXC_API(void) YXC_NPCreatePackageArgs(ybool_t bIsFromServer, ybyte_t byProtocolType, yuint32_t uDataLen, const void* pData,
		const YXC_NetPackageExArgs* pExArgs, YXC_NetPackage* pPackage);

	YXC_API(void) YXC_NPInitTransferInfo(const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

	YXC_API(void) YXC_NPSetPackageSrc(ybyte_t byPackageSrc);

	//YXC_API yuint32_t YXC_NPGetHeaderChkSum(const YXC_NetHeader* pHeader);

	//YXC_API yuint32_t YXC_NPGetDataChkSum(yuint32_t uCbData, const void* pBuffer);

	YXC_API(const ychar*) YXC_NPBaseGetName(void* pCtrl);

	YXC_API(YXC_Status) YXC_NPBaseCheckData(const void* pHeader, const void* pData, yuint32_t uCbData, void* pCtrl);

	YXC_API(YXC_Status) YXC_NPBaseCheckHeader(void* pHeader, yint32_t* pCbPakData, void* pCtrl);

	YXC_API(yuint32_t) YXC_NPBaseGetHeaderSize(void* pCtrl);

	YXC_API(void) YXC_NPBaseCreatePackage(const void* pData, yuint32_t uCbData, ybyte_t byProtocol, ybool_t bIsFromServer,
		const YXC_NetPackageExArgs* pArgs, YXC_NetPackage* pPackage, void* pCtrl);

	// Following for standard protocol.
	YXC_INLINE void YXC_NPStdInitTransferInfo(const YXC_NetProtocol* pProtocol, const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		ysize_t stBaseSize = pProtocol->pfnGetHeaderSize(pProtocol->pCtrl);
		pTransferInfo->stTotalHeaderSize = stBaseSize;
		if (pPackage != NULL)
		{
			for (ysize_t i = 0; i < pPackage->stNumExtHeaders; ++i)
			{
				pTransferInfo->stTotalHeaderSize += pPackage->extHeaders[i].stBufSize;
			}
		}
		pTransferInfo->stExtHeaderSize = pTransferInfo->stTotalHeaderSize - stBaseSize;
		pTransferInfo->stHeaderTransferred = 0;
		pTransferInfo->stDataTransferred = 0;
		pTransferInfo->stExtContentIdx = 0;
		pTransferInfo->stExtContentTransferred = 0;
	}

	YXC_INLINE void YXC_NPStdCreatePackage(const YXC_NetProtocol* pProtocol, yuint32_t uDataLen, const void* pData,
		ybyte_t byProtocol, ybool_t bIsFromServer, YXC_NetPackage* pPackage)
	{
		YXC_NetPackageExArgs exArgs = { 0 };
		pProtocol->pfnCreatePackage(pData, uDataLen, byProtocol, bIsFromServer, &exArgs, pPackage, pProtocol->pCtrl);
	}

	YXC_INLINE void YXC_NPStdCreatePackageEx(const YXC_NetProtocol* pProtocol, yuint32_t uDataLen, const void* pData,
		ybyte_t byProtocol, ybool_t bIsFromServer, ysize_t stNumExtDatas,
		const YXC_Buffer* pExtBuffers, YXC_NetPackage* pPackage)
	{
		YXC_NetPackageExArgs exArgs = { stNumExtDatas, pExtBuffers, 0, NULL };
		pProtocol->pfnCreatePackage(pData, uDataLen, byProtocol, bIsFromServer, &exArgs, pPackage, pProtocol->pCtrl);
	}

	YXC_INLINE void YXC_NPStdCreatePackageArgs(const YXC_NetProtocol* pProtocol, yuint32_t uDataLen, const void* pData,
		ybyte_t byProtocol, ybool_t bIsFromServer, const YXC_NetPackageExArgs* pExArgs, YXC_NetPackage* pPackage)
	{
		pProtocol->pfnCreatePackage(pData, uDataLen, byProtocol, bIsFromServer, pExArgs, pPackage, pProtocol->pCtrl);
	}

	YXC_INLINE YXC_NetProtocol YXC_NPGetBaseProtocol()
	{
		YXC_NetProtocol protocol;

		protocol.pCtrl = NULL;
		protocol.pfnCheckHeader = YXC_NPBaseCheckHeader;
		protocol.pfnCheckPackage = YXC_NPBaseCheckData;
		protocol.pfnGetHeaderSize = YXC_NPBaseGetHeaderSize;
		protocol.pfnGetProtocolName = YXC_NPBaseGetName;
		protocol.pfnCreatePackage = YXC_NPBaseCreatePackage;

		return protocol;
	}

	YXC_INLINE void YXC_NPSetProtocol(YXC_NetProtocol* pDst, const YXC_NetProtocol* pProtocol)
	{
		if (pProtocol)
		{
			*pDst = *pProtocol;
		}
		else
		{
			*pDst = YXC_NPGetBaseProtocol();
		}
	}

	typedef struct __YXC_SOCK_ADDR
	{
		union
		{
			yuint16_t afType;
			struct sockaddr_in6 v6Addr;
#if YXC_PLATFORM_WIN
			struct sockaddr_in v4Addr;
#else
            struct sockaddr_in v4Addr;
#endif /* YXC_PLATFORM_WIN */
		}un;
	}YXC_SockAddr;

	YXC_API(void) YXC_Str2IpAddrW(const wchar_t* pszIpAddr, YXC_SockAddr* pAddrOut);

	YXC_API(void) YXC_Str2IpAddrA(const char* pszIpAddr, YXC_SockAddr* pAddrOut);

	/* pszAddr must be at least 16 characters len. */
	YXC_API(void) YXC_IPV4ToStrW(YXC_IPV4STRW* pszAddr, yuint32_t uAddrV4);

	YXC_API(void) YXC_IPV4ToStrA(YXC_IPV4STRA* pszAddr, yuint32_t uAddrV4);

#if YCHAR_WCHAR_T
#define YXC_IPV4ToStr(szAddr, uAddrV4) YXC_IPV4ToStrW(szAddr, uAddrV4)
#define YXC_Str2IpAddr(pszIpAddr, pAddrOut) YXC_Str2IpAddrW(pszIpAddr, pAddrOut)
#else
#define YXC_IPV4ToStr(szAddr, uAddrV4) YXC_IPV4ToStrA(szAddr, uAddrV4)
#define YXC_Str2IpAddr(pszIpAddr, pAddrOut) YXC_Str2IpAddrA(pszIpAddr, pAddrOut)
#endif /* YCHAR_WCHAR_T */

	YXC_NetProtocol YXC_NPGetHttpProtocol();

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_NET_PROTOCOL_H__ */

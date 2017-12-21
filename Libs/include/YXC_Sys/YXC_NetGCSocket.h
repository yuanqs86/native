/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_GC_SOCKET_H__
#define __INC_YXC_SYS_BASE_NET_GC_SOCKET_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/YXC_Locker.h>

#define YXC_GC_MAX_GROUP_MEMBER_NAME 16
#define YXC_GC_BROADCAST_MTU 1472
#define YXC_GC_BROADCAST_MTU_INTRANET 572
#define YXC_GC_MAX_DATA_SIZE (YXC_GC_BROADCAST_MTU - sizeof(YXC_NetHeader))

#define YXC_GC_MAX_NUM_PACKETS (1 << 16)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	// group cast receiver socket
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_GCR);

	// group cast broadcaster socket
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_GCB);

	YXC_API(YXC_Status) YXC_GCRAttachA(const char* cpszGroupAddr, const char* cpszDstAddr, yuint32_t uPort,
		YXC_Event waitEvent, const YXC_NetProtocol* pProtocol, YXC_GCR* pReceiver);

	YXC_API(YXC_Status) YXC_GCRAttachW(const wchar_t* cpszGroupAddr, const wchar_t* cpszDstAddr, yuint32_t uPort,
		YXC_Event waitEvent, const YXC_NetProtocol* pProtocol, YXC_GCR* pReceiver);

	YXC_API(YXC_Status) YXC_GCRAttach(const ychar* cpszGroupAddr, const ychar* cpszDstAddr, yuint32_t uPort,
                                   YXC_Event waitEvent, const YXC_NetProtocol* pProtocol, YXC_GCR* pReceiver);

	YXC_API(YXC_Status) YXC_GCRReceive(YXC_GCR receiver, YXC_NetPackage* pNetPackage, ybyte_t* pData,
		ysize_t stBufData, yuint32_t uTimeout, ybool_t* pEventSignaled);

	YXC_API(void) YXC_GCRDetach(YXC_GCR receiver);

    YXC_API(YXC_Status) YXC_GCBCreate(const ychar* cpszGroupAddr, yuint32_t uPort, yuint32_t uMTU,
        const YXC_NetProtocol* pProtocol, YXC_GCB* pBroadcaster);

	YXC_API(YXC_Status) YXC_GCBCreateA(const char* cpszGroupAddr, yuint32_t uPort, yuint32_t uMTU,
		const YXC_NetProtocol* pProtocol, YXC_GCB* pBroadcaster);

	YXC_API(YXC_Status) YXC_GCBCreateW(const wchar_t* cpszGroupAddr, yuint32_t uPort, yuint32_t uMTU,
		const YXC_NetProtocol* pProtocol, YXC_GCB* pBroadcaster);

	YXC_API(YXC_Status) YXC_GCBBroadcast(YXC_GCB broadcaster, const YXC_NetPackage* pPackage);

	YXC_API(YXC_Status) YXC_GCBBroadcastVerified(YXC_GCB broadcaster, const YXC_NetPackage* pVerifiedPackage);

	//YXC_API(YXC_Status) YXC_GCRReceiveSubPacket(YXC_GCR receiver, YXC_NetPackage* pReservedPackage,
	//	yuint32_t uTimeout, yuint32_t* puPackageId, yuint32_t* puSubPacketId, ybool_t* pEventSignaled);

	//YXC_API(YXC_Status) YXC_GCBBroadcastFragment(YXC_GCB broadcaster, yuint32_t uPackageId, yuint32_t uFragmentId,
	//	ybyte_t* pBuffer, yuint32_t uBufSize);

	YXC_API(yuint32_t) YXC_GCBGetMTU(YXC_GCB broadcaster);

	YXC_API(void) YXC_GCBClose(YXC_GCB broadcaster);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_NET_GC_SOCKET_H__ */

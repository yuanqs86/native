#define __MODULE__ "EK.Net.GroupCast"

#include <YXC_Sys/YXC_NetGCSocket.h>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <stdio.h>

#if YXC_PLATFORM_WIN
#include <WS2tcpip.h>
#endif /* YXC_PLATFORM_WIN */

#define YXC_GC_BROADCAST_MTU 1472

namespace YXC_Inner
{
	typedef struct __GROUPCAST_RECEIVER
	{
		socket_t rSock;
		in_addr groupAddr;
		yuint32_t uGroupPort;
		in_addr dstAddr;
		YXC_Event waitEvent;
		YXC_NetProtocol protocol;
#if YXC_PLATFORM_WIN
		void* hSockEvent; /* For Windows, Shut down socket */
#endif /* YXC_PLATFORM_WIN */
	}_GCReceiver;

	typedef struct __GROUPCAST_BROADCASTER
	{
		socket_t rSock;
		sockaddr_in addrBroadcast;
		YXC_NetProtocol protocol;
		yuint32_t uMTU;
	}_GCBroadcaster;

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_GCR, _GCReceiver, _GCRPtr, _GCRHdl);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_GCB, _GCBroadcaster, _GCBPtr, _GCBHdl);

	static YXC_Status _CheckAndFillPackage(ybyte_t* byBuffer, int iLen, YXC_NetProtocol* pProtocol, YXC_NetPackage* pOutPackage)
	{
		yint32_t iCbHeader = pProtocol->pfnGetHeaderSize(pProtocol->pCtrl), iCbData = 0;
		_YXC_CHECK_REPORT_NEW_RET(iCbHeader < iLen, YXC_ERC_NET_INVALID_PROTOCOL, YC("(PT:%s)Received data size(%d) less than header size(%d)"),
			pProtocol->pfnGetProtocolName(pProtocol->pCtrl), iLen, iCbHeader);

		YXC_ClearErrors();
		YXC_Status rc = pProtocol->pfnCheckHeader(byBuffer, &iCbData, pProtocol->pCtrl);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, YXC_ERC_NET_CHECKSUM_FAILED, YC("(PT:%s)Invalid header check sum, drop it"),
			pProtocol->pfnGetProtocolName(pProtocol->pCtrl));

		yuint32_t uLenReal = iLen - iCbHeader;
		if (iCbData >= 0)
		{
			_YXC_CHECK_REPORT_NEW_RET(uLenReal == iCbData, YXC_ERC_NET_INVALID_PROTOCOL,
				YC("[%s]Length transferred(%d) not equal to expected(%d)"), uLenReal, iCbData);
		}

		pOutPackage->stNumExtHeaders = 0;
		pOutPackage->pvData = byBuffer + iCbHeader;
		pOutPackage->uDataLen = uLenReal;

		rc = pProtocol->pfnCheckPackage(byBuffer, byBuffer + iCbHeader, uLenReal, pProtocol->pCtrl);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("(PT:%s)Invalid data check sum, drop it"),
			pProtocol->pfnGetProtocolName(pProtocol->pCtrl));

		return YXC_ERC_SUCCESS;
	}
}

using namespace YXC_Inner;

extern "C"
{
	YXC_Status YXC_GCRAttachA(const char* cpszGroupAddr, const char* cpszDstAddr, yuint32_t uPort, YXC_Event waitEvent,
		const YXC_NetProtocol* pProtocol, YXC_GCR* pReceiver)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		socket_t sock = -1;
        int iRet = -1;
		sockaddr_in sock_addr;
        yuint32_t uAddrGroup = INADDR_ANY, uAddrDst = INADDR_ANY;

		_GCReceiver* pReceiver2 = (_GCReceiver*)malloc(sizeof(_GCReceiver));
		_YXC_CHECK_REPORT_NEW_RET(pReceiver2 != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for group cast socket failed"));

		YXC_Status rc = _CreateSocket(YXC_SOCKET_TYPE_UDP, TRUE, &sock);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create socket for group cast socket failed"));

		sock_addr.sin_family = AF_INET;
		sock_addr.sin_port = htons(uPort);
		sock_addr.sin_addr.s_addr = INADDR_ANY;

        iRet = ::bind(sock, (const sockaddr*)&sock_addr, sizeof(sockaddr_in));
		_YXC_CHECK_REPORT_NEW_RET(iRet != -1, _getsocketerror(), YC("Bind port %d for receive group cast failed"));

		uAddrGroup = inet_addr(cpszGroupAddr);
		if (cpszDstAddr != NULL)
		{
			uAddrDst = inet_addr(cpszDstAddr);
		}
		ip_mreq reqAddr;
		reqAddr.imr_interface.s_addr = INADDR_ANY;
		reqAddr.imr_multiaddr.s_addr = uAddrGroup;

		iRet = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&reqAddr, sizeof(ip_mreq));
		_YXC_CHECK_REPORT_NEW_GOTO(iRet != -1, _getsocketerror(), YC("Add to group member failed[G-%S D-%S]"), cpszGroupAddr, cpszDstAddr);

#if YXC_PLATFORM_WIN
		void* hEvent = ::WSACreateEvent();
		_YXC_CHECK_OS_GOTO(hEvent != NULL, L"Create WSA Event for group cast socket failed");

		iRet = WSAEventSelect(sock, hEvent, FD_READ | FD_CLOSE);
		_YXC_CHECK_REPORT_NEW_GOTO(iRet != SOCKET_ERROR, _getsocketerror(), L"Select read event for group cast socket failed");
		pReceiver2->hSockEvent = hEvent;
#endif /* YXC_PLATFORM_WIN */
		pReceiver2->groupAddr.s_addr = uAddrGroup;
		pReceiver2->dstAddr.s_addr = uAddrDst;
		pReceiver2->rSock = sock;
		pReceiver2->uGroupPort = uPort;
		pReceiver2->waitEvent = waitEvent;
		YXC_NPSetProtocol(&pReceiver2->protocol, pProtocol);

		*pReceiver = _GCRHdl(pReceiver2);

		return YXC_ERC_SUCCESS;
err_ret:
		free(pReceiver2);
        if (sock != -1) _closesocket(sock);
#if YXC_PLATFORM_WIN
		if (hEvent != NULL) WSACloseEvent(hEvent);
#else

#endif /* YXC_PLATFORM_WIN */
		return rcRet;
	}

	YXC_Status YXC_GCRAttachW(const wchar_t* cpszGroupAddr, const wchar_t* cpszDstAddr, yuint32_t uPort,
		YXC_Event waitEvent, const YXC_NetProtocol* pProtocol, YXC_GCR* pGCSocket)
	{
		char szGroupAddr[32], szDstAddr[32];
		sprintf(szGroupAddr, "%ls", cpszGroupAddr);

		if (cpszDstAddr != NULL)
		{
			sprintf(szDstAddr, "%ls", cpszDstAddr);

			return YXC_GCRAttachA(szGroupAddr, szDstAddr, uPort, waitEvent, pProtocol, pGCSocket);
		}
		else
		{
			return YXC_GCRAttachA(szGroupAddr, NULL, uPort, waitEvent, pProtocol, pGCSocket);
		}
	}

	YXC_Status YXC_GCBCreateA(const char* cpszGroupAddr, yuint32_t uPort, yuint32_t uMTUVal,
		const YXC_NetProtocol* pProtocol, YXC_GCB* pBroadcaster)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		_GCBroadcaster* pBroadcaster2 = (_GCBroadcaster*)malloc(sizeof(_GCBroadcaster));
		_YXC_CHECK_REPORT_NEW_RET(pBroadcaster2 != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for group cast socket failed"));

		socket_t sock;
		YXC_Status rc = _CreateSocket(YXC_SOCKET_TYPE_UDP, TRUE, &sock);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create socket for group cast socket failed"));

		pBroadcaster2->rSock = sock;
		pBroadcaster2->uMTU = uMTUVal;
		pBroadcaster2->addrBroadcast.sin_family = AF_INET;
		pBroadcaster2->addrBroadcast.sin_addr.s_addr = inet_addr(cpszGroupAddr);
		pBroadcaster2->addrBroadcast.sin_port = htons(uPort);
		YXC_NPSetProtocol(&pBroadcaster2->protocol, pProtocol);

		*pBroadcaster = _GCBHdl(pBroadcaster2);

		return YXC_ERC_SUCCESS;
err_ret:
		free(pBroadcaster2);
		return rcRet;
	}

	YXC_Status YXC_GCBCreateW(const wchar_t* cpszGroupAddr, yuint32_t uPort, yuint32_t uMTUVal,
		const YXC_NetProtocol* pProtocol, YXC_GCB* pBroadcaster)
	{
		char szGroupAddr[32];
		sprintf(szGroupAddr, "%ls", cpszGroupAddr);

		return YXC_GCBCreateA(szGroupAddr, uPort, uMTUVal, pProtocol, pBroadcaster);
	}

	YXC_Status YXC_GCRReceive(YXC_GCR receiver, YXC_NetPackage* pNetPackage, ybyte_t* pData, ysize_t stBufData,
		yuint32_t uTimeout, ybool_t* pEventSignaled)
	{
		_YXC_CHECK_REPORT_NEW_RET(stBufData >= YXC_GC_BROADCAST_MTU, YXC_ERC_BUFFER_NOT_ENOUGH,
			YC("Receive buffer is too small to hold a group cast package"));

		_GCReceiver* pReceiver = _GCRPtr(receiver);

#if YXC_PLATFORM_WIN
		HANDLE hEvents[2] = { pReceiver->hSockEvent, pReceiver->waitEvent };

		DWORD dwNumWaits = 2;
		if (pReceiver->waitEvent == NULL)
		{
			dwNumWaits = 1; // no event to wait here.
		}

		DWORD dwRet = ::WSAWaitForMultipleEvents(dwNumWaits, hEvents, FALSE, uTimeout, FALSE);

		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			return YXC_ERC_SOCK_TIMEDOUT;
		}
		else if (dwRet == WSA_WAIT_FAILED)
		{
			YXC_Status rc = _getsocketerror();
			_YXC_SOCK_REPORT(rc, L"Wait for receive events failed");
			return rc;
		}
		dwRet -= WSA_WAIT_EVENT_0;

		if (dwRet == 1)
		{
			*pEventSignaled = TRUE;
		}
		else
		{
			*pEventSignaled = FALSE;
			sockaddr_in addr;

			WSANETWORKEVENTS events = {0};
			::WSAEnumNetworkEvents(pReceiver->rSock, pReceiver->hSockEvent, &events);

			if (events.lNetworkEvents & FD_READ)
			{
				int iLen = sizeof(sockaddr_in);
				int iRet = recvfrom(pReceiver->rSock, (char*)pData, YXC_GC_BROADCAST_MTU, 0, (sockaddr*)&addr, &iLen);
				_YXC_CHECK_REPORT_NEW_RET(iRet != SOCKET_ERROR, _getsocketerror(), L"Receive data from remote group failed");
				_YXC_CHECK_REPORT_NEW_RET(iRet > sizeof(YXC_NetHeader), YXC_ERC_NET_INVALID_PROTOCOL, L"Invalid header received");

				YXC_Status rc = _CheckAndFillPackage(pData, iRet, &pReceiver->protocol, pNetPackage);
				return rc;
			}

			return YXC_ERC_NO_DATA;
		}
#else
        struct timeval tv;
        tv.tv_sec = uTimeout / 1000;
        tv.tv_usec = uTimeout % 1000 * 1000;

        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET(pReceiver->rSock, &fdSet);

        int ret = select(pReceiver->rSock + 1, &fdSet, NULL, NULL, &tv);
        _YXC_CHECK_OS_RET(ret != -1, YC("Failed to select"));
        _YXC_CHECK_OS_RET(ret != 0, YC("Timeout(%d) on select"), uTimeout);

#endif /* YXC_PLATFORM_WIN */
		return YXC_ERC_SUCCESS;
		// pSocket->
	}

	YXC_Status YXC_GCBBroadcast(YXC_GCB broadcaster, const YXC_NetPackage* pNetPackage)
	{
		_GCBroadcaster* pSocket = _GCBPtr(broadcaster);
		ysize_t stHeaderSize = sizeof(YXC_NetHeader);
		for (ysize_t i = 0; i < pNetPackage->stNumExtHeaders; ++i)
		{
			stHeaderSize += pNetPackage->extHeaders[i].stBufSize;
		}

		ysize_t stTotalSize = stHeaderSize + pNetPackage->uDataLen;
		_YXC_CHECK_REPORT_NEW_RET(stTotalSize < YXC_GC_BROADCAST_MTU, YXC_ERC_NET_PACKAGE_TOO_BIG, YC("Too big package to broadcast, size = %d, mtu = %d"),
			(yuint32_t)stTotalSize, pSocket->uMTU);

		return YXC_GCBBroadcastVerified(broadcaster, pNetPackage);
	}

	yuint32_t YXC_GCBGetMTU(YXC_GCB broadcaster)
	{
		_GCBroadcaster* pSocket = _GCBPtr(broadcaster);
		return pSocket->uMTU;
	}

	YXC_Status YXC_GCBBroadcastVerified(YXC_GCB broadcaster, const YXC_NetPackage* pVerifiedPackage)
	{
		_GCBroadcaster* pBroadcaster = _GCBPtr(broadcaster);
		ybyte_t byBuffer[YXC_GC_BROADCAST_MTU];

		ysize_t stHeaderSize = sizeof(YXC_NetHeader);
		for (ysize_t i = 0; i < pVerifiedPackage->stNumExtHeaders; ++i)
		{
			stHeaderSize += pVerifiedPackage->extHeaders[i].stBufSize;
		}

		ysize_t stTotalSize = stHeaderSize + pVerifiedPackage->uDataLen;

		memcpy(byBuffer, pVerifiedPackage->byHdrBuffer, stHeaderSize);
		memcpy(byBuffer + stHeaderSize, pVerifiedPackage->pvData, pVerifiedPackage->uDataLen);

		ssize_t iSend = ::sendto(pBroadcaster->rSock, (char*)byBuffer, (int)stTotalSize, 0,
			(sockaddr*)&pBroadcaster->addrBroadcast, sizeof(sockaddr_in));
		_YXC_CHECK_REPORT_NEW_RET(iSend != -1, _getsocketerror(), YC("Do group cast broadcast failed"));

		return YXC_ERC_SUCCESS;
	}

	void YXC_GCRDetach(YXC_GCR receiver)
	{
		_GCReceiver* pReceiver = _GCRPtr(receiver);

		_closesocket(pReceiver->rSock);
#if YXC_PLATFORM_WIN
		WSACloseEvent(pReceiver->hSockEvent);
#endif /* YXC_PLATFORM_WIN */
		free(pReceiver);
	}

	void YXC_GCBClose(YXC_GCB gcSendSock)
	{
		_GCBroadcaster* pSocket = _GCBPtr(gcSendSock);

		_closesocket(pSocket->rSock);
		free(pSocket);
	}
}

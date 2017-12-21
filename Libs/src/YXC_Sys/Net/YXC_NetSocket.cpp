#define __MODULE__ "EK.Net.Socket"

#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#if YXC_PLATFORM_WIN
#include <YXC_Sys/YXC_NetHook.h>
#define snprintf _snprintf
#endif /* YXC_PLATFORM_WIN */
#include <stdio.h>
#include <errno.h>

#include <algorithm>

using namespace YXC_Inner;

namespace
{
#if YXC_PLATFORM_WIN
	YXC_Status _WSASendAll(socket_t rSock, ysize_t stCbHeader, const ybyte_t* pHeader, ysize_t stCbExtHeader,
		const YXC_NetPackage* pPackage, ysize_t* pstSent)
	{
		DWORD dwSent;
		WSABUF wsaBuf[2 + YXC_NP_MAX_ADDTIONAL_CONTENTS];
		wsaBuf[0].len = (ULONG)stCbHeader;
		wsaBuf[0].buf = (CHAR*)pHeader;
		WSABUF* pWSABuf = &wsaBuf[1];

		ysize_t uRealDataLen = pPackage->uDataLen - stCbExtHeader;
		for (yuint32_t i = 0; i < pPackage->stNumExtContents; ++i)
		{
			const YXC_Buffer& content = pPackage->extContents[i];
			uRealDataLen -= content.stBufSize;

			pWSABuf->buf = (CHAR*)content.pBuffer;
			pWSABuf->len = (ULONG)content.stBufSize;
		}
		pWSABuf->buf = (CHAR*)pPackage->pvData;
		pWSABuf->len = uRealDataLen;
		int iSend = WSASend(rSock, wsaBuf, 2 + (DWORD)pPackage->stNumExtContents, &dwSent, 0, NULL, NULL);

		_YXC_NET_CHECK_SOCK_RET(iSend != SOCKET_ERROR, L"Send data to remote failed");
		*pstSent = dwSent;

		return YXC_ERC_SUCCESS;
	}
#endif /* YXC_PLATFORM_WIN */

}

extern "C"
{
#if YXC_PLATFORM_WIN
	YXC_Status YXC_SockWSAStart(WORD wVersion, WSADATA* pData)
	{
		int iInit = ::WSAStartup(wVersion, pData);
		_YXC_NET_CHECK_SOCK_RET(iInit == 0, L"Start Winsock failed, Version = (%d, %d)", HIBYTE(wVersion), LOBYTE(wVersion));
		return YXC_ERC_SUCCESS;
	}

	void YXC_SockWSACleanup()
	{
		::WSACleanup();
	}
#endif /* YXC_PLATFORM_WIN */

	YXC_Status YXC_SockConnectToServerA(YXC_SocketType sockType, const char* cpszIpAddr, yuint32_t uPort, yuint32_t uConnTime, YXC_Socket* pSock)
	{
		return YXC_SockConnectToServerExA(sockType, cpszIpAddr, uPort, uConnTime, 0, FALSE, pSock);
	}

	YXC_Status YXC_SockConnectToServerW(YXC_SocketType sockType, const wchar_t* cpszIpAddr, yuint32_t uPort, yuint32_t uConnTime, YXC_Socket* pSock)
	{
		char szIpAddr[20] = {0};
		snprintf(szIpAddr, 19, "%ls", cpszIpAddr);

		return YXC_SockConnectToServerExA(sockType, szIpAddr, uPort, uConnTime, 0, FALSE, pSock);
	}

	YXC_API(YXC_Status) YXC_SockConnectToServerExA(YXC_SocketType sockType, const char* cpszIpAddr, yuint32_t uPort,
		yuint32_t uConnTime, yuint32_t uLocalPort, ybool_t bReuseAddr, YXC_Socket* pSock)
	{
		socket_t sock = 0;
		YXC_Inner::_Socket* sockRet = NULL;
		YXC_Status rcRet = _CreateSocket(sockType, bReuseAddr, &sock);
		if (rcRet != YXC_ERC_SUCCESS) return rcRet;

		sockaddr_in sock_addr;
        int iRet, iSelect, iError = 0;
        ybool_t bRet;
        fd_set fdSet, fdRead, fdErr;
		timeval tm;
        socklen_t iNameLen = sizeof(sockaddr_in), iLen;

		sockRet = (_Socket*)calloc(1, sizeof(_Socket));
		_YXC_CHECK_REPORT_NEW_GOTO(sockRet != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for socket failed"));

		sock_addr.sin_family = AF_INET;
		sock_addr.sin_port = htons(uPort);
		sock_addr.sin_addr.s_addr = inet_addr(cpszIpAddr);

		_SetSocketBlock(sock, FALSE);

		if (uLocalPort != 0)
		{
			sockaddr_in sock_addr2;
			sock_addr2.sin_family = AF_INET;
			sock_addr2.sin_port = htons(uLocalPort);
			sock_addr2.sin_addr.s_addr = htonl(INADDR_ANY);

            iRet = ::bind(sock, (sockaddr*)&sock_addr2, sizeof(sockaddr_in));
			_YXC_NET_CHECK_SOCK_GOTO(iRet != -1, YC("Bind to local port %d failed"), uLocalPort);
		}

        iRet = ::connect(sock, (sockaddr*)&sock_addr, sizeof(sockaddr_in));
        bRet = iRet != -1 || _getsocketerror() == YXC_ERC_SOCK_WOULD_BLOCK;
		_YXC_NET_CHECK_SOCK_GOTO(bRet, YC("Connect to server %@:%d failed"), cpszIpAddr, uPort);

		FD_ZERO(&fdSet);
		FD_SET(sock, &fdSet);

		tm.tv_sec = uConnTime / 1000;
		tm.tv_usec = uConnTime % 1000 * 1000;

#if YXC_PLATFORM_WIN
        iSelect = select(sock, NULL, &fdSet, NULL, &tm);
#else
        fdRead = fdSet;
        fdErr = fdSet;
        iSelect = select(sock + 1, &fdRead, &fdSet, &fdErr, &tm);
#endif /* YXC_PLATFORM_WIN */

		_YXC_NET_CHECK_SOCK_GOTO(iSelect != -1, YC("Select event failed"));
		_YXC_NET_CHECK_GOTO(iSelect != 0, YXC_ERC_SOCK_TIMEDOUT, YC("Timeout occured when select, server [%@:%d]"), cpszIpAddr, uPort);

#if YXC_PLATFORM_UNIX
        if (!FD_ISSET(sock, &fdRead) && !FD_ISSET(sock, &fdSet)) {
            _YXC_REPORT_NEW_GOTO(YXC_ERC_SOCK_CONN_ABORTED, YC("No signal set"));
        }

        iRet = getsockopt(sock, SOL_SOCKET, SO_ERROR, &iError, &iLen);
        _YXC_NET_CHECK_SOCK_GOTO(iError == 0, YC("Connect server error, server [%s:%d]"), cpszIpAddr, uPort);

#endif /* YXC_PLATFORM_UNIX */

		_SetSocketBlock(sock, TRUE);

		sockRet->sockType = sockType;
		sockRet->uMaxPackageSize = _YXC_NET_MAX_PACKAGE_SIZE;
		sockRet->bResolveLeakIssue = FALSE;
		if (sockType == YXC_SOCKET_TYPE_TCP)
		{
			sockRet->uPacketSize = _YXC_NET_TCP_PACKET_SIZE;
		}
		else
		{
			sockRet->uPacketSize = _YXC_NET_UDP_PACKET_SIZE;
		}
		YXC_NPSetProtocol(&sockRet->protocol, NULL);
		sockRet->rSock = sock;

		getsockname(sock, (sockaddr*)&sockRet->inAddr, &iNameLen);

		*pSock = _SockHdl(sockRet);
		return YXC_ERC_SUCCESS;
err_ret:
		_closesocket(sock);
		if (sockRet != NULL) free(sockRet);
		return rcRet;
	}

	YXC_API(YXC_Status) YXC_SockConnectToServerExW(YXC_SocketType sockType, const wchar_t* cpszIpAddr, yuint32_t uPort,
		yuint32_t uConnTime, yuint32_t uLocalPort, ybool_t bReuseAddr, YXC_Socket* pSock)
	{
		char szIpAddr[20];
		sprintf(szIpAddr, "%ls", cpszIpAddr);

		return YXC_SockConnectToServerExA(sockType, szIpAddr, uPort, uConnTime, uLocalPort, bReuseAddr, pSock);
	}

	YXC_Status YXC_SockCreateServer(YXC_SocketType sockType, yuint32_t uPort, YXC_Socket* pServerSock)
	{
		return YXC_SockCreateServerEx(sockType, uPort, FALSE, pServerSock);
	}

	YXC_Status YXC_SockCreateServerEx(YXC_SocketType sockType, yuint32_t uPort, ybool_t bReuseAddr, YXC_Socket* pServerSock)
	{
#if YXC_PLATFORM_WIN
		if (gs_bDoSrvLeakHook)
		{
			YXC_Status rc = YXC_NetEnableLeakHook();
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Enable leak hook for server socket failed"));
		}
#endif /* YXC_PLATFORM_WIN */

		socklen_t iNameLen = sizeof(sockaddr_in);
		socket_t sock = 0;
        int iRet;
		_Socket* sockRet = NULL;
		YXC_Status rcRet = _CreateSocket(sockType, bReuseAddr, &sock);
		if (rcRet != YXC_ERC_SUCCESS) return rcRet;

		sockRet = (_Socket*)calloc(1, sizeof(_Socket));
		_YXC_CHECK_REPORT_NEW_GOTO(sockRet != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for socket failed"));

		sockaddr_in sock_addr;
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_port = htons(uPort);
		sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        iRet = ::bind(sock, (const sockaddr*)&sock_addr, sizeof(sock_addr));
		_YXC_NET_CHECK_SOCK_GOTO(iRet != -1, YC("Bind socket failed, port = %d"), uPort);

		if (sockType == YXC_SOCKET_TYPE_TCP)
		{
			iRet = ::listen(sock, _YXC_SOCK_BAKLOG);
			_YXC_NET_CHECK_SOCK_GOTO(iRet != -1, YC("Listen socket failed, port = %d, baklog = %d"), uPort, _YXC_SOCK_BAKLOG);
		}

		sockRet->rSock = sock;
		sockRet->sockType = sockType;
		sockRet->uMaxPackageSize = _YXC_NET_MAX_PACKAGE_SIZE;
		sockRet->bResolveLeakIssue = gs_bDoSrvLeakHook && sockType == YXC_SOCKET_TYPE_TCP;
		YXC_NPSetProtocol(&sockRet->protocol, NULL);
		if (sockType == YXC_SOCKET_TYPE_TCP)
		{
			sockRet->uPacketSize = _YXC_NET_TCP_PACKET_SIZE;
		}
		else
		{
			sockRet->uPacketSize = _YXC_NET_UDP_PACKET_SIZE;
		}

		getsockname(sock, (sockaddr*)&sockRet->inAddr, &iNameLen);

		*(_Socket**)pServerSock = sockRet;
		return YXC_ERC_SUCCESS;
err_ret:
		if (sockRet != NULL) free(sockRet);
		_closesocket(sock);
		return rcRet;
	}

	YXC_Status YXC_SockAcceptClient(YXC_Socket serverSock, YXC_Socket* pClientSock)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		_Socket* sockRet = NULL, *pServerSock = _SockPtr(serverSock);
		sockRet = (_Socket*)calloc(1, sizeof(_Socket));
		_YXC_NET_CHECK_RET(sockRet != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for socket failed"));

		sockaddr_in sock_addr;
		socklen_t iLen = sizeof(sockaddr_in);

		socket_t sock = accept(pServerSock->rSock, (sockaddr*)&sock_addr, &iLen);
		_YXC_NET_CHECK_SOCK_GOTO(sock != -1, YC("Accept socket error"));

		*pClientSock = _SockHdl(sockRet);

		sockRet->sockType = pServerSock->sockType;
		sockRet->uMaxPackageSize = pServerSock->uMaxPackageSize;
		sockRet->uPacketSize = pServerSock->uPacketSize;
		sockRet->bResolveLeakIssue = pServerSock->bResolveLeakIssue;
		sockRet->inAddr = sock_addr;
		sockRet->aliveProperty.bIsEnabled = FALSE;
		sockRet->rSock = sock;
		YXC_NPSetProtocol(&sockRet->protocol, NULL);

		iLen = sizeof(yuint32_t);
		getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&sockRet->uRecvBufSize, &iLen);
		getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sockRet->uSendBufSize, &iLen);
		getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&sockRet->uRecvTimeout, &iLen);
		getsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&sockRet->uSendTimeout, &iLen);

#if YXC_PLATFORM_WIN
		if (pServerSock->bResolveLeakIssue)
		{
			YXC_NetAddToHookList(sock);
		}
#endif /* YXC_PLATFORM_WIN */

		return YXC_ERC_SUCCESS;
err_ret:
		if (sockRet != NULL) free(sockRet);
		return rcRet;
	}

	YXC_Status YXC_SockSetOption(YXC_Socket socket, YXC_SocketOption option, const YXC_SocketOptValue* pOptionValue)
	{
		_Socket* pSocket = _SockPtr(socket);
		int iRetCode;
		switch (option)
		{
		case YXC_SOCKET_OPT_MAX_PACKAGE_SIZE:
			pSocket->uMaxPackageSize = pOptionValue->uMaxPackageSize;
			break;
		case YXC_SOCKET_OPT_PACKET_SIZE:
			pSocket->uPacketSize = pOptionValue->uPacketSize;
			break;
		case YXC_SOCKET_OPT_RECV_TIMEOUT:
			iRetCode = setsockopt(pSocket->rSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)pOptionValue, sizeof(yuint32_t));
			_YXC_NET_CHECK_SOCK_RET(iRetCode == 0, YC("Set receive timeout failed"));
			pSocket->uRecvTimeout = pOptionValue->uRecvTimeout;
			break;
		case YXC_SOCKET_OPT_SEND_TIMEOUT:
			iRetCode = setsockopt(pSocket->rSock, SOL_SOCKET, SO_SNDTIMEO, (const char*)pOptionValue, sizeof(yuint32_t));
			_YXC_NET_CHECK_SOCK_RET(iRetCode == 0, YC("Set send timeout failed"));
			pSocket->uSendTimeout = pOptionValue->uSendTimeout;
			break;
		case YXC_SOCKET_OPT_RECV_BUF_SIZE:
			iRetCode = setsockopt(pSocket->rSock, SOL_SOCKET, SO_RCVBUF, (const char*)pOptionValue, sizeof(yuint32_t));
			_YXC_NET_CHECK_SOCK_RET(iRetCode == 0, YC("Set receive buffer size failed"));
			pSocket->uRecvBufSize = pOptionValue->uRecvBufSize;
			break;
		case YXC_SOCKET_OPT_SEND_BUF_SIZE:
			iRetCode = setsockopt(pSocket->rSock, SOL_SOCKET, SO_SNDBUF, (const char*)&pOptionValue, sizeof(yuint32_t));
			_YXC_NET_CHECK_SOCK_RET(iRetCode == 0, YC("Set send buffer size failed"));
			pSocket->uSendBufSize = pOptionValue->uSendBufSize;
			break;
		case YXC_SOCKET_OPT_TCP_KEEPALIVE:
			_YXC_NET_CHECK_RET(pSocket->sockType == YXC_SOCKET_TYPE_TCP, YXC_ERC_NOT_SUPPORTED, YC("Only TCP can have keepalive property"));
			iRetCode = _SetSocketTCPAlive(pSocket->rSock, &pOptionValue->aliveProperty);
			_YXC_NET_CHECK_SOCK_RET(iRetCode == 0, YC("Set socket tcp alive property mode failed"));
			pSocket->aliveProperty = pOptionValue->aliveProperty;
			break;
		case YXC_SOCKET_OPT_WOULD_BLOCK:
			iRetCode = _SetSocketBlock(pSocket->rSock, pOptionValue->bWouldBlock);
			_YXC_NET_CHECK_SOCK_RET(iRetCode == 0, YC("Set socket block mode failed"));
			pSocket->bWouldBlock = pOptionValue->bWouldBlock;
			break;
		case YXC_SOCKET_OPT_NET_PROTOCOL:
			pSocket->protocol = pOptionValue->sockProtocol;
			break;
		case YXC_SOCKET_OPT_P2P_BIND_DATA:
			pSocket->pP2PBindData = pOptionValue->pP2PBindData;
			break;
		default:
			_YXC_SOCK_REPORT(YXC_ERC_INVALID_PARAMETER, YC("Invalid socket option to set"));
			return YXC_ERC_INVALID_PARAMETER;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_SockGetOption(YXC_Socket socket, YXC_SocketOption option, YXC_SocketOptValue* pOptionValue)
	{
		_Socket* pSocket = _SockPtr(socket);
		switch (option)
		{
		case YXC_SOCKET_OPT_MAX_PACKAGE_SIZE:
			pOptionValue->uMaxPackageSize = pSocket->uMaxPackageSize;
			return YXC_ERC_SUCCESS;
		case YXC_SOCKET_OPT_PACKET_SIZE:
			pOptionValue->uPacketSize = pSocket->uPacketSize;
			return YXC_ERC_SUCCESS;
		case YXC_SOCKET_OPT_RECV_TIMEOUT:
			pOptionValue->uRecvTimeout = pSocket->uRecvTimeout;
			break;
		case YXC_SOCKET_OPT_SEND_TIMEOUT:
			pSocket->uSendTimeout = pOptionValue->uSendTimeout;
			break;
		case YXC_SOCKET_OPT_RECV_BUF_SIZE:
			pOptionValue->uRecvBufSize = pSocket->uRecvBufSize;
			break;
		case YXC_SOCKET_OPT_SEND_BUF_SIZE:
			pOptionValue->uSendBufSize = pSocket->uSendBufSize;
			break;
		case YXC_SOCKET_OPT_TCP_KEEPALIVE:
			pOptionValue->aliveProperty = pSocket->aliveProperty;
			break;
		case YXC_SOCKET_OPT_WOULD_BLOCK:
			pOptionValue->bWouldBlock = pSocket->bWouldBlock;
			break;
		case YXC_SOCKET_OPT_SOCKET_VALUE:
			pOptionValue->rSock = pSocket->rSock;
			break;
		case YXC_SOCKET_OPT_NET_PROTOCOL:
			pOptionValue->sockProtocol = pSocket->protocol;
			break;
		case YXC_SOCKET_OPT_IP_END_POINT:
			pOptionValue->sockAddr = pSocket->inAddr;
			break;
		case YXC_SOCKET_OPT_P2P_BIND_DATA:
			pOptionValue->pP2PBindData = pSocket->pP2PBindData;
			break;
		default:
			_YXC_SOCK_REPORT(YXC_ERC_INVALID_PARAMETER, YC("Invalid socket option to set"));
			return YXC_ERC_INVALID_PARAMETER;
		}

		return YXC_ERC_SUCCESS;
	}

	void YXC_SockClose(YXC_Socket sock)
	{
		_YXC_SockClose(sock);
	}

	YXC_Status YXC_SockSendData(YXC_Socket socket, ysize_t stNumBytes, const ybyte_t* pData, ysize_t* pstSent)
	{
		_Socket* pSocket = _SockPtr(socket);

#if YXC_PLATFORM_WIN
		DWORD dwSent;
		WSABUF wsaBuf = { (ULONG)stNumBytes, (CHAR*)pData };
		int iSend = WSASend(pSocket->rSock, &wsaBuf, 1, &dwSent, 0, NULL, NULL);

		_YXC_NET_CHECK_SOCK_RET(iSend != -1, YC("Send data to remote failed"));
		*pstSent = dwSent;
#else
		ssize_t iSend = send(pSocket->rSock, (const char*)pData, stNumBytes, 0);

		_YXC_NET_CHECK_SOCK_RET(iSend != -1, YC("Send data to remote failed"));

		*pstSent = iSend;
#endif /* YXC_PLATFORM_WIN */
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_SockReceiveData(YXC_Socket socket, ysize_t stNumBytesInBuffer, ybyte_t* pBuffer, ybool_t bPeekData, ysize_t* pstReceived)
	{
		_Socket* pSocket = _SockPtr(socket);

		ssize_t iRecv = recv(pSocket->rSock, (char*)pBuffer, (int)stNumBytesInBuffer, bPeekData ? MSG_PEEK : 0);
		_YXC_NET_CHECK_SOCK_RET(iRecv != -1 && iRecv != 0, YC("Receive data from remote failed"));

		*pstReceived = iRecv;
		return YXC_ERC_SUCCESS;
	}

	socket_t YXC_SockGetValue(YXC_Socket socket)
	{
		return _SockPtr(socket)->rSock;
	}

	YXC_Status YXC_SockSendPackage(YXC_Socket socket, const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		_Socket* pSock = _SockPtr(socket);
		ysize_t stExtHeader = pTransferInfo->stExtHeaderSize;
		if (pTransferInfo->stHeaderTransferred < pTransferInfo->stTotalHeaderSize)
		{
			while (TRUE)
			{
				ysize_t stHeaderRemain = pTransferInfo->stTotalHeaderSize - pTransferInfo->stHeaderTransferred;
				ysize_t stHasSent;
#if YXC_PLATFORM_WIN
				YXC_Status rc = _WSASendAll(pSock->rSock, stHeaderRemain, pPackage->byHdrBuffer + pTransferInfo->stHeaderTransferred,
					stExtHeader, pPackage, &stHasSent);
#else
				YXC_Status rc = YXC_SockSendData(socket, pTransferInfo->stTotalHeaderSize - pTransferInfo->stHeaderTransferred,
					pPackage->byHdrBuffer + pTransferInfo->stHeaderTransferred, &stHasSent);
#endif /* YXC_PLATFORM_WIN */
				if (rc != YXC_ERC_SUCCESS) return rc;

				if (stHasSent >= stHeaderRemain)
				{
					pTransferInfo->stHeaderTransferred = pTransferInfo->stTotalHeaderSize;
					pTransferInfo->stDataTransferred = stHasSent - stHeaderRemain;
					break;
				}
				else
				{
					pTransferInfo->stHeaderTransferred += stHasSent;
				}
			}

			ysize_t stExPassed = 0;
			for (yuint32_t i = 0; i < pPackage->stNumExtContents; ++i)
			{
				if (pPackage->extContents[i].stBufSize + stExPassed <= pTransferInfo->stDataTransferred)
				{
					pTransferInfo->stExtContentIdx++;
					stExPassed += pPackage->extContents[i].stBufSize;
				}
				else
				{
					pTransferInfo->stExtContentTransferred = pTransferInfo->stDataTransferred - stExPassed;
					break;
				}
			}
		}

		ysize_t stHasSent;
		while (pTransferInfo->stExtContentIdx < pPackage->stNumExtContents)
		{
			const YXC_Buffer& content = pPackage->extContents[pTransferInfo->stExtContentIdx];
			YXC_Status rc = YXC_SockSendData(socket, content.stBufSize - pTransferInfo->stExtContentTransferred,
				(const ybyte_t*)content.pBuffer + pTransferInfo->stExtContentTransferred, &stHasSent);
			if (rc != YXC_ERC_SUCCESS) return rc;

			pTransferInfo->stExtContentTransferred += stHasSent;
			pTransferInfo->stDataTransferred += stHasSent;
			if (pTransferInfo->stExtContentTransferred == content.stBufSize)
			{
				++pTransferInfo->stExtContentIdx;
			}
		}

		ysize_t stTotal = pPackage->uDataLen - stExtHeader;
		while (pTransferInfo->stDataTransferred < stTotal)
		{
			ysize_t stNeedSend = std::min<ysize_t>(stTotal - pTransferInfo->stDataTransferred, pSock->uPacketSize);
			YXC_Status rc = YXC_SockSendData(socket, stNeedSend, pPackage->pvData + pTransferInfo->stDataTransferred, &stHasSent);
			if (rc != YXC_ERC_SUCCESS) return rc;

			pTransferInfo->stDataTransferred += stHasSent;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_SockReceivePackage(YXC_Socket socket, ysize_t stNumBytesInBuffer, ybyte_t* pBuffer, ysize_t* pNeeded,
		YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		_Socket* pSock = _SockPtr(socket);
		void* pCtrl = pSock->protocol.pCtrl;

		ybool_t bFixedRecvSize = TRUE;
		pPackage->stNumExtHeaders = 0;
		pPackage->stNumExtContents = 0;
		if (pTransferInfo->stHeaderTransferred < pTransferInfo->stTotalHeaderSize)
		{
			while (pTransferInfo->stHeaderTransferred < pTransferInfo->stTotalHeaderSize)
			{
				ssize_t nrecv = recv(pSock->rSock, (char*)pPackage->byHdrBuffer + pTransferInfo->stHeaderTransferred,
					pTransferInfo->stTotalHeaderSize - pTransferInfo->stHeaderTransferred, 0);

				if (nrecv > 0)
				{
					pTransferInfo->stHeaderTransferred += nrecv;
				}

				YXC_Status rc = _CheckRecvCode(pSock, (yint32_t)nrecv);
				_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);
			}

			YXC_ClearErrors();
			yint32_t iCbData;
			YXC_Status rc = pSock->protocol.pfnCheckHeader(pPackage->byHdrBuffer, &iCbData, pCtrl);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Check protocol header failed, protocol name = '%s'"),
				pSock->protocol.pfnGetProtocolName(pCtrl) );

			if (iCbData >= 0)
			{
				yuint32_t uCbData = (yuint32_t)iCbData;
				_YXC_NET_CHECK_RET(uCbData < pSock->uMaxPackageSize, YXC_ERC_NET_PACKAGE_TOO_BIG, YC("The max data size should max to 0x%x"),
                    pSock->uMaxPackageSize);
				*pNeeded = uCbData;
				pPackage->uDataLen = iCbData;

				_YXC_NET_CHECK_RET(uCbData <= stNumBytesInBuffer, YXC_ERC_BUFFER_NOT_ENOUGH, YC("Supplied buffer to small to ")
					YC("hold the package, req = %d, buf size = %d"), uCbData, stNumBytesInBuffer);
			}
			else
			{
				bFixedRecvSize = FALSE;
			}
		}
		else if (pTransferInfo->stTotalHeaderSize == 0)
		{
			bFixedRecvSize = FALSE;
		}

		pPackage->pvData = pBuffer;
		if (bFixedRecvSize)
		{
			ysize_t stTotal = pPackage->uDataLen;
			while (pTransferInfo->stDataTransferred < stTotal)
			{
				ysize_t stMaxRecv = std::min<ysize_t>(stTotal - pTransferInfo->stDataTransferred, pSock->uPacketSize);
				ssize_t iRecv = recv(pSock->rSock, (char*)(pBuffer + pTransferInfo->stDataTransferred), stMaxRecv, 0);
				YXC_Status rc = _CheckRecvCode(pSock, (yint32_t)iRecv);
				_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

				pTransferInfo->stDataTransferred += iRecv;
			}

			YXC_ClearErrors();
			YXC_Status rc = pSock->protocol.pfnCheckPackage(pPackage->byHdrBuffer, pPackage->pvData, pPackage->uDataLen, pCtrl);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Check protocol data failed, protocol name = '%s'"),
				pSock->protocol.pfnGetProtocolName(pCtrl) );
		}
		else
		{
			ssize_t iRecv = recv(pSock->rSock, (char*)pBuffer, stNumBytesInBuffer, 0);
			YXC_Status rc = _CheckRecvCode(pSock, (yint32_t)iRecv);
			_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

			pPackage->uDataLen = (yuint32_t)iRecv;
			*pNeeded = iRecv;
			pTransferInfo->stDataTransferred = iRecv;
		}

		return YXC_ERC_SUCCESS;
	}

	void YXC_SockInitTransferInfo(YXC_Socket socket, const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		_Socket* pSocket = _SockPtr(socket);

		YXC_NPStdInitTransferInfo(&pSocket->protocol, pPackage, pTransferInfo);
	}
}

#if YXC_PLATFORM_WIN
namespace YXC_Inner
{
	BOOL _NetDllInitHandler(unsigned int reason)
	{
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			_NetCloseSendHook();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		}
		return TRUE;
	}
}

#if YXC_EXPORTS_FLAG == YXC_EXPORTS_DISPATCH_DLL
extern "C"
{
	BOOL WINAPI DllMain(HANDLE dllHandle, unsigned int reason, void* lpReserved)
	{
		return YXC_Inner::_NetDllInitHandler(reason);
	}
}
#endif /* YXC_EXPORTS_FLAG */

#endif /* YXC_PLATFORM_WIN */

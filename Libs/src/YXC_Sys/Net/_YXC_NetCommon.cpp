#include <YXC_Sys/Net/_YXC_NetCommon.hpp>

#define __MODULE__ "EK.Net.Common"

#if YXC_PLATFORM_WIN
#include <MSTcpIP.h>
#else
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#endif /* YXC_PLATFORM_WIN */

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_NetHook.h>
#include <errno.h>

#include <algorithm>

namespace
{
#if YXC_PLATFORM_WIN
	YXC_Status _switchToNetError(int iPlatformErc)
	{
		switch (iPlatformErc)
		{
		case WSAEADDRINUSE:
			return YXC_ERC_SOCK_ADDR_IN_USE;
		case WSAEADDRNOTAVAIL:
			return YXC_ERC_SOCK_ADDR_NOT_AVAIL;
		case WSAEAFNOSUPPORT:
			return YXC_ERC_SOCK_AF_NO_SUPPORT;
		case WSAEALREADY:
			return YXC_ERC_SOCK_ALREADY;
		case WSAECANCELLED:
			return YXC_ERC_SOCK_CANCELED;
		case WSAECONNABORTED:
			return YXC_ERC_SOCK_CONN_ABORTED;
		case WSAECONNREFUSED:
			return YXC_ERC_SOCK_CONN_REFUSED;
		case WSAECONNRESET:
			return YXC_ERC_SOCK_CONN_RESET;
		case WSAEDESTADDRREQ:
			return YXC_ERC_SOCK_DEST_ADDR_REQ;
		case WSAEHOSTUNREACH:
			return YXC_ERC_SOCK_HOST_UNREACH;
		case WSAEINPROGRESS:
			return YXC_ERC_SOCK_IN_PROGRESS;
		case WSAEISCONN:
			return YXC_ERC_SOCK_IS_CONN;
		case WSAELOOP:
			return YXC_ERC_SOCK_LOOP;
		case WSAEMSGSIZE:
			return YXC_ERC_SOCK_MSG_SIZE;
		case WSAENETDOWN:
			return YXC_ERC_SOCK_NETWORK_DOWN;
		case WSAENETRESET:
			return YXC_ERC_SOCK_NETWORK_RESET;
		case WSAENETUNREACH:
			return YXC_ERC_SOCK_NETWORK_UNREACH;
		case WSAENOBUFS:
			return YXC_ERC_SOCK_NO_BUFS;
		case WSAENOTCONN:
			return YXC_ERC_SOCK_NOT_CONN;
		case WSAENOTSOCK:
			return YXC_ERC_SOCK_NOT_SOCK;
		case WSAEOPNOTSUPP:
			return YXC_ERC_SOCK_OP_NOT_SUPP;
		case WSAEPROTOTYPE:
			return YXC_ERC_SOCK_PROTO_TYPE;
		case WSAEPROTONOSUPPORT:
			return YXC_ERC_SOCK_PROTO_NO_SUPPORT;
		case WSAETIMEDOUT:
			return YXC_ERC_SOCK_TIMEDOUT;
		case WSAEWOULDBLOCK:
			return YXC_ERC_SOCK_WOULD_BLOCK;
		case 0:
			return YXC_ERC_SOCK_WOULD_BLOCK;
		default:
			return YXC_ERC_SOCK_WSA;
		}
	}
#elif YXC_PLATFORM_UNIX
	YXC_Status _switchToNetError(int iPlatformErc)
	{
		switch (iPlatformErc)
		{
		case EADDRINUSE:
			return YXC_ERC_SOCK_ADDR_IN_USE;
		case EADDRNOTAVAIL:
			return YXC_ERC_SOCK_ADDR_NOT_AVAIL;
		case EAFNOSUPPORT:
			return YXC_ERC_SOCK_AF_NO_SUPPORT;
		case EALREADY:
			return YXC_ERC_SOCK_ALREADY;
		case EBADMSG:
			return YXC_ERC_SOCK_BAD_MSG;
		case ECANCELED:
			return YXC_ERC_SOCK_CANCELED;
		case ECONNABORTED:
			return YXC_ERC_SOCK_CONN_ABORTED;
		case ECONNREFUSED:
			return YXC_ERC_SOCK_CONN_REFUSED;
		case ECONNRESET:
			return YXC_ERC_SOCK_CONN_RESET;
		case EDESTADDRREQ:
			return YXC_ERC_SOCK_DEST_ADDR_REQ;
		case EHOSTUNREACH:
			return YXC_ERC_SOCK_HOST_UNREACH;
		case EIDRM:
			return YXC_ERC_SOCK_IDRM;
		case EINPROGRESS:
			return YXC_ERC_SOCK_WOULD_BLOCK;
		case EISCONN:
			return YXC_ERC_SOCK_IS_CONN;
		case ELOOP:
			return YXC_ERC_SOCK_LOOP;
		case EMSGSIZE:
			return YXC_ERC_SOCK_MSG_SIZE;
		case ENETDOWN:
			return YXC_ERC_SOCK_NETWORK_DOWN;
		case ENETRESET:
			return YXC_ERC_SOCK_NETWORK_RESET;
		case ENETUNREACH:
			return YXC_ERC_SOCK_NETWORK_UNREACH;
		case ENOBUFS:
			return YXC_ERC_SOCK_NO_BUFS;
		case ENODATA:
			return YXC_ERC_SOCK_NO_DATA;
		case ENOLINK:
			return YXC_ERC_SOCK_NO_LINK;
		case ENOMSG:
			return YXC_ERC_SOCK_NO_MSG;
		case ENOSR:
			return YXC_ERC_SOCK_NO_PROTO_OPT;
		case ENOSTR:
			return YXC_ERC_SOCK_NO_SR;
		case ENOPROTOOPT:
			return YXC_ERC_SOCK_NO_STR;
		case ENOTCONN:
			return YXC_ERC_SOCK_NOT_CONN;
		case ENOTRECOVERABLE:
			return YXC_ERC_SOCK_NOT_RECOVERABLE;
		case ENOTSOCK:
			return YXC_ERC_SOCK_NOT_SOCK;
		case ENOTSUP:
			return YXC_ERC_SOCK_NOT_SUP;
#if YXC_PLATFORM_APPLE
		case EOPNOTSUPP:
			return YXC_ERC_SOCK_OP_NOT_SUPP;
#endif /* YXC_PLATFORM_APPLE */
		case EOVERFLOW:
			return YXC_ERC_SOCK_OVERFLOW;
		case EOWNERDEAD:
			return YXC_ERC_SOCK_OWNER_DEAD;
		case EPROTO:
			return YXC_ERC_SOCK_PROTO;
		case EPROTOTYPE:
			return YXC_ERC_SOCK_PROTO_TYPE;
		case EPROTONOSUPPORT:
			return YXC_ERC_SOCK_PROTO_NO_SUPPORT;
		case ETIME:
			return YXC_ERC_SOCK_TIME;
		case ETIMEDOUT:
			return YXC_ERC_SOCK_TIMEDOUT;
		case ETXTBSY:
			return YXC_ERC_SOCK_TEXT_BSY;
		case EWOULDBLOCK:
			return YXC_ERC_SOCK_WOULD_BLOCK;
		default:
			return YXC_ERC_SUCCESS;
		}
	}
#else
#error Only support windows and unix platforms.
#endif /* YXC_PLATFORM_WIN */
}

namespace YXC_Inner
{
	ybool_t gs_bDoSrvLeakHook = TRUE;

	void _closesocket(socket_t socket)
	{
#if YXC_PLATFORM_WIN
		closesocket(socket);
#else
		close(socket);
#endif /* YXC_PLATFORM_WIN */
	}

	YXC_Status _getsocketerror()
	{
#if YXC_PLATFORM_WIN
		int i = ::WSAGetLastError();
#elif YXC_PLATFORM_UNIX
		int i = errno;
#else
#error Not support socket platform
#endif /* YXC_PLATFORM_WIN */

		YXC_Status retCode = _switchToNetError(i);
		return retCode;
	}

	int _SetSocketBlock(socket_t socket, ybool_t bBlock)
	{
		u_long iBlockMode = bBlock ? 0 : 1;
#if YXC_PLATFORM_WIN
		return ioctlsocket(socket, FIONBIO, &iBlockMode);
#else
        return ioctl(socket, FIONBIO, &iBlockMode);
#endif /* YXC_PLATFORM_WIN */
	}

	int _SetSocketTCPAlive(socket_t socket, const YXC_TCPKeepAliveProperty* pProperty)
	{
#if YXC_PLATFORM_WIN
		tcp_keepalive keepAlive;
		keepAlive.onoff = pProperty->bIsEnabled;
		keepAlive.keepalivetime = pProperty->uKeepAliveTime;
		keepAlive.keepaliveinterval = pProperty->uKeepAliveInterval;

		DWORD dwNumBytesReturned;
		int i = WSAIoctl(socket, SIO_KEEPALIVE_VALS, &keepAlive, sizeof(tcp_keepalive), NULL, 0, &dwNumBytesReturned, NULL, NULL);
		return i;
#elif YXC_PLATFORM_APPLE
        int keep_alive = 1;
        int keepInterval = 15;

        int i = setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(int));
        if (i != 0) return i;

        int i2 = setsockopt(socket, IPPROTO_TCP, TCP_KEEPALIVE, (void *)&keepInterval, sizeof(int));
        if (i2 != 0) return i2;

        return 0;
#endif /* YXC_PLATFORM_WIN */
	}

	YXC_Status _CreateSocket(YXC_SocketType sockType, ybool_t bReuseAddr, socket_t* pSock)
	{
		socket_t sock = 0;
		switch (sockType)
		{
		case YXC_SOCKET_TYPE_TCP:
			sock = ::socket(AF_INET, SOCK_STREAM, 0);
			_YXC_NET_CHECK_SOCK_RET(sock != -1, YC("Create TCP socket failed"));
			break;
		case YXC_SOCKET_TYPE_UDP:
			sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			_YXC_NET_CHECK_SOCK_RET(sock != -1, YC("Create UDP socket failed"));
			break;
		default:
			_YXC_REPORT_ERR(YXC_ERC_INVALID_PARAMETER, YC("Invalid socket type"));
			return YXC_ERC_INVALID_PARAMETER;
		}

		if (bReuseAddr)
		{
			int iAddrReuse = (int)bReuseAddr;
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&iAddrReuse, sizeof(int));
		}

		*pSock = sock;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _CheckRecvCode(_Socket* pSock, int nrecv)
	{
		if (nrecv < 0)
		{
			YXC_Status iErrCode = _getsocketerror();
			if (iErrCode != YXC_ERC_SOCK_WOULD_BLOCK)
			{
				_YXC_SOCK_REPORT(iErrCode, YC("Receive from socket 0x%x failed."), pSock);
			}
			return iErrCode;
		}
		else if (nrecv == 0)
		{
			_YXC_SOCK_REPORT(YXC_ERC_NET_SHUTDOWN, YC("Socket 0x%x is shutdown by another side."), socket);
			return YXC_ERC_NET_SHUTDOWN;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _CheckNetHeader(_Socket* pSock, const YXC_NetHeader* pHeader)
	{
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SockSendData(_Socket* pSock, ysize_t stNumBytes, const ybyte_t* pData, ysize_t* pstSent)
	{
		ysize_t stTotal = stNumBytes, stSent = 0;
		*pstSent = 0;

		while (stSent < stTotal)
		{
			ysize_t stSend = std::min<ysize_t>(stTotal - stSent, pSock->uPacketSize);
			const char* pNextSend = (const char*)(pData + stSent);
			ssize_t iSend = send(pSock->rSock, pNextSend, (int)stSend, 0);

			if (iSend == -1)
			{
				YXC_Status rc = _getsocketerror();
                _YXC_NET_CHECK_SOCK_RET(rc == YXC_ERC_SOCK_WOULD_BLOCK, YC("Failed to send data, sock = 0x%x"), pSock);
			}

			*pstSent += iSend;
			stSent += iSend;
		}

		return YXC_ERC_SUCCESS;
	}

	void _YXC_SockClose(YXC_Socket socket)
	{
		_Socket* pSock = _SockPtr(socket);

#if YXC_PLATFORM_WIN
		if (pSock->bResolveLeakIssue)
		{
			YXC_NetRemoveFromHookList(pSock->rSock);
		}
#endif /* YXC_PLATFORM_WIN */

		if (pSock->rSock != -1)
		{
			_closesocket(pSock->rSock);
		}

		free(pSock);
	}
}

using namespace YXC_Inner;

extern "C"
{
    YXC_Status YXC_SockEventCreate(YXC_Socket socket, YXC_SocketEvent* pEvent)
    {
		_YCHK_MAL_R1(se, _SocketEvent);
		se->rSock = -1;

		YXC_Status rcRet;
		_Socket* pSock = _SockPtr(socket);
#if YXC_PLATFORM_UNIX
		if (pSock != NULL)
		{
			se->rSock = pSock->rSock;
		}

        int ret = pipe(se->nPipe);
        _YXC_CHECK_OS_GOTO(ret >= 0, YC("Failed to create pipe for sock event"));

        se->xWriEvent = FALSE;
#elif YXC_PLATFORM_WIN
        WSAEVENT eve = WSACreateEvent();
        _YXC_CHECK_OS_GOTO(eve != NULL, YC("Failed to create wsa event"));

        if (pSock != NULL)
        {
			WSAEventSelect(pSock->rSock, eve, FD_READ | FD_CLOSE);
			se->rSock = pSock->rSock;
        }

		se->wsaEvent = eve;
#else
#error Platform.INV
#endif /* YXC_PLATFORM_UNIX */

        *pEvent = (YXC_SocketEvent)se;
        return YXC_ERC_SUCCESS;
    err_ret:
        free(se);
        return rcRet;
    }

    void YXC_SockEventDestroy(YXC_SocketEvent event)
    {
        _SocketEvent* se = (_SocketEvent*)event;
#if YXC_PLATFORM_WIN
        ::CloseHandle(se->wsaEvent);
#else
        close(se->nPipe[0]);
        close(se->nPipe[1]);
#endif /* YXC_PLATFORM_WIN */

        free(se);
    }

    void YXC_SockEventSelect(YXC_Socket sock, YXC_SocketEvent event, long lEvent)
    {
        _SocketEvent* se = (_SocketEvent*)event;
#if YXC_PLATFORM_WIN
        _Socket* pSock = _SockPtr(sock);
        WSAEventSelect(pSock->rSock, se->wsaEvent, lEvent);
#else
        se->xWriEvent = (lEvent & FD_WRITE) != 0;
#endif /* YXC_PLATFORM_WIN */
    }

    YXC_Status YXC_SockEventSet(YXC_SocketEvent event)
    {
        _SocketEvent* se = (_SocketEvent*)event;

#if YXC_PLATFORM_UNIX
        int nDummy = 0;
        size_t ret = write(se->nPipe[1], &nDummy, sizeof(int));
        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to do pipe write"));
#else
        BOOL bRet = WSASetEvent(se->wsaEvent);
        _YXC_CHECK_OS_RET(bRet, YC("Failed to set wsa event"));
#endif /* YXC_PLATFORM_WIN */

        return YXC_ERC_SUCCESS;
    }

    yint32_t YXC_SockEventWait(yuint32_t uNumEvents, YXC_SocketEvent* pEvents, yuint32_t umsTimeout, YXC_SocketEventArr* pWaitArr)
    {
#if YXC_PLATFORM_WIN
        for (yuint32_t i = 0; i < uNumEvents; ++i)
        {
            _SocketEvent* se = (_SocketEvent*)pEvents[i];
            pWaitArr->eveArr[i] = se->wsaEvent;
        }
        pWaitArr->dwNumEvents = uNumEvents;
        DWORD dwRet = WSAWaitForMultipleEvents(uNumEvents, pWaitArr->eveArr, FALSE, umsTimeout, FALSE);
		if (dwRet == WAIT_FAILED || dwRet == WAIT_TIMEOUT)
		{
			return -1;
		}

		return dwRet - WAIT_OBJECT_0;
#elif YXC_PLATFORM_UNIX
        FD_ZERO(&pWaitArr->rdSet);
        FD_ZERO(&pWaitArr->wrSet);
        FD_ZERO(&pWaitArr->erSet);

        int fd_max = -1;

        for (int i = 0; i < uNumEvents; ++i)
        {
            _SocketEvent* se = (_SocketEvent*)pEvents[i];

            if (se->rSock != -1)
            {
                FD_SET(se->rSock, &pWaitArr->rdSet);
                FD_SET(se->rSock, &pWaitArr->erSet);
                if (se->xWriEvent)
                {
                    FD_SET(se->rSock, &pWaitArr->wrSet);
                }
                if (se->rSock > fd_max)
                {
                    fd_max = se->rSock;
                }
            }

            FD_SET(se->nPipe[0], &pWaitArr->rdSet);
            if (se->nPipe[0] > fd_max)
            {
                fd_max = se->nPipe[0];
            }
        }

        pWaitArr->fdMax = fd_max + 1;

        struct timeval tv;
        tv.tv_sec = umsTimeout / 1000;
        tv.tv_usec = umsTimeout / 1000 * 1000;

        int iSelect = select(pWaitArr->fdMax, &pWaitArr->rdSet, &pWaitArr->wrSet, &pWaitArr->erSet, &tv);
        return iSelect;
#endif /* YXC_PLATFORM_WIN */
    }

    yint32_t YXC_SockEventGetResult(yuint32_t uNumEvents, YXC_SocketEvent* events, yint32_t index, YXC_SocketEventArr* pWaitArr, yuint32_t* pSignaledState)
    {
#if YXC_PLATFORM_UNIX
        for (yuint32_t i = 0; i < uNumEvents; ++i)
        {
            _SocketEvent* se = (_SocketEvent*)events[i];

            if (se->rSock != -1)
            {
                if (FD_ISSET(se->rSock, &pWaitArr->rdSet))
                {
                    *pSignaledState = FD_READ;
                    return i;
                }

                if (se->xWriEvent)
                {
                    if (FD_ISSET(se->rSock, &pWaitArr->wrSet))
                    {
                        *pSignaledState = FD_WRITE;
                        return i;
                    }
                }

                if (FD_ISSET(se->rSock, &pWaitArr->erSet))
                {
                    *pSignaledState = FD_CLOSE;
                    return i;
                }
            }

            if (FD_ISSET(se->nPipe[0], &pWaitArr->rdSet))
            {
                int dummy;
                read(se->nPipe[0], &dummy, sizeof(int));
                *pSignaledState = 0;
                return i;
            }
        }
#else
		_SocketEvent* se = (_SocketEvent*)events[index];
		if (se->rSock == -1)
		{
			*pSignaledState = 0;
			return index;
		}

		WSANETWORKEVENTS eventsArrived;
		yuint32_t iEvent = ::WSAEnumNetworkEvents(se->rSock, se->wsaEvent, &eventsArrived);
		if (iEvent != SOCKET_ERROR)
		{
			//WSAResetEvent(se->wsaEvent);
			*pSignaledState = eventsArrived.lNetworkEvents;
			return index;
		}
#endif /* YXC_PLATFORM_UNIX */

        return -1;
    }
}

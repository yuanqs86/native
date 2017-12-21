/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_SOCKET_H__
#define __INC_YXC_SYS_BASE_NET_SOCKET_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_NetMarshal.h>

#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#else
#if YXC_PLATFORM_APPLE
#include <arpa/inet.h>
#endif /* YXC_PLATFORM_APPLE */
#include <sys/socket.h>
#endif /* YXC_PLATFORM_WIN */

#define YXC_SOCKET_MAX_DESC 20

#if YCHAR_WCHAR_T
#define YXC_SockConnectToServer YXC_SockConnectToServerW
#define YXC_SockConnectToServerEx YXC_SockConnectToServerExW
#else
#define YXC_SockConnectToServer YXC_SockConnectToServerA
#define YXC_SockConnectToServerEx YXC_SockConnectToServerExA
#endif /* YCHAR_WCHAR_T */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#if YXC_PLATFORM_WIN
	typedef SOCKET socket_t;
#else
	typedef int socket_t;
#endif /* YXC_PLATFORM_WIN */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_Socket);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_SocketEvent);

	typedef enum
	{
		YXC_SOCKET_TYPE_UDP,
		YXC_SOCKET_TYPE_TCP,
	}YXC_SocketType;

	typedef enum
	{
		YXC_SOCKET_OPT_UNKNOWN,
		YXC_SOCKET_OPT_RECV_BUF_SIZE,
		YXC_SOCKET_OPT_SEND_BUF_SIZE,
		YXC_SOCKET_OPT_MAX_PACKAGE_SIZE,
		YXC_SOCKET_OPT_PACKET_SIZE,
		YXC_SOCKET_OPT_RECV_TIMEOUT,
		YXC_SOCKET_OPT_SEND_TIMEOUT,
		YXC_SOCKET_OPT_TCP_KEEPALIVE,
		YXC_SOCKET_OPT_P2P_BIND_DATA,
		YXC_SOCKET_OPT_WOULD_BLOCK,
		YXC_SOCKET_OPT_IP_END_POINT,
		YXC_SOCKET_OPT_NET_PROTOCOL,
		YXC_SOCKET_OPT_SOCKET_VALUE,
	}YXC_SocketOption;

	typedef struct __YXC_TCP_KEEPALIVE_PROPERTY
	{
		ybool_t bIsEnabled;
		ybyte_t byMaxRetries;
		yuint32_t uKeepAliveTime;
		yuint32_t uKeepAliveInterval;
	}YXC_TCPKeepAliveProperty;

	typedef union _YXC_SOCKET_OPT_VALUE
	{
		yuint32_t uRecvBufSize;
		yuint32_t uSendBufSize;
		yuint32_t uRecvTimeout;
		yuint32_t uSendTimeout;
		yuint32_t uMaxPackageSize;
		yuint32_t uPacketSize;
		YXC_TCPKeepAliveProperty aliveProperty;
		sockaddr_in sockAddr;
		socket_t rSock;
		ybool_t bWouldBlock;
		void* pP2PBindData;
		YXC_NetProtocol sockProtocol;
	}YXC_SocketOptValue;

#if YXC_PLATFORM_WIN
	YXC_API(YXC_Status) YXC_SockWSAStart(WORD wVersion, WSADATA* pData); // need to be called by windows

	YXC_API(void) YXC_SockWSACleanup(); // need to be called by windows
#endif /* YXC_PLATFORM_WIN */

	YXC_API(YXC_Status) YXC_SockConnectToServerA(YXC_SocketType sockType, const char* cpszIpAddr, yuint32_t uPort, yuint32_t uConnTime, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_SockConnectToServerW(YXC_SocketType sockType, const wchar_t* cpszIpAddr, yuint32_t uPort, yuint32_t uConnTime, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_SockConnectToServerExA(YXC_SocketType sockType, const char* cpszIpAddr, yuint32_t uPort,
		yuint32_t uConnTime, yuint32_t uLocalPort, ybool_t bReuseAddr, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_SockConnectToServerExW(YXC_SocketType sockType, const wchar_t* cpszIpAddr, yuint32_t uPort,
		yuint32_t uConnTime, yuint32_t uLocalPort, ybool_t bResueAddr, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_SockCreateServer(YXC_SocketType sockType, yuint32_t uPort, YXC_Socket* pServerSock);

	YXC_API(YXC_Status) YXC_SockCreateServerEx(YXC_SocketType sockType, yuint32_t uPort, ybool_t bReuseAddr, YXC_Socket* pServerSock);

	YXC_API(YXC_Status) YXC_SockAcceptClient(YXC_Socket serverSock, YXC_Socket* pClientSock);

	YXC_API(void) YXC_SockClose(YXC_Socket sock);

	YXC_API(YXC_Status) YXC_SockSetOption(YXC_Socket socket, YXC_SocketOption option, const YXC_SocketOptValue* pOptionValue);

	YXC_API(YXC_Status) YXC_SockGetOption(YXC_Socket socket, YXC_SocketOption option, YXC_SocketOptValue* pOptionValue);

	YXC_API(YXC_Status) YXC_SockSendData(YXC_Socket socket, ysize_t stNumBytes, const ybyte_t* pData, ysize_t* pstSent);

	YXC_API(YXC_Status) YXC_SockReceiveData(YXC_Socket socket, ysize_t stNumBytesInBuffer, ybyte_t* pBuffer, ybool_t bPeekData, ysize_t* pstReceived);

	YXC_API(YXC_Status) YXC_SockSendPackage(YXC_Socket socket, const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

	YXC_API(YXC_Status) YXC_SockReceivePackage(YXC_Socket socket, ysize_t stNumBytesInBuffer, ybyte_t* pBuffer, ysize_t* pNeeded, YXC_NetPackage* pPackage,
		YXC_NetTransferInfo* pTransferInfo);

	YXC_API(socket_t) YXC_SockGetValue(YXC_Socket socket);

	YXC_API(void) YXC_SockInitTransferInfo(YXC_Socket socket, const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo);

#if YXC_PLATFORM_WIN
    typedef struct __YXC_SOCKET_EVENT_ARRAY
    {
        DWORD dwNumEvents;
        WSAEVENT eveArr[64];
    }YXC_SocketEventArr;
#elif YXC_PLATFORM_UNIX
    typedef struct __SOCKETEVENTARR
    {
        int fdMax;
        fd_set rdSet;
        fd_set wrSet;
        fd_set erSet;
    }YXC_SocketEventArr;
#endif /* YXC_PLATFORM_WIN */

    YXC_API(YXC_Status) YXC_SockEventCreate(YXC_Socket socket, YXC_SocketEvent* pEvent);

    YXC_API(void) YXC_SockEventDestroy(YXC_SocketEvent event);

    YXC_API(void) YXC_SockEventSelect(YXC_Socket sock, YXC_SocketEvent event, long lEvent);

    YXC_API(yint32_t) YXC_SockEventWait(yuint32_t uNumEvents, YXC_SocketEvent* events, yuint32_t umsTimeout, YXC_SocketEventArr* pWaitArr);

    YXC_API(yint32_t) YXC_SockEventGetResult(yuint32_t uNumEvents, YXC_SocketEvent* events, yint32_t index, YXC_SocketEventArr* pWaitArr, yuint32_t* pSignaledState);

    YXC_API(YXC_Status) YXC_SockEventSet(YXC_SocketEvent event);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __INC_YXC_SYS_BASE_NET_SOCKET_H__ */

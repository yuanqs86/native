#ifndef __INNER_INC_YXC_SYS_BASE_NET_COMMON_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_COMMON_HPP__

#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>

#define _YXC_SOCK_BAKLOG 10
#define _YXC_SEND_TIMEOUT 0
#define _YXC_RECV_TIMEOUT 0
#define _YXC_NET_TCP_PACKET_SIZE (1 << 19)
#define _YXC_NET_UDP_PACKET_SIZE (1 << 10)
#define _YXC_NET_MAX_PACKAGE_SIZE (1 << 27) // packet max to 128M

#if YXC_PLATFORM_UNIX
#include <sys/socket.h>
#include <sys/poll.h>
#endif /* YXC_PLATFORM_UNIX */

#if YXC_PLATFORM_APPLE
#include <arpa/inet.h>
#endif /* YXC_PLATFORM_APPLE */

#if !YXC_PLATFORM_WIN
#define FD_READ 0x1
#define FD_WRITE 0x2
#define FD_CLOSE 0x4
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
	extern ybool_t gs_bIsLittleEndianCPU;

	typedef struct __SOCKET
	{
		socket_t rSock;
		YXC_SocketType sockType;
		yuint32_t uMaxPackageSize;
		yuint32_t uPacketSize;

		yuint32_t uRecvBufSize;
		yuint32_t uSendBufSize;
		yuint32_t uRecvTimeout;
		yuint32_t uSendTimeout;

		sockaddr_in inAddr;

		ybool_t bWouldBlock;
		ybool_t bResolveLeakIssue;
		void* pP2PBindData;

		YXC_NetProtocol protocol;

		YXC_TCPKeepAliveProperty aliveProperty;
	}_Socket;

#if YXC_PLATFORM_WIN
    typedef struct __SOCKETEVENT
    {
        WSAEVENT wsaEvent;
		socket_t rSock;
    }_SocketEvent;
#else
    typedef struct __SOCKETEVENT
    {
        socket_t rSock;
        int nPipe[2];
        int xWriEvent;
    }_SocketEvent;
#endif /* YXC_PLATFORM_WIN */

	void _closesocket(socket_t socket);

	YXC_Status _getsocketerror();

	YXC_Status _CreateSocket(YXC_SocketType sockType, ybool_t bReuseAddr, socket_t* pSock);

	YXC_Status _SetSocketTCPAlive(socket_t socket, const YXC_TCPKeepAliveProperty* pProperty);

	YXC_Status _SetSocketBlock(socket_t socket, ybool_t bBlock);

	YXC_Status _CheckRecvCode(_Socket* pSock, int nrecv);

	YXC_Status _SockSendData(_Socket* pSock, ysize_t stNumBytes, const ybyte_t* pData, ysize_t* pstSent);

	inline void _InitTransferInfo(_Socket* pSock, const YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransferInfo)
	{
		YXC_NPStdInitTransferInfo(&pSock->protocol, pPackage, pTransferInfo);
	}

	ybool_t _IsLittleEndianCPU();

	extern ybool_t gs_bDoSrvLeakHook;

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_Socket, _Socket, _SockPtr, _SockHdl);

	void _YXC_SockClose(YXC_Socket socket);

    void _NetCloseSendHook();
}

#if YXC_PLATFORM_WIN
#define _YXC_SOCK_REPORT(rc, wszMsg, ...)                                                                                    \
	do {                                                                                                                    \
		YXC_Status code = (rc);                                                                                              \
		if (code == YXC_ERC_SOCK_WSA) {                                                                                      \
			_YXC_REPORT_NEW_ERR_EX(YXC_ERROR_SRC_OS, YXC_ERR_CAT_OS_WSA, ::WSAGetLastError(), YC("Detect winsock error"));		\
			_YXC_REPORT_ERR(code, wszMsg, ##__VA_ARGS__);                                                                      \
		} else {                                                                                                            \
			_YXC_REPORT_NEW_ERR(code, wszMsg, ##__VA_ARGS__);                                                                  \
		}                                                                                                                   \
	} while (0)

#define _YXC_NET_CHECK_SOCK_RET(cond, wszMsg, ...)					\
	do {															\
		if (!(cond)) {												\
			YXC_Status rc = YXC_Inner::_getsocketerror();				\
			_YXC_SOCK_REPORT(rc, wszMsg, ##__VA_ARGS__);				\
			return rc;												\
		}															\
	} while (0)

#define _YXC_NET_CHECK_SOCK_GOTO(cond, wszMsg, ...)					\
	do {															\
		if (!(cond)) {												\
			YXC_Status rc = YXC_Inner::_getsocketerror();				\
			_YXC_SOCK_REPORT(rc, wszMsg, ##__VA_ARGS__);				\
			__VARIABLE_RC__ = rc; goto __LABEL__;					\
		}															\
	} while (0)

#define _YXC_NET_CHECK_RET(cond, code, wszMsg, ...) _YXC_CHECK_REPORT_NEW_RET(cond, code, wszMsg, ##__VA_ARGS__)
#define _YXC_NET_CHECK_GOTO(cond, code, wszMsg, ...) _YXC_CHECK_REPORT_NEW_GOTO(cond, code, wszMsg, ##__VA_ARGS__)

#else

#define _YXC_SOCK_REPORT(rc, wszMsg, args...) _YXC_REPORT_NEW_ERR(rc, wszMsg, ##args)
#define _YXC_NET_CHECK_SOCK_RET(cond, wszMsg, args...)               \
    do {															\
        if (!(cond)) {												\
            YXC_Status rc = YXC_Inner::_getsocketerror();				\
            _YXC_SOCK_REPORT(rc, wszMsg, ##args);                    \
            return rc;												\
        }															\
    } while (0)

#define _YXC_NET_CHECK_SOCK_GOTO(cond, wszMsg, args...)              \
    do {															\
        if (!(cond)) {												\
            YXC_Status rc = YXC_Inner::_getsocketerror();				\
            _YXC_SOCK_REPORT(rc, wszMsg, ##args);                    \
            __VARIABLE_RC__ = rc; goto __LABEL__;					\
        }															\
    } while (0)

#define _YXC_NET_CHECK_RET(cond, code, wszMsg, args...) _YXC_CHECK_REPORT_NEW_RET(cond, code, wszMsg, ##args)
#define _YXC_NET_CHECK_GOTO(cond, code, wszMsg, args...) _YXC_CHECK_REPORT_NEW_GOTO(cond, code, wszMsg, ##args)

#endif /* YXC_PLATFORM_WIN */

#endif /* __INNER_INC_YXC_SYS_BASE_NET_COMMON_HPP__ */

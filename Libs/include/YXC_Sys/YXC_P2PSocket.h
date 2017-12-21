/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_P2P_SOCKET_H__
#define __INC_YXC_SYS_BASE_NET_P2P_SOCKET_H__

#include <YXC_Sys/YXC_NetSocket.h>

#define YXC_P2P_PT_REGISTER 0
#define YXC_P2P_PT_REGISTER_ACK 1
#define YXC_P2P_PT_HEART 2
#define YXC_P2P_PT_CONNECT_PEER 3
#define YXC_P2P_PT_RES_NAT_ADDR 4
#define YXC_P2P_PT_RES_NAT_NOT_FOUND 5
#define YXC_P2P_PT_MAKE_HOLE_REQ 6
#define YXC_P2P_PT_MAKE_HOLE_RES 7

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#pragma pack(push, 1)
	typedef struct __YXC_P2P_PT_TRANSFER_HEADER
	{
		ybyte_t byCtrlCmd;

		yuint32_t uVirtualIP;
		yuint32_t uVirtualPort;

		yuint32_t uNATIP;
		yuint32_t uNATPort;
	}YXC_P2PHeader;
#pragma pack(pop)

	YXC_API(YXC_Status) YXC_P2PCreateServerA(YXC_SocketType sockType, const char* cpszVirtualIP, yuint32_t uPort, const char* cpszP2PSrv,
		yuint32_t uP2PSrvPort, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_P2PCreateServerW(YXC_SocketType sockType, const wchar_t* cpszVirtualIP, yuint32_t uPort, const wchar_t* cpszP2PSrv,
		yuint32_t uP2PSrvPort, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_P2PAcceptServer(YXC_Socket sockSrv, YXC_Socket* pSockDst, yuint32_t stmsTimeout);

	YXC_API(void) YXC_P2PCloseServer(YXC_Socket socket);

	YXC_API(YXC_Status) YXC_P2PConnectToServerA(YXC_SocketType sockType, const char* cpszRemotePeer, yuint32_t uPort,
		const char* cpszP2PSrv, yuint32_t uP2PSrvPort, yuint32_t uTimeout, YXC_Socket* pSock);

	YXC_API(YXC_Status) YXC_P2PConnectToServerW(YXC_SocketType sockType, const wchar_t* cpszRemotePeer, yuint32_t uPort,
		const wchar_t* cpszP2PSrv, yuint32_t uP2PSrvPort, yuint32_t uTimeout, YXC_Socket* pSock);

	YXC_API(void) YXC_P2PCloseClient(YXC_Socket socket);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_NET_P2P_SOCKET_H__ */

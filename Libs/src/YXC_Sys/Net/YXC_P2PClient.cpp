#define __MODULE__ "EK.Net.P2P"

#include <stdio.h>
#include <errno.h>
#include <algorithm>
#include <time.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_P2PSocket.h>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#include <YXC_Sys/YXC_NetHook.h>

using namespace YXC_Inner;
namespace
{
	struct _P2PClientData
	{
		//euint32_t uVirtualAddr;
		//euint32_t uRemotePort;

		YXC_Socket listenSock;
		YXC_Socket clientSock;

		yuint32_t uNATAddr;
		yuint32_t uNATPort;
		yuint32_t uTimeout;

		yuint32_t uLocalPort;
	};

	void _WaitHole(_P2PClientData* pClientData)
	{
		YXC_Socket listenSock = pClientData->listenSock, cliSock;

		YXC_SocketOptValue optVal;
		YXC_Status rc = YXC_SockGetOption(listenSock, YXC_SOCKET_OPT_SOCKET_VALUE, &optVal);
		socket_t rSock = optVal.rSock;
		socklen_t iFdMax = (socklen_t)rSock + 1;

		fd_set fdReadSet;
		FD_ZERO(&fdReadSet);
		FD_SET(rSock, &fdReadSet);

		clock_t time1 = clock();

		while (TRUE)
		{
			clock_t time2 = clock();
			double delta = (double)(time2 - time1) / CLOCKS_PER_SEC * 1000;

			if (delta > pClientData->uTimeout) break;

			timeval tm;
			tm.tv_sec = 1;
			tm.tv_usec = 0;
			int iSelect = select(iFdMax, &fdReadSet, NULL, NULL, &tm);

			if (iSelect == -1)
			{
				return;
			}
			else if (iSelect == 0)
			{
				FD_SET(rSock, &fdReadSet);
			}
			else
			{
				rc = YXC_SockAcceptClient(listenSock, &cliSock);

				YXC_SocketOptValue optVal2;
				rc = YXC_SockGetOption(cliSock, YXC_SOCKET_OPT_IP_END_POINT, &optVal2);
				sockaddr_in sockAddr = optVal2.sockAddr;

				char ackData[50];
				ysize_t stReceived;

				optVal2.uRecvTimeout = 1000;
				rc = YXC_SockSetOption(cliSock, YXC_SOCKET_OPT_RECV_TIMEOUT, &optVal2);

				rc = YXC_SockReceiveData(cliSock, 50, (ybyte_t*)ackData, FALSE, &stReceived);

//#if 1
//				DWORD dwSleep = 500;
//				HKEY hKey;
//				LSTATUS rc2 = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\YXCLib\\VPN", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL);
//
//				if (rc2 == ERROR_SUCCESS)
//				{
//					DWORD dataMaxLen = 50;
//					ybyte_t buffer[50];
//					DWORD dwType = REG_DWORD;
//					rc2 = RegQueryValueExW(hKey, L"VPNSleep", NULL, &dwType, buffer, &dataMaxLen);
//					if (rc2 == ERROR_SUCCESS)
//					{
//						dwSleep = *(DWORD*)buffer;
//					}
//				}
//#else
//				DWORD dwSleep = 500;
//#endif /* 1 */
				//
				//YXC_SockClose(cliSock);
				//Sleep(dwSleep);

				if (sockAddr.sin_addr.s_addr == pClientData->uNATAddr && rc == YXC_ERC_SUCCESS && stReceived == 50)
				{
					pClientData->uNATPort = ntohs(sockAddr.sin_port); // == pClientData->uNATPort
					pClientData->clientSock = cliSock;
					return;
				}

				YXC_SockClose(cliSock);
				//in_addr addr_nat;
				//addr_nat.S_un.S_addr = pClientData->uNATAddr;
				//char* szNATIP = inet_ntoa(addr_nat);
				//rc = YXC_SockConnectToServerExW(YXC_SOCKET_TYPE_TCP, szNATIP, pClientData->uNATPort,
				//	2000, uLocalPort, TRUE, &pClientData->clientSock);

				//pClientData->clientSock = cliSock;
			}
		}
	}

	yuint32_t __stdcall _P2PClientProc(void* pParam)
	{
		_WaitHole((_P2PClientData*)pParam);
		return 0;
	}
}

extern "C"
{
	YXC_Status YXC_P2PConnectToServerA(YXC_SocketType sockType, const char* cpszRemotePeer, yuint32_t uRemotePort,
		const char* cpszP2PSrv, yuint32_t uP2PSrvPort, yuint32_t uTimeout, YXC_Socket* pSock)
	{
		_YXC_CHECK_REPORT_NEW_RET(sockType == YXC_SOCKET_TYPE_TCP, YXC_ERC_NOT_SUPPORTED, YC("Only support TCP P2P socket"));

		YXC_Status rcRet;

		_P2PClientData* pSockData = NULL;
		YXC_SocketOptValue optVal;
        yuint32_t uLocalPort;
		YXC_P2PHeader p2pHeader;
		YXC_NetPackage package;
		YXC_NetTransferInfo transInfo;
        ethread_t uHdl;

		YXC_Socket cliSock = NULL, srvSock = NULL;
		YXC_Status rc = YXC_SockConnectToServerExA(sockType, cpszP2PSrv, uP2PSrvPort, uTimeout, 0, TRUE, &cliSock);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, YXC_ERC_P2P_CONNECT_SERVER_FAILED, YC("Connect to VPN server %S:%d failed"), cpszP2PSrv, uP2PSrvPort);

		optVal.uRecvTimeout = 3000;
		rc = YXC_SockSetOption(cliSock, YXC_SOCKET_OPT_RECV_TIMEOUT, &optVal);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Set socket recv timeout failed"));

		rc = YXC_SockGetOption(cliSock, YXC_SOCKET_OPT_IP_END_POINT, &optVal);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Get socket addr info failed"));

        uLocalPort = ntohs(optVal.sockAddr.sin_port);

		p2pHeader.byCtrlCmd = YXC_P2P_PT_CONNECT_PEER;
		p2pHeader.uVirtualIP = inet_addr(cpszRemotePeer);
		p2pHeader.uVirtualPort = uRemotePort;

		YXC_NPCreatePackage(FALSE, YXC_NP_PROTOCOL_TYPE_P2P, sizeof(p2pHeader), &p2pHeader, &package);
		YXC_SockInitTransferInfo(cliSock, &package, &transInfo);

		rc = YXC_SockSendPackage(cliSock, &package, &transInfo);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Send register to VPN server failed"));

		ysize_t stNeeded;
		YXC_SockInitTransferInfo(cliSock, NULL, &transInfo);
		rc = YXC_SockReceivePackage(cliSock, sizeof(YXC_P2PHeader), (ybyte_t*)&p2pHeader, &stNeeded, &package, &transInfo);

		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Receive register ack from VPN server failed"));
		_YXC_CHECK_REPORT_NEW_GOTO(package.uDataLen == sizeof(YXC_P2PHeader), YXC_ERC_NET_INVALID_PROTOCOL,
			YC("Receive register ack from VPN server failed"));

		_YXC_CHECK_REPORT_NEW_GOTO(p2pHeader.byCtrlCmd == YXC_P2P_PT_RES_NAT_ADDR, YXC_ERC_SOCK_NETWORK_UNREACH, YC("Can't find NAT addr for virtual addr %S:%d"),
			cpszRemotePeer, uRemotePort);

		rc = YXC_SockCreateServerEx(sockType, uLocalPort, TRUE, &srvSock);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create hole socket failed"));

		pSockData = (_P2PClientData*)malloc(sizeof(_P2PClientData));
		_YXC_CHECK_REPORT_NEW_GOTO(pSockData != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for socket data failed"));
		pSockData->uLocalPort = uLocalPort;
		pSockData->uNATAddr = p2pHeader.uNATIP;
		pSockData->uNATPort = p2pHeader.uNATPort;
		pSockData->uTimeout = uTimeout;
		pSockData->listenSock = srvSock;
		pSockData->clientSock = NULL;

		uHdl = YXCLib::OSCreateThread(_P2PClientProc, pSockData, NULL);
		_YXC_CHECK_OS_GOTO(uHdl != 0, YC("Create thread for P2P server thread failed"));
		YXCLib::OSWaitThread(uHdl);
		YXCLib::OSCloseThreadHandle(uHdl);

		_YXC_CHECK_REPORT_NEW_GOTO(pSockData->clientSock != NULL, YXC_ERC_SOCK_TIMEDOUT, YC("Can't accept for p2p data."));

		optVal.pP2PBindData = pSockData;
		YXC_SockSetOption(pSockData->clientSock, YXC_SOCKET_OPT_P2P_BIND_DATA, &optVal);

		YXC_SockClose(srvSock);
		*pSock = pSockData->clientSock;
		return YXC_ERC_SUCCESS;

		//in_addr addr_nat;
		//addr_nat.S_un.S_addr = pSockData->uNATAddr;
		//char* szNATIP = inet_ntoa(addr_nat);
		//rcRet = YXC_SockConnectToServerExA(sockType, szNATIP, pSockData->uNATPort, uTimeout, uLocalPort, TRUE, pSock);

		//YXC_SockClose(pSockData->clientSock);
err_ret:
		if (cliSock != NULL)
		{
			YXC_SockClose(cliSock);
		}

		if (srvSock != NULL)
		{
			YXC_SockClose(srvSock);
		}

		if (pSockData != NULL)
		{
			free(pSockData);
		}
		return rcRet;
	}

	YXC_Status YXC_P2PConnectToServerW(YXC_SocketType sockType, const wchar_t* cpszRemotePeer, yuint32_t uPort,
		const wchar_t* cpszP2PSrv, yuint32_t uP2PSrvPort, yuint32_t uTimeout, YXC_Socket* pSock)
	{
		char szP2PSrv[32], szVirtualIP[32];
		sprintf(szP2PSrv, "%S", cpszP2PSrv);
		sprintf(szVirtualIP, "%S", cpszRemotePeer);

		return YXC_P2PConnectToServerA(sockType, szVirtualIP, uPort, szP2PSrv, uP2PSrvPort, uTimeout, pSock);
	}

	void YXC_P2PCloseClient(YXC_Socket socket)
	{
		YXC_SocketOptValue optVal;
		YXC_SockGetOption(socket, YXC_SOCKET_OPT_P2P_BIND_DATA, &optVal);

		_P2PClientData* pData = (_P2PClientData*)optVal.pP2PBindData;
		free(pData);

		YXC_SockClose(socket);
	}
};

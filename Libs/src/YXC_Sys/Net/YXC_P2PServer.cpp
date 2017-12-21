#define __MODULE__ "EK.Net.P2P"

#include <stdio.h>
#include <errno.h>
#include <algorithm>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_P2PSocket.h>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#include <YXC_Sys/YXC_NetHook.h>

using namespace YXC_Inner;
namespace
{
	struct _P2PSockData
	{
		YXC_Socket clientSock;
		char szServerAddr[32];
		char szVirtualIp[32];
		yuint32_t uServerPort;
		yuint32_t uLocalPort;
		ybool_t bRun;
		ethread_t upThread;

		ysize_t cbAcceptQueue;
		ysize_t cbAcceptBuf;
		ysize_t cbCurIndex;
		YXC_Socket* pAcceptedSocks;
		YXC_Sem sem;

		YXX_Crit crit;
	};

	static ybool_t _PopSockQueue(_P2PSockData* pData, YXC_Socket* pDstSock, yuint32_t stmsTimeout)
	{
		YXC_Status rc = YXC_WaitSingleKObjectTimeout(pData->sem, stmsTimeout);
		if (rc != YXC_ERC_SUCCESS) return FALSE;

		YXX_CritLocker locker(pData->crit);
		ysize_t cbQueue = pData->cbAcceptQueue;
		if (cbQueue == 0) return FALSE;

		ysize_t uCbIndex = pData->cbCurIndex;
		*pDstSock = pData->pAcceptedSocks[uCbIndex];
		if (++pData->cbCurIndex == pData->cbAcceptBuf)
		{
			pData->cbCurIndex = 0;
		}
		--pData->cbAcceptQueue;

		return TRUE;
	}

	ybool_t _PushSockQueue(_P2PSockData* pData, YXC_Socket newSock)
	{
		YXX_CritLocker locker(pData->crit);
		ysize_t cbQueue = pData->cbAcceptQueue;
		if (pData->cbAcceptQueue == pData->cbAcceptBuf)
		{
			ysize_t cbNew = YXCLib::TMax<ysize_t>((cbQueue + 1) * 2, 16);
			YXC_Socket* pNewArr = (YXC_Socket*)malloc(cbNew * sizeof(YXC_Socket));
			if (pNewArr != NULL)
			{
				for (ysize_t i = 0; i < cbQueue; ++i)
				{
					ysize_t cIndex = i + pData->cbCurIndex;
					if (cIndex >= pData->cbAcceptBuf) cIndex -= pData->cbAcceptBuf;
					pNewArr[i] = pData->pAcceptedSocks[cIndex];
				}

				pNewArr[cbQueue] = newSock;

				if (pData->pAcceptedSocks) free(pData->pAcceptedSocks);
				pData->pAcceptedSocks = pNewArr;
				pData->cbAcceptBuf = cbNew;
				pData->cbAcceptQueue = cbQueue + 1;
				pData->cbCurIndex = 0;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			ysize_t cbCurIndex = pData->cbCurIndex + pData->cbAcceptQueue;
			if (cbCurIndex >= pData->cbAcceptBuf) cbCurIndex -= pData->cbAcceptBuf;

			pData->pAcceptedSocks[cbCurIndex] = newSock;
			++pData->cbAcceptQueue;
		}
		YXC_SemUnlock(pData->sem);
		return TRUE;
	}

	YXC_Status _InitP2PAttr(YXC_Socket p2pSocket, const char* cpszVirtualIP, yuint32_t uLocalPort)
	{
		YXC_SocketOptValue optVal;
		optVal.uRecvTimeout = 3000;
		YXC_Status rc = YXC_SockSetOption(p2pSocket, YXC_SOCKET_OPT_RECV_TIMEOUT, &optVal);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Set recv timeout for P2P client socket failed"));

		optVal.aliveProperty.bIsEnabled = TRUE;
		optVal.aliveProperty.byMaxRetries = 3;
		optVal.aliveProperty.uKeepAliveInterval = 2000;
		optVal.aliveProperty.uKeepAliveTime = 5000;
		YXC_SockSetOption(p2pSocket, YXC_SOCKET_OPT_TCP_KEEPALIVE, &optVal);

		optVal.uSendTimeout = 1;
		rc = YXC_SockSetOption(p2pSocket, YXC_SOCKET_OPT_SEND_TIMEOUT, &optVal);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Set send timeout for P2P client socket failed"));

		YXC_P2PHeader p2pHeader;
		p2pHeader.byCtrlCmd = YXC_P2P_PT_REGISTER;
		p2pHeader.uVirtualIP = inet_addr(cpszVirtualIP);
		p2pHeader.uVirtualPort = uLocalPort;

		YXC_NetPackage package;
		YXC_NetTransferInfo transInfo;
		YXC_NPCreatePackage(FALSE, YXC_NP_PROTOCOL_TYPE_P2P, sizeof(p2pHeader), &p2pHeader, &package);
		YXC_SockInitTransferInfo(p2pSocket, &package, &transInfo);

		rc = YXC_SockSendPackage(p2pSocket, &package, &transInfo);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Send register to VPN server failed"));

		ysize_t stNeeded;
		YXC_SockInitTransferInfo(p2pSocket, NULL, &transInfo);
		rc = YXC_SockReceivePackage(p2pSocket, sizeof(YXC_P2PHeader), (ybyte_t*)&p2pHeader, &stNeeded, &package, &transInfo);

		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Receive register ack from VPN server failed"));
		_YXC_CHECK_REPORT_NEW_RET(package.uDataLen == sizeof(YXC_P2PHeader) && p2pHeader.byCtrlCmd == YXC_P2P_PT_REGISTER_ACK,
			YXC_ERC_NET_INVALID_PROTOCOL, YC("Receive invalid ack from VPN server"));

		optVal.uRecvTimeout = 1000;
		rc = YXC_SockSetOption(p2pSocket, YXC_SOCKET_OPT_RECV_TIMEOUT, &optVal);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Set recv timeout for P2P client socket failed"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _ReconnectP2P(_P2PSockData* pSockData, YXC_Socket* pReturnedSock)
	{
		YXC_SockClose(pSockData->clientSock);
		pSockData->clientSock = NULL;
		while (pSockData->bRun)
		{
			YXC_Status rc = YXC_SockConnectToServerExA(YXC_SOCKET_TYPE_TCP, pSockData->szServerAddr,
				pSockData->uServerPort, 3000, pSockData->uLocalPort, TRUE, &pSockData->clientSock);

			if (rc == YXC_ERC_SUCCESS)
			{
				rc = _InitP2PAttr(pSockData->clientSock, pSockData->szVirtualIp, pSockData->uLocalPort);

				if (rc == YXC_ERC_SUCCESS)
				{
					return rc;
				}
				else
				{
					YXC_SockClose(pSockData->clientSock);
				}
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _MakeHole(const char* cpszVirtualIP, yuint32_t uLocalPort, yuint32_t uRemoteIP, yuint32_t uRemotePort,
		YXC_Socket* pRetSock)
	{
		YXC_Socket socket;

		in_addr inAddr;
		inAddr.s_addr = uRemoteIP;

		char* pszIp = inet_ntoa(inAddr);

		printf("Make hole with %d\n", uLocalPort);
		YXC_Status rc = YXC_SockConnectToServerExA(YXC_SOCKET_TYPE_TCP, pszIp, uRemotePort, 2000, uLocalPort, TRUE, &socket);

		if (rc == YXC_ERC_SUCCESS)
		{
			char ackData[50];
			ysize_t stNumSent;
			rc = YXC_SockSendData(socket, 50, (ybyte_t*)ackData, &stNumSent);
			if (rc == YXC_ERC_SUCCESS)
			{
				*pRetSock = socket;
			}
			else
			{
				YXC_SockClose(socket);
			}
		}

		return rc;
	}

	yuint32_t __stdcall _P2PServerProc(void* pParam)
	{
		_P2PSockData* pSockData = (_P2PSockData*)pParam;

		YXC_Socket cliESock = pSockData->clientSock;

		YXC_NetPackage package;
		YXC_NetTransferInfo transInfo;
		YXC_P2PHeader p2pHeader;

		int iTimeoutCount = 0;
		const int DO_HEART_IN_TIMEOUT_COUNT = 5;

		while (pSockData->bRun)
		{
			cliESock = pSockData->clientSock;
			// YXC_Socket cl
			ysize_t stNeeded;
			YXC_SockInitTransferInfo(cliESock, NULL, &transInfo);
			YXC_Status rc = YXC_SockReceivePackage(cliESock, sizeof(YXC_P2PHeader), (ybyte_t*)&p2pHeader, &stNeeded, &package, &transInfo);

            if (rc == YXC_ERC_SUCCESS)
			{
				if (package.uDataLen != sizeof(YXC_P2PHeader)) // invalid protocol ?
				{
					_ReconnectP2P(pSockData, &cliESock);
				}

				if (p2pHeader.byCtrlCmd == YXC_P2P_PT_MAKE_HOLE_REQ)
				{
					YXC_Socket retSock;
					YXC_Status rc = _MakeHole(pSockData->szVirtualIp, pSockData->uLocalPort, p2pHeader.uNATIP,
						p2pHeader.uNATPort, &retSock);

					if (rc == YXC_ERC_SUCCESS)
					{
						YXC_P2PHeader mhRes = { YXC_P2P_PT_MAKE_HOLE_RES };
						mhRes.uVirtualIP = inet_addr(pSockData->szVirtualIp);
						mhRes.uVirtualPort = pSockData->uLocalPort;

						YXC_NetPackage mhPackage;
						YXC_NetTransferInfo mhTransinfo;
						YXC_NPCreatePackage(FALSE, YXC_NP_PROTOCOL_TYPE_P2P, sizeof(YXC_P2PHeader), &mhRes, &mhPackage);
						YXC_SockInitTransferInfo(cliESock, &mhPackage, &mhTransinfo);

						YXC_SockSendPackage(cliESock, &mhPackage, &mhTransinfo);

						_PushSockQueue(pSockData, retSock);
					}
				}

				iTimeoutCount = 0;
			}
			else if (rc == YXC_ERC_SOCK_TIMEDOUT)
			{
				++iTimeoutCount;

				if (iTimeoutCount >= DO_HEART_IN_TIMEOUT_COUNT)
				{
					YXC_P2PHeader heart = { YXC_P2P_PT_HEART };
					heart.uVirtualIP = inet_addr(pSockData->szVirtualIp);
					heart.uVirtualPort = pSockData->uLocalPort;
					YXC_NetPackage heartPackage;
					YXC_NetTransferInfo heartTransInfo;
					YXC_NPCreatePackage(FALSE, YXC_NP_PROTOCOL_TYPE_P2P, sizeof(YXC_P2PHeader), &heart, &heartPackage);
					YXC_SockInitTransferInfo(cliESock, &heartPackage, &heartTransInfo);
					YXC_SockSendPackage(cliESock, &heartPackage, &heartTransInfo);
					iTimeoutCount = 0;
				}
			}
			else if (rc == YXC_ERC_BUFFER_NOT_ENOUGH) // todo : deal with this
			{
				break;
			}
            else
            {
				_ReconnectP2P(pSockData, &cliESock);
				iTimeoutCount = 0;
            }
		}

		if (pSockData->clientSock != NULL)
		{
			YXC_SockClose(pSockData->clientSock);
		}
		free(pSockData);
		return 0;
	}
}

extern "C"
{
	YXC_Status YXC_P2PCreateServerA(YXC_SocketType sockType, const char* cpszVirtualIP, yuint32_t uPort, const char* cpszP2PSrv,
		yuint32_t uP2PSrvPort, YXC_Socket* pSock)
	{
		_YXC_CHECK_REPORT_NEW_RET(sockType == YXC_SOCKET_TYPE_TCP, YXC_ERC_NOT_SUPPORTED, YC("Only support TCP P2P socket"));

		YXC_Status rcRet;

		_P2PSockData* pSockData = NULL;

		YXC_Socket cliSock = NULL, srvSock = NULL;
		yuint32_t uTimeout = 2000;
        ethread_t uHdl;
		YXC_SocketOptValue optVal;

		YXC_Status rc = YXC_SockConnectToServerExA(sockType, cpszP2PSrv, uP2PSrvPort, uTimeout, uPort, TRUE, &cliSock);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, YXC_ERC_P2P_CONNECT_SERVER_FAILED, YC("Connect to VPN server(%S:%d) failed"), cpszP2PSrv, uP2PSrvPort);

		rc = _InitP2PAttr(cliSock, cpszVirtualIP, uPort);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Init P2P sock attribute failed"));

		pSockData = (_P2PSockData*)malloc(sizeof(_P2PSockData));
		_YXC_CHECK_REPORT_NEW_GOTO(pSockData != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for socket data failed"));

		memset(pSockData, 0, sizeof(_P2PSockData));

		pSockData->bRun = TRUE;
		pSockData->clientSock = cliSock;
		strcpy(pSockData->szServerAddr, cpszP2PSrv);
		strcpy(pSockData->szVirtualIp, cpszVirtualIP);
		pSockData->uServerPort = uP2PSrvPort;
		pSockData->uLocalPort = uPort;
		pSockData->cbAcceptBuf = 0;
		pSockData->cbCurIndex = 0;
		pSockData->cbAcceptQueue = 0;
		pSockData->pAcceptedSocks = NULL;
		pSockData->crit.Init(4000);
		YXC_SemCreate(0, &pSockData->sem);

		rc = YXC_SockCreateServerEx(sockType, uPort, TRUE, &srvSock);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create P2P sock server failed"));

        uHdl = YXCLib::OSCreateThread(_P2PServerProc, pSockData, NULL);
		_YXC_CHECK_OS_GOTO(uHdl != 0, YC("Create thread for P2P server thread failed"));

		optVal.pP2PBindData = pSockData;
		YXC_SockSetOption(srvSock, YXC_SOCKET_OPT_P2P_BIND_DATA, &optVal);

		*pSock = srvSock;

		return YXC_ERC_SUCCESS;
err_ret:
		if (cliSock != NULL)
		{
			YXC_SockClose(cliSock);
		}

		if (pSockData != NULL)
		{
			free(pSockData);
		}

		return rcRet;
	}

	YXC_Status YXC_P2PCreateServerW(YXC_SocketType sockType, const wchar_t* cpszVirtualIP, yuint32_t uPort,
		const wchar_t* cpszP2PSrv, yuint32_t uP2PSrvPort, YXC_Socket* pSock)
	{
		char szP2PSrv[32], szVirtualIP[32];
		sprintf(szP2PSrv, "%S", cpszP2PSrv);
		sprintf(szVirtualIP, "%S", cpszVirtualIP);

		return YXC_P2PCreateServerA(sockType, szVirtualIP, uPort, szP2PSrv, uP2PSrvPort, pSock);
	}

	YXC_Status YXC_P2PAcceptServer(YXC_Socket sockSrv, YXC_Socket* pSockDst, yuint32_t stmsTimeout)
	{
		YXC_SocketOptValue optVal;
		YXC_SockGetOption(sockSrv, YXC_SOCKET_OPT_P2P_BIND_DATA, &optVal);

		_P2PSockData* pData = (_P2PSockData*)optVal.pP2PBindData;

		ybool_t bRet = _PopSockQueue(pData, pSockDst, stmsTimeout);
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_SOCK_TIMEDOUT, YC("Can't pop queue for client in %d seconds"), stmsTimeout);

		return YXC_ERC_SUCCESS;
	}

	void YXC_P2PCloseServer(YXC_Socket socket)
	{
		YXC_SocketOptValue optVal;
		YXC_SockGetOption(socket, YXC_SOCKET_OPT_P2P_BIND_DATA, &optVal);

		_P2PSockData* pData = (_P2PSockData*)optVal.pP2PBindData;
		pData->bRun = FALSE;

		YXCLib::OSWaitThread(pData->upThread);
		YXCLib::OSCloseThreadHandle(pData->upThread);

		for (ysize_t i = 0; i < pData->cbAcceptQueue; ++i)
		{
			YXC_Socket sock = pData->pAcceptedSocks[i + pData->cbCurIndex];
			YXC_SockClose(sock);
		}
		if (pData->pAcceptedSocks)
		{
			free(pData->pAcceptedSocks);
		}
		free(pData);

		_YXC_SockClose(socket);
	}
};

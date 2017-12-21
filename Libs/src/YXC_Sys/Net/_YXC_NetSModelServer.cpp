#define __MODULE__ "EK.Net.SModel.Server"

#if YXC_PLATFORM_WIN32
#include <process.h>
#endif /* YXC_PLATFORM_WIN32 */
#include <assert.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

#include <YXC_Sys/Net/_YXC_NetSModelServer.hpp>
#include <YXC_Sys/YXC_NetHook.h>
#include <YXC_Sys/YXC_P2PSocket.h>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>
// #include <YXC_NetModel/_YXC_NetClientProxy.hpp>

namespace YXC_Inner
{
	YXC_Status _NetSModelServer::Create(const YXC_NetServerCreateArgs* pCreateArgs, YXC_NetServerOnInitClientAttr pfnOnInitClient,
		YXC_NetServerOnDestroyClientAttr pfnOnDestroyClient, void* pExtData)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;
		ThreadStatus thrInit = THREAD_STATUS_PAUSED;

		this->_pfnOnInitAttr = pfnOnInitClient;
		this->_pfnOnDestroyAttr = pfnOnDestroyClient;
		this->_pExtData = pExtData;
		this->_byServerProtocol = pCreateArgs->byServerProtocol;
		this->_csServer.Init(4000);
		this->_ctrlVar = NULL;
		this->_supervisor = NULL;
		this->_upThread = 0;

		new (&this->_mapGroups) std::map<yuint32_t, _NetSModelServerGroup*>();

		this->_serverSock = pCreateArgs->serverSock;

		YXC_SocketOptValue opt;
		YXC_Status rc = YXC_SockGetOption(pCreateArgs->serverSock, YXC_SOCKET_OPT_SOCKET_VALUE, &opt);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to get socket handle"));
		this->_rSock = opt.rSock;

		if (pCreateArgs->bAutoStart) thrInit = THREAD_STATUS_RUNNING;
		rc = YXC_CondVarCreate(1, sizeof(ThreadStatus), &thrInit, &this->_ctrlVar);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to create control condition variable"));

		rc = YXC_CondVarCreateSupervisor(this->_ctrlVar, CVThreadMatchFunc, NULL, &this->_supervisor);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to create controllor supervisor"));

		this->_upThread = OSCreateThread(_NetSModelServerThreadProc, this, NULL);
		_YXC_CHECK_OS_GOTO(this->_upThread != 0, YC("Create thread for _NetSModelServer failed"));

		return YXC_ERC_SUCCESS;
err_ret:
		this->_csServer.Close();
		if (this->_ctrlVar) YXC_CondVarForceDestroy(this->_ctrlVar);
		return rcRet;
	}

	_NetSModelServerGroup* _NetSModelServer::_TryRefGroup(yuint32_t uGroupId)
	{
		YXX_CritLocker locker(this->_csServer);
		GROUP_ITER gi = this->_mapGroups.find(uGroupId);
		_YXC_CHECK_REPORT_NEW_RET2(gi != this->_mapGroups.end(), YXC_ERC_KEY_NOT_FOUND, NULL, YC("Not a valid group to broadcast"));

		_NetSModelServerGroup* pGroup = gi->second;
		pGroup->Reference();
		return pGroup;
	}

	YXC_Status _NetSModelServer::Broadcast(yuint32_t uGroupId, const YXC_NetServerRawData* pRawData)
	{
		if (uGroupId == YXC_NET_MODEL_BROADCAST_TO_ALL)
		{
			YXX_CritLocker locker(this->_csServer);
			YXC_Status rcRet = YXC_ERC_SUCCESS;
			for (GROUP_ITER gi = this->_mapGroups.begin(); gi != this->_mapGroups.end(); ++gi)
			{
				YXC_Status rc = gi->second->Broadcast(pRawData);
				if (rc != YXC_ERC_SUCCESS) rcRet = rc;
			}

			return rcRet;
		}
		else
		{
			_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
			_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

			YXC_Status rc = pGroup->Broadcast(pRawData);
			pGroup->Unreference();
			return rc;
		}
	}

	YXC_Status _NetSModelServer::CreateGroup(const YXC_NetSrvGroupCreateArgs* pGroupArgs,
		YXC_NetServerOnClientConnected pfnOnConnected, YXC_NetServerOnClientClose pfnOnClose,
		YXC_NetServerOnClientRecv pfnOnRecv, void* pExtData)
	{
		YXX_CritLocker locker(this->_csServer);
		GROUP_ITER gi = this->_mapGroups.find(pGroupArgs->uGroupId);
		_YXC_CHECK_REPORT_NEW_RET(gi == this->_mapGroups.end(), YXC_ERC_INVALID_PARAMETER, YC("A group with id %d already exists"), pGroupArgs->uGroupId);

		_NetSModelServerGroup* pGroup = (_NetSModelServerGroup*)malloc(sizeof(_NetSModelServerGroup));
		_YXC_CHECK_REPORT_NEW_RET(pGroup != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for group pointer failed"));

		YXC_Status rc = pGroup->Create(pGroupArgs, this->_byServerProtocol, pfnOnConnected, pfnOnClose, pfnOnRecv, pExtData, this);
		if (rc != YXC_ERC_SUCCESS)
		{
			free(pGroup);
			return rc;
		}

		try
		{
			this->_mapGroups.insert(std::make_pair(pGroupArgs->uGroupId, pGroup));
		}
		catch (...)
		{
			_YXC_REPORT_NEW_ERR(YXC_ERC_OUT_OF_MEMORY, YC("Insert group to map failed"));
			free(pGroup);
			return YXC_ERC_OUT_OF_MEMORY;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelServer::RefGroup(yuint32_t uGroupId, _NetSModelServerGroup** ppGroup)
	{
		_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
		_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

		*ppGroup = pGroup;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelServer::GetNumClients(yuint32_t uGroupId, yuint32_t* puNumClients)
	{
		*puNumClients = 0;
		if (uGroupId == YXC_NET_MODEL_BROADCAST_TO_ALL)
		{
			YXX_CritLocker locker(this->_csServer);
			for (GROUP_ITER gi = this->_mapGroups.begin(); gi != this->_mapGroups.end(); ++gi)
			{
				*puNumClients += gi->second->GetNumClients();
			}
		}
		else
		{
			_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
			_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

			*puNumClients = pGroup->GetNumClients();
			pGroup->Unreference();
		}

		return YXC_ERC_SUCCESS;
	}

	void _NetSModelServer::ValidateClient(YXC_Socket socket)
	{
		this->_HandleClientRequest(socket);
	}

	YXC_Status _NetSModelServer::GetClientSockOpt(yuint32_t uGroupId, YXC_NetSModelClient client, YXC_SocketOption opt, YXC_SocketOptValue* pOptVal)
	{
		_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
		_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

		YXC_Status rc = pGroup->GetClientSockOpt(client, opt, pOptVal);
		pGroup->Unreference();
		return rc;
	}

	YXC_Status _NetSModelServer::SetClientSockOpt(yuint32_t uGroupId, YXC_NetSModelClient client, YXC_SocketOption opt, const YXC_SocketOptValue* pOptVal)
	{
		_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
		_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

		YXC_Status rc = pGroup->SetClientSockOpt(client, opt, pOptVal);
		pGroup->Unreference();
		return rc;
	}

	YXC_Status _NetSModelServer::SendPackageToClient(yuint32_t uGroupId, YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage, yuint32_t stTimeoutInMilliseconds)
	{
		_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
		_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

		YXC_Status rc = pGroup->SendPackageToClient(client, pPackage, stTimeoutInMilliseconds);
		pGroup->Unreference();
		return rc;
	}

	YXC_Status _NetSModelServer::CloseClient(yuint32_t uGroupId, YXC_NetSModelClient client, YXC_Event* pWaitHandle)
	{
		_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
		_YXC_CHECK_RET(pGroup != NULL, YXC_ERC_INVALID_PARAMETER);

		YXC_Status rc = pGroup->CloseClient(client, pWaitHandle);
		pGroup->Unreference();
		return rc;
	}

	YXC_Status _NetSModelServer::_SetSocketAttr(YXC_Socket socket, socket_t rSock)
	{
		YXC_SocketOptValue optVal;
		optVal.bWouldBlock = FALSE;
		YXC_Status rc = YXC_SockSetOption(socket, YXC_SOCKET_OPT_WOULD_BLOCK, &optVal);

		if (rc != YXC_ERC_SUCCESS)
		{
			_YXC_KREPORT_THREAD_INFO(YC("Set block options for client failed, close client, hdl(%d)."), rSock);
			return rc;
		}

		optVal.aliveProperty.bIsEnabled = TRUE;
		optVal.aliveProperty.byMaxRetries = 3;
		optVal.aliveProperty.uKeepAliveInterval = 2000;
		optVal.aliveProperty.uKeepAliveTime = 5000;

		rc = YXC_SockSetOption(socket, YXC_SOCKET_OPT_TCP_KEEPALIVE, &optVal);

		if (rc != YXC_ERC_SUCCESS)
		{
			_YXC_KREPORT_THREAD_INFO(YC("Set keep alive options for client failed, close client, hdl(%d)."), rSock);
			return rc;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelServer::_CreateClientAndAddToGroup(YXC_Socket socket, yuint32_t stMaxSendBlocks, ysize_t stSendBufferSize, ysize_t stRecvBufferSize,
		yuint32_t stMaxNumBlocksOnQueue, yuint32_t uGroupId, void*& pClientExt)
	{
		socket_t rSock = YXC_SockGetValue(socket);
		_NetSModelClient* pClient = _NetSModelClient::CreateClient(socket);
		ybool_t bClientCreated = FALSE;
		if (pClient == NULL)
		{
			_YXC_KREPORT_INFO(YC("Alloc for client ptr failed, hdl(0x%d)."), rSock);
			return YXC_ERC_OUT_OF_MEMORY;
		}

		YXC_Status rc = YXC_ERC_UNKNOWN;

		_NetSModelServerGroup* pGroup = this->_TryRefGroup(uGroupId);
		if (pGroup == NULL)
		{
			_YXC_KREPORT_THREAD_INFO(YC("Find group for client failed, close client, handle(%d), group id(%d)."), rSock, uGroupId);
			rc = YXC_ERC_KEY_NOT_FOUND;
			goto func_ret;
		}
		else
		{
			YXC_NetSModelClientCbkArgs callbackArgs = { pGroup->GetCloseCallback(), pGroup->GetRecvCallback(),
				pGroup->GetNoDataCallback(), pGroup->GetDestroyCallback(),
				0, pClientExt
			};

			if (stMaxNumBlocksOnQueue == 0) stMaxNumBlocksOnQueue = pGroup->GetClientMaxNumBlocks();
			if (stRecvBufferSize == 0) stRecvBufferSize = pGroup->GetRecvBufferSize();
			if (stMaxSendBlocks == 0) stMaxSendBlocks = pGroup->GetMaxSendBlockCount();
			if (stSendBufferSize == 0) stSendBufferSize = pGroup->GetSendBufferSize();

			YXC_PNCMP memPool = pGroup->GetMemPool();
			if (memPool != NULL)
			{
				rc = pClient->Create(&callbackArgs, memPool, stMaxNumBlocksOnQueue, stRecvBufferSize,
					stMaxSendBlocks, stSendBufferSize);
			}
			else
			{
				rc = pClient->Create(&callbackArgs, stMaxSendBlocks, stSendBufferSize, stRecvBufferSize);
			}
			if (rc != YXC_ERC_SUCCESS)
			{
				_YXC_KREPORT_THREAD_INFO(YC("Create memory for client failed, close client, hdl(%d)."), rSock);
				goto func_ret;
			}
			bClientCreated = TRUE;
			pClient->Reference(); /* Add a reference, promise valid ptr. */
			pClient->SetMgr(pGroup);
			pClient->SetMgrExt(pGroup);
			pClient->SetConnectCallback(pGroup->GetConnectCallback());

			yuintptr_t uHandleOut;
			rc = pGroup->AddClient(pClient, &uHandleOut);
			if (rc != YXC_ERC_SUCCESS)
			{
				_YXC_KREPORT_THREAD_INFO(YC("Add client to group failed, close client, hdl(%d)."), rSock);
				pClient->Unreference();
				goto func_ret;
			}

			pClient->SetBlockCheck(pGroup->GetBlockCheckFunc()); // Set it after init other client informations.
			pClient->Unreference();

			_YXC_KREPORT_THREAD_VERBOSE(YC("Add client to service list successfully, hdl(%d), ptr(0x%x)."), rSock, pClient);
			pGroup->Unreference();
		}

		return YXC_ERC_SUCCESS;
func_ret:
		if (pGroup) pGroup->Unreference();
		_NetSModelClient::DestroyClient(pClient, FALSE);
		if (bClientCreated)
		{
			pClientExt = NULL;
		}
		return rc;
	}

	void _NetSModelServer::_HandleClientRequest(YXC_Socket socket)
	{
		socket_t rSock = YXC_SockGetValue(socket);

		ysize_t stSendBufferSize = 0, stRecvBufferSize = 0;
		yuint32_t stMaxSendBlocks = 0, stMaxNumBlocksOnQueue = 0;
		yuint32_t uGroupId = 0;
		void* pClientExt = NULL;

		if (this->_pfnOnInitAttr != NULL)
		{
			ybool_t bInitAttr = this->_pfnOnInitAttr(_SModelHdl(this), socket, &stMaxSendBlocks, &stSendBufferSize, &stRecvBufferSize,
				&stMaxNumBlocksOnQueue, this->_pExtData, &uGroupId, &pClientExt);
			if (!bInitAttr)
			{
				_YXC_KREPORT_THREAD_INFO(YC("Validate client failed, close client, handle(%d)."), rSock);
				YXC_SockClose(socket);
				return;
			}

			_YXC_KREPORT_THREAD_VERBOSE(YC("Validate client successfully, handle(%d)."), rSock);
		}

		YXC_Status rc = this->_SetSocketAttr(socket, rSock);
		if (rc == YXC_ERC_SUCCESS)
		{
			rc = this->_CreateClientAndAddToGroup(socket, stMaxSendBlocks, stSendBufferSize,
				stRecvBufferSize, stMaxNumBlocksOnQueue, uGroupId, pClientExt);
		}

		if (rc != YXC_ERC_SUCCESS)
		{
			// don't close socket, will be closed inner if error
			if (pClientExt != NULL && this->_pfnOnDestroyAttr != NULL)
			{
				this->_pfnOnDestroyAttr(_SModelHdl(this), uGroupId, socket, this->_pExtData, pClientExt);
			}
			YXC_SockClose(socket);
		}
	}

	void _NetSModelServer::Close(ybool_t bWaitForClosed)
	{
        ThreadStatus s = THREAD_STATUS_STOPPED;

		ethread_t upThread = this->_upThread;
		YXC_Status rc = YXC_CondVarWake(this->_ctrlVar, CVThreadChangeStatusFunc, &s);
		if (rc != YXC_ERC_SUCCESS)
		{
			_YXC_KREPORT_ERR(YC("Failed to set thread stop status"));
		}

		// this pointer maybe unavailable here!!
		if (bWaitForClosed)
		{
			OSWaitThread(upThread);
		}

		OSCloseThreadHandle(upThread);
	}

	YXC_Status _NetSModelServer::Start()
	{
        ThreadStatus s = THREAD_STATUS_RUNNING;

		YXC_Status rc = YXC_CondVarWake(this->_ctrlVar, CVThreadChangeStatusFunc, &s);
		_YXC_CHECK_STATUS_RET(rc, YC("Failed to set thread start status"));

		return YXC_ERC_SUCCESS;
	}

	void _NetSModelServer::CloseGroup(yuint32_t uGroupId)
	{
		if (uGroupId == YXC_NET_MODEL_BROADCAST_TO_ALL)
		{
			this->_csServer.Lock();
			std::map<yuint32_t, _NetSModelServerGroup*> copiedGroups = this->_mapGroups;
			for (GROUP_ITER gi = this->_mapGroups.begin(); gi != this->_mapGroups.end(); ++gi)
			{
				gi->second->Reference();
			}
			this->_mapGroups.clear();
			this->_csServer.Unlock();

			for (GROUP_ITER gi = copiedGroups.begin(); gi != copiedGroups.end(); ++gi)
			{
				gi->second->Unreference();
				gi->second->CloseAndWait();
			}
		}
		else
		{
			this->_csServer.Lock();

			GROUP_ITER gi = this->_mapGroups.find(uGroupId);
			if (gi == this->_mapGroups.end())
			{
				this->_csServer.Unlock();
				return;
			}

			_NetSModelServerGroup* pGroup = gi->second;
			this->_mapGroups.erase(gi);
			this->_csServer.Unlock();

			pGroup->CloseAndWait();
		}
	}

	void _NetSModelServer::Destroy()
	{
		this->_csServer.Close();
	}

	struct __NetClientInfo
	{
		YXC_Socket socket;
		_NetSModelServer* pModel;
	};

	unsigned int __stdcall _NetSModelServer::_NetSModelServerClientAcceptProc(void* pArgs)
	{
		__NetClientInfo* pClientInfo = (__NetClientInfo*)pArgs;

		YXC_Socket socket = pClientInfo->socket;
		_NetSModelServer* pModel = pClientInfo->pModel;

		pModel->ValidateClient(socket);

		free(pClientInfo);
		return 0;
	}

	unsigned int __stdcall _NetSModelServer::_NetSModelServerThreadProc(void* pArgs)
	{
		_NetSModelServer* pModel = (_NetSModelServer*)pArgs;

		YXC_Socket serverSock = pModel->_serverSock;
		socket_t listenSock = pModel->_rSock;
		fd_set fdReadSet;
		FD_ZERO(&fdReadSet);
		timeval tm = { 0, 500 * 1000  }; /* 500 ms */

		YXC_SocketOptValue opt;
		YXC_SockGetOption(serverSock, YXC_SOCKET_OPT_P2P_BIND_DATA, &opt);

		socklen_t iFdMax = (socklen_t)listenSock + 1;

		while (TRUE)
		{
			ThreadStatus thrStatus;
			YXC_Status rc = YXC_CondVarSupervisorWait(pModel->_ctrlVar, pModel->_supervisor, YXC_INFINITE, &thrStatus);
			if (rc != YXC_ERC_SUCCESS)
			{
				_YXC_KREPORT_ERR(YC("Failed to wait condition variable."));
				break;
			}
			if (thrStatus == THREAD_STATUS_STOPPED) break;

			YXC_Socket retSock = NULL;
			if (opt.pP2PBindData == NULL)
			{
				int iSelect = select(iFdMax, &fdReadSet, NULL, NULL, &tm);

				if (iSelect == -1)
				{
					FD_SET(listenSock, &fdReadSet);
					_YXC_KREPORT_THREAD_ERR(YC("Select socket failed, iFdMax = %d"), iFdMax);
					continue;
				}
				else if (iSelect == 0)
				{
					FD_SET(listenSock, &fdReadSet);
					continue;
				}
				else
				{
					for (yuint32_t i = 0; i < 1; ++i)
					{
						if (FD_ISSET(listenSock, &fdReadSet))
						{
							YXC_Socket clientSock;
							rc = YXC_SockAcceptClient(serverSock, &clientSock);

							if (rc == YXC_ERC_SUCCESS)
							{
								retSock = clientSock;
								break;
							}
						}
					}
				}
			}
			else
			{
				YXC_Status rc = YXC_P2PAcceptServer(serverSock, &retSock, 500);
				if (rc != YXC_ERC_SUCCESS)
				{
					continue;
				}
			}

			_YXC_KREPORT_INFO(YC("Accept new client, handle(%d)."), YXC_SockGetValue(retSock));
			__NetClientInfo* pInfo = (__NetClientInfo*)malloc(sizeof(__NetClientInfo));
			pInfo->pModel = pModel;
			pInfo->socket = retSock;

			if (pInfo != NULL)
			{
				ethread_t up = OSCreateThread(_NetSModelServerClientAcceptProc, pInfo, NULL);
				if (up == 0)
				{
					_YXC_KREPORT_ERR(YC("Can't create thread to accept client, handle(%d)."), YXC_SockGetValue(retSock));
					pModel->ValidateClient(retSock);
					free(pInfo);
				}
				OSCloseThreadHandle(up);
			}
		}

		pModel->CloseGroup(YXC_NET_MODEL_BROADCAST_TO_ALL);
		pModel->Destroy();
		free(pModel);

		return 0;
	}
}

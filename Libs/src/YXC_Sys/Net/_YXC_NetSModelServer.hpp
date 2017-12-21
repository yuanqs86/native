#ifndef __INNER_INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_HPP__

#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_StlAlloc.hpp>

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/Net/_YXC_NetSModelClientMgr.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelServerGroup.hpp>

#include <YXC_Sys/YXC_CondVar.h>

#include <map>

namespace YXC_Inner
{
	class _NetSModelServer
	{
	public:
		YXC_Status Create(const YXC_NetServerCreateArgs* pCreateArgs, YXC_NetServerOnInitClientAttr pfnOnInitClient,
			YXC_NetServerOnDestroyClientAttr pfnOnDestroyClient, void* pExtData);

		YXC_Status CreateGroup(const YXC_NetSrvGroupCreateArgs* pGroupArgs, YXC_NetServerOnClientConnected pfnOnConn,
			YXC_NetServerOnClientClose pfnOnClose, YXC_NetServerOnClientRecv pfnOnRecv, void* pExtData);

		YXC_Status RefGroup(yuint32_t uGroupId, _NetSModelServerGroup** ppGroup);

		YXC_Status Broadcast(yuint32_t uGroupId, const YXC_NetServerRawData* pRawData);

		YXC_Status GetNumClients(yuint32_t uGroupId, yuint32_t* puNumClients);

		YXC_Status GetClientSockOpt(yuint32_t uGroupId, YXC_NetSModelClient client, YXC_SocketOption opt, YXC_SocketOptValue* pOptVal);

		YXC_Status SetClientSockOpt(yuint32_t uGroupId, YXC_NetSModelClient client, YXC_SocketOption opt, const YXC_SocketOptValue* pOptVal);

		YXC_Status SendPackageToClient(yuint32_t uGroupId, YXC_NetSModelClient client, const YXC_NetPackage* pPackage, yuint32_t stTimeoutInMilliseconds);

		YXC_Status CloseClient(yuint32_t uGroupId, YXC_NetSModelClient client, YXC_Event* pWaitHandle);

		YXC_Status Start();

		void Close(ybool_t bWaitForClosed);

		void CloseGroup(yuint32_t uGroupId);

		void Destroy();

		void ValidateClient(YXC_Socket sClient);

	public:
		inline YXC_Socket GetSocket() { return this->_serverSock; }

		inline void DestroyClient(yuint32_t uGroupId, void* pClientExt, YXC_Socket socket)
		{
			if (this->_pfnOnDestroyAttr != NULL)
			{
				this->_pfnOnDestroyAttr((YXC_NetSModelServer)this, uGroupId, socket, this->_pExtData, pClientExt);
			}
		}

	private:
		void _HandleClientRequest(YXC_Socket socket);

		YXC_Status _SetSocketAttr(YXC_Socket socket, socket_t rSock);

		YXC_Status _CreateClientAndAddToGroup(YXC_Socket socket, yuint32_t stMaxSendBlocks, ysize_t stSendBufferSize, ysize_t stRecvBufferSize,
			yuint32_t stMaxNumBlocksOnQueue, yuint32_t uGroupId, void*& pClientExt);

		_NetSModelServerGroup* _TryRefGroup(yuint32_t iGroupId);

	private:
		static yuint32_t __stdcall _NetSModelServerThreadProc(void* pArgs);
		static yuint32_t __stdcall _NetSModelServerClientAcceptProc(void* pArgs);

	private:
		YXX_Crit _csServer;
		ybyte_t _byServerProtocol;

		typedef std::map<yuint32_t, _NetSModelServerGroup*>::iterator GROUP_ITER;
		std::map<yuint32_t, _NetSModelServerGroup*> _mapGroups;

		YXC_CondVar _ctrlVar;
		YXC_CondVarSV _supervisor;

		YXC_Socket _serverSock;
		socket_t _rSock;

		ethread_t _upThread;

		YXC_NetServerOnInitClientAttr _pfnOnInitAttr;
		YXC_NetServerOnDestroyClientAttr _pfnOnDestroyAttr;
		void* _pExtData;

		ybool_t _bResolveLeakIssue;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_NetSModelServer, _NetSModelServer, _SModelPtr, _SModelHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_GROUP_HPP__ */

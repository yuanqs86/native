#ifndef __INNER_INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_GROUP_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_GROUP_HPP__

#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_StlAlloc.hpp>

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/Net/_YXC_NetSModelClientMgr.hpp>

namespace YXC_Inner
{
	class _NetSModelServer;
	class _NetSModelServerGroup;
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_NetSModelGroupMgr, _NetSModelServerGroup, _SGroupPtr, _SGroupHdl);

	class _NetSModelServerGroup
	{
	public:
		YXC_Status Create(const YXC_NetSrvGroupCreateArgs* pCreateArgs, ybyte_t byServerProtocol,
			YXC_NetServerOnClientConnected pfnOnConnected, YXC_NetServerOnClientClose pfnOnClose,
			YXC_NetServerOnClientRecv pfnOnRecv, void* pExtData, _NetSModelServer* pServer);

		YXC_Status Broadcast(const YXC_NetServerRawData* pRawData);

		YXC_Status AddClient(_NetSModelClient* pClient, yuintptr_t* pClientHdl);

		YXC_Status GetClientSockOpt(YXC_NetSModelClient client, YXC_SocketOption opt, YXC_SocketOptValue* pOptVal);

		YXC_Status SetClientSockOpt(YXC_NetSModelClient client, YXC_SocketOption opt, const YXC_SocketOptValue* pOptVal);

		YXC_Status DropBlocks(YXC_NetSModelClient client, ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
			void* pDropCtrl, yuint32_t* pstNumBlocksDropped);

		YXC_Status SendPackageToClient(YXC_NetSModelClient client, const YXC_NetPackage* pPackage, yuint32_t stTimeoutInMilliseconds);

		YXC_Status CloseClient(YXC_NetSModelClient client, YXC_Event* pWaitHandle);

		YXC_Status NotifyCloseClient(YXC_NetSModelClient client);

		void CloseAndWait();

		void Destroy();

	public:
		inline yuint32_t GetClientMaxNumBlocks() { return this->_stMaxNumBlocksOnClient; }

		inline yuint32_t GetMaxSendBlockCount() { return this->_stMaxNumSendBlocks; }

		inline ysize_t GetSendBufferSize() { return this->_stSendBufferSize; }

		inline ysize_t GetRecvBufferSize() { return this->_stRecvBufferSize; }

		inline yuint32_t GetNumClients() { return this->_mcMgr.GetNumClients(); }

		inline yuint32_t GetThreadNumClients() { return (yuint32_t)this->_stThreadNumClients; }

		inline yuint32_t GetGroupId() { return this->_uGroupId; }

		inline void Reference()
		{
			YXCLib::Interlocked::Increment(&this->_uRefCount);
		}

		inline void Unreference()
		{
			yuint32_t iRet = YXCLib::Interlocked::Decrement(&this->_uRefCount);

			if (iRet == 0)
			{
				DestroyGroup(this);
			}
		}

		static inline void DestroyGroup(_NetSModelServerGroup* pGroup)
		{
			pGroup->Destroy();
			free(pGroup);
		}

		inline YXC_PNCMP GetMemPool() { return this->_memPool; }

		inline YXC_NetSModelClientOnClose GetCloseCallback() { return OnClientClose; }

		inline YXC_NetSModelClientOnConnect GetConnectCallback() { return OnClientConnect; }

		inline YXC_NetSModelClientOnRecv GetRecvCallback() { return OnClientRecv; }

		inline YXC_NetSModelClientOnNoData GetNoDataCallback() { return OnClientNoData; }

		inline YXC_NetSModelClientOnDestroy GetDestroyCallback() { return OnClientDestroy; }

		inline YXC_PNCMPConsumerBlockCheckFunc GetBlockCheckFunc() { return OnBlockCheck; }

		inline void OnClientConnected(YXC_NetSModelClient client, void* pCExt)
		{
			if (this->_pfnOnConnected != NULL)
			{
				this->_pfnOnConnected(_SGroupHdl(this), client, this->_pExtData, pCExt);
			}
		}

	private:
		YXC_Status _CreateMemPool(const YXC_NetSrvCreateArgsMC* pArgs);

		static void OnClientClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pMgrExt, void* pCExt, ybool_t bClosedActively);

		static void OnClientConnect(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pMgrExt, void* pCExt);

		static void OnClientDestroy(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pMgrExt, void* pCExt, YXC_Socket socket);

		static void OnClientRecv(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage, void* pMgrExt, void* pCExt);

		static void OnClientNoData(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, ysize_t stMilliSeconds, void* pMgrExt, void* pCExt);

		static YXC_Status OnBlockCheck(YXC_PNCMPConsumer consumer, const void* pBlockDesc, YXC_PNCMPConsumerInfo* cInfo, void* pConsumerCtrl);

	private:
		YXC_PNCMP _memPool;
		YXC_PNCMPProducer _memProducer;

		YXX_Crit _csModel;
		yuint32_t _uRefCount;
		yuint32_t _uGroupId;

		yuint32_t _stThreadNumClients;
		yuint32_t _stMaxNumBlocksOnClient;
		yuint32_t _stMaxNumSendBlocks;
		ysize_t _stSendBufferSize;
		ysize_t _stRecvBufferSize;

		_NetSModelClientMgr _mcMgr;
		_NetSModelServer* _pServer;

		YXC_Event _eveClose;

		ybyte_t _byServerProtocol;
		YXC_NetProtocol _protocol;

		YXC_NetServerOnClientConnected _pfnOnConnected;
		YXC_NetServerOnClientClose _pfnOnClose;
		YXC_NetServerOnClientRecv _pfnOnRecv;
		YXC_NetServerOnBroadcastCheck _pfnOnBroadcastCheck;
		void* _pExtData;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_GROUP_HPP__ */

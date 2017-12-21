#define __MODULE__ "EK.Net.SModel.Server"

#if YXC_PLATFORM_WIN
#include <process.h>
#endif /* YXC_PLATFORM_WIN */
#include <assert.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

#include <YXC_Sys/Net/_YXC_NetSModelServerGroup.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelServer.hpp>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>

namespace YXC_Inner
{
	YXC_Status _NetSModelServerGroup::Create(const YXC_NetSrvGroupCreateArgs* pCreateArgs, ybyte_t byServerProtocol,
		YXC_NetServerOnClientConnected pfnOnConnected, YXC_NetServerOnClientClose pfnOnClose,
		YXC_NetServerOnClientRecv pfnOnRecv, void* pExtData, _NetSModelServer* pServer)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		this->_pfnOnClose = pfnOnClose;
		this->_pfnOnRecv = pfnOnRecv;
		this->_pfnOnConnected = pfnOnConnected;
		this->_pExtData = pExtData;
		this->_byServerProtocol = byServerProtocol;
		this->_csModel.Init(4000);
		this->_uGroupId = pCreateArgs->uGroupId;
		this->_pServer = pServer;
		this->_eveClose = NULL;
		this->_stMaxNumBlocksOnClient = 0;
		this->_stThreadNumClients = pCreateArgs->stThreadNumClients;
		if (this->_stThreadNumClients > _YXC_THREAD_MAX_CLIENT) this->_stThreadNumClients = _YXC_THREAD_MAX_CLIENT;
		if (this->_stThreadNumClients == 0)
		{
			this->_stThreadNumClients = _YXC_THREAD_DEFAULT_CLIENTS;
		}

		this->_memPool = NULL;
		this->_uRefCount = 0x80000000; // not closed yet

		YXC_Status rc = this->_CreateMemPool(pCreateArgs->pMCArgs);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create multi cast pool failed"));

		this->_stRecvBufferSize = _YXC_MODEL_DEF_RECV_BUF_SIZE;
		if (pCreateArgs->stRecvBufferSize != 0)
		{
			this->_stRecvBufferSize = pCreateArgs->stRecvBufferSize;
		}
		this->_stMaxNumSendBlocks = _YXC_MODEL_DEF_SEND_BLOCKS;
		if (pCreateArgs->stMaxNumSendBlocks != 0)
		{
			this->_stMaxNumSendBlocks = pCreateArgs->stMaxNumSendBlocks;
		}
		this->_stSendBufferSize = _YXC_MODEL_DEF_SEND_BUF_SIZE;
		if (pCreateArgs->stSendBufferSize != 0)
		{
			this->_stSendBufferSize = pCreateArgs->stSendBufferSize;
		}

		rc = YXC_EventCreate(TRUE, FALSE, &this->_eveClose);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create exit event failed"));

		rc = this->_mcMgr.Create(pCreateArgs->pMCArgs != NULL, (yuint32_t)pCreateArgs->stMaxNumClients, this->GetThreadNumClients(),
			(yuint32_t)pCreateArgs->stStackSize, this);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create multi cast client manager failed"));

		YXC_NPSetProtocol(&this->_protocol, pCreateArgs->pProtocol);
		return YXC_ERC_SUCCESS;
err_ret:
		if (this->_memPool != NULL) YXC_PNCMPForceDestroy(this->_memPool);
		if (this->_eveClose != NULL) YXC_EventDestroy(this->_eveClose);

		this->_csModel.Close();
		return rcRet;
	}

	YXC_Status _NetSModelServerGroup::Broadcast(const YXC_NetServerRawData* pRawData)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_memPool != NULL, YXC_ERC_NOT_SUPPORTED, YC("Broadcast with a server which don't enable multi cast manager"));

		ybyte_t byBuffer[sizeof(YXC_NetPackage) + sizeof(YXC_NetTransferInfo)];
		YXC_NetTransferInfo* pTransInfo = (YXC_NetTransferInfo*)byBuffer;

		YXC_NetPackage package;
		YXC_NPStdCreatePackageEx(&this->_protocol, (yuint32_t)pRawData->stDataLen, pRawData->pData, this->_byServerProtocol, TRUE,
			(yuint32_t)pRawData->stNumHeaderDatas, pRawData->headers, &package);
		YXC_NPStdInitTransferInfo(&this->_protocol, &package, pTransInfo);

		YXC_NetPackage* pPackage2 = (YXC_NetPackage*)(byBuffer + sizeof(YXC_NetTransferInfo));
		memcpy(pPackage2->byHdrBuffer, package.byHdrBuffer, pTransInfo->stTotalHeaderSize);
		ysize_t stTotal = pTransInfo->stTotalHeaderSize + sizeof(YXC_NetTransferInfo);
		YXC_Status rc = YXC_PNCMPProducerPushBlockCond(this->_memProducer, pRawData->pData, pRawData->stDataLen,
			byBuffer, stTotal, pRawData, 0);

		this->_mcMgr.SignalAll();

		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Not all clients received the broadcast content"));
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelServerGroup::_CreateMemPool(const YXC_NetSrvCreateArgsMC* pArgs)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		this->_memPool = NULL;
		this->_memProducer = NULL;

		if (pArgs == NULL) return YXC_ERC_SUCCESS;

		YXC_Status rcPool = YXC_ERC_UNKNOWN, rcProd = YXC_ERC_UNKNOWN;
		rcPool = YXC_PNCMPCreate(pArgs->stMaxNumBlocks, pArgs->stMaxNumClients, pArgs->mmParam, NULL, &this->_memPool);
		_YXC_CHECK_REPORT_GOTO(rcPool == YXC_ERC_SUCCESS, rcPool, YC("Create multi cast pool for _NetSModelServer failed"));

		rcProd = YXC_PNCMPProducerAttachEx(this->_memPool, 0, FALSE, FALSE, &this->_memProducer);
		_YXC_CHECK_REPORT_GOTO(rcProd == YXC_ERC_SUCCESS, rcProd, YC("Attach producer for _NetSModelServer failed"));

		this->_stMaxNumBlocksOnClient = pArgs->stMaxNumBlocksOnClient;
		this->_pfnOnBroadcastCheck = pArgs->pfnOnBroadcastCheck;

		return YXC_ERC_SUCCESS;
err_ret:
		if (this->_memPool != NULL) YXC_PNCMPForceDestroy(this->_memPool);
		return rcRet;
	}

	YXC_Status _NetSModelServerGroup::AddClient(_NetSModelClient* pClient, yuintptr_t* pClientHdl)
	{
		return this->_mcMgr.AddClient(pClient, pClientHdl);
	}

	YXC_Status _NetSModelServerGroup::GetClientSockOpt(YXC_NetSModelClient client, YXC_SocketOption opt, YXC_SocketOptValue* pOptVal)
	{
		return this->_mcMgr.GetClientSockOption((yuintptr_t)client, opt, pOptVal);
	}

	YXC_Status _NetSModelServerGroup::SetClientSockOpt(YXC_NetSModelClient client, YXC_SocketOption opt, const YXC_SocketOptValue* pOptVal)
	{
		return this->_mcMgr.SetClientSockOption((yuintptr_t)client, opt, pOptVal);
	}

	YXC_Status _NetSModelServerGroup::DropBlocks(YXC_NetSModelClient client, ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl, yuint32_t* pstNumBlocksDropped)
	{
		return this->_mcMgr.DropBlocks((yuintptr_t)client, bMCBlock, pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
	}

	YXC_Status _NetSModelServerGroup::SendPackageToClient(YXC_NetSModelClient client, const YXC_NetPackage* pPackage, yuint32_t stTimeoutInMilliseconds)
	{
		return this->_mcMgr.SendData((yuintptr_t)client, pPackage, stTimeoutInMilliseconds);
	}

	YXC_Status _NetSModelServerGroup::CloseClient(YXC_NetSModelClient client, YXC_Event* pWaitHandle)
	{
		return this->_mcMgr.CloseClient((yuintptr_t)client, pWaitHandle);
	}

	YXC_Status _NetSModelServerGroup::NotifyCloseClient(YXC_NetSModelClient client)
	{
		return this->_mcMgr.NotifyCloseClient((yuintptr_t)client);
	}

	void _NetSModelServerGroup::CloseAndWait()
	{
		YXC_Event event = (YXC_Event)YXC_HandleDuplicate(this->_eveClose);
		if (YXCLib::Interlocked::ExchangeAnd(&this->_uRefCount, 0x7fffffff) == 0x80000000) // No client now
		{
			DestroyGroup(this);
		}
		else /* Wait for group quit */
		{
			YXC_EventLock(event);
			YXC_EventDestroy(event);
		}
	}

	void _NetSModelServerGroup::Destroy()
	{
		this->_mcMgr.Close();
		this->_mcMgr.WaitForExit();
		YXC_EventDestroy(this->_eveClose);
		this->_mcMgr.Destroy();

		if (this->_memPool != NULL)
		{
			YXC_PNCMPProducerDetach(this->_memPool, this->_memProducer);
			YXC_PNCMPReleaseWhenLastConsumerDetach(this->_memPool);
		}

		this->_csModel.Close();
	}

	void _NetSModelServerGroup::OnClientClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pMgrExt,
		void* pExtData, ybool_t bClosedActively)
	{
		_NetSModelServerGroup* pGroup = (_NetSModelServerGroup*)pMgrExt;

		if (pGroup->_pfnOnClose != NULL)
		{
			pGroup->_pfnOnClose(_SGroupHdl(pGroup), client, pGroup->_pExtData, pExtData, bClosedActively);
		}
	}

	void _NetSModelServerGroup::OnClientConnect(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pMgrExt,
		void* pExtData)
	{
		_NetSModelServerGroup* pGroup = (_NetSModelServerGroup*)pMgrExt;

		if (pGroup->_pfnOnConnected != NULL)
		{
			pGroup->_pfnOnConnected(_SGroupHdl(pGroup), client, pGroup->_pExtData, pExtData);
		}
	}

	void _NetSModelServerGroup::OnClientRecv(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage, void* pMgrExt, void* pCExt)
	{
		_NetSModelServerGroup* pGroup = (_NetSModelServerGroup*)pMgrExt;

		if (pGroup->_pfnOnRecv != NULL)
		{
			pGroup->_pfnOnRecv(_SGroupHdl(pGroup), client, pPackage, pGroup->_pExtData, pCExt);
		}
	}

	void _NetSModelServerGroup::OnClientDestroy(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client,
		void* pMgrExt, void* pCExt, YXC_Socket socket)
	{
		_NetSModelServerGroup* pGroup = (_NetSModelServerGroup*)pMgrExt;
		pGroup->_pServer->DestroyClient(pGroup->_uGroupId, pCExt, socket);
	}

	void _NetSModelServerGroup::OnClientNoData(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, ysize_t stMilliSeconds,
		void* pMgrExt, void* pCExt)
	{
		//_NetSModelServerGroup* pGroup = (_NetSModelServerGroup*)pExtData;

		//if (pGroup->_pfnOnRecv != NULL)
		//{
		//	pGroup->_pfnOnRecv(_SModelHdl(pGroup->_pServer), pGroup->_uGroupId, client, pPackage, pGroup->_pExtData);
		//}
	}

	// rely on lock of YXC_PNCMP
	YXC_Status _NetSModelServerGroup::OnBlockCheck(YXC_PNCMPConsumer consumer, const void* pBlockDesc, YXC_PNCMPConsumerInfo* cInfo, void* pConsumerCtrl)
	{
		_NetSModelClient* pClient = (_NetSModelClient*)pConsumerCtrl;
		_NetSModelServerGroup* pGroupMgr = (_NetSModelServerGroup*)pClient->GetMgr();
		if (pGroupMgr->_pfnOnBroadcastCheck)
		{
			const YXC_NetServerRawData* pRawData = (const YXC_NetServerRawData*)pBlockDesc;
			return pGroupMgr->_pfnOnBroadcastCheck((YXC_NetSModelGroupMgr)pGroupMgr, (YXC_NetSModelClient)pClient->GetHandle(),
				cInfo, pRawData, pGroupMgr->_pExtData, pClient->GetExtPtr());
		}

		return YXC_ERC_SUCCESS;
	}
}

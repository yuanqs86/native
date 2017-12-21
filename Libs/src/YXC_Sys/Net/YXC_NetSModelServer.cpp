#define __MODULE__ "EK.Net.SModel.Server"

#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/Net/_YXC_NetSModelCmn.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelServer.hpp>

#include <new>

using namespace YXC_Inner;

extern "C"
{
	YXC_Status YXC_NetSModelServerCreate(const YXC_NetServerCreateArgs* pCreateArgs, YXC_NetServerOnInitClientAttr pfnOnInitClient,
		YXC_NetServerOnDestroyClientAttr pfnOnDestroyClient, void* pExtData, YXC_NetSModelServer* psModel)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;
		_NetSModelServer* pModel = (_NetSModelServer*)malloc(sizeof(_NetSModelServer));
		_YXC_CHECK_REPORT_NEW_GOTO(pModel != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc pointer for NetServerSModel failed"));

        rc = pModel->Create(pCreateArgs, pfnOnInitClient, pfnOnDestroyClient, pExtData);
		_YXC_CHECK_REPORT_NEW_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create NetServerSModel failed"));

		*psModel = _SModelHdl(pModel);
		return YXC_ERC_SUCCESS;

err_ret:
		if (pModel != NULL) free(pModel);
		return rcRet;
	}

	YXC_Status YXC_NetSModelServerBroadcast(YXC_NetSModelServer sModel, yuint32_t uGroupId, const YXC_NetServerRawData* pRawData)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		YXC_Status rc = pModel->Broadcast(uGroupId, pRawData);

		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Broadcast for NetServerSModel failed"));

		return rc;
	}

	void YXC_NetSModelServerGetNumClients(YXC_NetSModelServer sModel, yuint32_t uGroupId, yuint32_t* puNumClients)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		if (puNumClients != NULL) pModel->GetNumClients(uGroupId, puNumClients);
	}

	YXC_Status YXC_NetSModelServerSetSockOpt(YXC_NetSModelServer sModel, yuint32_t uGroupId, YXC_NetSModelClient client,
		YXC_SocketOption opt, const YXC_SocketOptValue* pOptVal)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->SetClientSockOpt(uGroupId, client, opt, pOptVal);
	}

	YXC_Status YXC_NetSModelServerGetSockOpt(YXC_NetSModelServer sModel, yuint32_t uGroupId, YXC_NetSModelClient client,
		YXC_SocketOption opt, YXC_SocketOptValue* pOptVal)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->GetClientSockOpt(uGroupId, client, opt, pOptVal);
	}

	YXC_Status YXC_NetSModelServerSendToClient(YXC_NetSModelServer sModel, yuint32_t uGroupId, YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage, yuint32_t stTimeoutInMiliseconds)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->SendPackageToClient(uGroupId, client, pPackage, stTimeoutInMiliseconds);
	}

	YXC_Status YXC_NetSModelServerCloseClient(YXC_NetSModelServer sModel, yuint32_t uGroupId, YXC_NetSModelClient client)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->CloseClient(uGroupId, client, NULL);
	}

	YXC_Status YXC_NetSModelServerCloseClientEx(YXC_NetSModelServer sModel, yuint32_t uGroupId, YXC_NetSModelClient client, YXC_Event* pWaitEvent)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->CloseClient(uGroupId, client, pWaitEvent);
	}

	YXC_Status YXC_NetSModelServerStart(YXC_NetSModelServer sModel)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->Start();
	}

	void YXC_NetSModelServerClose(YXC_NetSModelServer sModel, ybool_t bWaitForClosed)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		pModel->Close(bWaitForClosed);
	}

	YXC_Status YXC_NetSModelServerGroupCreate(YXC_NetSModelServer sModel, const YXC_NetSrvGroupCreateArgs* pGroupArgs,
		YXC_NetServerOnClientConnected pfnOnConn, YXC_NetServerOnClientClose pfnOnClose,
		YXC_NetServerOnClientRecv pfnOnRecv, void* pGroupData)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->CreateGroup(pGroupArgs, pfnOnConn, pfnOnClose, pfnOnRecv, pGroupData);
	}

	void YXC_NetSModelServerGroupClose(YXC_NetSModelServer sModel, yuint32_t uGroupId)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->CloseGroup(uGroupId);
	}

	void YXC_NetSModelGroupGetNumClients(YXC_NetSModelGroupMgr groupMgr, yuint32_t* puNumClients)
	{
		_NetSModelServerGroup* pModel = _SGroupPtr(groupMgr);
		*puNumClients = pModel->GetNumClients();
	}

	YXC_Status YXC_NetSModelGroupBroadcast(YXC_NetSModelGroupMgr groupMgr, const YXC_NetServerRawData* pRawData)
	{
		_NetSModelServerGroup* pModel = _SGroupPtr(groupMgr);
		return pModel->Broadcast(pRawData);
	}

	YXC_Status YXC_NetSModelServerGroupRef(YXC_NetSModelServer sModel, yuint32_t uGroupId, YXC_NetSModelGroupMgr* pGroupMgr)
	{
		_NetSModelServer* pModel = _SModelPtr(sModel);

		return pModel->RefGroup(uGroupId, (_NetSModelServerGroup**)pGroupMgr);
	}

	void YXC_NetSModelServerGroupUnref(YXC_NetSModelGroupMgr mgr)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(mgr);

		pGroupMgr->Unreference();
	}

	YXC_Status YXC_NetSModelGroupSetSockOpt(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client,
		YXC_SocketOption opt, const YXC_SocketOptValue* pOptVal)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->SetClientSockOpt(client, opt, pOptVal);
	}

	YXC_Status YXC_NetSModelGroupGetSockOpt(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client,
		YXC_SocketOption opt, YXC_SocketOptValue* pOptVal)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->GetClientSockOpt(client, opt, pOptVal);
	}

	YXC_Status YXC_NetSModelGroupDropBlocks(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client, ybool_t bMC, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl, yuint32_t* pstNumBlocksDropped)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->DropBlocks(client, bMC, pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
	}

	YXC_Status YXC_NetSModelGroupSendToClient(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage, yuint32_t stTimeoutInMiliseconds)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->SendPackageToClient(client, pPackage, stTimeoutInMiliseconds);
	}

	yuint32_t YXC_NetSModelQueryGroupId(YXC_NetSModelGroupMgr mgr)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(mgr);

		return pGroupMgr->GetGroupId();
	}

	YXC_Status YXC_NetSModelGroupCloseClient(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->CloseClient(client, NULL);
	}

	YXC_Status YXC_NetSModelGroupNotifyClose(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->NotifyCloseClient(client);
	}

	YXC_Status YXC_NetSModelGroupCloseClientEx(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client, YXC_Event* pWaitEvent)
	{
		_NetSModelServerGroup* pGroupMgr = _SGroupPtr(groupMgr);

		return pGroupMgr->CloseClient(client, pWaitEvent);
	}
};

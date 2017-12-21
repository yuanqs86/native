#define __MODULE__ "EK.NSM.Group"

#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_Sys/YXC_NSMClientBase.hpp>
#include <YXC_Sys/Net/_YXC_NSMHelper.hpp>
#include <YXC_Sys/YXC_NSMGroupBase.hpp>
#include <YXC_Sys/YXC_NSMServerBase.hpp>

namespace
{
	class DummyContainer : public YXCLib::INSMClientContainer
	{
		virtual void OnAddClient(YXCLib::NSMClientBase* client) {}

		virtual void OnRemoveClient(YXCLib::NSMClientBase* client) {}
	};

	DummyContainer gs_dummy_container;
}

namespace YXCLib
{
	NSMGroupBase::NSMGroupBase(INSMClientContainer* c) : _pServer(NULL), _groupMgr(NULL),
		_iGroupId(YXC_NET_MODEL_BROADCAST_TO_ALL), _iContainer(c)
	{
		if (this->_iContainer == NULL)
		{
			this->_iContainer = &gs_dummy_container;
		}
	}

	NSMGroupBase::~NSMGroupBase()
	{
		this->Close();
	}

	void NSMGroupBase::Close()
	{
		if (this->_iGroupId != YXC_NET_MODEL_BROADCAST_TO_ALL)
		{
			YXC_NetSModelServerGroupUnref(this->_groupMgr);
			YXC_NetSModelServerGroupClose(this->_pServer->GetServer(), this->_iGroupId);
			_NSMHelper::RemoveGroup(this->_pServer, this);
			this->_groupMgr = NULL;
			this->_iGroupId = YXC_NET_MODEL_BROADCAST_TO_ALL;
		}
	}

	YXC_Status NSMGroupBase::Create(NSMServerBase* pServer, const YXC_NetProtocol* pProtocol, yuint32_t stMaxClients, int iGroupId, ysize_t stRecvBufSize,
		yuint32_t stMaxSendBlocks, ysize_t stSendBufSize)
	{
		YXC_NetSrvGroupCreateArgs groupCArgs = {0};
		groupCArgs.stMaxNumClients = stMaxClients;
		groupCArgs.stMaxNumSendBlocks = stMaxSendBlocks;
		groupCArgs.stRecvBufferSize = stRecvBufSize;
		groupCArgs.stSendBufferSize = stSendBufSize;
		groupCArgs.pProtocol = pProtocol;
		groupCArgs.stStackSize = 0;
		groupCArgs.pMCArgs = NULL;
		groupCArgs.stThreadNumClients = 0;
		groupCArgs.uGroupId = iGroupId;
		groupCArgs.stStackSize = 256 << 10; //256K

		return this->Create(pServer, &groupCArgs);
	}

	YXC_Status NSMGroupBase::Create(NSMServerBase* pServer, const YXC_NetSrvGroupCreateArgs* pArgs)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_iGroupId == YXC_NET_MODEL_BROADCAST_TO_ALL, YXC_ERC_ALREADY_CREATED, YC("The group has already been created"));

		YXC_Status rc = YXC_NetSModelServerGroupCreate(pServer->GetServer(), pArgs, OnClientConnected, OnClientClose, OnClientRecv, this);
		if (rc == YXC_ERC_SUCCESS)
		{
			this->_pServer = pServer;
			this->_iGroupId = pArgs->uGroupId;

			YXC_NetSModelServerGroupRef(pServer->GetServer(), pArgs->uGroupId, &this->_groupMgr);

			YXC_Status rc = _NSMHelper::AddGroup(pServer, this);
			if (rc != YXC_ERC_SUCCESS)
			{
				YXC_NetSModelServerGroupUnref(this->_groupMgr);
				YXC_NetSModelServerGroupClose(pServer->GetServer(), pArgs->uGroupId);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	void NSMGroupBase::OnClientClose(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient sModelClient, void* pGroupExt, void* pClientExt, ybool_t bClosedActively)
	{
		NSMGroupBase* pGroupBase = (NSMGroupBase*)pGroupExt;
		NSMClientBase* pClientBase = (NSMClientBase*)pClientExt;

		pClientBase->SetHandle(NULL);

		pGroupBase->_iContainer->OnRemoveClient(pClientBase);
		pClientBase->_OnClose(bClosedActively);
	}

	void NSMGroupBase::OnClientRecv(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient sModelClient, const YXC_NetPackage* pPackage, void* pGroupExt, void* pCExt)
	{
		NSMClientBase* pClientBase = (NSMClientBase*)pCExt;
		pClientBase->_OnReceive(pPackage);
	}

	void NSMGroupBase::OnClientConnected(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient sModelClient, void* pGroupExt, void* pClientExt)
	{
		NSMGroupBase* pGroupBase = (NSMGroupBase*)pGroupExt;
		NSMClientBase* pClientBase = (NSMClientBase*)pClientExt;
		_NSMHelper::SetClientHdl(pClientBase, pGroupBase, sModelClient);

		pGroupBase->_iContainer->OnAddClient(pClientBase);
		pClientBase->_OnConnected();
	}

	YXC_Status NSMGroupBase::Broadcast(const YXC_NetServerRawData* pRawData)
	{
		return YXC_NetSModelGroupBroadcast(this->_groupMgr, pRawData);
	}

	yuint32_t NSMGroupBase::GetClientCount()
	{
		if (this->_groupMgr == NULL) return 0;

		yuint32_t uNumClients;
		YXC_NetSModelGroupGetNumClients(this->_groupMgr, &uNumClients);

		return uNumClients;
	}
}

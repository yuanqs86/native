/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_SMODEL_GROUP_BASE_HPP__
#define __INC_YXC_SYS_BASE_NET_SMODEL_GROUP_BASE_HPP__

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <set>
#include <map>
#include <YXC_Sys/YXC_NSMServerBase.hpp>

namespace YXCLib
{
	class INSMClientContainer
	{
	public:
		virtual void OnAddClient(NSMClientBase* client) = 0;

		virtual void OnRemoveClient(NSMClientBase* client) = 0;
	};

	class NSMClientContainer : public INSMClientContainer
	{
	public:
		inline void OnAddClient(NSMClientBase* client)
		{
			YXX_CritLocker locker(this->csCliMap);
			try
			{
				std::set<NSMClientBase*>::iterator it = this->cliMap.find(client);
				if (it == this->cliMap.end())
				{
					this->cliMap.insert(client);
				}
			}
			catch (const std::exception&)
			{
				return;
			}
		}

		inline void OnRemoveClient(NSMClientBase* client)
		{
			YXX_CritLocker locker(this->csCliMap);
			std::set<NSMClientBase*>::iterator it = this->cliMap.find(client);
			if (it != this->cliMap.end())
			{
				this->cliMap.erase(it);
			}
		}

		std::set<NSMClientBase*> cliMap;
		YXX_Crit csCliMap;
	};

	class YXC_CLASS NSMGroupBase
	{
	public:
		NSMGroupBase(INSMClientContainer* c = NULL);

		virtual ~NSMGroupBase() = 0;

	public:
		YXC_Status Create(NSMServerBase* pServer, const YXC_NetProtocol* pProtocol, yuint32_t stMaxClients, int iGroupId,
			ysize_t stRecvBufSize, yuint32_t stMaxSendBlocks, ysize_t stSendBufSize);

		YXC_Status Create(NSMServerBase* pModelBase, const YXC_NetSrvGroupCreateArgs* pArgs);

		YXC_Status Broadcast(const YXC_NetServerRawData* pRawData);

		yuint32_t GetClientCount();

		void Close();

	public:
		inline YXC_NetSModelGroupMgr GetGroupMgr() { return this->_groupMgr; }

		inline yuint32_t GetGroupId() { return this->_iGroupId; }

		inline NSMServerBase* GetServer() { return this->_pServer; }

	private:
		static void OnClientConnected(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient sModelClient,
			void* pGroupExt, void* pClientExt);

		static void OnClientClose(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient sModelClient,
			void* pGroupExt, void* pClientExt, ybool_t bClosedActively);

		static void OnClientRecv(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient sModelClient,
			const YXC_NetPackage* pPackage, void* pGroupExt, void* pCExt);

	protected:
		NSMServerBase* _pServer;
		YXC_NetSModelGroupMgr _groupMgr;

		INSMClientContainer* _iContainer;

		int _iGroupId;
	};
}

#endif /* __INC_YXC_SYS_BASE_NET_SMODEL_GROUP_BASE_HPP__ */

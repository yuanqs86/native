#ifndef __INNER_INC_YXX_BASE_NET_SMODEL_HELPER_HPP__
#define __INNER_INC_YXX_BASE_NET_SMODEL_HELPER_HPP__

#include <YXC_Sys/YXC_NSMClientBase.hpp>
#include <YXC_Sys/YXC_NSMGroupBase.hpp>
#include <YXC_Sys/YXC_NSMServerBase.hpp>

namespace YXCLib
{
	class _NSMHelper
	{
	public:
		static inline void SetClientHdl(NSMClientBase* pClient, NSMGroupBase* pGroup, YXC_NetSModelClient clientHdl)
		{
			pClient->_SetHandles(pGroup, clientHdl);
		}

		static inline void RemoveGroup(NSMServerBase* pServer, NSMGroupBase* pGroup)
		{
			pServer->_RemoveGroup(pGroup->GetGroupId());
		}

		static inline YXC_Status AddGroup(NSMServerBase* pServer, NSMGroupBase* pGroup)
		{
			return pServer->_AddGroup(pGroup->GetGroupId(), pGroup);
		}
	};
}

#endif /* __INNER_INC_YXX_BASE_NET_SMODEL_HELPER_HPP__ */

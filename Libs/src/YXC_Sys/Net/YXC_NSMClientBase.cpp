#define __MODULE__ "EK.NSM.Client"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>

#include <YXC_Sys/YXC_NSMClientBase.hpp>
#include <YXC_Sys/Net/_YXC_NSMHelper.hpp>
#include <YXC_Sys/YXC_NSMGroupBase.hpp>
#include <YXC_Sys/YXC_NSMServerBase.hpp>

#if YXC_PLATFORM_UNIX
#include <netdb.h>
#endif /* YXC_PLATFORM_UNIX */

namespace YXCLib
{
	NSMClientBase::NSMClientBase(const YXC_SockAddr* pAddr) : _pGroupBase(NULL), _cHdl(NULL)
	{
		this->_SetIpInformation(pAddr);
	}

	NSMClientBase::~NSMClientBase() /* Don't do anything, will call here by _OnCloseClientAttr. */
	{

	}

	void NSMClientBase::NotifyClose()
	{
		YXC_NetSModelGroupNotifyClose(this->_pGroupBase->GetGroupMgr(), this->_cHdl);
	}

	YXC_Status NSMClientBase::Close(ybool_t bWaitForClosed)
	{
		if (bWaitForClosed)
		{
			YXC_Event uWaitEvent;
			YXC_Status rc = YXC_NetSModelGroupCloseClientEx(this->_pGroupBase->GetGroupMgr(), this->_cHdl, &uWaitEvent);
			if (rc == YXC_ERC_SUCCESS)
			{
				if (uWaitEvent != NULL)
				{
					YXC_WaitSingleKObject(uWaitEvent);
					return YXC_ERC_SUCCESS;
				}
				return YXC_ERC_SUCCESS;
			}
		}
		else
		{
			return YXC_NetSModelGroupCloseClient(this->_pGroupBase->GetGroupMgr(), this->_cHdl);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status NSMClientBase::GetSockOpt(YXC_SocketOption option, YXC_SocketOptValue* pOptVal)
	{
		return YXC_NetSModelGroupGetSockOpt(this->_pGroupBase->GetGroupMgr(), this->_cHdl, option, pOptVal);
	}

	YXC_Status NSMClientBase::SetSockOpt(YXC_SocketOption option, const YXC_SocketOptValue* pOptVal)
	{
		return YXC_NetSModelGroupSetSockOpt(this->_pGroupBase->GetGroupMgr(), this->_cHdl, option, pOptVal);
	}

	YXC_Status NSMClientBase::SendPackage(const YXC_NetPackage* pPackage, yuint32_t stmsTimeout)
	{
		return YXC_NetSModelGroupSendToClient(this->_pGroupBase->GetGroupMgr(), this->_cHdl, pPackage, stmsTimeout);
	}

	void NSMClientBase::_OnClose(ybool_t bActively)
	{

	}

	void NSMClientBase::_OnReceive(const YXC_NetPackage* pPackage)
	{

	}

	void NSMClientBase::_OnConnected()
	{

	}

	void NSMClientBase::_SetIpInformation(const YXC_SockAddr* pAddr)
	{
		if (pAddr->un.afType == AF_INET)
		{
			getnameinfo((sockaddr*)pAddr, sizeof(sockaddr_in), this->_szIpAddrA, YXC_MAX_IP, NULL, NULL, NI_NUMERICHOST);
			// inet_ntop(pAddr->un.afType, (void*)&pAddr->un.v4Addr.sin_addr, this->_szIpAddrA, YXC_MAX_IP);
			this->_uPort = ntohs(pAddr->un.v4Addr.sin_port);
		}
		else
		{
			getnameinfo((sockaddr*)pAddr, sizeof(sockaddr_in6), this->_szIpAddrA, YXC_MAX_IP, NULL, NULL, NI_NUMERICHOST);
			// inet_ntop(pAddr->un.afType, (void*)&pAddr->un.v6Addr.sin6_addr, this->_szIpAddrA, YXC_MAX_IP);
			this->_uPort = ntohs((pAddr->un.v6Addr.sin6_port));
		}

		YXC_TECharToWChar(this->_szIpAddrA, YXC_STR_NTS, this->_szIpAddrW, YXC_MAX_IP - 1, NULL, NULL);

		this->_addr = *pAddr;
	}

	void NSMClientBase::_SetHandles(NSMGroupBase* pGroupBase, YXC_NetSModelClient cHdl)
	{
		this->_pGroupBase = pGroupBase;

		this->_cHdl = cHdl;
	}
}

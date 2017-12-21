/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASEEX_NET_CSMODEL_CLIENT_HPP__
#define __INC_YXC_BASEEX_NET_CSMODEL_CLIENT_HPP__

#include <YXC_Sys/YXC_NetSModelClient.h>
#include <YXC_Sys/YXC_Locker.hpp>

namespace YXCLib
{
	class YXC_CLASS NCMClient
	{
	public:
		NCMClient();

		virtual ~NCMClient();

		YXC_Status SendPackage(const YXC_NetPackage* pPackage, yuint32_t stTimeout);

		void Close(ybool_t bWaitForClosed);

		ybool_t IsConnected() const;

		YXC_Status ConnectAndCreateW(YXC_NetSModelClientMgr cliMgr, const wchar_t* cszIp, yuint32_t uPort, yuint32_t uTimeout);

		YXC_Status ConnectAndCreate(YXC_NetSModelClientMgr cliMgr, const ychar* cszIp, yuint32_t uPort, yuint32_t uTimeout);

		YXC_Status ConnectAndCreateA(YXC_NetSModelClientMgr cliMgr, const char* cwszIp, yuint32_t uPort, yuint32_t uTimeout);

		YXC_Status AttachSocket(YXC_NetSModelClientMgr cliMgr, YXC_Socket sock);

		YXC_Status GetSocketOpt(YXC_SocketOption option, YXC_SocketOptValue* pOptVal);

		YXC_Status SetSocketOpt(YXC_SocketOption option, const YXC_SocketOptValue* pOptVal);

		void NotifyClose();

	protected:
		virtual YXC_Status _OnInitSockAttr(YXC_Socket sock);

		virtual void _OnClose(ybool_t bClosedActively, const YXC_Error* pClosedReason);

		virtual void _OnReceive(const YXC_NetPackage* pPackage);

	private:
		NCMClient(const NCMClient& rhs);

		NCMClient& operator =(const NCMClient& rhs);

		YXC_Status _AttachSocket(YXC_NetSModelClientMgr cliMgr, YXC_Socket sock);

	private:
		static void OnClientClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pExtData, void* pClientExt,
			ybool_t bClosedActively);

		static void OnClientRecv(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage,
			void* pExtData, void* pClientExt);

	private:
		YXC_NetSModelClient _client;

		YXC_NetSModelClientMgr _mgr;
	};
}

#endif /* __INC_YXC_BASEEX_NET_CSMODEL_CLIENT_HPP__ */

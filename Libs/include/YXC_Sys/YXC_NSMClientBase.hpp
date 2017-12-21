/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_BASE_HPP__
#define __INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_BASE_HPP__

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <set>
#include <map>
#include <YXC_Sys/YXC_NSMServerBase.hpp>

namespace YXCLib
{
	class YXC_CLASS NSMClientBase
	{
	public:
		NSMClientBase(const YXC_SockAddr* pAddr);

		virtual ~NSMClientBase();

	public:
		YXC_Status GetSockOpt(YXC_SocketOption option, YXC_SocketOptValue* pOptVal);

		YXC_Status SetSockOpt(YXC_SocketOption option, const YXC_SocketOptValue* pOptVal);

		YXC_Status SendPackage(const YXC_NetPackage* pPackage, yuint32_t stmsTimeout);

		YXC_Status Close(ybool_t bWaitForClosed);

		void NotifyClose();

	public:
		inline const wchar_t* GetIpStrW() { return this->_szIpAddrW; }

		inline const char* GetIpStrA() { return this->_szIpAddrA; }

        inline const ychar* GetIpStr()
        {
#if YCHAR_WCHAR_T
            return this->_szIpAddrW;
#else
            return this->_szIpAddrA;
#endif /* YCHAR_WCHAR_T */
        }

		inline yuint32_t GetPort() { return this->_uPort; }

		inline const YXC_SockAddr* GetSockAddr() { return &this->_addr; }

		inline YXC_NetSModelClient GetHandle() { return this->_cHdl; }

		inline void SetHandle(YXC_NetSModelClient cHdl) { this->_cHdl = cHdl; }

	public: /* Virtual functions, must be called in its derived class. */
		virtual void _OnClose(ybool_t bActively);

		virtual void _OnReceive(const YXC_NetPackage* pPackage);

		virtual void _OnConnected();

	private:
		void _SetHandles(NSMGroupBase* pGroupBase, YXC_NetSModelClient cHdl);

		void _SetIpInformation(const YXC_SockAddr* pAddr);

		friend class _NSMHelper;

	protected:
		NSMGroupBase* _pGroupBase;

		YXC_NetSModelClient _cHdl;

		char _szIpAddrA[YXC_MAX_IP];
		wchar_t _szIpAddrW[YXC_MAX_IP];
		yuint32_t _uPort;

		YXC_SockAddr _addr;
	};
}

#endif /* __INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_BASE_HPP__ */

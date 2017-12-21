/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_SMODEL_SERVER_HPP__
#define __INC_YXC_SYS_BASE_NET_SMODEL_SERVER_HPP__

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <map>

namespace YXCLib
{
	class NSMClientBase;
	class NSMGroupBase;
	class NSMServerBaseHelper;
	class YXC_CLASS NSMServerBase
	{
	public:
		NSMServerBase();

		virtual ~NSMServerBase();

	public:
		YXC_Status Create(YXC_SocketType sockType, yuint32_t uPort, yuint32_t stMaxClients, yuint32_t stMaxGroups, ybyte_t byProtocol);

		YXC_Status Close();

		ybool_t IsCreated();

		YXC_Status Start();

	public:
		inline YXC_NetSModelServer GetServer() { return this->_server; }

	protected:
		virtual ybool_t OnInitClientAttr(
			YXC_Socket clientSocket,
			const YXC_SockAddr* pAddr,
			yuint32_t* pstMaxNumSendBlocks,
			ysize_t* pstSendBufSize,
			ysize_t* pstRecvBufSize,
			yuint32_t* pstMaxConsumerBlocks,
			yuint32_t* puGroupId,
			NSMClientBase** ppClientBase
		) = 0;

		virtual void OnCloseClientAttr(
			yuint32_t uGroupId,
			YXC_Socket socket,
			NSMClientBase* pClientBase
		) = 0;

	private:
		static ybool_t _OnInitClientAttr(
			YXC_NetSModelServer server,
			YXC_Socket clientSocket,
			yuint32_t* pstMaxNumSendBlocks,
			ysize_t* pstSendBufSize,
			ysize_t* pstRecvBufSize,
			yuint32_t* pstMaxConsumerBlocks,
			void* pExtData,
			yuint32_t* puGroupId,
			void** ppClientExt
		);

		static void _OnCloseClientAttr(
			YXC_NetSModelServer server,
			yuint32_t uGroupId,
			YXC_Socket socket,
			void* pSrvExt,
			void* pClientExt
		);

	private:
		friend class _NSMHelper;
		friend class NSMServerBaseHelper;

		void _RemoveGroup(yuint32_t uGroupId);

		YXC_Status _AddGroup(yuint32_t uGroupId, NSMGroupBase* pGroup);

	protected:
		typedef std::map<yuint32_t, NSMGroupBase*> _GroupMap;
		typedef std::map<yuint32_t, NSMGroupBase*>::iterator _GroupIter;

	protected:
		YXC_NetSModelServer _server;

		NSMServerBaseHelper* _hlp;

		YXC_Socket _serverSock;

	};
}

#endif /* __INC_YXC_SYS_BASE_NET_SMODEL_SERVER_HPP__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_SMODEL_SYNC_CLIENT_HPP__
#define __INC_YXC_SYS_BASE_NET_SMODEL_SYNC_CLIENT_HPP__

#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/YXC_NetSModelClient.h>
#include <YXC_Sys/YXC_PNCMP.h>

namespace YXCLib
{
	struct PResHdr
	{
		ybool_t bIsClosed;
		const YXC_Error* pError;
	};

	/* A Sync work mode net client, not thread safe. */
	class YXC_CLASS NCMSyncClient
	{
	public:
		NCMSyncClient();

		virtual ~NCMSyncClient();

	public:
		YXC_Status Init(const YXC_NetProtocol* pProtocol, ybyte_t byProtoType);

		YXC_Status InitAndConnect(const YXC_NetProtocol* pProtocol, ybyte_t byProtoType, const ychar* pszIp,
			yuint32_t uPort, ysize_t stmsTimeout);

		virtual void Close();

		virtual void CloseConnection(ybool_t bNotify);

		YXC_Status Connect(const ychar* pszIp, yuint32_t uPort, ysize_t stCbSendBuf, yuint32_t stCbBlocks, yuint32_t stmsTimeout);

		YXC_Status SendPackage(const YXC_NetPackage* pPackage, yuint32_t stmsTimeout);

		YXC_Status SendCommand(const void* pvData, yuint32_t uDataLen, yuint32_t stmsTimeout);

		YXC_Status SendCommandEx(const void* pvData, yuint32_t uDataLen, yuint32_t uNumHeaders, const YXC_Buffer* pExtHeaders, ysize_t stmsTimeout);

		YXC_Status RecvResponse(const void** pvData, yuint32_t* puDataLen, yuint32_t stmsTimeout);

		void ReleaseResponse(const void* pData);

		void ClearPool();

	public:
		inline ybool_t IsAvailable() { return this->_cli != NULL; }

	protected:
		virtual void OnNetClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pGExt, ybool_t bClosedActively);

		virtual void OnNetReceive(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage,
			void* pGExt);

	private:
		static void _OnNetReceive(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage,
			void* pGExt, void* pCExt);

		static void _OnNetClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pGExt, void* pCExt,
			ybool_t bClosedActively);

	protected:
		YXC_NetSModelClientMgr _mgr;
		YXC_NetSModelClient _cli;

		YXC_PNCMP _netDataPool;
		YXC_PNCMPProducer _netDataProducer;
		YXC_PNCMPConsumer _netDataConsumer;

		YXC_NetProtocol _protocol;
		ybyte_t _byProtoType;
	};
}

#endif /* __INC_YXC_SYS_BASE_NET_SMODEL_SYNC_CLIENT_HPP__ */

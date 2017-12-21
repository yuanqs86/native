#define __MODULE__ "ED.NSM.SyncClient"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_NCMSyncClient.hpp>
#include <YXC_Sys/YXC_Locker.hpp>

namespace YXCLib
{
	NCMSyncClient::NCMSyncClient() : _mgr(NULL), _cli(NULL), _netDataPool(NULL),
		_netDataProducer(NULL), _netDataConsumer(NULL), _byProtoType(0)
	{

	}

	NCMSyncClient::~NCMSyncClient()
	{
		this->Close();
	}

	YXC_Status NCMSyncClient::Init(const YXC_NetProtocol* protocol, ybyte_t byProtoType)
	{
		YXC_Status rc = YXC_NetSModelClientMgrCreate(3, 3, this, &this->_mgr);
		_YXC_CHECK_STATUS_RET(rc, YC("Create client manager failed"));

		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		rc = YXC_PNCMPCreate(30, 1, _YXC_PNCMPMMParam(YXC_MM_ALLOCATOR_TYPE_FIXED, 2 << 20), NULL, &this->_netDataPool);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Create net data pool failed"));
		rc = YXC_PNCMPConsumerAttach(this->_netDataPool, 0, 0, &this->_netDataConsumer);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Create net data consumer failed"));
		rc = YXC_PNCMPProducerAttachEx(this->_netDataPool, YXC_INFINITE, TRUE, FALSE, &this->_netDataProducer);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Create net data producer failed"));

		this->_protocol = protocol == NULL ? YXC_NPGetBaseProtocol() : *protocol;
		this->_byProtoType = byProtoType;
		return YXC_ERC_SUCCESS;
err_ret:
		this->Close();
		return rcRet;
	}

	void NCMSyncClient::CloseConnection(ybool_t bNotify)
	{
		if (this->_mgr && this->_cli)
		{
			if (bNotify)
			{
				YXC_NetSModelClientNotifyClose(this->_mgr, this->_cli);
			}
			else
			{
				YXC_Event event = NULL;
				YXC_NetSModelClientCloseEx(this->_mgr, this->_cli, &event);
				if (event != NULL)
				{
					YXC_WaitSingleKObject(event);
					YXC_EventDestroy(event);
				}
				YXC_PNCMPClearPool(this->_netDataPool);
				this->_cli = NULL;
			}
		}
	}

	void NCMSyncClient::Close()
	{
		if (this->_netDataProducer)
		{
			YXC_PNCMPProducerOptVal producerOpt;
			producerOpt.bWaitForConsumer = FALSE;
			YXC_PNCMPProducerSetOption(this->_netDataProducer, YXC_PNCMP_PRODUCER_OPTION_WAIT_FOR_CONSUMER, &producerOpt);
		}

		if (this->_mgr)
		{
			YXC_NetSModelClientMgrDestroy(this->_mgr);
			this->_mgr = NULL;
			this->_cli = NULL;
		}

		/* Net thread will be destroyed, so no any use of data consumer and producer, here, destroy it. */
		if (this->_netDataPool)
		{
			YXC_PNCMPForceDestroy(this->_netDataPool);
			this->_netDataPool = NULL;
			this->_netDataProducer = NULL;
			this->_netDataConsumer = NULL;
		}
	}

	YXC_Status NCMSyncClient::Connect(const ychar* pszIp, yuint32_t uPort, ysize_t stCbSendBuf, yuint32_t stCbBlocks, yuint32_t stmsTimeout)
	{
		YXC_Socket sock;
		YXC_Status rc = YXC_SockConnectToServer(YXC_SOCKET_TYPE_TCP, pszIp, uPort, (yuint32_t)stmsTimeout, &sock);
		_YXC_CHECK_STATUS_RET(rc, YC("Connect to server %s:%d failed"), pszIp, uPort);

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_NetSModelClientCbkArgs args = {0};
		YXC_SocketOptValue optVal;
		optVal.sockProtocol = this->_protocol;
		rc = YXC_SockSetOption(sock, YXC_SOCKET_OPT_NET_PROTOCOL, &optVal);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Set net protocol failed"));

		args.pCExt = this;
		args.pfnOnClose = _OnNetClose;
		args.pfnOnRecv = _OnNetReceive;

		rc = YXC_NetSModelClientCreateEx(this->_mgr, sock, &args, stCbBlocks, stCbSendBuf, 1 << 16, &this->_cli);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Create select model client failed"));

		return YXC_ERC_SUCCESS;
err_ret:
		YXC_SockClose(sock);
		return rcRet;
	}

	YXC_Status NCMSyncClient::InitAndConnect(const YXC_NetProtocol* pProtocol, ybyte_t byProtoType, const ychar* pszIp,
		yuint32_t uPort, ysize_t stmsTimeout)
	{
		YXC_Status rc = this->Init(pProtocol, byProtoType);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		rc = this->Connect(pszIp, uPort, 6 << 20, 30, stmsTimeout);
		if (rc != YXC_ERC_SUCCESS)
		{
			this->Close();
			return rc;
		}

		YXC_PNCMPClearPool(this->_netDataPool);
		return YXC_ERC_SUCCESS;
	}

	void NCMSyncClient::ClearPool()
	{
		YXC_PNCMPClearPool(this->_netDataPool);
	}

	YXC_Status NCMSyncClient::SendCommand(const void* pvData, yuint32_t uDataLen, yuint32_t stmsTimeout)
	{
		return SendCommandEx(pvData, uDataLen, 0, NULL, stmsTimeout);
	}

	YXC_Status NCMSyncClient::SendPackage(const YXC_NetPackage* pPackage, yuint32_t stmsTimeout)
	{
		YXC_Status rc = YXC_NetSModelClientSend(this->_mgr, this->_cli, pPackage, stmsTimeout);
		if (rc == YXC_ERC_INVALID_HANDLE) /* Invalid handle, maybe closed by server. */
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_NET_SHUTDOWN, YC("Client is disconnected, handle(%d) is reset"), this->_cli);
		}
		_YXC_CHECK_STATUS_RET(rc, YC("Send package failed, length(%d), ext headers(%ld)"), pPackage->uDataLen, pPackage->stNumExtHeaders);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status NCMSyncClient::SendCommandEx(const void* pvData, yuint32_t uDataLen, yuint32_t uNumHeaders,
		const YXC_Buffer* pExtHeaders, ysize_t stmsTimeout)
	{
		YXC_NetPackage npPackage;
		YXC_NPStdCreatePackageEx(&this->_protocol, uDataLen, pvData, this->_byProtoType, FALSE,
			uNumHeaders, pExtHeaders, &npPackage);

		return this->SendPackage(&npPackage, stmsTimeout);
	}

	void NCMSyncClient::ReleaseResponse(const void* pData)
	{
		YXC_PNCMPConsumerUnreferenceBlock(this->_netDataConsumer, pData);
	}

	void NCMSyncClient::OnNetClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pGExt, ybool_t bClosedActively)
	{
		if (!bClosedActively)
		{
			ybyte_t* pBuffer = NULL;
			YXC_PNCMPClearPool(this->_netDataPool);
			YXC_Status rc = YXC_PNCMPMMAlloc(this->_netDataPool, YXC_BASE_ERROR_BUFFER, sizeof(PResHdr), (void**)&pBuffer);
			if (rc == YXC_ERC_SUCCESS)
			{
				PResHdr* pErrHdr = (PResHdr*)(pBuffer + YXC_BASE_ERROR_BUFFER);
				const YXC_Error* pErrBuf = YXC_CopyLastError(YXC_BASE_ERROR_BUFFER, pBuffer, NULL);
				pErrHdr->pError = pErrBuf;
				pErrHdr->bIsClosed = TRUE;

				YXC_PNCMPProducerMMPush(this->_netDataProducer, TRUE, pBuffer, NULL, 0);
				return;
			}
		}
	}

	void NCMSyncClient::_OnNetClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pGExt, void* pCExt, ybool_t bClosedActively)
	{
		NCMSyncClient* pNetClient = (NCMSyncClient*)pCExt;
		pNetClient->OnNetClose(mgr, client, pGExt, bClosedActively);
	}

	void NCMSyncClient::OnNetReceive(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage, void* pGExt)
	{
		PResHdr resHdr = { FALSE, NULL };
		YXC_PNCMPProducerPushBlockEx(this->_netDataProducer, pPackage->pvData, pPackage->uDataLen, &resHdr, sizeof(PResHdr), 0);
	}

	void NCMSyncClient::_OnNetReceive(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage, void* pGExt, void* pCExt)
	{
		NCMSyncClient* pNetClient = (NCMSyncClient*)pCExt;
		pNetClient->OnNetReceive(mgr, client, pPackage, pGExt);
	}

	YXC_Status NCMSyncClient::RecvResponse(const void** pvData, yuint32_t* puDataLen, yuint32_t stmsTimeout)
	{
		const PResHdr* pResHdr = NULL;
		const void* pData = NULL;
		ysize_t stHdrLen = 0, stCbData = 0;

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = YXC_PNCMPConsumerPullBlockEx(this->_netDataConsumer, &pData, &stCbData, (const void**)&pResHdr, &stHdrLen, stmsTimeout);
		_YXC_CHECK_RC_RETP(rc);

		const YXC_Error* pError = pResHdr->pError;
		_YXC_CHECK_REPORT_GOTO_EX(pResHdr->bIsClosed == FALSE, pError->errSrc, 0, pError->llErrCode, YXC_ERC_NET_SHUTDOWN,
			YC("Net connection is broken, reason : %s"), pError->wszMsg);

		*pvData = pData;
		*puDataLen = (yuint32_t)stCbData;
		return YXC_ERC_SUCCESS;
err_ret:
		this->ReleaseResponse(pData);
		return rcRet;
	}
}

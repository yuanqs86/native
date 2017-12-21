#define __MODULE__ "EK.Net.SModel.Client"

#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClient.hpp>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>

namespace YXC_Inner
{
	void _NetSModelClient::CheckConnnected()
	{
		if (!this->_bConnected)
		{
			if (this->_pfnConnect)
			{
				this->_pfnConnect((YXC_NetSModelClientMgr)this->_pMgr, (YXC_NetSModelClient)this->_upHandle,
					this->_pMgrExt, this->_pExt);
			}
			this->_bConnected = TRUE;
		}
	}

	void _NetSModelClient::NotifyToClose()
	{
		this->_bNotifyClosed = TRUE;
	}

	YXC_Status _NetSModelClient::CreateBase(const YXC_NetSModelClientCbkArgs* pCbkArgs, yuint32_t stMaxNumSendBlocks, ysize_t stSendBufferSize,
		ysize_t stRecvBufferSize)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		_YXC_CHECK_REPORT_NEW_RET(!this->_bCreated, YXC_ERC_NET_SRV_CLIENT_ALREADY_INITED, YC("The client has already been initialized"));

		this->_sendPool = NULL;
		this->_consumer = NULL;
		this->_producer = NULL;
		this->_recvInfo.pBuffer = NULL;
		this->_eveClose = NULL;
        this->_event = NULL;

		this->_pfnClose = pCbkArgs->pfnOnClose;
		this->_pfnRecv = pCbkArgs->pfnOnRecv;
		this->_pfnNoData = pCbkArgs->pfnOnNoData;
		this->_pfnDestroy = pCbkArgs->pfnOnDestroy;
		this->_pfnConnect = NULL;

		YXC_SocketOptValue opt;
        YXC_Status rc;

		/* Reserve three blocks for timeout waiting, 1-using, 1-transferring, 1-remote, sure to alloc block. */
		YXC_Status rcSendPool = YXC_PNCMPCreate(stMaxNumSendBlocks + 3, 1, _YXC_PNCMPMMParam(YXC_MM_ALLOCATOR_TYPE_FLAT, stSendBufferSize), NULL, &this->_sendPool);
		_YXC_CHECK_GOTO(rcSendPool == YXC_ERC_SUCCESS, rcSendPool);

		rcSendPool = YXC_PNCMPProducerAttachEx(this->_sendPool, 0, TRUE, FALSE, &this->_producer);
		_YXC_CHECK_GOTO(rcSendPool == YXC_ERC_SUCCESS, rcSendPool);

		rcSendPool = YXC_PNCMPConsumerAttach(this->_sendPool, 0, stMaxNumSendBlocks, &this->_consumer);
		_YXC_CHECK_GOTO(rcSendPool == YXC_ERC_SUCCESS, rcSendPool);

        rc = YXC_SockGetOption(this->_socket, YXC_SOCKET_OPT_SOCKET_VALUE, &opt);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Can't get socket value"));

		this->_rSock = opt.rSock;
		this->_bClosed = FALSE;
		this->_bNotifyClosed = FALSE;

		this->_recvInfo.stBufferSize = stRecvBufferSize;
		this->_recvInfo.pBuffer = (ybyte_t*)malloc(stRecvBufferSize);
		_YXC_CHECK_REPORT_NEW_GOTO(this->_recvInfo.pBuffer != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Can't create receive buffer"));

		rc = YXC_EventCreate(FALSE, FALSE, &this->_eveClose);
		_YXC_CHECK_OS_GOTO(rc == YXC_ERC_SUCCESS, YC("Create close event for client failed"));

		rc = YXC_SockEventCreate(this->_socket, &this->_event);
		_YXC_CHECK_RC_GOTO(rc);
		this->_lEvent = FD_CLOSE | FD_READ;

		YXC_SockEventSelect(this->_socket, this->_event, this->_lEvent);

		this->_bClosed = FALSE;
		this->_bCreated = TRUE;
		this->_uRefCount = 0;
		this->_rSock = opt.rSock;
		this->_crit.Init(4000);
		this->_stSendTimeout = 0;
		this->_pfnNoData = pCbkArgs->pfnOnNoData;
		this->_uNoDataMaxTime = (yuint32_t)pCbkArgs->stTimeoutOnNoData;
		this->_pExt = pCbkArgs->pCExt;
		this->_uNoDataTime = 0;

		memset(&this->_sentInfo, 0, sizeof(this->_sentInfo));
		_InitTransferInfo(_SockPtr(this->_socket), NULL, &this->_recvInfo.transInfo);

		return YXC_ERC_SUCCESS;

err_ret:
		if (this->_sendPool != NULL) YXC_PNCMPForceDestroy(this->_sendPool);
		if (this->_recvInfo.pBuffer != NULL)
		{
			free(this->_recvInfo.pBuffer);
			this->_recvInfo.pBuffer = NULL;
		}
		if (this->_eveClose != NULL) YXC_EventDestroy(this->_eveClose);
		return rcRet;
	}

	YXC_Status _NetSModelClient::Create(const YXC_NetSModelClientCbkArgs* pCbkArgs, YXC_PNCMP memPool, yuint32_t stMaxConsumerQueue,
		ysize_t stRecvBufferSize, yuint32_t stMaxNumSendBlocks, ysize_t stSendBufferSize)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		YXC_Status rc = this->CreateBase(pCbkArgs, stMaxNumSendBlocks, stSendBufferSize, stRecvBufferSize);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		this->_mcConsumer = NULL;
		// sure pointers and pool inited before input block check funcs.
		YXC_Status rc2 = YXC_PNCMPConsumerAttachEx(memPool, 0, stMaxConsumerQueue, NULL,
			FALSE, NULL, &this->_mcConsumer);
		_YXC_CHECK_REPORT_GOTO(rc2 == YXC_ERC_SUCCESS, rc2, YC("Attach consumer to send pool failed"));

		this->_mcPool = memPool;

		return YXC_ERC_SUCCESS;
err_ret:
		if (this->_mcConsumer != NULL) YXC_PNCMPConsumerDetach(memPool, this->_mcConsumer);
		this->DestroyBase(FALSE);
		return rcRet;
	}

	YXC_Status _NetSModelClient::Create(const YXC_NetSModelClientCbkArgs* pCbkArgs, yuint32_t stMaxNumSendBlocks,
		ysize_t stSendBufferSize, ysize_t stRecvBufferSize)
	{
		this->_mcConsumer = NULL;
		this->_mcPool = NULL;
		YXC_Status rc = this->CreateBase(pCbkArgs, stMaxNumSendBlocks, stSendBufferSize, stRecvBufferSize);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		return YXC_ERC_SUCCESS;
	}

	void _NetSModelClient::SetBlockCheck(YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck)
	{
		if (this->_mcConsumer)
		{
			YXC_PNCMPConsumerSetBlockCheck(this->_mcConsumer, pfnBlockCheck, this);
		}
	}

	void _NetSModelClient::Destroy(ybool_t bCloseSock)
	{
		/* First detach the consumer, prevent calling of OnCheckBlock callback */
		if (this->_mcPool != NULL)
		{
			YXC_PNCMPConsumerDetach(this->_mcPool, this->_mcConsumer);
		}

		if (this->_sendPool != NULL)
		{
			YXC_PNCMPForceDestroy(this->_sendPool);
		}

		if (this->_recvInfo.pBuffer != NULL)
		{
			free(this->_recvInfo.pBuffer);
		}
		this->DestroyBase(bCloseSock);
	}

	void _NetSModelClient::DestroyBase(ybool_t bCloseSock)
	{
        YXC_SockEventDestroy(this->_event);

		YXC_EventSet(this->_eveClose);
		YXC_EventDestroy(this->_eveClose);
		this->_crit.Close();

		if (bCloseSock)
		{
			YXC_SockClose(this->_socket);
		}

		this->_bCreated = FALSE;
	}

	YXC_Status _NetSModelClient::ReceiveData()
	{
		ysize_t stNeeded;

		int iMaxNum = 5;
		while (--iMaxNum >= 0 && !this->_bClosed)
		{
			YXC_Status rc = YXC_SockReceivePackage(this->_socket, this->_recvInfo.stBufferSize, this->_recvInfo.pBuffer, &stNeeded,
				&this->_recvInfo.package, &this->_recvInfo.transInfo);

			if (rc == YXC_ERC_SUCCESS)
			{
				if (this->_pfnRecv != NULL)
				{
					this->_pfnRecv((YXC_NetSModelClientMgr)this->_pMgr, (YXC_NetSModelClient)this->_upHandle, &this->_recvInfo.package,
						this->_pMgrExt, this->_pExt);
				}
				_InitTransferInfo(_SockPtr(this->_socket), NULL, &this->_recvInfo.transInfo);
			}
			else if (rc == YXC_ERC_BUFFER_NOT_ENOUGH) /* Recvd out of buffer. */
			{
				void* pBuffer = ::malloc(stNeeded);
				_YXC_CHECK_OS_RET(pBuffer != NULL, YC("Failed to alloc for more net buffer(%d), lost package."), stNeeded);
                memcpy(pBuffer, this->_recvInfo.pBuffer, this->_recvInfo.transInfo.stDataTransferred);
				free(this->_recvInfo.pBuffer);
				this->_recvInfo.pBuffer = (ybyte_t*)pBuffer;
				this->_recvInfo.stBufferSize = stNeeded;
				continue;
			}

            if (rc != YXC_ERC_SOCK_WOULD_BLOCK) return rc;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelClient::SendPackage(const YXC_NetPackage* pPackage, yuint32_t stTimeout)
	{
		ybyte_t byBuffer[sizeof(YXC_NetPackage) + sizeof(YXC_NetTransferInfo)];
		YXC_NetTransferInfo* pTransInfo = (YXC_NetTransferInfo*)byBuffer;

		_Socket* pSocket = _SockPtr(this->_socket);
		YXC_NPStdInitTransferInfo(&pSocket->protocol, pPackage, pTransInfo);

		YXC_NetPackage* pPackage2 = (YXC_NetPackage*)(byBuffer + sizeof(YXC_NetTransferInfo));
		memcpy(pPackage2->byHdrBuffer, pPackage->byHdrBuffer, pTransInfo->stTotalHeaderSize);
		ysize_t stTotal = pTransInfo->stTotalHeaderSize + sizeof(YXC_NetTransferInfo);

		if (this->_stSendTimeout != stTimeout)
		{
			YXC_PNCMPProducerOptVal optVal;
			optVal.stTimeoutInMilliSeconds = stTimeout;
			YXC_Status rc = YXC_PNCMPProducerSetOption(this->_producer, YXC_PNCMP_PRODUCER_OPTION_WAIT_CONSUMER_TIMEOUT, &optVal);

			if (rc == YXC_ERC_SUCCESS) this->_stSendTimeout = stTimeout;
		}

		yuint32_t uLen = pPackage->uDataLen - (yuint32_t)pTransInfo->stExtHeaderSize;

		ybyte_t* pAllocated;
		YXC_Status rc = YXC_PNCMPMMAlloc(this->_sendPool, uLen, stTotal, (void**)&pAllocated);
		if (rc == YXC_ERC_SUCCESS)
		{
			/* put transfer info and package header before package data. */
			memcpy(pAllocated + uLen, byBuffer, stTotal);
			ysize_t uOffset = 0;
			/* merge package contents and data */
			for (yuint32_t i = 0; i < pPackage->stNumExtContents; ++i)
			{
				memcpy(pAllocated + uOffset, pPackage->extContents[i].pBuffer, pPackage->extContents[i].stBufSize);
				uOffset += pPackage->extContents[i].stBufSize;
			}
			memcpy(pAllocated + uOffset, pPackage->pvData, uLen - uOffset);

			rc = YXC_PNCMPProducerMMPush(this->_producer, TRUE, pAllocated, NULL, 0);
			if (rc == YXC_ERC_SUCCESS)
			{
				YXCLib::Locker<YXX_Crit> locker(this->_crit);
				this->_GetSendDataFromPool();
                YXC_SockEventSet(this->_event);
				this->EnableSockEventVal(TRUE, FD_WRITE);
			}
		}
		return rc;
	}

	YXC_Status _NetSModelClient::_GetSendDataFromPool()
	{
		if (this->_sentInfo.pData == NULL)
		{
			const void* pBuffer = NULL, *pData = NULL;
			ysize_t stBufSize = 0, stExBuffer = 0;
			YXC_Status rc = YXC_PNCMPConsumerPullBlockEx(this->_consumer, &pData, &stBufSize, &pBuffer, &stExBuffer, 0);
			if (rc == YXC_ERC_SUCCESS)
			{
				this->_sentInfo.consumer = this->_consumer;
			}

			if (rc != YXC_ERC_SUCCESS && this->_mcConsumer != NULL) // try get from mc pool
			{
				rc = YXC_PNCMPConsumerPullBlockEx(this->_mcConsumer, &pData, &stBufSize, &pBuffer, &stExBuffer, 0);
				if (rc == YXC_ERC_SUCCESS)
				{
					this->_sentInfo.consumer = this->_mcConsumer;
				}
			}

			if (rc == YXC_ERC_SUCCESS)
			{
				this->_sentInfo.pData = (const ybyte_t*)pData;

				memcpy(&this->_sentInfo.transInfo, (const ybyte_t*)pBuffer, sizeof(YXC_NetTransferInfo));
				memcpy(&this->_sentInfo.package.byHdrBuffer, (const ybyte_t*)pBuffer + sizeof(YXC_NetTransferInfo),
					stExBuffer - sizeof(YXC_NetTransferInfo));
				this->_sentInfo.package.pvData = (ybyte_t*)pData; // trick here
				this->_sentInfo.package.uDataLen = (yuint32_t)(stBufSize + this->_sentInfo.transInfo.stExtHeaderSize);
				this->_sentInfo.package.stNumExtHeaders = 0; // no use here
				this->_sentInfo.package.stNumExtContents = 0;
			}
			return rc;
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelClient::GetSendDataFromPool()
	{
		YXX_CritLocker locker(this->_crit);

		return this->_GetSendDataFromPool();
	}

	YXC_Status _NetSModelClient::GetMulticastData()
	{
		YXX_CritLocker locker(this->_crit);

		YXC_Status rc = this->_GetSendDataFromPool();
		if (rc == YXC_ERC_SUCCESS)
		{
			this->EnableSockEventVal(TRUE, FD_WRITE);
			YXC_SockEventSet(this->_event);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelClient::SendBufferedData()
	{
		YXX_CritLocker locker(this->_crit);
		YXC_Status rc = YXC_ERC_SUCCESS;

		while (this->_sentInfo.pData != NULL)
		{
			rc = YXC_SockSendPackage(this->_socket, &this->_sentInfo.package, &this->_sentInfo.transInfo);

			if (rc != YXC_ERC_SUCCESS) break;

			YXC_PNCMPConsumerUnreferenceBlock(this->_sentInfo.consumer, this->_sentInfo.pData);

			memset(&this->_sentInfo, 0, sizeof(this->_sentInfo));

			this->_GetSendDataFromPool();
		}

		if (this->_sentInfo.pData == NULL)
		{
			this->EnableSockEventVal(FALSE, FD_WRITE);
		}

		return rc;
	}

	void _NetSModelClient::DropBlocks(ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec, void* pDropCtrl,
		yuint32_t* pstNumBlocksDropped)
	{
		if (bMCBlock)
		{
			YXC_PNCMPConsumerDropBlocks(this->_mcConsumer, pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
		}
		else
		{
			YXC_PNCMPConsumerDropBlocks(this->_consumer, pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
		}
	}
}

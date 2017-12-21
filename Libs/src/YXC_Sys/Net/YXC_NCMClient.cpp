#include <YXC_Sys/YXC_NCMClient.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <stdio.h>

namespace YXCLib
{
	NCMClient::NCMClient() : _client(NULL), _mgr(NULL)
	{
	}

	NCMClient::~NCMClient()
	{
		this->Close(TRUE);
	}

	void NCMClient::Close(ybool_t bWaitForClosed)
	{
		if (this->_mgr != NULL)
		{
			if (!bWaitForClosed)
			{
				::YXC_NetSModelClientClose(this->_mgr, this->_client);
			}
			else
			{
				YXC_Event upWaitEvent = NULL;
				YXC_Status rc = ::YXC_NetSModelClientCloseEx(this->_mgr, this->_client, &upWaitEvent);

				if (rc == YXC_ERC_SUCCESS && upWaitEvent != NULL)
				{
                    YXC_EventLock(upWaitEvent);
                    YXC_EventDestroy(upWaitEvent);
				}
			}

			this->_mgr = NULL;
		}
	}

	YXC_Status NCMClient::SendPackage(const YXC_NetPackage* pPackage, yuint32_t stTimeout)
	{
		return ::YXC_NetSModelClientSend(this->_mgr, this->_client, pPackage, stTimeout);
	}

	YXC_Status NCMClient::GetSocketOpt(YXC_SocketOption option, YXC_SocketOptValue* pOptVal)
	{
		return ::YXC_NetSModelClientGetSockOpt(this->_mgr, this->_client, option, pOptVal);
	}

	YXC_Status NCMClient::SetSocketOpt(YXC_SocketOption option, const YXC_SocketOptValue* pOptVal)
	{
		return ::YXC_NetSModelClientSetSockOpt(this->_mgr, this->_client, option, pOptVal);
	}

	void NCMClient::OnClientClose(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pExtData,
		void* pClientExt, ybool_t bClosedActively)
	{
		NCMClient* pClient = (NCMClient*)pClientExt;
		YXC_NetSModelClient clientHdl = (YXC_NetSModelClient)YXCLib::Interlocked::ExchangeAddPtr((void* volatile*)&pClient->_client, 0);

		// If the closed client is not current client, this means user connects again, so don't do callback
		if (clientHdl == client) // Closed client is current client
		{
			if (!bClosedActively)
			{
				ybyte_t byBuffer[YXC_BASE_ERROR_BUFFER], *pbyBuffer = byBuffer;
				ysize_t stNeeded;
				const YXC_Error* pError = ::YXC_CopyLastError(YXC_BASE_ERROR_BUFFER, pbyBuffer, &stNeeded);
				if (pError == NULL && stNeeded > YXC_BASE_ERROR_BUFFER)
				{
					void* pData = ::malloc(stNeeded);
					if (pData == NULL) return;

					pbyBuffer = (ybyte_t*)pData;
					pError = ::YXC_CopyLastError(stNeeded, pbyBuffer, &stNeeded);
				}

				pClient->_OnClose(FALSE, pError);
				if (pbyBuffer != byBuffer) free(pbyBuffer);
			}
			else
			{
				pClient->_OnClose(TRUE, NULL);
			}

			YXCLib::Interlocked::ExchangePtr<void>((void* volatile*)&pClient->_client, NULL);
		}
	}

	void NCMClient::OnClientRecv(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage,
		void* pExtData, void* pClientExt)
	{
		NCMClient* pClient = (NCMClient*)pClientExt;
		pClient->_OnReceive(pPackage);
	}

	YXC_Status NCMClient::ConnectAndCreateA(YXC_NetSModelClientMgr cliMgr, const char* cszIp, yuint32_t uPort, yuint32_t uTimeout)
	{
		if (this->_mgr == NULL)
		{
			// Connect, even if the previous socket is not closed
			YXC_Socket socket;
			YXC_Status rc = YXC_SockConnectToServerA(YXC_SOCKET_TYPE_TCP, cszIp, uPort, uTimeout, &socket);

			if (rc != YXC_ERC_SUCCESS) return rc;

			rc = this->_OnInitSockAttr(socket);
			if (rc != YXC_ERC_SUCCESS) return rc;

			return this->_AttachSocket(cliMgr, socket);
		}

		return YXC_ERC_ALREADY_CREATED;
	}

    YXC_Status NCMClient::ConnectAndCreate(YXC_NetSModelClientMgr cliMgr, const ychar* cwszIp, yuint32_t uPort, yuint32_t uTimeout)
    {
#if YCHAR_WCHAR_T
        return ConnectAndCreateW(cliMgr, cwszIp, uPort, uTimeout);
#else
        return ConnectAndCreateA(cliMgr, cwszIp, uPort, uTimeout);
#endif /* YCHAR_WCHAR_T */
    }

	YXC_Status NCMClient::ConnectAndCreateW(YXC_NetSModelClientMgr cliMgr, const wchar_t* cwszIp, yuint32_t uPort, yuint32_t uTimeout)
	{
		char szIp[32] = {0};
		sprintf(szIp, "%ls", cwszIp);

		return this->ConnectAndCreateA(cliMgr, szIp, uPort, uTimeout);
	}

	YXC_Status NCMClient::_AttachSocket(YXC_NetSModelClientMgr cliMgr, YXC_Socket sock)
	{
		YXC_NetSModelClient client;
		YXC_NetSModelClientCbkArgs cbkArgs = { OnClientClose, OnClientRecv, NULL, NULL, 0, this };
		YXC_Status rc = YXC_NetSModelClientCreate(cliMgr, sock, &cbkArgs, &client);
		if (rc == YXC_ERC_SUCCESS)
		{
			this->_mgr = cliMgr;
			YXCLib::Interlocked::ExchangePtr<void>((void* volatile*)&this->_client, client);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status NCMClient::AttachSocket(YXC_NetSModelClientMgr cliMgr, YXC_Socket sock)
	{
		if (this->_mgr == NULL)
		{
			return this->_AttachSocket(cliMgr, sock);
		}

		return YXC_ERC_ALREADY_CREATED;
	}

	YXC_Status NCMClient::_OnInitSockAttr(YXC_Socket sock)
	{
		return YXC_ERC_SUCCESS;
	}

	ybool_t NCMClient::IsConnected() const
	{
		YXC_NetSModelClient clientHdl = (YXC_NetSModelClient)YXCLib::Interlocked::ExchangeAddPtr((void* volatile*)&this->_client, 0);

		return clientHdl != NULL;
	}

	void NCMClient::_OnClose(ybool_t bClosedActively, const YXC_Error* pClosedReason)
	{

	}

	void NCMClient::_OnReceive(const YXC_NetPackage* pPackage)
	{

	}

	void NCMClient::NotifyClose()
	{
		YXC_NetSModelClient clientHdl = (YXC_NetSModelClient)YXCLib::Interlocked::ExchangeAddPtr((void* volatile*)&this->_client, 0);

		if (clientHdl != NULL)
		{
			YXC_NetSModelClientNotifyClose(this->_mgr, this->_client);
		}
	}
}

#define __MODULE__ "EK.Net.SModel.ClientMgr"

#include <limits.h>
#include <assert.h>
#include <stdio.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClientThrMgr.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClientMgr.hpp>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>

#if YXC_PLATFORM_WIN
#include <process.h>
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
	yint32_t _NetSModelClientThrMgr::_FindClientIndex(_NetSModelClient* pClient)
	{
		for (yuint32_t i = 0; i < this->_uNumClients; ++i)
		{
			if (this->_pClients[i] == pClient)
			{
				return i;
			}
		}

		return -1;
	}

	void _NetSModelClientThrMgr::_RemoveIndex(yuint32_t uIndex)
	{
		memmove(this->_pClients + uIndex, this->_pClients + uIndex + 1, sizeof(_NetSModelClient*) * (this->_uNumClients - uIndex - 1));
		memmove(this->_lEvents + uIndex, this->_lEvents + uIndex + 1, sizeof(YXC_SocketEvent) * (this->_uNumClients - uIndex - 1));

		for (ysize_t j = 0; j < this->_uMaxClientsInThread * _YXC_CLI_HANDLE_RATIO; ++j)
		{
			ybyte_t& byIndex = this->_byHandleIndecies[j];
			if (byIndex == uIndex) byIndex = -1;
			else if (byIndex != 0xFF && byIndex > uIndex) --byIndex; // move map to previous
		}

		this->_pMgr->DecreaseClient();
		if (--this->_uNumClients == 0)
		{
			if (!this->_bClosed)
			{
				this->_teThread.Reset();
			}
		}
	}

	ybool_t _NetSModelClientThrMgr::AddClient(_NetSModelClient* pClient, yuintptr_t upBase, yuintptr_t* pClientHandle)
	{
		YXX_CritLocker locker(this->_crit);

		if (this->_uNumClients >= this->_uMaxClientsInThread) return FALSE;

		this->_pClients[this->_uNumClients] = pClient;
		this->_lEvents[this->_uNumClients] = pClient->GetSockEvent();

		if (this->_uNumClients == 0)
		{
			this->_teThread.Set();
		}

		yuintptr_t upLastHandle = this->_upLastHandle;
		for (yuintptr_t i = upLastHandle + 1; i != upLastHandle; ++i)
		{
			if (i == _YXC_CLI_HANDLE_RATIO * this->_uMaxClientsInThread) i = 0;

			if (this->_byHandleIndecies[i] == 0xFF)
			{
				pClient->SetHandle(i + upBase);
				*pClientHandle = i + upBase;
				this->_byHandleIndecies[i] = this->_uNumClients;
				this->_upLastHandle = i;
				_YXC_KREPORT_VERBOSE(YC("Connected new client, handle(%d), ptr(0x%p)"), i, pClient);
				this->_pMgr->IncreaseClient();
				break;
			}
		}

		++this->_uNumClients;

		return TRUE;
	}

	void _NetSModelClientThrMgr::CloseAndRemoveClient(_NetSModelClient* pClient)
	{
		pClient->NotifyClosed(); // Marked as no data will arrived later, prevent of dead lock
		pClient->SetBlockCheck(NULL);
		pClient->CallCloseCallback(FALSE);

		this->_crit.Lock();
		yint32_t iIndex = this->_FindClientIndex(pClient);
		if (iIndex != -1)
		{
			_YXC_KREPORT_INFO(YC("Client is closed passively, index(%d), mgr_ptr(0x%p), ptr(0x%p)."), iIndex, this, pClient);
			this->_RemoveIndex((yuint32_t)iIndex);
		}

		this->_crit.Unlock();

		pClient->Close();
	}

	void _NetSModelClientThrMgr::CloseAndRemoveClientByUser(_NetSModelClient* pClient, YXC_Event* pWaitHandle)
	{
		pClient->SetBlockCheck(NULL);
		pClient->CallCloseCallback(TRUE);

		YXX_CritLocker locker(this->_crit);
		yint32_t iIndex = this->_FindClientIndex(pClient);
		if (iIndex != -1)
		{
			this->_RemoveIndex((yuint32_t)iIndex);
			_YXC_KREPORT_VERBOSE(YC("User close client actively, index(%d), mgr_ptr(0x%p), cli_ptr(0x%p), wait_hdl(0x%p)."), iIndex, this, pClient, pWaitHandle);
		}

		if (OSGetCurrentThreadId() == this->_uSendThreadId) // in call back function, same thread
		{
			pClient->Close();
			pClient->Unreference();
			return;
		}

		// make sure for client really closed if user closed it.
		pClient->Close();
		if (pWaitHandle == NULL)
		{
			pClient->Unreference();
		}
		else
		{
			pClient->DeReferenceWithWaitHandle(pWaitHandle);
		}
	}

	void _NetSModelClientThrMgr::SignalForSendDataArrived()
	{
		this->_tsSendData.Unlock();
	}

	YXC_Status _NetSModelClientThrMgr::Create(yuint32_t uMaxNumClients, ysize_t stStackSize, ysize_t stTimeout,
		_NetSModelClientMgr* pMgr)
	{
		this->_crit.Init(4000);
		this->_stTimeout = stTimeout;
		this->_uNumClients = 0;
		this->_uMaxClientsInThread = uMaxNumClients;

		this->_upRetrieveSendDataThread = 0;
		this->_upSendRecvThread = 0;
		this->_pMgr = pMgr;

		this->_upLastHandle = 0;
		memset(this->_byHandleIndecies, 0xFF, sizeof(this->_byHandleIndecies));

		YXC_Status rcRet = YXC_ERC_SUCCESS;

		ybool_t bEve = FALSE, bSem = FALSE;
		YXC_Status rc = this->_teThread.Create(TRUE, FALSE);
		_YXC_CHECK_RC_GOTO(rc);
		bEve = TRUE;
		rc = this->_tsSendData.Create(0);
		_YXC_CHECK_RC_GOTO(rc);
		bSem = TRUE;

        rc = YXC_SockEventCreate(NULL, &this->_lFinalizeEvent);
		_YXC_CHECK_RC_GOTO(rc);

		if (pMgr->IsMCMgr())
		{
			this->_upRetrieveSendDataThread = OSCreateThreadStack(_RetrieveSendDataThreadProc, 20 << 10, this, NULL);
			_YXC_CHECK_OS_GOTO(this->_upRetrieveSendDataThread != 0, YC("Create multicast receive thread failed"));
		}

		this->_upSendRecvThread = OSCreateThreadStack(_SendRecvThreadProc, stStackSize, this, &this->_uSendThreadId);
		_YXC_CHECK_OS_GOTO(this->_upSendRecvThread != 0, YC("Create multicast send thread failed"));

		this->_bClosed = FALSE;
		return YXC_ERC_SUCCESS;
err_ret:
		this->_bClosed = TRUE;
		this->_crit.Close();
		if (bEve)
		{
			this->_teThread.Set();
		}
		if (bSem)
		{
			this->_tsSendData.Unlock();
		}

        if (this->_lFinalizeEvent)
        {
            YXC_SockEventSet(this->_lFinalizeEvent);

            if (this->_upRetrieveSendDataThread != 0)
            {
                OSWaitThread(this->_upRetrieveSendDataThread);
                OSCloseThreadHandle(this->_upRetrieveSendDataThread);
            }
            if (this->_upSendRecvThread != 0)
            {
                OSWaitThread(this->_upSendRecvThread);
                OSCloseThreadHandle(this->_upSendRecvThread);
            }
            YXC_SockEventDestroy(this->_lFinalizeEvent);
        }

        if (bEve)
        {
            this->_teThread.Close();
        }

        if (bSem)
        {
            this->_tsSendData.Close();
        }

		return rcRet;
	}

	YXC_Status _NetSModelClientThrMgr::SendData(yuintptr_t uHandle, const YXC_NetPackage* pPackage, yuint32_t stTimeout)
	{
		_NetSModelClient* pClient = this->_TryRefClient(uHandle);
		_YXC_CHECK_RET(pClient != NULL, YXC_ERC_INVALID_HANDLE);

		YXC_Status rc = pClient->SendPackage(pPackage, stTimeout);
		pClient->Unreference();
		return rc;
	}

	YXC_Status _NetSModelClientThrMgr::GetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, YXC_SocketOptValue* pOptVal)
	{
		_NetSModelClient* pClient = this->_TryRefClient(uHandle);
		_YXC_CHECK_RET(pClient != NULL, YXC_ERC_INVALID_HANDLE);

		YXC_Status rc = YXC_SockGetOption(pClient->GetSocket(), option, pOptVal);
		pClient->Unreference();
		return rc;
	}

	YXC_Status _NetSModelClientThrMgr::SetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, const YXC_SocketOptValue* pOptVal)
	{
		_NetSModelClient* pClient = this->_TryRefClient(uHandle);
		_YXC_CHECK_RET(pClient != NULL, YXC_ERC_INVALID_HANDLE);

		YXC_Status rc = YXC_SockSetOption(pClient->GetSocket(), option, pOptVal);
		pClient->Unreference();
		return rc;
	}

	YXC_Status _NetSModelClientThrMgr::DropBlocks(yuintptr_t uHandle, ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl, yuint32_t* pstNumBlocksDropped)
	{
		_NetSModelClient* pClient = this->_TryRefClient(uHandle);
		_YXC_CHECK_RET(pClient != NULL, YXC_ERC_INVALID_HANDLE);

		pClient->DropBlocks(bMCBlock, pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
		pClient->Unreference();
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelClientThrMgr::CloseClient(yuintptr_t uHandle, YXC_Event* pWaitHandle)
	{
		_NetSModelClient* pClient = this->_TryRefClient(uHandle);
		_YXC_CHECK_RET(pClient != NULL, YXC_ERC_INVALID_HANDLE);

		this->CloseAndRemoveClientByUser(pClient, pWaitHandle);
		// pClient->Unreference(); don't Unreference here, will be closed internal.
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _NetSModelClientThrMgr::NotifyCloseClient(yuintptr_t uHandle)
	{
		_NetSModelClient* pClient = this->_TryRefClient(uHandle);
		_YXC_CHECK_RET(pClient != NULL, YXC_ERC_INVALID_HANDLE);

		YXX_CritLocker locker(this->_crit);
		pClient->NotifyToClose();
		pClient->Unreference();
		return YXC_ERC_SUCCESS;
	}

	_NetSModelClient* _NetSModelClientThrMgr::_TryRefClient(yuintptr_t uHandle)
	{
		_NetSModelClient* pClient = NULL;

		YXX_CritLocker locker(this->_crit);
		ybyte_t byIndex = this->_byHandleIndecies[uHandle];
		_YXC_CHECK_REPORT_NEW_RET2(byIndex != 0xFF, YXC_ERC_INVALID_HANDLE, NULL, YC("Invalid handle(%d) specified"), (yuint32_t)uHandle);
		pClient = this->_pClients[byIndex];
		pClient->Reference();

		return pClient;
	}

	void _NetSModelClientThrMgr::Close()
	{
        YXC_SockEventSet(this->_lFinalizeEvent);
        // YXC_SockEventSet(this->_lFinalizeEvent); /* Set second time? */

		this->_teThread.Set();
        this->SignalForSendDataArrived();

		YXX_CritLocker locker(this->_crit);
		this->_bClosed = TRUE;
	}

	void _NetSModelClientThrMgr::Destroy()
	{
		if (this->_upRetrieveSendDataThread != 0)
		{
			OSCloseThreadHandle(this->_upRetrieveSendDataThread);
		}
		OSCloseThreadHandle(this->_upSendRecvThread);

		for (ysize_t i = 0; i < this->_uNumClients; ++i)
		{
			this->_pClients[i]->Close();
			_NetSModelClient::DestroyClient(this->_pClients[i], TRUE);
		}

        YXC_SockEventDestroy(this->_lFinalizeEvent);
		this->_crit.Close();
		this->_teThread.Close();
		this->_tsSendData.Close();
	}

	void _NetSModelClientThrMgr::WaitForExit()
	{
		if (this->_upRetrieveSendDataThread != 0)
		{
			OSWaitThread(this->_upRetrieveSendDataThread);
		}

		OSWaitThread(this->_upSendRecvThread);
	}

	void _NetSModelClientThrMgr::_HandleClientEvents(long lNetworkEvents, _NetSModelClient* pClient)
	{
		if (lNetworkEvents == 0) // manual event for multicast data
		{
			YXC_Status rc = pClient->SendBufferedData();
			if (rc != YXC_ERC_SUCCESS && rc != YXC_ERC_SOCK_WOULD_BLOCK)
			{
				_YXC_REPORT_ERR(rc, YC("Send multicast buffered data failed"));
				this->CloseAndRemoveClient(pClient);
			}
		}
		else
		{
			if (lNetworkEvents & FD_WRITE)
			{
				YXC_Status rc = pClient->SendBufferedData();
				if (rc != YXC_ERC_SUCCESS && rc != YXC_ERC_SOCK_WOULD_BLOCK)
				{
					_YXC_REPORT_ERR(rc, YC("Send buffered data failed"));
					this->CloseAndRemoveClient(pClient);
					return;
				}
			}
			if (lNetworkEvents & FD_READ)
			{
				YXC_Status rc = pClient->ReceiveData();
				if (rc != YXC_ERC_SUCCESS && rc != YXC_ERC_SOCK_WOULD_BLOCK)
				{
					_YXC_REPORT_ERR(rc, YC("Receive data from remote side failed"));
					this->CloseAndRemoveClient(pClient);
					return;
				}
			}
			if (lNetworkEvents & FD_CLOSE)
			{
				_YXC_REPORT_NEW_ERR(YXC_ERC_NET_SHUTDOWN, YC("The socket is closed by remote side"));
				this->CloseAndRemoveClient(pClient);
			}
		}
	}

	unsigned int __stdcall _NetSModelClientThrMgr::_RetrieveSendDataThreadProc(void* pArgs)
	{
		_NetSModelClientThrMgr* pMcctMgr = (_NetSModelClientThrMgr*)pArgs;

		_NetSModelClient* pClientBuf[_YXC_THREAD_MAX_CLIENT];
		while (TRUE)
		{
            YXC_Status rc = pMcctMgr->_tsSendData.Lock(3);
            if (rc != YXC_ERC_SUCCESS)
            {
                continue;
            }

            YXC_SocketEventArr eveArr;
            yint32_t iRet = YXC_SockEventWait(1, &pMcctMgr->_lFinalizeEvent, 0, &eveArr);
			if (iRet == 0)
			{
				yuint32_t uSignalState;
				iRet = YXC_SockEventGetResult(1, &pMcctMgr->_lFinalizeEvent, 0, &eveArr, &uSignalState);
				if (iRet == 0)
				{
					break;
				}
			}

			pMcctMgr->_crit.Lock();

			yuint32_t uNumSSMCClients = pMcctMgr->_uNumClients;
			memcpy(pClientBuf, pMcctMgr->_pClients, sizeof(_NetSModelClient*) * uNumSSMCClients);
			for (yuint32_t i = 0; i < uNumSSMCClients; ++i)
			{
				pClientBuf[i]->Reference();
			}

			pMcctMgr->_crit.Unlock();

			for (yuint32_t i = 0; i < uNumSSMCClients; ++i)
			{
				_NetSModelClient* pSsmcClient = pClientBuf[i];

				pSsmcClient->GetMulticastData();
			}

			for (yuint32_t i = 0; i < uNumSSMCClients; ++i)
			{
				pClientBuf[i]->Unreference();
			}
		}

		_YXC_KREPORT_THREAD_INFO(YC("Multi send thread ended since net is closed."));
		return 0;
	}

	unsigned int __stdcall _NetSModelClientThrMgr::_SendRecvThreadProc(void* pArgs)
	{
		_NetSModelClientThrMgr* pMcctMgr = (_NetSModelClientThrMgr*)pArgs;

		YXC_SocketEvent eveBuffer[_YXC_THREAD_MAX_CLIENT + 1] = { pMcctMgr->_lFinalizeEvent };
		_NetSModelClient* pClientBuf[_YXC_THREAD_MAX_CLIENT];

		ybool_t bEnded = FALSE;
		while (pMcctMgr->_teThread.Lock() == YXC_ERC_SUCCESS)
		{
			if (bEnded) break;

			pMcctMgr->_crit.Lock();

			yuint32_t uNumTotalSSMCClients = pMcctMgr->_uNumClients, uNumSSMCClients = 0;
			yuint32_t stTimeout = (yuint32_t)pMcctMgr->_stTimeout;
			for (yuint32_t i = 0; i < uNumTotalSSMCClients; ++i)
			{
				_NetSModelClient* pClient = pMcctMgr->_pClients[i];
				pClient->Reference();
				eveBuffer[i + 1] = pMcctMgr->_lEvents[i];
				pClientBuf[i] = pClient;
			}

			pMcctMgr->_crit.Unlock();

			for (yuint32_t i = 0; i < uNumTotalSSMCClients; ++i)
			{
				_NetSModelClient* pClient = pClientBuf[i];
				pClient->CheckConnnected();
				if (pClient->IsNotifyClosed())
				{
					pClient->SendBufferedData(); /* Send remain data, and close client. */
					_YXC_REPORT_NEW_ERR(YXC_ERC_NET_MANUALLY_CLOSE, YC("Net connection is notify closed by user"));
					pMcctMgr->CloseAndRemoveClient(pClient);
					pClient->Unreference();
				}
				else
				{
					eveBuffer[uNumSSMCClients + 1] = eveBuffer[i + 1]; /* Always move forward here. */
					pClientBuf[uNumSSMCClients] = pClient; /* Always move forward here. */
					++uNumSSMCClients;
				}
			}

            yuint32_t uSignalState, uIndex;
            _NetSModelClient* pSsmcClient;

            YXC_SocketEventArr resultArr;
			yint32_t iRet = YXC_SockEventWait(uNumSSMCClients + 1, eveBuffer, stTimeout, &resultArr);
			if (iRet < 0)
			{
				goto loop_back;
			}

            iRet = YXC_SockEventGetResult(uNumSSMCClients + 1, eveBuffer, iRet, &resultArr, &uSignalState);
            if (iRet < 0)
            {
                goto loop_back;
            }

            uIndex = iRet;
			if (uIndex == 0) // last event, finalize event
			{
				bEnded = TRUE;
				goto loop_back;
			}

			--uIndex; // map to real index
			pSsmcClient = pClientBuf[uIndex];

            pMcctMgr->_HandleClientEvents(uSignalState, pSsmcClient);

			// loop for other events
			for (yuint32_t i = uIndex + 1; i < uNumSSMCClients; ++i)
			{
                iRet = YXC_SockEventGetResult(1, eveBuffer + i + 1, 0, &resultArr, &uSignalState);

                if (iRet == 0)
                {
                    pMcctMgr->_HandleClientEvents(uSignalState, pClientBuf[i]);
                }
			}

loop_back:
			for (yuint32_t i = 0; i < uNumSSMCClients; ++i)
			{
				pClientBuf[i]->Unreference();
			}
		}

		_YXC_KREPORT_THREAD_INFO(YC("Send thread ended since net is closed."));
		return 0;
	}
}

#define __MODULE__ "EK.Net.SModel.Client"

#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_Sys/YXC_NetSModelClient.h>
#include <YXC_Sys/Net/_YXC_NetSModelClient.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClientMgr.hpp>

#include <new>

using namespace YXC_Inner;

extern "C"
{
	YXC_Status YXC_NetSModelClientMgrCreate(yuint32_t uNumMaxSockets, yuint32_t uThreadNumSockets, void* pMgrExt,
		YXC_NetSModelClientMgr* pManager)
	{
		_YXC_CHECK_REPORT_NEW_RET(uThreadNumSockets <= _YXC_THREAD_MAX_CLIENT, YXC_ERC_INVALID_PARAMETER, YC("The manager only support 63 max clients"));

		_NetSModelClientMgr* pMgr = (_NetSModelClientMgr*)malloc(sizeof(_NetSModelClientMgr));
		_YXC_CHECK_REPORT_NEW_RET(pMgr != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc pointer for client manager failed"));

		YXC_Status rc = pMgr->Create(FALSE, uNumMaxSockets, uThreadNumSockets, 0, pMgrExt);

		if (rc != YXC_ERC_SUCCESS)
		{
			*pManager = NULL;
			free(pMgr);
			return rc;
		}

		*pManager = _NSCMgrHdl(pMgr);
		return YXC_ERC_SUCCESS;
	}

	void YXC_NetSModelClientMgrDestroy(YXC_NetSModelClientMgr manager)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(manager);

		pMgr->Close();
		pMgr->WaitForExit();
		pMgr->Destroy();

		free(pMgr);
	}

	YXC_Status YXC_NetSModelClientCreate(YXC_NetSModelClientMgr manager, YXC_Socket socket, const YXC_NetSModelClientCbkArgs* pCbkArgs,
		YXC_NetSModelClient* pClientHdl)
	{
		return YXC_NetSModelClientCreateEx(manager, socket, pCbkArgs, _YXC_MODEL_DEF_SEND_BLOCKS,
			_YXC_MODEL_DEF_SEND_BUF_SIZE, _YXC_MODEL_DEF_RECV_BUF_SIZE, pClientHdl);
	}

	YXC_Status YXC_NetSModelClientCreateEx(YXC_NetSModelClientMgr clientMgr, YXC_Socket socket, const YXC_NetSModelClientCbkArgs* pCbkArgs,
		yuint32_t stMaxNumSendBlocks, ysize_t stSendBufferSize, ysize_t stRecvBufferSize, YXC_NetSModelClient* pClientHdl)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS, rc = YXC_ERC_SUCCESS, rcMgr;
		_NetSModelClient* pClient = NULL;

		YXC_SocketOptValue optVal;
		optVal.aliveProperty.bIsEnabled = TRUE;
		optVal.aliveProperty.byMaxRetries = 3;
		optVal.aliveProperty.uKeepAliveInterval = 2000;
		optVal.aliveProperty.uKeepAliveTime = 5000;

		if (stMaxNumSendBlocks == 0) stMaxNumSendBlocks = _YXC_MODEL_DEF_SEND_BLOCKS;
		if (stSendBufferSize == 0) stSendBufferSize = _YXC_MODEL_DEF_SEND_BUF_SIZE;
		if (stRecvBufferSize == 0) stRecvBufferSize = _YXC_MODEL_DEF_RECV_BUF_SIZE;

		YXC_SockSetOption(socket, YXC_SOCKET_OPT_TCP_KEEPALIVE, &optVal);

		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);
		pClient = _NetSModelClient::CreateClient(socket);
		_YXC_CHECK_REPORT_NEW_GOTO(pClient != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for client failed"));

		rc = pClient->Create(pCbkArgs, stMaxNumSendBlocks, stSendBufferSize, stRecvBufferSize);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create client failed"));

		pClient->SetMgr(pMgr);
		pClient->SetMgrExt(pMgr->GetExtData());

        rcMgr = pMgr->AddClient(pClient, (yuintptr_t*)pClientHdl);
		_YXC_CHECK_REPORT_GOTO(rcMgr == YXC_ERC_SUCCESS, rcMgr, YC("Add client to client manager failed"));

		return YXC_ERC_SUCCESS;

err_ret:
		if (rc == YXC_ERC_SUCCESS) _NetSModelClient::DestroyClient(pClient, FALSE);
		return rcRet;
	}

	YXC_Status YXC_NetSModelClientSend(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage, yuint32_t stTimeout)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);

		return pMgr->SendData((yuintptr_t)client, pPackage, stTimeout);
	}

	YXC_Status YXC_NetSModelClientClose(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);

		return pMgr->CloseClient((yuintptr_t)client, NULL);
	}

	YXC_Status YXC_NetSModelClientNotifyClose(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);

		return pMgr->NotifyCloseClient((yuintptr_t)client);
	}

	YXC_Status YXC_NetSModelClientCloseEx(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client, YXC_Event* pWaitHandle)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);

		return pMgr->CloseClient((yuintptr_t)client, pWaitHandle);
	}

	YXC_Status YXC_NetSModelClientGetSockOpt(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client, YXC_SocketOption option, YXC_SocketOptValue* pOpt)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);

		return pMgr->GetClientSockOption((yuintptr_t)client, option, pOpt);
	}

	YXC_Status YXC_NetSModelClientSetSockOpt(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client, YXC_SocketOption option, const YXC_SocketOptValue* pOpt)
	{
		_NetSModelClientMgr* pMgr = _NSCMgrPtr(clientMgr);

		return pMgr->SetClientSockOption((yuintptr_t)client, option, pOpt);
	}

//	YXC_Status YXC_NetClientCreateMC(YXC_Socket socket, YXC_NetSModelClientOnClose pfnOnClose, YXC_NetSModelClientOnRecv pfnOnReceive,
//		void* pExtData, YXC_NetClient* pClient)
//	{
//		YXC_Status rcRet = YXC_ERC_SUCCESS;
//
//		_NetClient* pCli = _NetClient::CreateClient(socket);
//		_YXC_CHECK_REPORT_RET(pCli != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc pointer for client failed");
//
//		YXC_Status rc = pCli->InitAttribute(NULL, NULL, NULL, NULL, pfnOnClose, pfnOnReceive, NULL);
//		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Initialize client attribute failed");
//
//		pCli->Reference();
//		*pClient = _NCHdl(pCli);
//err_ret:
//		return rcRet;
//	}
//
//	YXC_Status YXC_NetClientSend(YXC_NetClient client, const YXC_NetPackage* pPackage, ysize_t stTimeout)
//	{
//		return _NCPtr(client)->SendPackage(pPackage, stTimeout);
//	}
//
//	void YXC_NetClientClose(YXC_NetClient client, ybool_t bCloseAttachedSocket)
//	{
//
//	}
};

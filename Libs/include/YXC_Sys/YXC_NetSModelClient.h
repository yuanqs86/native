/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_CLIENT_MODEL_H__
#define __INC_YXC_SYS_BASE_NET_CLIENT_MODEL_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/YXC_PNCMP.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_NetSModelClient);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_NetSModelClientMgr);

	YXC_API(YXC_Status) YXC_NetSModelClientMgrCreate(
		yuint32_t uNumMaxSockets,
		yuint32_t uThreadNumSockets,
		void* pMgrExt,
		YXC_NetSModelClientMgr* pManager
	);

	YXC_API(void) YXC_NetSModelClientMgrDestroy(YXC_NetSModelClientMgr manager);

	typedef void (*YXC_NetSModelClientOnClose)(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pGExt, void* pCExt, ybool_t bClosedActively);

	typedef void (*YXC_NetSModelClientOnConnect)(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, void* pGExt, void* pCExt);

	typedef void (*YXC_NetSModelClientOnRecv)(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage,
		void* pGExt, void* pCExt);

	typedef void (*YXC_NetSModelClientOnDestroy)(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client,
		void* pGExt, void* pCExt, YXC_Socket sock);

	typedef void (*YXC_NetSModelClientOnNoData)(YXC_NetSModelClientMgr mgr, YXC_NetSModelClient client,
		ysize_t stMilliSeconds, void* pGExt, void* pCExt);

	typedef struct _YXC_NET_SMODEL_CLIENT_CALLBACK_ARGS
	{
		YXC_NetSModelClientOnClose pfnOnClose;
		YXC_NetSModelClientOnRecv pfnOnRecv;
		YXC_NetSModelClientOnNoData pfnOnNoData;
		YXC_NetSModelClientOnDestroy pfnOnDestroy;
		ysize_t stTimeoutOnNoData;
		void* pCExt;
	}YXC_NetSModelClientCbkArgs;

	YXC_API(YXC_Status) YXC_NetSModelClientCreate(
		YXC_NetSModelClientMgr clientMgr,
		YXC_Socket socket,
		const YXC_NetSModelClientCbkArgs* pCbkArgs,
		YXC_NetSModelClient* pClient
	);

	YXC_API(YXC_Status) YXC_NetSModelClientCreateEx(
		YXC_NetSModelClientMgr clientMgr,
		YXC_Socket socket,
		const YXC_NetSModelClientCbkArgs* pCbkArgs,
		yuint32_t stMaxNumSendBlocks,
		ysize_t stSendBufferSize,
		ysize_t stRecvBufferSize,
		YXC_NetSModelClient* pClient
	);

	YXC_API(YXC_Status) YXC_NetSModelClientSend(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client, const YXC_NetPackage* pPackage, yuint32_t stTimeout);

	// socket will also be closed here
	YXC_API(YXC_Status) YXC_NetSModelClientClose(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client);

	// Notify close socket, socket will be closed in callback.
	YXC_API(YXC_Status) YXC_NetSModelClientNotifyClose(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client);

	// close client and return an out handle, Use this function in callback thread will cause a failure
	YXC_API(YXC_Status) YXC_NetSModelClientCloseEx(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client, YXC_Event* puWaitEvent);

	YXC_API(YXC_Status) YXC_NetSModelClientGetSockOpt(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client,
		YXC_SocketOption option, YXC_SocketOptValue* pOpt);

	YXC_API(YXC_Status) YXC_NetSModelClientSetSockOpt(YXC_NetSModelClientMgr clientMgr, YXC_NetSModelClient client,
		YXC_SocketOption option, const YXC_SocketOptValue* pOpt);

	YXC_API(YXC_Status) YXC_NetSModelClientDropBlocks(
		YXC_NetSModelClientMgr clientMgr,
		YXC_NetSModelClient client,
		YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl,
		ysize_t* pstNumBlocksDropped
	);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_NET_CLIENT_MODEL_H__ */

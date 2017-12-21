/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_H__
#define __INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_MMInterface.h>
#include <YXC_Sys/YXC_PNCMP.h>
#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_NetSocket.h>

#include <YXC_Sys/YXC_NetSModelClient.h>

#define YXC_NET_MODEL_BROADCAST_TO_ALL ((yuint32_t)-1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_NetSModelServer);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_NetSModelGroupMgr);

	typedef struct __YXC_NET_SERVER_RAW_DATA
	{
		ysize_t stNumHeaderDatas;
		YXC_Buffer headers[YXC_NP_MAX_ADDTIONAL_HEADERS];

		void* pData;
		ysize_t stDataLen;
	}YXC_NetServerRawData;

	typedef struct __YXC_NET_SERVER_CREATE_ARGS
	{
		ybyte_t byServerProtocol;
		YXC_Socket serverSock;

		yuint32_t uMaxGroups; // max number of groups, 0 for default
		yuint32_t stMaxNumClients;

		ybool_t bAutoStart;
	}YXC_NetServerCreateArgs;

	typedef YXC_Status (*YXC_NetServerOnBroadcastCheck)(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client,
		YXC_PNCMPConsumerInfo* pSwapInfo, const YXC_NetServerRawData* pRawData, void* pMgrExt, void* pCExt);

	typedef struct __YXC_NET_SERVER_MC_CREATE_ARGS
	{
		yuint32_t stMaxNumBlocks;
		yuint32_t stMaxNumClients;
		yuint32_t stMaxNumBlocksOnClient; // 0 for default
		YXC_NetServerOnBroadcastCheck pfnOnBroadcastCheck;
		YXC_PNCMPMMParam mmParam;
	}YXC_NetSrvCreateArgsMC;

	typedef struct __YXC_NET_SERVER_GROUP_CREATE_ARGS
	{
		yuint32_t uGroupId;
		YXC_NetSrvCreateArgsMC* pMCArgs;
		ysize_t stRecvBufferSize; // 0 for default
		yuint32_t stMaxNumClients;
		yuint32_t stMaxNumSendBlocks; // 0 for default
		ysize_t stSendBufferSize; // 0 for default
		ysize_t stStackSize; // 0 for default
		yuint32_t stThreadNumClients; // 0 for default
		const YXC_NetProtocol* pProtocol;
	}YXC_NetSrvGroupCreateArgs;

	typedef void (*YXC_NetServerOnClientClose)(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client, void* pGroupExt,
		void* pClientExt, ybool_t bClosedActively);

	typedef void (*YXC_NetServerOnClientRecv)(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage, void* pGroupExt, void* pClientExt);

	typedef void (*YXC_NetServerOnClientConnected)(YXC_NetSModelGroupMgr groupMgr, YXC_NetSModelClient client, void* pGroupExt, void* pClientExt);

	typedef ybool_t (*YXC_NetServerOnInitClientAttr)(
		YXC_NetSModelServer server,
		YXC_Socket clientSocket,
		yuint32_t* pstMaxNumSendBlocks,
		ysize_t* pstSendBufSize,
		ysize_t* pstRecvBufSize,
		yuint32_t* pstMaxConsumerBlocks,
		void* pSrvExt,
		yuint32_t* puGroupId,
		void** ppClientExt
	);

	typedef void (*YXC_NetServerOnDestroyClientAttr)(
		YXC_NetSModelServer server,
		yuint32_t uGroupId,
		YXC_Socket socket,
		void* pSrvExt,
		void* pClientExt
	);

	YXC_API(YXC_Status) YXC_NetSModelServerCreate(
		const YXC_NetServerCreateArgs* pCreateArgs,
		YXC_NetServerOnInitClientAttr pfnOnInitClient,
		YXC_NetServerOnDestroyClientAttr pfnOnDestroyClient,
		void* pExtData,
		YXC_NetSModelServer* psModel
	);

	YXC_API(void) YXC_NetSModelServerGetNumClients(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		yuint32_t* puNumClients
	);

	YXC_API(YXC_Status) YXC_NetSModelServerSetSockOpt(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		YXC_NetSModelClient client,
		YXC_SocketOption opt,
		const YXC_SocketOptValue* pOptVal
	);

	YXC_API(YXC_Status) YXC_NetSModelServerGetSockOpt(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		YXC_NetSModelClient client,
		YXC_SocketOption opt,
		YXC_SocketOptValue* pOptVal
	);

	YXC_API(YXC_Status) YXC_NetSModelServerSendToClient(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage,
		yuint32_t stTimeoutInMiliseconds
	);

	YXC_API(YXC_Status) YXC_NetSModelServerCloseClient(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		YXC_NetSModelClient client
	);

	YXC_API(YXC_Status) YXC_NetSModelServerCloseClientEx(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		YXC_NetSModelClient client,
		YXC_Event* pWaitEvent
	);

	YXC_API(YXC_Status) YXC_NetSModelServerBroadcast(
		YXC_NetSModelServer server,
		yuint32_t uGroupId,
		const YXC_NetServerRawData* pRawData
	);

	YXC_API(YXC_Status) YXC_NetSModelServerGroupCreate(
		YXC_NetSModelServer server,
		const YXC_NetSrvGroupCreateArgs* pGroupArgs,
		YXC_NetServerOnClientConnected pfnOnConn,
		YXC_NetServerOnClientClose pfnOnClose,
		YXC_NetServerOnClientRecv pfnOnRecv,
		void* pGroupData
	);

	YXC_API(void) YXC_NetSModelServerGroupClose(YXC_NetSModelServer sModel, yuint32_t uGroupId);

	YXC_API(YXC_Status) YXC_NetSModelServerStart(YXC_NetSModelServer sModel);

	YXC_API(void) YXC_NetSModelServerClose(YXC_NetSModelServer sModel, ybool_t bWaitForClosed);

	YXC_API(YXC_Status) YXC_NetSModelServerGroupRef(
		YXC_NetSModelServer sModel,
		yuint32_t uGroupId,
		YXC_NetSModelGroupMgr* pGroupMgr
	);

	YXC_API(void) YXC_NetSModelServerGroupUnref(YXC_NetSModelGroupMgr groupMgr);

	YXC_API(YXC_Status) YXC_NetSModelGroupBroadcast(
		YXC_NetSModelGroupMgr groupMgr,
		const YXC_NetServerRawData* pRawData
	);

	YXC_API(void) YXC_NetSModelGroupGetNumClients(
		YXC_NetSModelGroupMgr groupMgr,
		yuint32_t* puNumClients
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupSetSockOpt(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client,
		YXC_SocketOption opt,
		const YXC_SocketOptValue* pOptVal
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupGetSockOpt(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client,
		YXC_SocketOption opt,
		YXC_SocketOptValue* pOptVal
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupDropBlocks(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client,
		ybool_t bDropMCBlock,
		YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl,
		yuint32_t* pstNumBlocksDropped
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupSendToClient(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client,
		const YXC_NetPackage* pPackage,
		yuint32_t stTimeoutInMiliseconds
	);

	YXC_API(yuint32_t) YXC_NetSModelQueryGroupId(
		YXC_NetSModelGroupMgr mgr
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupNotifyClose(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupCloseClient(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client
	);

	YXC_API(YXC_Status) YXC_NetSModelGroupCloseClientEx(
		YXC_NetSModelGroupMgr groupMgr,
		YXC_NetSModelClient client,
		YXC_Event* pWaitEvent
	);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_NET_SELECT_MODEL_SERVER_H__ */

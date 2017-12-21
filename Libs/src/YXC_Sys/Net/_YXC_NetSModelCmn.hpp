#ifndef __INNER_INC_YXC_SYS_BASE_NET_SMODEL_COMMON_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_SMODEL_COMMON_HPP__

#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Locker.hpp>

#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/YXC_NetSModelClient.h>

#define _YXC_THREAD_MAX_CLIENT (64 - 1)
#define _YXC_THREAD_DEFAULT_CLIENTS 20
#define _YXC_CLI_HANDLE_RATIO 4

#define _YXC_MODEL_DEF_SEND_BLOCKS 100
#define _YXC_MODEL_DEF_SEND_BUF_SIZE (1 * (1 << 20)) // 1M
#define _YXC_MODEL_DEF_RECV_BUF_SIZE (6 * (1 << 20)) // 6M

namespace YXC_Inner
{
	struct _NetPackageSentInfo
	{
		const ybyte_t* pData;
		YXC_NetPackage package;
		YXC_NetTransferInfo transInfo;
		YXC_PNCMPConsumer consumer;
	};

	struct _NetPackageRecvInfo
	{
		ybyte_t* pBuffer;
		ysize_t stBufferSize;
		YXC_NetPackage package;
		YXC_NetTransferInfo transInfo;
	};

	class _NetSModelClientThrMgr;

//#if YXC_PLATFORM_WIN
//	BOOL InitializeSendHook();
//
//	void AddClientSocketHook(socket_t socket);
//
//	void RemoveHookedClientSocket(socket_t socket);
//#endif /* YXC_PLATFORM_WIN */
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_SMODEL_COMMON_HPP__ */

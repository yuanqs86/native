/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_NET_HOOK_H__
#define __INC_YXC_SYS_BASE_NET_HOOK_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NetSocket.h>

#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#else
#include <sys/socket.h>
#endif /* YXC_PLATFORM_WIN */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_API(YXC_Status) YXC_NetEnableLeakHook();

	YXC_API(void) YXC_SockSetSrvLeakHook(ybool_t bEnableHook);

	YXC_API(void) YXC_NetAddToHookList(socket_t socket);

	YXC_API(void) YXC_NetRemoveFromHookList(socket_t socket);

	YXC_API(void) YXC_NetReleaseAllLeakedMem();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_NET_HOOK_H__ */

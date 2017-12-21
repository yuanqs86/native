#ifndef __INNER_INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_THREAD_MANAGER_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_THREAD_MANAGER_HPP__

#include <YXC_Sys/Net/_YXC_NetSModelCmn.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClient.hpp>

namespace YXC_Inner
{
	class _NetSModelClientMgr;
	class _NetSModelClientThrMgr
	{
	public:
		YXC_Status Create(yuint32_t uMaxNumClients, ysize_t stStackSize, ysize_t stTimeout, _NetSModelClientMgr* pMgr);

		void Destroy();

		ybool_t AddClient(_NetSModelClient* pClient, yuintptr_t upBase, yuintptr_t* pHandle);

		void CloseAndRemoveClient(_NetSModelClient* pClient);

		void CloseAndRemoveClientByUser(_NetSModelClient* pClient, YXC_Event* pWaitHandle);

		YXC_Status SendData(yuintptr_t uHandle, const YXC_NetPackage* pPackage, yuint32_t stTimeout);

		YXC_Status CloseClient(yuintptr_t uHandle, YXC_Event* pWaitHandle);

		YXC_Status SetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, const YXC_SocketOptValue* pOptVal);

		YXC_Status GetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, YXC_SocketOptValue* pOptVal);

		YXC_Status DropBlocks(yuintptr_t uHandle, ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
			void* pDropCtrl, yuint32_t* pstNumBlocksDropped);

		YXC_Status NotifyCloseClient(yuintptr_t uHandle);

		void WaitForExit();

		void Close();

		void SignalForSendDataArrived();

	private:
		_NetSModelClient* _TryRefClient(yuintptr_t uHandle);

		yint32_t _FindClientIndex(_NetSModelClient* pClient);

		void _RemoveIndex(yuint32_t uIndex);

	private:
		static unsigned int __stdcall _RetrieveSendDataThreadProc(void* pArgs);

		static unsigned int __stdcall _SendRecvThreadProc(void* pArgs);

	private:
		void _HandleClientEvents(long lNetworkEvents, _NetSModelClient* pClient);

	private:
		yuint32_t _uMaxClientsInThread;
		yuint32_t _uNumClients;
		ybool_t _bClosed;

		_NetSModelClient* _pClients[_YXC_THREAD_MAX_CLIENT];

		ybyte_t _byHandleIndecies[_YXC_THREAD_MAX_CLIENT * 4]; // sure for no overlapped handles
		yuintptr_t _upLastHandle;

        YXC_SocketEvent _lFinalizeEvent;
        YXC_SocketEvent _lEvents[_YXC_THREAD_MAX_CLIENT];
        ysize_t _stTimeout;

		YXX_Crit _crit;
		YXX_Sem _tsSendData;
		YXX_Event _teThread;

		_NetSModelClientMgr* _pMgr;

		ethread_t _upRetrieveSendDataThread;
		ethread_t _upSendRecvThread;

		etid_t _uSendThreadId;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_THREAD_MANAGER_HPP__ */

#ifndef __INNER_INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_MANAGER_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_SMODEL_CLIENT_MANAGER_HPP__

#include <vector>

#include <YXC_Sys/YXC_StlAlloc.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelCmn.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClientThrMgr.hpp>

namespace YXC_Inner
{
	// Not thread safe here
	class _NetSModelClientMgr
	{
	public:
		YXC_Status Create(ybool_t bIsMCMgr, yuint32_t uNumMaxClients, yuint32_t uThreadNumClients,
			yuint32_t stStackSize, void* pMgrExt);

		YXC_Status AddClient(_NetSModelClient* pClient, yuintptr_t* pClientHandle);

		YXC_Status SendData(yuintptr_t uHandle, const YXC_NetPackage* pPackage, yuint32_t stTimeout);

		YXC_Status DropBlocks(yuintptr_t uHandle, ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
			void* pDropCtrl, yuint32_t* pstNumBlocksDropped);

		YXC_Status CloseClient(yuintptr_t uHandle, YXC_Event* pWaitHandle);

		YXC_Status NotifyCloseClient(yuintptr_t uHandle);

		YXC_Status SetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, const YXC_SocketOptValue* pOptVal);

		YXC_Status GetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, YXC_SocketOptValue* pOptVal);

		void SignalAll();

		void Close();

		void WaitForExit();

		void Destroy();

	public:
		inline void IncreaseClient() { YXCLib::Interlocked::Increment(&this->_uNumClients); }

		inline void DecreaseClient() { YXCLib::Interlocked::Decrement(&this->_uNumClients); }

		inline yuint32_t GetNumClients() { return YXCLib::Interlocked::ExchangeAdd(&this->_uNumClients, 0); }

		inline ybool_t IsMCMgr() { return this->_bIsMCMgr; }

		inline void* GetExtData() { return this->_pMgrExt; }

	private:
		_NetSModelClientThrMgr* _pThrMgrs;

		void* _pMgrExt;

		ysize_t _stThreadNumClients;

		ysize_t _stNumThreads;

		ysize_t _stMaxNumClients;

		yuint32_t _uNumClients;

		ybool_t _bIsMCMgr;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_NetSModelClientMgr, _NetSModelClientMgr, _NSCMgrPtr, _NSCMgrHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_CLIENT_MANAGER_HPP__ */

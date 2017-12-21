#define __MODULE__ "EK.Net.SModel.ClientMgr"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/Net/_YXC_NetSModelClientMgr.hpp>

namespace YXC_Inner
{
	YXC_Status _NetSModelClientMgr::AddClient(_NetSModelClient* pClient, yuintptr_t* pHandle)
	{
		for (ysize_t i = 0; i < this->_stNumThreads; ++i)
		{
			yuintptr_t upHdlBase = 1 + _YXC_CLI_HANDLE_RATIO * i * this->_stThreadNumClients;
			ybool_t bRet = this->_pThrMgrs[i].AddClient(pClient, upHdlBase, pHandle);
			if (bRet)
			{
				return YXC_ERC_SUCCESS;
			}
		}

		_YXC_REPORT_NEW_RET(YXC_ERC_EXCEED_MAX_COUNT, YC("No thread can handle client pointer."));
	}

	YXC_Status _NetSModelClientMgr::NotifyCloseClient(yuintptr_t uHandle)
	{
		ysize_t iThreadIndex = (uHandle - 1) / (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);

		if (iThreadIndex < this->_stNumThreads)
		{
			uintptr_t uHandleInThread = (uHandle - 1) % (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);
			return this->_pThrMgrs[iThreadIndex].NotifyCloseClient(uHandleInThread);
		}

		return YXC_ERC_INVALID_HANDLE;
	}

	YXC_Status _NetSModelClientMgr::CloseClient(yuintptr_t uHandle, YXC_Event* pWaitHandle)
	{
		ysize_t iThreadIndex = (uHandle - 1) / (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);

		if (iThreadIndex < this->_stNumThreads)
		{
			uintptr_t uHandleInThread = (uHandle - 1) % (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);
			return this->_pThrMgrs[iThreadIndex].CloseClient(uHandleInThread, pWaitHandle);
		}

		return YXC_ERC_INVALID_HANDLE;
	}

	YXC_Status _NetSModelClientMgr::SendData(yuintptr_t uHandle, const YXC_NetPackage* pPackage, yuint32_t stTimeout)
	{
		ysize_t iThreadIndex = (uHandle - 1) / (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);

		if (iThreadIndex < this->_stNumThreads)
		{
			uintptr_t uHandleInThread = (uHandle - 1) % (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);
			return this->_pThrMgrs[iThreadIndex].SendData(uHandleInThread, pPackage, stTimeout);
		}

		return YXC_ERC_INVALID_HANDLE;
	}

	YXC_Status _NetSModelClientMgr::GetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, YXC_SocketOptValue* pOptVal)
	{
		ysize_t iThreadIndex = (uHandle - 1) / (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);

		if (iThreadIndex < this->_stNumThreads)
		{
			uintptr_t uHandleInThread = (uHandle - 1) % (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);
			return this->_pThrMgrs[iThreadIndex].GetClientSockOption(uHandleInThread, option, pOptVal);
		}

		return YXC_ERC_INVALID_HANDLE;
	}

	YXC_Status _NetSModelClientMgr::SetClientSockOption(yuintptr_t uHandle, YXC_SocketOption option, const YXC_SocketOptValue* pOptVal)
	{
		ysize_t iThreadIndex = (uHandle - 1) / (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);

		if (iThreadIndex < this->_stNumThreads)
		{
			uintptr_t uHandleInThread = (uHandle - 1) % (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);
			return this->_pThrMgrs[iThreadIndex].SetClientSockOption(uHandleInThread, option, pOptVal);
		}

		return YXC_ERC_INVALID_HANDLE;
	}

	YXC_Status _NetSModelClientMgr::DropBlocks(yuintptr_t uHandle, ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec,
		void* pDropCtrl, yuint32_t* pstNumBlocksDropped)
	{
		ysize_t iThreadIndex = (uHandle - 1) / (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);

		if (iThreadIndex < this->_stNumThreads)
		{
			uintptr_t uHandleInThread = (uHandle - 1) % (_YXC_CLI_HANDLE_RATIO * this->_stThreadNumClients);
			return this->_pThrMgrs[iThreadIndex].DropBlocks(uHandleInThread, bMCBlock, pfnBlockSpec, pDropCtrl, pstNumBlocksDropped);
		}

		return YXC_ERC_INVALID_HANDLE;
	}

	void _NetSModelClientMgr::SignalAll()
	{
		for (ysize_t i = 0; i < this->_stNumThreads; ++i)
		{
			this->_pThrMgrs[i].SignalForSendDataArrived();
		}
	}

	YXC_Status _NetSModelClientMgr::Create(ybool_t bIsMCMgr, yuint32_t stMaxNumClients, yuint32_t stThreadNumClients,
		yuint32_t stStackSize, void* pMgrExt)
	{
		ysize_t i = 0;
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		this->_stNumThreads = (stMaxNumClients - 1) / stThreadNumClients + 1;
		this->_bIsMCMgr = bIsMCMgr;

		this->_pThrMgrs = (_NetSModelClientThrMgr*)malloc(sizeof(_NetSModelClientThrMgr) * this->_stNumThreads);
		_YXC_CHECK_REPORT_NEW_RET(this->_pThrMgrs != NULL, YXC_ERC_OUT_OF_MEMORY, YC("No buffer for thread managers"));

		for (ysize_t i = 0; i < this->_stNumThreads; ++i)
		{
			_NetSModelClientThrMgr* pMgr = &this->_pThrMgrs[i];
			YXC_Status rc = pMgr->Create(stThreadNumClients, stStackSize, 1000, this);

			_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create thread manager %d failed"), (yint32_t)i);
		}

		this->_stMaxNumClients = stMaxNumClients;
		this->_stThreadNumClients = stThreadNumClients;
		this->_uNumClients = 0;
		this->_pMgrExt = pMgrExt;

		return YXC_ERC_SUCCESS;

err_ret:
		for (ysize_t j = 0; j < i; ++j)
		{
			this->_pThrMgrs[j].Close();
			this->_pThrMgrs[j].WaitForExit();
			this->_pThrMgrs[j].Destroy();
		}

		free(this->_pThrMgrs);
		return rcRet;
	}

	void _NetSModelClientMgr::Close()
	{
		for (ysize_t j = 0; j < this->_stNumThreads; ++j)
		{
			this->_pThrMgrs[j].Close();
		}
	}

	void _NetSModelClientMgr::WaitForExit()
	{
		for (ysize_t j = 0; j < this->_stNumThreads; ++j)
		{
			this->_pThrMgrs[j].WaitForExit();
		}
	}

	void _NetSModelClientMgr::Destroy()
	{
		for (ysize_t j = 0; j < this->_stNumThreads; ++j)
		{
			this->_pThrMgrs[j].Destroy();
		}

		free(this->_pThrMgrs);
	}
}

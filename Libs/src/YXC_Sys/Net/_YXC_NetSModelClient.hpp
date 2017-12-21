#ifndef __INNER_INC_YXC_SYS_BASE_NET_MODEL_CLIENT_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_MODEL_CLIENT_HPP__

#include <YXC_Sys/Net/_YXC_NetSModelCmn.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>

using namespace YXCLib;

namespace YXC_Inner
{
	class _NetSModelClient
	{
	public:
		YXC_Status CreateBase(const YXC_NetSModelClientCbkArgs* pCbkArgs, yuint32_t stMaxNumSendBlocks, ysize_t stSendBufferSize,
			ysize_t stRecvBufferSize);

		YXC_Status Create(const YXC_NetSModelClientCbkArgs* pCbkArgs, yuint32_t stMaxNumSendBlocks,
			ysize_t stSendBufferSize, ysize_t stRecvBufferSize);

		YXC_Status Create(const YXC_NetSModelClientCbkArgs* pCbkArgs, YXC_PNCMP memPool, yuint32_t stMaxConsumerQueue,
			ysize_t stRecvBufferSize, yuint32_t stMaxNumSendBlocks, ysize_t stSendBufferSize);

		void SetBlockCheck(YXC_PNCMPConsumerBlockCheckFunc pfnBlockCheck);

		YXC_Status SendBufferedData();

		YXC_Status ReceiveData();

		YXC_Status SendPackage(const YXC_NetPackage* pPackage, yuint32_t stTimeout);

		void DropBlocks(ybool_t bMCBlock, YXC_PNCMPSpecBlockFunc pfnBlockSpec, void* pDropCtrl, yuint32_t* pstNumBlocksDropped);

		YXC_Status GetMulticastData();

		YXC_Status GetSendDataFromPool();

		void CheckConnnected();

		void NotifyToClose();

	public:
		inline void Lock() { this->_crit.Lock(); }

		inline void UnLock() { this->_crit.Unlock(); }

		inline void SetHandle(yuintptr_t hdl) { this->_upHandle = hdl; }

		inline yuintptr_t GetHandle() { return this->_upHandle; }

		inline void SetMgr(void* pMgr) { this->_pMgr = pMgr; }

		inline void SetMgrExt(void* pMgrExt) { this->_pMgrExt = pMgrExt; }

		inline void* GetMgr() { return this->_pMgr; }

		inline void* GetExtPtr() { return this->_pExt; }

		inline YXC_Socket GetSocket() { return this->_socket; }

		inline socket_t GetRawSocket() { return this->_rSock; }

		inline void Reference()
		{
			YXCLib::Interlocked::Increment(&this->_uRefCount);
		}

		inline void NotifyClosed()
		{
			YXC_EventSet(this->_eveClose);

			YXC_PNCMPProducerOptVal optVal;
			optVal.bWaitForConsumer = FALSE;
			YXC_PNCMPProducerSetOption(this->_producer, YXC_PNCMP_PRODUCER_OPTION_WAIT_FOR_CONSUMER,
				&optVal);
		}

		inline ybool_t IsNotifyClosed()
		{
			return this->_bNotifyClosed;
		}

		inline void Unreference()
		{
			yuint32_t iRet = YXCLib::Interlocked::Decrement(&this->_uRefCount);

			if (this->_bClosed && iRet == 0)
			{
				DestroyClient(this, TRUE);
			}
		}

		inline void DeReferenceWithWaitHandle(YXC_Event* pWaitHandle)
		{
			YXC_Event upHandle = YXC_HandleDuplicate(this->_eveClose);
			yuint32_t iRet = YXCLib::Interlocked::Decrement(&this->_uRefCount);

			if (this->_bClosed && iRet == 0)
			{
                YXC_EventDestroy(upHandle);
				DestroyClient(this, TRUE);
				return;
			}

			*pWaitHandle = upHandle;
			return;
		}

		inline void Close()
		{
			this->_bClosed = TRUE;
		}

		inline ybool_t IsCreated() { return this->_bCreated; }

		inline ybool_t IsClosed() { return this->_bClosed; }

		inline void SetConnectCallback(YXC_NetSModelClientOnConnect pfnOnConnected)
		{
			this->_pfnConnect = pfnOnConnected;
		}

		inline void CallCloseCallback(ybool_t bClosedActively)
		{
			if (this->_pfnClose != NULL)
			{
				this->_pfnClose((YXC_NetSModelClientMgr)this->_pMgr, (YXC_NetSModelClient)this->_upHandle, this->_pMgrExt,
					this->_pExt, bClosedActively);
			}
		}

		inline YXC_SocketEvent GetSockEvent() { return this->_event; }

		inline long GetSockEventVal() { return this->_lEvent; }

		inline void EnableSockEventVal(ybool_t bEnable, long lValBit)
		{
			if (bEnable && (this->_lEvent & lValBit) != lValBit)
			{
				this->_lEvent |= lValBit;
                YXC_SockEventSelect(this->_socket, this->_event, this->_lEvent);
			}
			else if (!bEnable && (this->_lEvent ^ lValBit) != 0)
			{
				this->_lEvent ^= lValBit;
				YXC_SockEventSelect(this->_socket, this->_event, this->_lEvent);
			}
		}

	public:
		static inline _NetSModelClient* CreateClient(YXC_Socket socket)
		{
			_NetSModelClient* pClient = (_NetSModelClient*)malloc(sizeof(_NetSModelClient));
			if (pClient == NULL) return NULL;

			pClient->_bCreated = FALSE;
			pClient->_bConnected = FALSE;
			pClient->_pfnClose = NULL;
			pClient->_pfnRecv = NULL;
			pClient->_pfnDestroy = NULL;
			pClient->_pfnNoData = NULL;
			pClient->_socket = socket;
			pClient->_pMgr = NULL;
			pClient->_pMgrExt = NULL;

			return pClient;
		}

		static inline void DestroyClient(_NetSModelClient* pClient, ybool_t bCloseSock)
		{
			if (pClient->_bCreated)
			{
				if (pClient->_pfnDestroy)
				{
					pClient->_pfnDestroy((YXC_NetSModelClientMgr)pClient->_pMgr, (YXC_NetSModelClient)pClient->_upHandle,
						pClient->_pMgrExt, pClient->_pExt, pClient->_socket);
				}
				pClient->Destroy(bCloseSock);
			}
			free(pClient);
		}

	protected:
		void DestroyBase(ybool_t bCloseSock);

		void Destroy(ybool_t bCloseSock);

	private:
		YXC_Status _GetSendDataFromPool();

	protected:
		YXX_Crit _crit;
		YXC_Event _eveClose;

		yuintptr_t _upHandle;

		YXC_Socket _socket;
		socket_t _rSock;

		ybool_t _bClosed;
		ybool_t _bConnected;
		ybool_t _bNotifyClosed;
		ybool_t _bCreated;
		volatile yuint32_t _uRefCount;

		_NetPackageSentInfo _sentInfo;
		yuint32_t _stSendTimeout;
		_NetPackageRecvInfo _recvInfo;

		YXC_NetSModelClientOnConnect _pfnConnect;
		YXC_NetSModelClientOnClose _pfnClose;
		YXC_NetSModelClientOnDestroy _pfnDestroy;
		YXC_NetSModelClientOnRecv _pfnRecv;
		YXC_NetSModelClientOnNoData _pfnNoData;
		yuint32_t _uNoDataMaxTime;
		yuint32_t _uNoDataTime;
		void* _pMgrExt;
		void* _pExt;

		YXC_PNCMP _sendPool;
		YXC_PNCMPConsumer _consumer;
		YXC_PNCMPProducer _producer;

		YXC_PNCMP _mcPool;
		YXC_PNCMPConsumer _mcConsumer;

		void* _pMgr;

		YXC_SocketEvent _event;
		long _lEvent;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_MODEL_CLIENT_HPP__ */

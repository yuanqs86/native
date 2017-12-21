#ifndef __INNER_INC_YXC_SYS_BASE_CONDITION_VARIABLE_HPP__
#define __INNER_INC_YXC_SYS_BASE_CONDITION_VARIABLE_HPP__

#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_CondVar.h>

namespace YXC_Inner
{
	class _CondVar;
	struct _CondVarSupervisor
	{
		YXC_Sem tsResource;
		YXC_CondVarSVMatchFunc pfnMatch;
		void* pExternal;
		ybool_t bMatched;
		ybool_t bVarClosed;
		_CondVar* pCondVar;
		void* pWakedCond;
	};

	class _CondVar
	{
	public:
		void _ChangeSupervisorState(_CondVarSupervisor& supervisor);

		YXC_Status Create(ysize_t stBufferSize, void* pBuffer, const void* pInitBuffer, yuint32_t uMaxWaitThreads);

		YXC_Status CreateSupervisor(YXC_CondVarSVMatchFunc pfnMatch, void* pExternal, YXC_CondVarSV* pSupervisor);

		YXC_Status ControlSupervisor(_CondVarSupervisor* pSupervisor, YXC_CondVarSVMatchFunc pfnMatch, void* pSuper);

		YXC_Status WakeSupervisors(YXC_CondVarChangeFunc pfnCondChange, void* pCtrl);

		void RemoveSupervisor(_CondVarSupervisor* pSupervisor);

        void CopyCond(void* pData);

		void Close();

		void Destroy();

	private:
		void _ClearSupervisors(ybool_t bDestroy);

	public:
		inline ysize_t GetCondSize() { return this->_stCondBufSize; }

		inline ybool_t IsClosed()
		{
			yuint32_t uCount = YXCLib::Interlocked::ExchangeOr(&this->_uCount, 0);
			return (uCount & 0x80000000) != 0;
		}

	private:
		struct Node
		{
			_CondVarSupervisor supervisor;
			Node* next;
		};

	private:
		volatile yuint32_t _uCount;
		yuint32_t _uMaxCount;
		void* _pCondBuf;
		ysize_t _stCondBufSize;
		YXX_Crit _csLock;

		Node* _pHead;
		Node* _pTail;
		Node* _pInsert;
		Node _storage[2];

		YXCLib::FixedMemoryPool _emp;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_CondVar, _CondVar, _CondVarPtr, _CondVarHdl);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_CondVarSV, _CondVarSupervisor, _CondVarPtr_S, _CondVarHdl_S);
}

#endif /* __INNER_INC_YXC_SYS_BASE_CONDITION_VARIABLE_HPP__ */

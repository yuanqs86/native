#define __MODULE__ "EK.CV"

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/OS/_YXC_CondVar.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

namespace YXC_Inner
{
	void _CondVar::_ChangeSupervisorState(_CondVarSupervisor& supervisor)
	{
		ybool_t bMatched = supervisor.pfnMatch(this->_pCondBuf, supervisor.pExternal);
		if (bMatched)
		{
			if (!supervisor.bMatched)
			{
				memcpy(supervisor.pWakedCond, this->_pCondBuf, this->_stCondBufSize);
				YXC_SemUnlock(supervisor.tsResource);
				supervisor.bMatched = TRUE;
			}
			else
			{
				YXC_SemLock(supervisor.tsResource);
				memcpy(supervisor.pWakedCond, this->_pCondBuf, this->_stCondBufSize);
				YXC_SemUnlock(supervisor.tsResource);
			}
		}
		else if (supervisor.bMatched)
		{
			YXC_SemLock(supervisor.tsResource);
			memcpy(supervisor.pWakedCond, this->_pCondBuf, this->_stCondBufSize);
			supervisor.bMatched = FALSE;
		}
	}

	YXC_Status _CondVar::Create(ysize_t stBufferSize, void* pBuffer, const void* pInitBuffer, yuint32_t uMaxWaitThreads)
	{
		this->_pCondBuf = NULL;
		this->_uCount = 0;
		_pHead = _storage;
		_pTail = _storage + 1;
		_pHead->next = _pTail;
        _pInsert = _pHead;
		this->_csLock.Init(4000);

		this->_uMaxCount = uMaxWaitThreads;
		YXC_Status rc = this->_emp.Create(TRUE, (sizeof(_CondVarSupervisor) + stBufferSize) * uMaxWaitThreads, 0);
		_YXC_CHECK_REPORT_NEW_RET(rc == YXC_ERC_SUCCESS, rc, YC("Create internal buffer failed"));

		memcpy(pBuffer, pInitBuffer, stBufferSize);
		this->_stCondBufSize = stBufferSize;
		this->_pCondBuf = pBuffer;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _CondVar::CreateSupervisor(YXC_CondVarSVMatchFunc pfnMatch, void* pExternal, YXC_CondVarSV* pSupervisor)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		yuint32_t uCount = YXCLib::Interlocked::ExchangeAdd(&this->_uCount, 0);
		if (uCount >= this->_uMaxCount)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_INDEX_OUT_OF_RANGE, YC("Exceed max supervisor count %d"), this->_uMaxCount);
		}

		YXX_CritLocker locker(this->_csLock);

		Node* pNode = static_cast<Node*>(::malloc(sizeof(Node) + this->_stCondBufSize));
		_YXC_CHECK_REPORT_NEW_RET(pNode != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc supervisor node failed"));
		void* pBuffer = pNode + 1;
		memcpy(pBuffer, this->_pCondBuf, this->_stCondBufSize);

		ybool_t bMatch = pfnMatch(this->_pCondBuf, pExternal);

		_CondVarSupervisor& supervisor = pNode->supervisor;
		YXC_Status rc = YXC_SemCreate(bMatch ? 1 : 0, &supervisor.tsResource);
		_YXC_CHECK_OS_GOTO(rc == YXC_ERC_SUCCESS, YC("Create semaphore for condition variable failed"));

		supervisor.pfnMatch = pfnMatch;
		supervisor.pExternal = pExternal;
		supervisor.bMatched = bMatch;
		supervisor.pWakedCond = pBuffer;
		supervisor.pCondVar = this;

        YXCLib::Interlocked::Increment(&this->_uCount);
		*pSupervisor = _CondVarHdl_S(&supervisor);

		this->_pInsert->next = pNode;
        pNode->next = this->_pTail;
		this->_pInsert = pNode;

		return YXC_ERC_SUCCESS;
    err_ret:
		if (pNode) free(pNode);
		return rcRet;
	}

	YXC_Status _CondVar::ControlSupervisor(_CondVarSupervisor* pSupervisor, YXC_CondVarSVMatchFunc pfnMatch, void* pSuper)
	{
		YXX_CritLocker locker(this->_csLock);

		pSupervisor->pfnMatch = pfnMatch;
		pSupervisor->pExternal = pSuper;

		this->_ChangeSupervisorState(*pSupervisor);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _CondVar::WakeSupervisors(YXC_CondVarChangeFunc pfnCondChange, void* pCtrl)
	{
		YXX_CritLocker locker(this->_csLock);

		pfnCondChange(this->_pCondBuf, pCtrl);
		for (Node* p = _pHead->next; p != _pTail; p = p->next)
		{
			this->_ChangeSupervisorState(p->supervisor);
		}

		return YXC_ERC_SUCCESS;
	}

	void _CondVar::RemoveSupervisor(_CondVarSupervisor* pSupervisor)
	{
		ybool_t bDestroy = FALSE;
		do
		{
			YXX_CritLocker locker(this->_csLock);
            Node* p = this->_pHead;
			while (p->next != this->_pTail)
			{
				if (&p->next->supervisor == pSupervisor)
				{
					Node* pFree = p->next;
					p->next = pFree->next;

                    if (pFree == this->_pInsert)
                    {
                        this->_pInsert = p;
                    }

                    YXC_SemDestroy(pFree->supervisor.tsResource);
					free(pFree);

                    yuint32_t uCount = YXCLib::Interlocked::Decrement(&this->_uCount);
					bDestroy = uCount == 0x80000000;

					break;
				}
			}
		} while (0);

		if (bDestroy)
		{
			YXC_CondVarForceDestroy(_CondVarHdl(this));
		}
	}

	void _CondVar::Destroy()
	{
		this->_ClearSupervisors(TRUE);
		this->_csLock.Close();
		this->_emp.Destroy();
	}

	void _CondVar::_ClearSupervisors(ybool_t bDestroy)
	{
        if (bDestroy)
        {
            Node* p = this->_pHead->next;
            while (p != this->_pTail)
            {
                Node* pFree = p;
                p = pFree->next;

                YXC_SemDestroy(pFree->supervisor.tsResource);
                free(pFree);
            }
            this->_pHead->next = this->_pTail;
            this->_pInsert = this->_pHead;
        }
        else
        {
            for (Node* p = this->_pHead->next; p != this->_pTail; p = p->next)
            {
                YXC_SemUnlock(p->supervisor.tsResource); // Unlock them, and tell them I'm closed.
            }
        }
	}

	void _CondVar::Close()
	{
		if (YXCLib::Interlocked::ExchangeOr(&this->_uCount, 0x80000000) == 0)
		{
			YXC_CondVarForceDestroy(_CondVarHdl(this));
			return;
		}

		this->_csLock.Lock();
		this->_ClearSupervisors(FALSE);
		this->_csLock.Unlock();
	}

    void _CondVar::CopyCond(void* pData)
    {
		YXX_CritLocker locker(this->_csLock);

        memcpy(pData, this->_pCondBuf, this->_stCondBufSize);
    }
}

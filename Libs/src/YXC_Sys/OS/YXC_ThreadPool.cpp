#include <YXC_Sys/YXC_ThreadPool.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_CondVar.h>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>
#include <new>

using namespace YXCLib;
using namespace YXC_Inner;
namespace
{
	struct _TThread
	{
		YXC_TPoolThreadProc fnProc;
		void* lParam;
		ethread_t hThread;

		YXC_CondVar condVar;
		YXC_CondVarSV condVarSVExt;
		YXC_CondVarSV condVarSVInner;

		ybool_t bUsing;
	};

	struct _TThreadPool
	{
		_TThread* pThreads;
		YXC_Sem hSem;
		yuint32_t uNumThreads;
		yuint32_t uMaxThreads;
		YXX_Crit crit;

		_TThreadPool() : crit(2000), pThreads(NULL), hSem(NULL), uNumThreads(0), uMaxThreads(0)
		{

		}
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_TPool, _TThreadPool, _TPoolPtr, _TPoolHdl);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_TPoolThread, _TThread, _TThreadPtr, _TThreadHdl);

	ybool_t _TThreadMatchFunc(const void* pCond, void* pInner)
	{
		const ThreadStatus* pStatus = (const ThreadStatus*)pCond;

		ybool_t bCanThreadUse = *pStatus == THREAD_STATUS_PAUSED;

		return (ybool_t)pInner ? !bCanThreadUse : bCanThreadUse; /* InnerThread signal when running, OutThread signal when paused. */
	}

	void _TThreadChangeStatusFunc(void* pData, void* pExt)
	{
		ThreadStatus* pStatus = (ThreadStatus*)pData;
		*pStatus = *(ThreadStatus*)pExt;
	}

	yuint32_t __stdcall _TThreadProc(void* pParam)
	{
		_TThread* pThread = (_TThread*)pParam;
		while (TRUE)
		{
			ThreadStatus thrStatus;

			YXC_Status rc = YXC_CondVarSupervisorWait(pThread->condVar, pThread->condVarSVInner, YXC_INFINITE, &thrStatus);
			if (rc == YXC_ERC_SUCCESS)
			{
				if (thrStatus == THREAD_STATUS_STOPPED)
				{
					/* Thread exit here.*/
					break;
				}
				else
				{
					pThread->fnProc(pThread->lParam);
					_YXC_KREPORT_INFO(L"Thread pool proc completed.");

					ThreadStatus pauseStatus = THREAD_STATUS_PAUSED;
					rc = YXC_CondVarWake(pThread->condVar, _TThreadChangeStatusFunc, &pauseStatus);
					if (rc != YXC_ERC_SUCCESS)
					{
						_YXC_KREPORT_THREAD_ERR(L"Failed to change thread pool thread status.");
					}
				}
			}
			else
			{
				_YXC_KREPORT_THREAD_ERR(L"Failed to do wait operation in thread pool worker thread.");
			}
		}

		return 0;
	}

	YXC_Status _InitTThread(_TThread* pThread)
	{
		ThreadStatus initStatus = THREAD_STATUS_PAUSED;

		YXC_CondVar cVar;
		YXC_CondVarSV svInner;
		YXC_CondVarSV svExt;
		YXC_Status rc = YXC_CondVarCreate(2, sizeof(ThreadStatus), &initStatus, &cVar);
		_YXC_CHECK_STATUS_RET(rc, L"Create thread control variable failed");
		YXCLib::HandleRef<YXC_CondVar> resVar(cVar, YXC_CondVarForceDestroy);

		rc = YXC_CondVarCreateSupervisor(cVar, _TThreadMatchFunc, (void*)TRUE, &svInner);
		_YXC_CHECK_STATUS_RET(rc, L"Create thread control inner supervisor failed");

		rc = YXC_CondVarCreateSupervisor(cVar, _TThreadMatchFunc, (void*)FALSE, &svExt);
		_YXC_CHECK_STATUS_RET(rc, L"Create thread control ext supervisor failed");

		pThread->bUsing = FALSE;
		pThread->fnProc = NULL;
		pThread->lParam = NULL;
		pThread->condVarSVInner = svInner;
		pThread->condVarSVExt = svExt;
		pThread->condVar = cVar;

		ethread_t hThread = YXCLib::OSCreateThread(_TThreadProc, pThread, NULL);
		_YXC_CHECK_OS_RET(hThread != NULL, L"Failed to create thread pool worker thread.");

		pThread->hThread = hThread;

		resVar.Detach();
		return YXC_ERC_SUCCESS;
	}

	void _DestroyTThread(_TThread* pThread)
	{
		YXC_CondVarWake(pThread->condVar, CVThreadChangeStatusFunc, (void*)THREAD_STATUS_STOPPED);
		OSWaitThread(pThread->hThread);
		OSCloseThreadHandle(pThread->hThread);
		YXC_CondVarSupervisorClose(pThread->condVar, pThread->condVarSVExt);
		YXC_CondVarSupervisorClose(pThread->condVar, pThread->condVarSVInner);
		YXC_CondVarNotifyClose(pThread->condVar);
	}
}

extern "C"
{
	YXC_API(YXC_Status) YXC_TPoolCreate(yuint32_t uNumMaxThreads, YXC_TPool* pPool)
	{
		_YCHK_MAL_ARR_R1(pBuffer, ybyte_t, sizeof(_TThreadPool) + uNumMaxThreads * sizeof(_TThread));

		yuint32_t uThreadInited = 0;

		_TThreadPool* pPool2 = (_TThreadPool*)pBuffer;
		new (pPool2) _TThreadPool();
		pPool2->uMaxThreads = uNumMaxThreads;

		_TThread* pThreads = (_TThread*)(pPool2 + 1);
		memset(pThreads, 0, uNumMaxThreads * sizeof(_TThread));
		pPool2->pThreads = pThreads;

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = YXC_SemCreate(uNumMaxThreads, &pPool2->hSem);
		*pPool = _TPoolHdl(pPool2);

		for (yuint32_t i = 0; i < uNumMaxThreads; ++i)
		{
			_TThread* pThreadsPtr = pThreads + i;

			rc = _InitTThread(pThreads + i);
			_YXC_CHECK_STATUS_GOTO(rc, L"Init thread %d failed", i);
			++uThreadInited;
		}

		return YXC_ERC_SUCCESS;
err_ret:
		if (pPool2)
		{
			if (pPool2->hSem) YXC_SemDestroy(pPool2->hSem);

			for (yuint32_t i = 0; i < uThreadInited; ++i)
			{
				_TThread* pThread = pPool2->pThreads + i;
				_DestroyTThread(pThread);
			}

			pPool2->~_TThreadPool();
			free(pPool2);
		}

		return rcRet;
	}

	YXC_API(void) YXC_TPoolDestroy(YXC_TPool pool)
	{
		_TThreadPool* pPool2 = _TPoolPtr(pool);

		for (yuint32_t i = 0; i < pPool2->uNumThreads; ++i) /* Wait for all of threads returned. */
		{
			YXC_SemLock(pPool2->hSem);
		}

		for (yuint32_t i = 0; i < pPool2->uMaxThreads; ++i)
		{
			_TThread* pThread = pPool2->pThreads + i;
			_DestroyTThread(pThread);
		}

		YXC_SemDestroy(pPool2->hSem);
		pPool2->~_TThreadPool();
		free(pPool2);
	}

	YXC_API(YXC_Status) YXC_TPoolGetThread(YXC_TPool pool, yuint32_t stTimeout, YXC_TPoolThread* pThread)
	{
		_TThreadPool* pPool2 = _TPoolPtr(pool);

		YXC_Status rc = YXC_SemLockTimeout(pPool2->hSem, stTimeout);
		if (rc == YXC_ERC_SUCCESS)
		{
			YXX_CritLocker locker(pPool2->crit);
			for (yuint32_t i = 0; i < pPool2->uMaxThreads; ++i)
			{
				_TThread* pThreadPtr = pPool2->pThreads + i;
				if (!pThreadPtr->bUsing)
				{
					*pThread = _TThreadHdl(pThreadPtr);
					pThreadPtr->bUsing = TRUE;

					++pPool2->uNumThreads;
					return YXC_ERC_SUCCESS;
				}
			}

			YXC_SemUnlock(pPool2->hSem);
			_YXC_REPORT_NEW_RET(YXC_ERC_NO_RES_AVAILABLE, L"No real thread resource available");
		}

		_YXC_REPORT_NEW_RET(YXC_ERC_NO_RES_AVAILABLE, L"Thread sem full, no thread available to get now");
	}

	YXC_API(YXC_Status) YXC_TPoolThreadWait(YXC_TPoolThread th, yuint32_t stTimeoutMS)
	{
		_TThread* pThread = _TThreadPtr(th);

		ThreadStatus status = THREAD_STATUS_UNKNOWN;
		YXC_Status rc = YXC_CondVarSupervisorWait(pThread->condVar, pThread->condVarSVExt, stTimeoutMS, &status);
		_YXC_CHECK_STATUS_RET(rc, L"Failed to wait for thread ended.");

		return YXC_ERC_SUCCESS;
	}

	YXC_API(YXC_Status) YXC_TPoolThreadReturn(YXC_TPool pool, YXC_TPoolThread thReturn)
	{
		_TThread* pThread = _TThreadPtr(thReturn);
		_TThreadPool* pPool = _TPoolPtr(pool);

		YXX_CritLocker locker(pPool->crit);
		pThread->bUsing = FALSE;
		--pPool->uNumThreads;
		YXC_SemUnlock(pPool->hSem);

		return YXC_ERC_SUCCESS;
	}

	YXC_API(void) YXC_TPoolThreadSetParam(YXC_TPoolThread th, YXC_TPoolThreadProc proc, void* pParam)
	{
		_TThread* pThread = _TThreadPtr(th);
		pThread->fnProc = proc;
		pThread->lParam = pParam;
	}

	YXC_API(YXC_Status) YXC_TPoolThreadStart(YXC_TPoolThread th)
	{
		_TThread* pThread = _TThreadPtr(th);
		ThreadStatus status = THREAD_STATUS_RUNNING;
		YXC_Status rc = YXC_CondVarWake(pThread->condVar, _TThreadChangeStatusFunc, &status);
		_YXC_CHECK_STATUS_RET(rc, L"Failed to start thread pool thread");

		return YXC_ERC_SUCCESS;
	}

	YXC_API(YXC_Status) YXC_TPoolThreadSuspend(YXC_TPoolThread th)
	{
		_TThread* pThread = _TThreadPtr(th);
		YXC_Status rc = YXCLib::OSSuspendThread(pThread->hThread);

		_YXC_CHECK_OS_RET(rc == YXC_ERC_SUCCESS, L"Failed to suspend TPool thread");
		return YXC_ERC_SUCCESS;
	}

	YXC_API(YXC_Status) YXC_TPoolThreadResume(YXC_TPoolThread th)
	{
		_TThread* pThread = _TThreadPtr(th);
		YXC_Status rc = YXCLib::OSResumeThread(pThread->hThread);

		_YXC_CHECK_OS_RET(rc == YXC_ERC_SUCCESS, L"Failed to suspend TPool thread");
		return YXC_ERC_SUCCESS;
	}
};

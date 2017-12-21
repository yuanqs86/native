#define __MODULE__ "EK.CV"

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_CondVar.h>
#include <YXC_Sys/OS/_YXC_CondVar.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <new>

using namespace YXC_Inner;

extern "C"
{
	YXC_Status YXC_CondVarCreate(ysize_t stMaxNumClients, ysize_t stBufferSize, const void* pInitBuffer, YXC_CondVar* pOutVar)
	{
		_CondVar* pcVar = (_CondVar*)malloc(sizeof(_CondVar) + stBufferSize);
		_YXC_CHECK_REPORT_NEW_RET(pcVar != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for condition variable failed"));

		new (pcVar) _CondVar();

		void* pBuffer = pcVar + 1;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		YXC_Status rc = pcVar->Create(stBufferSize, pBuffer, pInitBuffer, (yuint32_t)stMaxNumClients);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Create condition variable failed"));

		*pOutVar = _CondVarHdl(pcVar);
		return YXC_ERC_SUCCESS;
err_ret:
		pcVar->~_CondVar();
		free(pcVar);
		return rcRet;
	}

	YXC_Status YXC_CondVarCreateSupervisor(YXC_CondVar cVar, YXC_CondVarSVMatchFunc pfnMatch,void* pExternal, YXC_CondVarSV* pSupervisor)
	{
		_CondVar* pcVar = _CondVarPtr(cVar);

		return pcVar->CreateSupervisor(pfnMatch, pExternal, pSupervisor);
	}

	YXC_Status YXC_CondVarControlSupervisor(YXC_CondVar cVar, YXC_CondVarSV supervisor, YXC_CondVarSVMatchFunc pfnNewMatch, void* pNewExternal)
	{
		_CondVarSupervisor* pSupervisor = _CondVarPtr_S(supervisor);

		return pSupervisor->pCondVar->ControlSupervisor(pSupervisor, pfnNewMatch, pNewExternal);
	}

	YXC_Status YXC_CondVarSupervisorWait(YXC_CondVar cVar, YXC_CondVarSV supervisor, yuint32_t stmsTimeout, void* pCurrentCond)
	{
		_CondVarSupervisor* pSupervisor = _CondVarPtr_S(supervisor);
		_CondVar* pVar = _CondVarPtr(cVar);

		YXC_Status rc = YXC_SemLockTimeout(pSupervisor->tsResource, stmsTimeout);
		_YXC_CHECK_REPORT_NEW_RET(rc == YXC_ERC_SUCCESS, rc, YC("Lock semaphore to wait supervisor failed"));

		memcpy(pCurrentCond, pSupervisor->pWakedCond, pVar->GetCondSize());
		YXC_SemUnlock(pSupervisor->tsResource);

		_YXC_CHECK_REPORT_NEW_RET(!pVar->IsClosed(), YXC_ERC_CLOSED_HANDLE, YC("The condition var is closed"));
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_CondVarWake(YXC_CondVar cVar, YXC_CondVarChangeFunc pfnCondChange, void* pCtrl)
	{
		return _CondVarPtr(cVar)->WakeSupervisors(pfnCondChange, pCtrl);
	}

    void YXC_CondVarCopyCond(YXC_CondVar cVar, void* pData)
    {
        return _CondVarPtr(cVar)->CopyCond(pData);
    }

	void YXC_CondVarNotifyClose(YXC_CondVar cVar)
	{
		_CondVarPtr(cVar)->Close();
	}

	void YXC_CondVarForceDestroy(YXC_CondVar cVar)
	{
		_CondVar* pcVar = _CondVarPtr(cVar);
		pcVar->Destroy();
		free(pcVar);
	}

	void YXC_CondVarSupervisorClose(YXC_CondVar cVar, YXC_CondVarSV supervisor)
	{
		_CondVarSupervisor* pSupervisor = _CondVarPtr_S(supervisor);
		_CondVar* pVar = _CondVarPtr(cVar);

		pVar->RemoveSupervisor(pSupervisor);
	}
}

#define __MODULE__ "EK.Thread.Controllor"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_ThreadControllor.hpp>
#include <YXC_Sys/YXC_Sys.h>

namespace YXCLib
{
	ThreadControllor::ThreadControllor() : _var(NULL), _supervisor(NULL)
	{

	}

	ThreadControllor::~ThreadControllor()
	{
		this->Destroy();
	}

	YXC_Status ThreadControllor::Init(ThreadStatus initStatus)
	{
		YXC_Status rc = YXC_CondVarCreate(1, sizeof(ThreadStatus), &initStatus, &this->_var);
		_YXC_CHECK_STATUS_RET(rc, YC("Failed to create cond variable"));

		rc = YXC_CondVarCreateSupervisor(this->_var, YXCLib::CVThreadMatchFunc, NULL, &this->_supervisor);
		if (rc != YXC_ERC_SUCCESS)
		{
			YXC_CondVarForceDestroy(this->_var);
			_YXC_CHECK_STATUS_RET(rc, YC("Failed to create supervisor of cond var"));
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status ThreadControllor::Run()
	{
        YXCLib::ThreadStatus st = THREAD_STATUS_RUNNING;
		return YXC_CondVarWake(this->_var, CVThreadChangeStatusFunc, &st);
	}

	YXC_Status ThreadControllor::RunPeriodly()
	{
        YXCLib::ThreadStatus st = THREAD_STATUS_PERIOD_RUNNING;
		return YXC_CondVarWake(this->_var, CVThreadChangeStatusFunc, &st);
	}

	YXC_Status ThreadControllor::Complete()
	{
        YXCLib::ThreadStatus st = THREAD_STATUS_COMPLETED;
		return YXC_CondVarWake(this->_var, CVThreadChangeStatusFunc, &st);
	}

	YXC_Status ThreadControllor::Pause()
	{
        YXCLib::ThreadStatus st = THREAD_STATUS_PAUSED;
		return YXC_CondVarWake(this->_var, CVThreadChangeStatusFunc, &st);
	}

	YXC_Status ThreadControllor::Stop()
	{
        YXCLib::ThreadStatus st = THREAD_STATUS_STOPPED;
		return YXC_CondVarWake(this->_var, CVThreadChangeStatusFunc, &st);
	}

	YXC_Status ThreadControllor::Wait(yuint32_t stmsTimeout, ThreadStatus* pReturnedStatus)
	{
		YXC_Status rc = YXC_CondVarSupervisorWait(this->_var, this->_supervisor, stmsTimeout, (void*)pReturnedStatus);
		if (rc != YXC_ERC_SUCCESS)
		{
			YXC_CondVarCopyCond(this->_var, (void*)pReturnedStatus);
		}

		return rc;
	}

	void ThreadControllor::Destroy()
	{
		if (this->_var)
		{
			YXC_CondVarSupervisorClose(this->_var, this->_supervisor);
			YXC_CondVarForceDestroy(this->_var);

			this->_var = NULL;
			this->_supervisor = NULL;
		}
	}
}

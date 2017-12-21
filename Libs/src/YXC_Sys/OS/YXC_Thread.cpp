#define __MODULE__ "EK.Thread"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_Thread.hpp>
#include <YXC_Sys/YXC_Sys.h>

namespace YXCLib
{
	Thread::Thread() : _ctrl(), _thread(NULL), _param(NULL), _pfnStart(NULL), _pfnLoop(NULL), _tid(),
		_msCallDur(0), _bStarted(FALSE)
	{

	}

	Thread::~Thread()
	{
		this->Stop();
	}

	YXC_Status Thread::Init(ThreadProc pfnStart, ThreadProc pfnLoop, ysize_t stackSize, yuint32_t msCallDur, void* param)
	{
		YXC_Status rc = this->_ctrl.Init(YXCLib::THREAD_STATUS_PAUSED);
		_YXC_CHECK_RC_RET(rc);

		this->_thread = YXCLib::OSCreateThreadStack(_ThreadOriginalProc, stackSize, this, &this->_tid);
		_YXC_CHECK_OS_RET(this->_thread != NULL, YC("YXCLib::OSCreateThread"));

		this->_pfnStart = pfnStart;
		this->_pfnLoop = pfnLoop;
		this->_param = param;
		this->_msCallDur = msCallDur;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status Thread::Run()
	{
		YXC_Status rc = this->_ctrl.RunPeriodly();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status Thread::Complete()
	{
		YXC_Status rc = this->_ctrl.Complete();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status Thread::Pause()
	{
		YXC_Status rc = this->_ctrl.Pause();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status Thread::Stop(yuint32_t* puExitCode)
	{
		if (this->_thread != NULL)
		{
			YXC_Status rc = this->NotifyStop();
			_YXC_CHECK_RC_RET(rc);

			rc = this->Wait(YXC_INFINITE);
			_YXC_CHECK_RC_RET(rc);

			if (puExitCode)
			{
#if YXC_PLATFORM_WIN
				GetExitCodeThread(this->_thread, (LPDWORD)puExitCode);
#endif /* YXC_PLATFORM_WIN */
			}

			YXCLib::OSCloseThreadHandle(this->_thread);
			this->_thread = NULL;
			this->_tid = 0;
		}
		else
		{
			if (puExitCode) *puExitCode = 0;
		}

		this->_bStarted = FALSE;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status Thread::Wait(yuint32_t stmsTimeout)
	{
		if (this->_thread)
		{
			YXC_Status rc = YXC_WaitSingleKObjectTimeout(this->_thread, stmsTimeout);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status Thread::NotifyStop()
	{
		 YXC_Status rc = this->_ctrl.Stop();
		 _YXC_CHECK_RC_RET(rc);

		 return YXC_ERC_SUCCESS;
	}

	yuint32_t Thread::_ThreadOriginalProc(void* param)
	{
		Thread* t = (Thread*)param;

		return t->_ThreadOriginalLoopProc();
	}

	yuint32_t Thread::_ThreadOriginalLoopProc()
	{
		while (TRUE)
		{
			YXCLib::ThreadStatus t;
			YXC_Status rc = this->_ctrl.Wait(this->_msCallDur, &t);

			if (rc == YXC_ERC_TIMEOUT) rc = YXC_ERC_SUCCESS;
			_YXC_FATAL_ASSERT_SYSEXPR(rc);

			switch (t)
			{
			case YXCLib::THREAD_STATUS_COMPLETED:
				return 0;
			case YXCLib::THREAD_STATUS_STOPPED:
				_YXC_REPORT_NEW_RET(YXC_ERC_OPERATION_CANCELED, YC("User cancels this thread"));
			case YXCLib::THREAD_STATUS_RUNNING:
			case YXCLib::THREAD_STATUS_PERIOD_RUNNING:
				if (this->_bStarted)
				{
					rc = this->_pfnLoop(this->_param);
					_YXC_CHECK_RC_RET(rc);
				}
				else
				{
					rc = this->_pfnStart(this->_param);
					_YXC_CHECK_RC_RET(rc);

					this->_bStarted = TRUE;
				}
				break;
			default:
				break;
			}
		}
	}
}

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_THREAD_HPP__
#define __INC_YXC_BASE_THREAD_HPP__

#include <YXC_Sys/YXC_ThreadControllor.hpp>

namespace YXCLib
{
	typedef YXC_Status (*ThreadProc)(void* param);

	class YXC_CLASS Thread
	{
	public:
		Thread();

		~Thread();

	public:
		YXC_Status Init(ThreadProc pfnStart, ThreadProc pfnLoop, ysize_t stackSize, yuint32_t msCallDur, void* param);

		YXC_Status Stop(yuint32_t* puRetCode = NULL);

		YXC_Status NotifyStop();

		YXC_Status Complete();

		YXC_Status Pause();

		YXC_Status Run();

		YXC_Status Wait(yuint32_t stmsTimeout);

	private:
		static yuint32_t __stdcall _ThreadOriginalProc(void* param);

		yuint32_t _ThreadOriginalLoopProc();

	public:
		inline etid_t GetThreadId() { return this->_tid; }

	private:
		ThreadControllor _ctrl;

		ethread_t _thread;
		etid_t _tid;

		ThreadProc _pfnStart;
		ThreadProc _pfnLoop;
		void* _param;
		yuint32_t _msCallDur;

		ybool_t _bStarted;
	};
}

#endif /* __INC_YXC_BASE_THREAD_HPP__ */

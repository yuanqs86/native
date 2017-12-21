/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_THREAD_CONTROLLOR_HPP__
#define __INC_YXC_BASE_THREAD_CONTROLLOR_HPP__

#include <YXC_Sys/YXC_CondVar.h>

namespace YXCLib
{
	class YXC_CLASS ThreadControllor
	{
	public:
		ThreadControllor();

		~ThreadControllor();

	public:
		YXC_Status Init(YXCLib::ThreadStatus initStatus);

		void Destroy();

		YXC_Status Run();

		YXC_Status RunPeriodly();

		YXC_Status Stop();

		YXC_Status Complete();

		YXC_Status Pause();

		YXC_Status Wait(yuint32_t stmsTimeout, YXCLib::ThreadStatus* pReturnedStatus);

	public:
		ybool_t IsAvailable() const { return this->_var != NULL; }

	private:
		YXC_CondVar _var;

		YXC_CondVarSV _supervisor;
	};
}

#endif /* __INC_YXC_BASE_THREAD_CONTROLLOR_HPP__ */

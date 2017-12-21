#ifndef __INNER_INC_YXC_SYS_BASE_PNCMP_PRODUCER_HPP__
#define __INNER_INC_YXC_SYS_BASE_PNCMP_PRODUCER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_PNCMP.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXC_Inner
{
	class _PNCMP;
	struct _PNCMPProducer
	{
		_PNCMP* pPool;
		ybool_t bWaitForConsumer;
		ybool_t bWaitForAllConsumers;
		yuint32_t stTimeout;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_PNCMPProducer, _PNCMPProducer, _PNCMPPtr_P, _PNCMPHdl_P);
}

#endif /* __INNER_INC_YXC_SYS_BASE_PNCMP_PRODUCER_HPP__ */

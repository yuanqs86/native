#ifndef __INNER_INC_YXC_SYS_BASE_MM_MODEL_BASE_HPP__
#define __INNER_INC_YXC_SYS_BASE_MM_MODEL_BASE_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXC_Inner
{
	class _MMModelBase
	{
	public:
		virtual void* Alloc(ysize_t stNumBytes) = 0;

		virtual void Free(void* pPtr) = 0;

		virtual ~_MMModelBase() {}
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_MM_MODEL_BASE_HPP__ */

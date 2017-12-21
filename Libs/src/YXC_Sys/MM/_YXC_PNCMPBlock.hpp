#ifndef __INNER_INC_YXC_SYS_BASE_PNCMP_BLOCK_HPP__
#define __INNER_INC_YXC_SYS_BASE_PNCMP_BLOCK_HPP__

#include <YXC_Sys/YXC_Sys.h>

namespace YXC_Inner
{
	struct _PNCMPBlock
	{
		volatile yuint32_t uUseCount;
		volatile yuint32_t uDiskPushNum;
		yuint32_t uBlockId;
		yuint32_t uBlockLifeTime;
		ysize_t stBlockSize;
		ysize_t stExtSize;
	};

	struct _PNCMPMemory
	{
		_PNCMPBlock block;
		ybyte_t buffer[1];
	};

}

#endif /* __INNER_INC_YXC_SYS_BASE_PNCMP_BLOCK_HPP__ */

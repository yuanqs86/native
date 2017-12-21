#ifndef __INNER_INC_YXC_SYS_BASE_PNCMP_BLOCK_QUEUE_HPP__
#define __INNER_INC_YXC_SYS_BASE_PNCMP_BLOCK_QUEUE_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/MM/_YXC_PNCMPBlock.hpp>

namespace YXC_Inner
{
	class _PNCMP;
	struct _PNCMPBlockQueue
	{
		yuint32_t uQueueBufSize;
		yuint32_t uQueueCount;
		yuint32_t uQueueIndex;
		_PNCMPBlock** pQueueBlocks;

		void Create(_PNCMPBlock** pBlockBuffer, yuint32_t uMaxCount);

		ybool_t InsertBlock(_PNCMPBlock* pBlock);

		void InsertBlock2(_PNCMPBlock* pBlock);

		ysize_t GetTotalSize();

		void ClearQueue();

		_PNCMPBlock* PopBlock();

		_PNCMPBlock* GetFirstBlock();
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_PNCMP_BLOCK_QUEUE_HPP__ */

#define __MODULE__ "EK.Utils.LocalMM"

#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

#include <algorithm>

// #define YXC_MEMORY_PAGE_SIZE (1 << 12) // 4K
#if YXC_IS_64BIT
#define YXC_MEMORY_BOUND 8
#else
#define YXC_MEMORY_BOUND 4
#endif /* YXC_IS_64BIT */

#define YXC_BLOCK_RATIO (1 << 7)
#define YXC_MIN_BLOCK_SIZE (1 << 7)

namespace
{
	static inline void _GetMemoryPoolParams(ysize_t& rstCapability, ysize_t& rstBaseBlkSize,
		ysize_t& rstNumBlks, ysize_t& rstRemain)
	{
		rstCapability = (rstCapability - 1) / YXC_MEMORY_BOUND *
			YXC_MEMORY_BOUND + YXC_MEMORY_BOUND; // round to size

		if (rstBaseBlkSize == 0)
		{
			rstBaseBlkSize = rstCapability / YXC_BLOCK_RATIO; // creat
		}
		if (rstBaseBlkSize < YXC_MIN_BLOCK_SIZE)
		{
			rstBaseBlkSize = YXC_MIN_BLOCK_SIZE;
		}

		size_t stNumBlksExp = 1;
		size_t stBlockSize = rstBaseBlkSize;
		size_t stRemainSize = rstCapability;
		while (stRemainSize > stBlockSize)
		{
			stRemainSize -= stBlockSize;
			stBlockSize <<= 1;
			++stNumBlksExp;
		}

		rstNumBlks = stNumBlksExp;
		rstRemain = stRemainSize;
	}
}

namespace YXCLib
{
	void EasyMemoryPool::_FillBuffer(void* pBuffer, ybool_t bOwnBuffer, ysize_t stBufSize)
	{
		this->_pBuffer = (ybyte_t*)pBuffer;
		this->_bOwnBuffer = bOwnBuffer;
		this->_size = stBufSize;
		this->_offset = 0;
	}

	EasyMemoryPool::EasyMemoryPool()
	{
		this->_pBuffer = NULL;
	}

	EasyMemoryPool::~EasyMemoryPool()
	{

	}

	YXC_Status EasyMemoryPool::Create(ybool_t bReportError, ysize_t stCapability)
	{
		this->_bReportError = bReportError;

		ybyte_t* pBuffer = (ybyte_t*)malloc(stCapability);
		if (pBuffer == NULL)
		{
			if (this->_bReportError)
			{
				_YXC_REPORT_NEW_ERR(YXC_ERC_OUT_OF_MEMORY, YC("Can't allocate memory for memory pool"));
			}
			return YXC_ERC_OUT_OF_MEMORY;
		}

		this->_FillBuffer(pBuffer, TRUE, stCapability);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status EasyMemoryPool::Create(void* pAddr, ybool_t bReportError, ysize_t stCapability)
	{
		this->_bReportError = bReportError;

		this->_FillBuffer(pAddr, FALSE, stCapability);

		return YXC_ERC_SUCCESS;
	}

	void* EasyMemoryPool::Alloc(ysize_t stBufLen)
	{
		if (stBufLen >= this->_size / 2) return NULL;

		if (this->_offset + stBufLen <= this->_size)
		{
			ybyte_t* ptr = this->_pBuffer + this->_offset;
			this->_offset += stBufLen;

            if (this->_offset % sizeof(void*) != 0)
            {
                this->_offset -= this->_offset % sizeof(void*);
                this->_offset += sizeof(void*);
            }
			return ptr;
		}

		if (this->_bReportError)
		{
			_YXC_REPORT_NEW_ERR(YXC_ERC_OUT_OF_MEMORY, YC("No block avaiable from the allocation"));
		}

		return NULL;
	}

	void EasyMemoryPool::Clear()
	{
		this->_offset = 0;
	}

	void EasyMemoryPool::Destroy()
	{
		if (this->_bOwnBuffer && this->_pBuffer != NULL)
		{
			free(this->_pBuffer);
			this->_pBuffer = NULL;
		}
	}

	void YMAlloc::SetBaseBuffer(ysize_t stCbBase)
	{
		this->_uBlkCount = _YXC_MAX_MEM_BLOCKS;
		this->_uCurrentCount = 0;
		this->_stCbBase = stCbBase;
	}

	void YMAlloc::Destroy()
	{
		for (yuint32_t i = 0; i < this->_uCurrentCount; ++i)
		{
			free(this->_memBlocks[i].pAllocBase);
		}
	}

	void* YMAlloc::_InflateAlloc(ysize_t stBuffer)
	{
		_YXC_CHECK_REPORT_NEW_RET2(this->_uCurrentCount < this->_uBlkCount, YXC_ERC_OUT_OF_MEMORY, NULL, YC("Number of memory buffer ")
			YC("is full(%d), Alloc size(%lu)"), this->_uCurrentCount, stBuffer);

		ysize_t stCbAlloc = this->_stCbBase * (1 << this->_uCurrentCount);
		void* pMem1 = ::malloc(stCbAlloc);
		_YXC_CHECK_REPORT_NEW_RET2(pMem1 != NULL, YXC_ERC_OUT_OF_MEMORY, NULL, YC("Alloc buffer failed, buf size = %lu"), stCbAlloc);

		this->_memBlocks[this->_uCurrentCount].stBuf = stCbAlloc;
		this->_memBlocks[this->_uCurrentCount].pAllocBase = pMem1;
		this->_memBlocks[this->_uCurrentCount].stOffset = 0;
		++this->_uCurrentCount;

		return this->Alloc(stBuffer);
	}

	void YMAlloc::Clear()
	{
		for (yuint32_t i = 0; i < this->_uCurrentCount; ++i)
		{
			this->_memBlocks[i].stOffset = 0;
		}
	}

    void YMAlloc::Free(void* pBase)
    {
        if (this->_stCbBase == 0)
        {
            free(pBase);
        }
    }

	void* YMAlloc::Alloc(ysize_t stBuffer)
	{
		if (this->_stCbBase == 0)
		{
			return ::malloc(stBuffer);
		}

		if (this->_uCurrentCount == 0)
		{
			return this->_InflateAlloc(stBuffer);
		}
		yuint32_t uIndex = this->_uCurrentCount - 1;

		_EMBlock& rBlock = this->_memBlocks[uIndex];
		if (rBlock.stOffset + stBuffer <= rBlock.stBuf)
		{
			void* pRet = (ybyte_t*)rBlock.pAllocBase + rBlock.stOffset;
			rBlock.stOffset += stBuffer;

            if (stBuffer % sizeof(void*) != 0)
            {
                rBlock.stOffset -= rBlock.stOffset % sizeof(void*);
                rBlock.stOffset += sizeof(void*);
            }
			return pRet;
		}

		return this->_InflateAlloc(stBuffer);
	}

	CYMAlloc::CYMAlloc(ysize_t stCbBase) : _attached(YES)
	{
		this->SetBaseBuffer(stCbBase);
	}

	CYMAlloc::~CYMAlloc()
	{
		if (this->_attached)
		{
			this->Destroy();
		}
	}

	void CYMAlloc::Detach()
	{
		this->_attached = NO;
	}

//		NIC_API(_Memory) nicCreateMemory(yuint32_t size, yuint32_t maxSegments, _CriticalSection threadLock)
//		{
//			_Memory mem;
//			yuint16_t i, *p;
//			if (maxSegments > MAX_ALLOWED_SEGMENTS) maxSegments = MAX_ALLOWED_SEGMENTS;
//			mem = (_Memory)malloc(size + sizeof(struct _MemoryBlock)
//				+ maxSegments * (sizeof(struct _MemoryNode) + sizeof(yuint16_t)));
//			mem->useHead = NULL;
//			mem->threadLock.hdl = NULL;
//			mem->memNodes = (_MemoryNode*)(mem + 1);
//			mem->nodesStorage.offsets = (yuint16_t*)(mem->memNodes + maxSegments);
//			mem->nodesStorage.stackTop = maxSegments;
//			mem->data = (_8U*)(mem->nodesStorage.offsets + maxSegments);
//			mem->size = size;
//			mem->segNum = 0;
//			mem->maxSegments = maxSegments;
//			p = mem->nodesStorage.offsets;
//			for (i = 0; i < maxSegments; ++i)
//			{
//				p[i] = i;
//			}
//			mem->threadLock = threadLock;
//			return mem;
//		}
//
//		NIC_API(void) nicDestroyMemory(_Memory memory)
//		{
//			free(memory);
//		}
//
//		NIC_API(void*) nicMemoryGetBlock(_Memory memory, yuint32_t size, _Boolean zeroFill)
//		{
//			_MemoryNode* p, find;
//			nicEnterCriticalSection(memory->threadLock);
//			size = nicInnerAlign(size);
//			if (memory->size < size || memory->segNum++ >= memory->maxSegments)
//			{
//				nicLeaveCriticalSection(memory->threadLock);
//				return NULL;
//			}
//			if (memory->useHead == NULL)
//			{
//				memory->useHead = __popNodeFromRecycle(&memory->nodesStorage) + memory->memNodes;
//				memory->useHead->baseAddr = memory->size - size;
//				memory->useHead->memSize = size;
//				if (zeroFill) memset(memory->data + memory->size - size, 0, size);
//				return memory->data + memory->size - size;
//			}
//			else
//			{
//				p = memory->useHead;
//				if (p->baseAddr >= size)
//				{
//					find = memory->memNodes + __popNodeFromRecycle(&memory->nodesStorage);
//					memory->useHead = find;
//					find->baseAddr = p->baseAddr - size;
//					find->memSize = size;
//					find->next = p;
//					nicLeaveCriticalSection(memory->threadLock);
//					if (zeroFill) memset(memory->data + find->baseAddr, 0, size);
//					return memory->data + find->baseAddr;
//				}
//				while (p->next != NULL) // loop to find enough block
//				{
//					if (p->next->baseAddr - (p->baseAddr + p->memSize) >= size)
//					{
//						find = memory->memNodes + __popNodeFromRecycle(&memory->nodesStorage);
//						find->baseAddr = p->next->baseAddr - size;
//						goto ret;
//					}
//				}
//				// attemp the last block
//				if (memory->size - (p->baseAddr + p->memSize) >= size)
//				{
//					find = memory->memNodes + __popNodeFromRecycle(&memory->nodesStorage);
//					find->baseAddr = memory->size - size;
//					goto ret;
//				}
//			}
//			return NULL;
//ret:
//			find->next = p->next;
//			p->next = find;
//			find->memSize = size;
//			nicLeaveCriticalSection(memory->threadLock);
//			if (zeroFill) memset(memory->data + find->baseAddr, 0, size);
//			return memory->data + find->baseAddr;
//		}
//
//		NIC_API(_Boolean) nicMemoryFreeBlock(_Memory memory, void* block)
//		{
//			_MemoryNode* p, find;
//			yuint32_t baseAddr;
//			nicEnterCriticalSection(memory->threadLock);
//			baseAddr = (_8U*)block - memory->data;
//			if (memory->useHead == NULL || (_8U*)block < memory->data || (_8U*)block >= memory->data + memory->size)
//			{
//				nicLeaveCriticalSection(memory->threadLock);
//				return NIC_FALSE;
//			}
//			p = memory->useHead;
//			if (p->baseAddr == baseAddr)
//			{
//				__pushNodeToRecycle(&memory->nodesStorage, p - memory->memNodes);
//				memory->useHead = NULL;
//				nicLeaveCriticalSection(memory->threadLock);
//				return NIC_TRUE;
//			}
//			while (p->next != NULL)
//			{
//				if (p->next->baseAddr == baseAddr)
//				{
//					find = p->next;
//					p->next = find->next;
//					__pushNodeToRecycle(&memory->nodesStorage, find - memory->memNodes);
//					nicLeaveCriticalSection(memory->threadLock);
//					return NIC_TRUE;
//				}
//			}
//			return NIC_FALSE;
//		}
}

namespace YXCLib
{
	template <typename T>
	SElementPool<T>::SElementPool()
	{
	}

	template <typename T>
	YXC_Status SElementPool<T>::Create(ysize_t stEleSize, T numElements)
	{
		ysize_t stRoundSize = (stEleSize + sizeof(void*) - 1) / sizeof(void*) * sizeof(void*);
		this->_pEleData = (ybyte_t*)::malloc(stRoundSize * numElements + numElements * sizeof(T));
		this->_bExtAllocated = FALSE;

		_YXC_CHECK_REPORT_NEW_RET(this->_pEleData != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for single element pool failed"));
		this->_pOffsets = (T*)(this->_pEleData + stEleSize * numElements);
		this->_remain = numElements;

		for (T i = 0; i < numElements; ++i)
		{
			this->_pOffsets[i] = i;
		}

		this->_stRoundSize = stRoundSize;
		this->_stElementSize = stEleSize;

		return YXC_ERC_SUCCESS;
	}

	template <typename T>
	void* SElementPool<T>::Alloc(ysize_t stSize)
	{
		if (stSize != this->_stElementSize)
		{
			_YXC_CHECK_REPORT_NEW_RET2(!this->_bExtAllocated, YXC_ERC_OUT_OF_MEMORY, NULL, YC("The external memory has been allocated."));
			_YXC_CHECK_REPORT_NEW_RET2(stSize < _YXC_SEPOOL_MAX_EXT_SIZE, YXC_ERC_OUT_OF_MEMORY, NULL, YC("The external memory size too big, ")
				YC("up to %lu"), _YXC_SEPOOL_MAX_EXT_SIZE);

			this->_bExtAllocated = TRUE;
			return this->_byExtAlloc;
		}
		else
		{
			_YXC_CHECK_REPORT_NEW_RET2(this->_remain > 0, YXC_ERC_OUT_OF_MEMORY, NULL, YC("No elements availble from alloc"));

			T offset = this->_pOffsets[--this->_remain];
			return this->_pEleData + offset * this->_stRoundSize;
		}
	}

	template <typename T>
	void SElementPool<T>::Free(void* pPtr)
	{
		if (pPtr == this->_byExtAlloc)
		{
			this->_bExtAllocated = FALSE;
		}
		else
		{
			T offset = (T)(((ybyte_t*)pPtr - this->_pEleData) / this->_stRoundSize);
			this->_pOffsets[this->_remain++] = offset;
		}
	}

	template <typename T>
	void SElementPool<T>::Destroy()
	{
		free(this->_pEleData);
	}

	template class SElementPool<ybyte_t>;
	template class SElementPool<yuint16_t>;
	template class SElementPool<yuint32_t>;
}

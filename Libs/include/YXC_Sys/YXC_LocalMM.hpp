/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_LOCAL_MM_HPP__
#define __INC_YXC_SYS_BASE_LOCAL_MM_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <list>

#define _YXC_LOCAL_MM_MAX_ALLOWED_SEGMENTS (1 << 15)
#define _YXC_SEPOOL_MAX_EXT_SIZE 100
#define _YXC_MAX_MEM_BLOCKS 10

namespace YXCLib
{
	class YXC_CLASS EasyMemoryPool
	{
	public:
		YXC_Status Create(ybool_t bReportError, ysize_t stPoolCapability);

		YXC_Status Create(void* pAddr, ybool_t bReportError, ysize_t stPoolCapability);

		void Destroy();

		void* Alloc(ysize_t stBufLen);

		void Clear();

	public:
		inline ybool_t IsCreated() { return this->_pBuffer != NULL; }

		inline ysize_t GetOffset() { return this->_offset; }

		inline void SetOffset(ysize_t stOffset) { this->_offset = stOffset; }

		inline ybyte_t* GetBuffer() { return this->_pBuffer; }

	private:
		void _FillBuffer(void* pBuffer, ybool_t bOwnBuffer, ysize_t stBufSize);

	public:
		EasyMemoryPool();

		~EasyMemoryPool();

	private:
		EasyMemoryPool(const EasyMemoryPool&);

		EasyMemoryPool& operator =(const EasyMemoryPool&);

	private:
		ybyte_t* _pBuffer;

		ysize_t _offset;
		ysize_t _size;
		ybool_t _bOwnBuffer;
		ybool_t _bReportError;
	};

	class SysMemoryPool
	{
	public:
		inline YXC_Status Create(ybool_t bReportError, ysize_t stPoolCapability, ysize_t stBaseBlkSize)
		{
			return YXC_ERC_SUCCESS;
		}

		inline void Destroy()
		{

		}

		inline void* Alloc(ysize_t stBufLen)
		{
			return malloc(stBufLen);
		}

		inline void Free(void* pAddr)
		{
			free(pAddr);
		}

		inline void Clear()
		{

		}
	};

	typedef SysMemoryPool FixedMemoryPool;

	struct _EMBlock
	{
		ysize_t stOffset;
		ysize_t stBuf;
		void* pAllocBase;
	};

	class YXC_CLASS YMAlloc
	{
	public:
		void SetBaseBuffer(ysize_t stCbBase);

		void* Alloc(ysize_t stBuffer);

        void Free(void* pBase);

		void Destroy();

		void Clear();

		template <typename T>
		inline T* AllocT()
		{
			return (T*)this->Alloc(sizeof(T));
		}

	private:
		void* _InflateAlloc(ysize_t stBuffer);

	private:
		yuint32_t _uCurrentCount;

		yuint32_t _uBlkCount;

		ysize_t _stCbBase;

		_EMBlock _memBlocks[_YXC_MAX_MEM_BLOCKS];
	};

	class YXC_CLASS CYMAlloc : public YMAlloc
	{
	public:
		CYMAlloc(ysize_t stCbBase = 1 << 10);

		~CYMAlloc();

	public:
		void Detach();

	private:
		ybool_t _attached;
	};

	//class ListMemoryPool
	//{
	//public:
	//	YXC_Status Create(ybool_t bReportError, ysize_t stPoolCapability, ysize_t stMaxNumBlocks);

	//	void Destroy();

	//	void* Alloc(ysize_t stBufLen);

	//	void Free(void* pAddr);

	//	void Clear();

	//private:
	//	std::list<void*> _liUsedBlocks;

	//	std::list<void*> _liFreeBlocks;

	//	ybyte_t* _data;

	//	ysize_t _len;

	//	ysize_t _freeLen;//Œ¥∑÷≈‰≥§∂»
	//
	//struct _MemoryNode
	//{
	//	struct _MemoryNode* next;
	//	yuint32_t baseAddr;
	//	yuint32_t memSize;
	//};

	//struct _MemoryBlock
	//{
	//	YXC_Crit threadLock;
	//	_MemoryNode* useHead;
	//	// struct _MemoryNode* freeHead;
	//	yuint16_t segNum;
	//	yuint16_t maxSegments;
	//	yuint32_t size;
	//	_MemoryNode* memNodes;
	//	_NodeBinStack nodesStorage;
	//	ybyte_t* data;
	//};
	//};

	template <typename T>
	class YXC_CLASS SElementPool
	{
	public:
		SElementPool();

		YXC_Status Create(ysize_t stEleSize, T numElements);

		void Destroy();

		void* Alloc(ysize_t stSize);

		void Free(void* pPtr);

	private:
		SElementPool(const SElementPool&);

		SElementPool& operator =(const SElementPool&);

	private:
		ybyte_t _byExtAlloc[_YXC_SEPOOL_MAX_EXT_SIZE]; // for stl
		ybool_t _bExtAllocated;

		ysize_t _stElementSize;
		ysize_t _stRoundSize;

		T* _pOffsets;
		T _remain;

		ybyte_t* _pEleData;
	};

	static inline void _MakeSureBufferOrFree(void*& pBuffer, ysize_t& stBufferSize, ysize_t cbRequired)
	{
		if (stBufferSize < cbRequired)
		{
			if (pBuffer)
			{
				free(pBuffer);
			}

			pBuffer = ::malloc(cbRequired);
			stBufferSize = (pBuffer != NULL) ? cbRequired : 0;
		}
	}
}

#endif /* __INC_YXC_SYS_BASE_LOCAL_MM_HPP__ */

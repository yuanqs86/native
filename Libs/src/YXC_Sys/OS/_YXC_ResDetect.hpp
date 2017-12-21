#ifndef __INNER_INC_YXC_SYS_BASE_HEAP_DETECT_HPP__
#define __INNER_INC_YXC_SYS_BASE_HEAP_DETECT_HPP__

#include <YXC_Sys/YXC_HeapDetect.h>
#include <YXC_Sys/YXC_GDIDetect.h>
#include <YXC_Sys/YXC_StlAlloc.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <set>
#include <map>

#define _YXC_DEF_PE_MODULES 400
#define _YXC_PE_MAX_FILE_NAME 64

#define _YXC_RES_REC_GDI 0
#define _YXC_RES_REC_HEAP 1

#define _YXC_NT_MAX_HEAP_PTR_COUNT (2 << 20)
#define _YXC_NT_MAX_HEAP_LINE_COUNT (1 << 18)
#define _YXC_NT_MAX_GDI_LINE_COUNT (1 << 18)
#define _YXC_NT_MAX_GDI_OBJ_COUNT (1 << 18)

#define _YXC_NT_RES_NAMED_PIPE_INSTANCES 4

namespace YXC_Inner
{
	template <typename T>
	struct _NodeCvt : public std::_Tree_nod<T>
	{
	public:
		typedef typename _Node Node;
	};

	template <typename TVal, typename TValM>
	struct _ATypeCvt
	{
		typedef std::pair<void*, TValM> _PairPtr;
		typedef YXCLib::StlAlloc< YXCLib::SElementPool<yuint32_t>, std::pair<void* const, TValM> > _PtrAlloc;
		typedef std::_Tmap_traits<void*, TValM, std::less<void*>, _PtrAlloc, false> _PtrMapTraits;
		typedef typename _NodeCvt<_PtrMapTraits>::Node _PtrMapNode;
		typedef std::map<void*, TValM, std::less<void*>, _PtrAlloc> _PtrMap;

		typedef YXCLib::StlAlloc< YXCLib::SElementPool<yuint32_t>, TVal > _LineAlloc;
		typedef std::_Tset_traits<TVal, std::less<TVal>, _LineAlloc, false> _LineSetTraits;
		typedef typename _NodeCvt<_LineSetTraits>::Node _LineSetNode;
		typedef std::set<TVal, std::less<TVal>, _LineAlloc> _LineSet;
	};

	typedef std::pair<YXC_HeapAllocRecord*, ysize_t> _HDMapValue;
	typedef YXC_GdiAllocRecord* _GDMapValue;

	typedef _ATypeCvt< YXC_HeapAllocRecord, _HDMapValue > _HeapCvt;
	typedef _ATypeCvt< YXC_GdiAllocRecord, _GDMapValue > _GdiCvt;

	typedef _HeapCvt::_PairPtr _HDMapPair;
	typedef _HeapCvt::_PtrAlloc _HDMapAlloc;
	typedef _HeapCvt::_PtrMap _HDMap;
	typedef _HeapCvt::_PtrMapNode _HDMapNode;
	typedef _HeapCvt::_LineAlloc _HDLSetAlloc;
	typedef _HeapCvt::_LineSetNode _HDLSetNode;
	typedef _HeapCvt::_LineSet _HDLSet;

	typedef _GdiCvt::_PairPtr _GDMapPair;
	typedef _GdiCvt::_PtrAlloc _GDMapAlloc;
	typedef _GdiCvt::_PtrMap _GDMap;
	typedef _GdiCvt::_PtrMapNode _GDMapNode;
	typedef _GdiCvt::_LineAlloc _GDLSetAlloc;
	typedef _GdiCvt::_LineSetNode _GDLSetNode;
	typedef _GdiCvt::_LineSet _GDLSet;

	struct _ModuleInfoX
	{
		wchar_t szFilename[_YXC_PE_MAX_FILE_NAME];
		yuintptr_t upBaseAddr;
		ysize_t stImageSize;
	};

#pragma pack(push, 4)
	struct _ResRecordHeader
	{
		yuint32_t uProcessId;
		yuint32_t uNumRecords;
		ybyte_t byRecType;
	};
#pragma pack(pop)

	//template <typename TVal, typename TValM>
	//struct _ResDetectImpl
	//{
	//	YXC_Status _AllocResultHeapResult(_HDLSet** ppLineSet, SElementPool<yuint32_t>** ppSPool, yuint32_t uMaxCount);
	//};

	//typedef std::pair<YXC_HeapAllocRecord*, ysize_t> _HPtrValue;
	//typedef std::pair<void*, _HPtrValue> _PairHPtr;
	//typedef StlAlloc< SElementPool<yuint32_t>, std::pair<void* const, _HPtrValue> > _HPtrAlloc;
	//typedef std::_Tmap_traits<void*, _HPtrValue, std::less<void*>, _HPtrAlloc, false> _HPtrMapTraits;
	//typedef _NodeCvt<_HPtrMapTraits>::Node _HPtrMapNode;
	//typedef std::map<void*, _HPtrValue, std::less<void*>, _HPtrAlloc> _HPtrMap;

	//typedef StlAlloc<SElementPool<yuint32_t>, YXC_HeapAllocRecord> _LineAlloc;
	//typedef std::_Tset_traits<YXC_HeapAllocRecord, std::less<YXC_HeapAllocRecord>, _LineAlloc, false> _LineSetTraits;
	//typedef _NodeCvt<_LineSetTraits>::Node _LineSetNode;
	//typedef std::set<YXC_HeapAllocRecord, std::less<YXC_HeapAllocRecord>, _LineAlloc> _LineSet;

	//typedef YXC_GdiAllocRecord* _GdiValue;
	//typedef std::pair<void*, _HPtrValue> _PairHPtr;
	//typedef StlAlloc< SElementPool<yuint32_t>, std::pair<void* const, _HPtrValue> > _HPtrAlloc;
	//typedef std::_Tmap_traits<void*, _HPtrValue, std::less<void*>, _HPtrAlloc, false> _HPtrMapTraits;
	//typedef _NodeCvt<_HPtrMapTraits>::Node _HPtrMapNode;
	//typedef std::map<void*, _HPtrValue, std::less<void*>, _HPtrAlloc> _HPtrMap;

	//typedef StlAlloc<SElementPool<yuint32_t>, YXC_HeapAllocRecord> _LineAlloc;
	//typedef std::_Tset_traits<YXC_HeapAllocRecord, std::less<YXC_HeapAllocRecord>, _LineAlloc, false> _LineSetTraits;
	//typedef _NodeCvt<_LineSetTraits>::Node _LineSetNode;
	//typedef std::set<YXC_HeapAllocRecord, std::less<YXC_HeapAllocRecord>, _LineAlloc> _LineSet;

	static inline int _CompareRecord(const YXC_ResourceAlloc& r1, const YXC_ResourceAlloc& r2)
	{
		if (r1.uNumStacks < r2.uNumStacks)
		{
			return -1;
		}
		else if (r1.uNumStacks > r2.uNumStacks)
		{
			return 1;
		}
		else
		{
			for (yuint32_t i = 0; i < r1.uNumStacks; ++i)
			{
				if (r1.upAllocStacks[i] < r2.upAllocStacks[i]) return -1;
				else if (r1.upAllocStacks[i] > r2.upAllocStacks[i]) return 1;
			}
			return 0;
		}
	}

	static inline void _FindCallStack(YXC_ResourceAlloc* pRecord, CONTEXT* pContext)
	{
		yuint32_t uMachineType = IMAGE_FILE_MACHINE_I386;

#if YXC_IS_32BIT
		yuintptr_t uEip = pContext->Eip;
		yuintptr_t uEbpBase = pContext->Ebp;
#else
		yuintptr_t uEip = pContext->Rip;
		yuintptr_t uEbpBase = pContext->Rbp;
#endif /* YXC_IS_32BIT */

		pRecord->uNumStacks = ::RtlCaptureStackBackTrace(0, YXC_RESOURCE_MAX_STACK, (PVOID*)pRecord->upAllocStacks, NULL);
		//__try
		//{
		//	while (TRUE)
		//	{
		//		pRecord->upAllocStacks[pRecord->uNumStacks++] = uEip;

		//		if (pRecord->uNumStacks >= YXC_RESOURCE_MAX_STACK) break;

		//		yuintptr_t* puBase = (yuintptr_t*)uEbpBase;

		//		uEbpBase = *puBase;
		//		if (uEbpBase == 0) break;

		//		uEip = *(puBase + 1);
		//	}
		//}
		//__except (1)
		//{
		//}

		return;
	}

	void _DumpResCallStack(HANDLE hProcess, const YXC_ResourceAlloc& record, _ModuleInfoX* pModules,
		yuint32_t uNumModules, FILE* fp);

	YXC_Status _InitModuleInfo(HANDLE hProcess, _ModuleInfoX** ppModules, yuint32_t* puNumModules);

	void _FreeModuleInfo(_ModuleInfoX* pModules);

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_HeapAInfo, _HDLSet, _HeapAPtr, _HeapAHdl);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_GdiAInfo, _GDLSet, _GdiAPtr, _GdiAHdl);

}

#endif /* __INC_YXC_SYS_BASE_HEAP_DETECT_HPP__ */

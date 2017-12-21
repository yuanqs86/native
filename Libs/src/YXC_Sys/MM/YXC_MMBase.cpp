#define __MODULE__ "EK.MMBase"
#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_Sys/YXC_MMInterface.h>
#include <YXC_Sys/MM/_YXC_MMModelBase.hpp>
#include <YXC_Sys/MM/_YXC_MMModelFixed.hpp>
#include <YXC_Sys/MM/_YXC_MMModelFlat.hpp>

using namespace YXC_Inner;

//#define YXC_MM_MODEL_TYPE_FIXED 0
//#define YXC_MM_MODEL_TYPE_FLAT 1
//#define YXC_MM_MODEL_TYPE_C_RUNTIME 2

namespace
{
	static void* Alloc(ysize_t stNumBytes, YXC_Inner::_MMModelBase* pModelBase)
	{
		return pModelBase->Alloc(stNumBytes);
	}

	static void Free(void* pData, YXC_Inner::_MMModelBase* pModelBase)
	{
		pModelBase->Free(pData);
	}

	static void* CAlloc(ysize_t stNumBytes, void*)
	{
		return malloc(stNumBytes);
	}

	static void CFree(void* pData, void*)
	{
		free(pData);
	}

	void _DestroyMMModel(YXC_Inner::_MMModelBase* pModel)
	{
		pModel->~_MMModelBase();
		free(pModel);
	}
}

extern "C"
{
	YXC_Status YXC_MMCreateFixed(ysize_t stNumBlocks, ysize_t stMaxBlkSize, YXC_MMAllocator* pAllocator)
	{
		_MMModelFixed* pFixedModel = (_MMModelFixed*)malloc(sizeof(_MMModelFixed));

		_YXC_CHECK_REPORT_NEW_RET(pFixedModel != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc fix model pointer failed"));

		new (pFixedModel) _MMModelFixed();

		YXC_Status rc = pFixedModel->Init(stMaxBlkSize, stNumBlocks);

		if (rc != YXC_ERC_SUCCESS)
		{
			pFixedModel->~_MMModelFixed();
			free(pFixedModel);
			return rc;
		}

		pAllocator->pAllocator = pFixedModel;
		pAllocator->pAlloc = (YXC_MMAllocFunc)Alloc;
		pAllocator->pFree = (YXC_MMFreeFunc)Free;
		pAllocator->allocType = YXC_MM_ALLOCATOR_TYPE_FIXED;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_MMCreateFlat(ysize_t stPoolSize, YXC_MMAllocator* pAllocator)
	{
		_MMModelFlat* pFlatModel = (_MMModelFlat*)malloc(sizeof(_MMModelFlat));

		_YXC_CHECK_REPORT_RET(pFlatModel != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc flat model pointer failed"));

		new (pFlatModel) _MMModelFlat();

		YXC_Status rc = pFlatModel->Init(stPoolSize);

		if (rc != YXC_ERC_SUCCESS)
		{
			pFlatModel->~_MMModelFlat();
			free(pFlatModel);
			return rc;
		}

		pAllocator->pAllocator = pFlatModel;
		pAllocator->pAlloc = (YXC_MMAllocFunc)Alloc;
		pAllocator->pFree = (YXC_MMFreeFunc)Free;
		pAllocator->allocType = YXC_MM_ALLOCATOR_TYPE_FLAT;

		return YXC_ERC_SUCCESS;
	}

	void YXC_MMCreateCRunTime(YXC_MMAllocator* pAllocator)
	{
		pAllocator->pAllocator = NULL;
		pAllocator->pAlloc = (YXC_MMAllocFunc)CAlloc;
		pAllocator->pFree = (YXC_MMFreeFunc)CFree;
		pAllocator->allocType = YXC_MM_ALLOCATOR_TYPE_C_RUNTIME;
	}

	void YXC_MMDestroy(YXC_MMAllocator* pAllocator)
	{
		switch (pAllocator->allocType)
		{
		case YXC_MM_ALLOCATOR_TYPE_FLAT:
		case YXC_MM_ALLOCATOR_TYPE_FIXED:
			_DestroyMMModel((_MMModelBase*)pAllocator->pAllocator);
			break;
		default:
			break;
		}
	}

	YXC_Status YXC_MMCExpandBuffer(void** ppBuffer, ysize_t* pstBufferSize, ysize_t cbRequired)
	{
		void*& pBuffer = *ppBuffer;
		ysize_t& stBufferSize = *pstBufferSize;
		if (stBufferSize < cbRequired)
		{
			ysize_t cbReal = YXCLib::TMax<ysize_t>(cbRequired * 2, 1024);

			void* pNewBuffer = ::malloc(cbReal);
			_YXC_CHECK_REPORT_NEW_RET(pNewBuffer != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Failed to alloc for memory(%ld)"), cbRequired);

			if (pBuffer)
			{
				memcpy(pNewBuffer, pBuffer, stBufferSize);
				free(pBuffer);
			}
			pBuffer = pNewBuffer;
			stBufferSize = cbReal;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_MMCMakeSureBufferOrFree(void** ppBuffer, ysize_t* pstBufferSize, ysize_t cbRequired)
	{
		void*& pBuffer = *ppBuffer;
		ysize_t& stBufferSize = *pstBufferSize;
		if (stBufferSize < cbRequired)
		{
			if (pBuffer)
			{
				free(pBuffer);
			}

			pBuffer = ::malloc(cbRequired);
			stBufferSize = (pBuffer != NULL) ? cbRequired : 0;
			_YXC_CHECK_REPORT_NEW_RET(pBuffer != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Failed to alloc for memory(%ld)"), cbRequired);
		}
		return YXC_ERC_SUCCESS;
	}

	void YXC_MMCFreeData(void* buffer)
	{
		free(buffer);
	}
}

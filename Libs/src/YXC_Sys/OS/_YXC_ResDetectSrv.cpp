#define __MODULE__ "EK.Heap.Detect"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/OS/_YXC_ResDetectSrv.hpp>
#include <YXC_Sys/OS/_YXC_ResDetect.hpp>
#include <YXC_Sys/YXC_HeapDetect.h>
#include <YXC_Sys/YXC_GdiDetect.h>
#include <YXC_Sys/YXC_Locker.hpp>

#define __YXC_HEAP_DETECT_DEF_BUF_SIZE (1 << 18)

using namespace YXC_Inner;
namespace YXC_Inner
{
	extern YXX_Crit& gs_resPipeLock;
}
namespace
{
	ybool_t gs_resDetectEnabled;

	void _SetBufferOrNull(_ResDetectSrvData* pDetectSrv, ysize_t stCbRequired)
	{
		if (pDetectSrv->stBufferSize < stCbRequired)
		{
			if (pDetectSrv->pbyBuffer)
			{
				free(pDetectSrv->pbyBuffer);
			}

			void* pBuffer = ::malloc(stCbRequired);
			if (pBuffer != NULL)
			{
				pDetectSrv->stBufferSize = stCbRequired;
				pDetectSrv->pbyBuffer = (ybyte_t*)pBuffer;
			}
			else
			{
				pDetectSrv->stBufferSize = 0;
				pDetectSrv->pbyBuffer = NULL;
			}
		}

		pDetectSrv->stRetSize = pDetectSrv->pbyBuffer != NULL ? stCbRequired : 0;
	}

	void _ReturnError(_ResDetectSrvData* pDetectSrv, YXC_Status rc, const wchar_t* cpszMsg)
	{
		yuint32_t uStrLen = (yuint32_t)wcslen(cpszMsg), uStrSize = YXC_STRING_SIZEW(uStrLen);

		_SetBufferOrNull(pDetectSrv, 2 * sizeof(yuint32_t) + uStrSize);

		if (pDetectSrv->stRetSize != 0)
		{
			yuint32_t* pStatus = (yuint32_t*)pDetectSrv->pbyBuffer;
			*pStatus = rc;

			yuint32_t* puStrLen = pStatus + 1;
			*puStrLen = uStrLen;

			wchar_t* pszMsg = (wchar_t*)puStrLen + 1;
			memcpy(pszMsg, cpszMsg, uStrSize);
		}
	}

	YXC_Status _HeapQuery(_ResDetectSrvData* pDetectSrv)
	{
		YXC_HeapAInfo heapAllocInfo;
		YXC_Status rc = YXC_QueryHeapAllocs(&heapAllocInfo);

		if (rc == YXC_ERC_SUCCESS)
		{
			_HDLSet* pLineSet = _HeapAPtr(heapAllocInfo);
			ysize_t stCbBuf = pLineSet->size() * sizeof(YXC_HeapAllocRecord);

			_SetBufferOrNull(pDetectSrv, stCbBuf + 2 * sizeof(yuint32_t));

			if (pDetectSrv->stRetSize != 0)
			{
				yuint32_t* pStatus = (yuint32_t*)pDetectSrv->pbyBuffer;
				*pStatus = YXC_ERC_SUCCESS;

				yuint32_t* pRecNum = pStatus + 1;
				*pRecNum = pLineSet->size();

				YXC_HeapAllocRecord* pRecBuf = (YXC_HeapAllocRecord*)(pRecNum + 1);
				for (_HDLSet::iterator it = pLineSet->begin(); it != pLineSet->end(); ++it)
				{
					*pRecBuf++ = *it;
				}
			}
			else
			{
				rc = YXC_ERC_OUT_OF_MEMORY;
				_ReturnError(pDetectSrv, YXC_ERC_OUT_OF_MEMORY, L"Not memory available for heap query");
			}

			YXC_FreeHeapAllocs(heapAllocInfo);
		}
		else
		{
			_ReturnError(pDetectSrv, rc, ::YXC_GetLastErrorMessage());
		}

		return rc;
	}

	YXC_Status _GdiQuery(_ResDetectSrvData* pDetectSrv)
	{
		//YXC_GdiAInfo gdiAllocInfo;
		//YXC_Status rc = YXC_QueryGdiAllocs(&gdiAllocInfo);

		//if (rc == YXC_ERC_SUCCESS)
		//{
		//	_GDLSet* pLineSet = _GdiAPtr(gdiAllocInfo);
		//	ysize_t stCbBuf = pLineSet->size() * sizeof(YXC_GdiAllocRecord);

		//	_SetBufferOrNull(pDetectSrv, stCbBuf + 2 * sizeof(yuint32_t));

		//	if (pDetectSrv->stRetSize != 0)
		//	{
		//		yuint32_t* pStatus = (yuint32_t*)pDetectSrv->pbyBuffer;
		//		*pStatus = YXC_ERC_SUCCESS;

		//		yuint32_t* pRecNum = pStatus + 1;
		//		*pRecNum = pLineSet->size();

		//		YXC_GdiAllocRecord* pRecBuf = (YXC_GdiAllocRecord*)(pRecNum + 1);
		//		for (_GDLSet::iterator it = pLineSet->begin(); it != pLineSet->end(); ++it)
		//		{
		//			*pRecBuf++ = *it;
		//		}
		//	}
		//	else
		//	{
		//		rc = YXC_ERC_OUT_OF_MEMORY;
		//		_ReturnError(pDetectSrv, YXC_ERC_OUT_OF_MEMORY, L"Not memory available for gdi query");
		//	}

		//	YXC_FreeGdiAllocs(gdiAllocInfo);
		//}
		//else
		//{
		//	_ReturnError(pDetectSrv, rc, ::YXC_GetLastErrorMessage());
		//}

		return 0;
	}
}

namespace YXC_Inner
{
	_ResDetectSrv gs_hDetectSrv;

	static YXC_Status _ResQueryProc(const void* pInBuffer, ysize_t stCbIn, void** pOutBuffer,
		ysize_t* pstCbOut, void* pSrvData)
	{
		_ResDetectSrvData* pDetectSrv = (_ResDetectSrvData*)pSrvData;

		if (stCbIn != sizeof(ybyte_t)) return YXC_ERC_NET_INVALID_PROTOCOL;
		ybyte_t byQueryType = *(ybyte_t*)pInBuffer;

		YXC_Status rc = YXC_ERC_UNKNOWN;

		switch (byQueryType)
		{
		case _YXC_RES_REC_HEAP:
			rc = _HeapQuery(pDetectSrv);
			break;
		case _YXC_RES_REC_GDI:
			rc = _GdiQuery(pDetectSrv);
			break;
		default:
			_YXC_REPORT_NEW_ERR(YXC_ERC_INVALID_TYPE, L"Invalid resource query type %d", byQueryType);
			rc = YXC_ERC_INVALID_TYPE;
			break;
		}

		*pOutBuffer = pDetectSrv->pbyBuffer;
		*pstCbOut = pDetectSrv->stRetSize;
		return rc;
	}

	YXC_Status _ResDetectSrv::Create(ybool_t bInDllMain, yuint32_t uNumSrvs)
	{
		if (uNumSrvs > _YXC_NT_RES_NAMED_PIPE_INSTANCES)
		{
			uNumSrvs = _YXC_NT_RES_NAMED_PIPE_INSTANCES;
		}
		memset(this, 0, sizeof(*this));

		void* ppSrvs[_YXC_NT_RES_NAMED_PIPE_INSTANCES];
		for (yuint32_t i = 0; i < uNumSrvs; ++i)
		{
			ppSrvs[i] = &this->_arrSubSrvs[i];
		}

		yuint32_t uPid = YXCLib::OSGetCurrentProcessId();

		wchar_t szNamedPipe[YXC_NAMED_PIPE_MAX_NAME];
		swprintf_s(szNamedPipe, L"__yxc_hDetect_xfv_42_%d_ax_fxed", uPid);

		yuint32_t uFlags =  bInDllMain ? YXC_NPIPE_FLAGS_NO_THREAD_WAITING : 0;

		YXC_Status rc = YXC_NPipeServerCreateEx(szNamedPipe, NULL, uNumSrvs, 1000, 1024, 1 << 16,
			ppSrvs, _ResQueryProc, uFlags, &this->_srv);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Create resource detect service failed");

		this->_uNumSrvs = uNumSrvs;
		return rc;
	}

	void _ResDetectSrv::Close()
	{
		YXC_NPipeServerClose(this->_srv);

		for (yuint32_t i = 0; i < this->_uNumSrvs; ++i)
		{
			if (this->_arrSubSrvs[i].pbyBuffer)
			{
				free(this->_arrSubSrvs[i].pbyBuffer);
			}
		}
	}

	YXC_Status _InitResDetectSrv(ybool_t bInDllMain)
	{
		YXX_CritLocker locker(gs_resPipeLock);

		if (gs_resDetectEnabled) return YXC_ERC_SUCCESS;

		YXC_Status rc = gs_hDetectSrv.Create(bInDllMain, _YXC_NT_RES_NAMED_PIPE_INSTANCES);
		gs_resDetectEnabled = rc == YXC_ERC_SUCCESS;
		return rc;
	}

	void _FiniResDetectSrv()
	{
		YXX_CritLocker locker(gs_resPipeLock);

		if (gs_resDetectEnabled)
		{
			gs_hDetectSrv.Close();
			gs_resDetectEnabled = FALSE;
		}
	}
}

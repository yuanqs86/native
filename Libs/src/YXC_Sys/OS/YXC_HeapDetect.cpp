#define __MODULE__ "EK.ResDetect.Heap"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_HeapDetect.h>
#include <YXC_Sys/OS/_YXC_ResDetect.hpp>
#include <YXC_Sys/OS/_YXC_ResDetectSrv.hpp>
#include <YXC_Sys/OS/_YXC_FuncHook.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_StlAlloc.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <map>
#include <set>
#include <algorithm>

using YXCLib::SElementPool;

#if YXC_PLATFORM_WIN
#include <DbgHelp.h>
#include <Shlwapi.h>

static bool operator <(const YXC_HeapAllocRecord& r1, const YXC_HeapAllocRecord& r2)
{
	return YXC_Inner::_CompareRecord(r1.rBase, r2.rBase) < 0;
}

static bool _SortByBytesFunc(const YXC_HeapAllocRecord& r1, const YXC_HeapAllocRecord& r2)
{
	return r1.stAllocationSize > r2.stAllocationSize;
}

using namespace YXC_Inner;
namespace
{
	ybyte_t gs_byCritLock[sizeof(YXX_Crit)];
	YXX_Crit& gs_heapLock = *(YXX_Crit*)gs_byCritLock;
	ybool_t gs_bHeapDetectionInited = FALSE;

	typedef LPVOID (__stdcall* HeapAllocFunc)(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
	typedef BOOL (__stdcall* HeapFreeFunc)(HANDLE hHeap, DWORD dwFlags, LPVOID pMem);
	typedef LPVOID (__stdcall* HeapReAllocFunc)(HANDLE hHeap, DWORD dwFlags, LPVOID pMem, SIZE_T dwBytes);
	HeapAllocFunc gs_OldHeapAlloc = NULL;
	HeapFreeFunc gs_OldHeapFree = NULL;
	HeapReAllocFunc gs_OldHeapRealloc = NULL;

	SElementPool<yuint32_t> gs_allocByLine;
	SElementPool<yuint32_t> gs_allocByPtr;

	ybyte_t gs_byLineSet[sizeof(_HDLSet)];
	ybyte_t gs_byPtrMap[sizeof(_HDMap)];
	_HDLSet* gs_pLineSet = (_HDLSet*)gs_byLineSet; // (std::less<YXC_HeapAllocRecord>(), _LineAlloc(&gs_allocByLine, -1));
	_HDMap* gs_pPtrMap = (_HDMap*)gs_byPtrMap; // (std::less<void*>(), _HPtrAlloc(&gs_allocByPtr, -1));

	static BYTE gs_copiedHAllocBytes[_YXC_ASM_HOOKED_PROC_SIZE];
	static BYTE gs_copiedHFreeBytes[_YXC_ASM_HOOKED_PROC_SIZE];
	static BYTE gs_copiedHReallocBytes[_YXC_ASM_HOOKED_PROC_SIZE];
	static HeapAllocFunc gs_copiedHAlloc = (HeapAllocFunc)&gs_copiedHAllocBytes[0];
	static HeapFreeFunc gs_copiedHFree = (HeapFreeFunc)&gs_copiedHFreeBytes[0];
	static HeapReAllocFunc gs_copiedHRealloc = (HeapReAllocFunc)&gs_copiedHReallocBytes[0];

	void _AddAllocContext(void* pMemory, ysize_t uNumBytes, CONTEXT* pContext)
	{
		YXC_ResourceAlloc resAlloc = { 0, 1, { 0 } };
		_FindCallStack(&resAlloc, pContext);

		YXC_HeapAllocRecord heapRecord = { resAlloc, uNumBytes };

		_HDLSet::iterator it = gs_pLineSet->find(heapRecord);
		YXC_HeapAllocRecord* pRecord = NULL;

		if (gs_pPtrMap->size() >= gs_pPtrMap->max_size() - 1)
		{
			return;
		}

		if (it != gs_pLineSet->end())
		{
			pRecord = (YXC_HeapAllocRecord*)&(*it);
			++pRecord->rBase.iCallCount;
			pRecord->stAllocationSize += uNumBytes;
		}
		else
		{
			if (gs_pLineSet->size() >= gs_pLineSet->max_size() - 1) // max heap line reached, can't do detection!.
			{
				return;
			}

			pRecord = (YXC_HeapAllocRecord*)&(*gs_pLineSet->insert(heapRecord).first);
		}

		gs_pPtrMap->insert(_HDMapPair(pMemory, _HDMapValue(pRecord, uNumBytes)));
	}

	void _RemoveAllocContext(void* pMemory)
	{
		_HDMap::iterator it = gs_pPtrMap->find(pMemory);

		if (it != gs_pPtrMap->end()) // Allocated in hook
		{
			YXC_HeapAllocRecord* pRecord = it->second.first;
			yuint32_t i = --pRecord->rBase.iCallCount;
			if (i == 0)
			{
				gs_pLineSet->erase(*pRecord);
			}
			else
			{
				ysize_t uSz = it->second.second;
				pRecord->stAllocationSize -= uSz;
			}
			gs_pPtrMap->erase(it);
		}
	}

	YXC_Status _FillResultHeapSet(_HDLSet* pLineSet)
	{
		try
		{
			YXX_CritLocker locker(gs_heapLock);
			_YXC_CHECK_REPORT_NEW_RET(gs_bHeapDetectionInited, YXC_ERC_NOT_INITIALIZED, L"Heap hook is not initialized yet");
			pLineSet->insert(gs_pLineSet->begin(), gs_pLineSet->end());
			return YXC_ERC_SUCCESS;
		}
		catch (std::exception& e)
		{
			_YXC_REPORT_NEW_ERR(YXC_ERC_C_RUNTIME, L"Caught c++ exception %S", e.what());
			return YXC_ERC_C_RUNTIME;
		}
	}

	void _HeapDiff(_HDLSet* pAlloc1, _HDLSet* pAlloc2, _HDLSet* pResult)
	{
		_HDLSet::iterator it1 = pAlloc1->begin();
		_HDLSet::iterator it2 = pAlloc2->begin();

		while (it1 != pAlloc1->end() && it2 != pAlloc2->end())
		{
			if (*it1 < *it2) /* add iter1 to result and mark as deallocated */
			{
				YXC_HeapAllocRecord hFreed = *it1++;
				hFreed.stAllocationSize = -hFreed.stAllocationSize;
				hFreed.rBase.iCallCount = -hFreed.rBase.iCallCount;
				pResult->insert(hFreed);
			}
			else if (*it2 < *it1)
			{
				YXC_HeapAllocRecord hAlloc = *it2++;
				pResult->insert(hAlloc);
			}
			else
			{
				YXC_HeapAllocRecord h1 = *it1++;
				YXC_HeapAllocRecord h2 = *it2++;
				YXC_HeapAllocRecord hDiff = h1;

				hDiff.stAllocationSize = h2.stAllocationSize - h1.stAllocationSize;
				hDiff.rBase.iCallCount = h2.rBase.iCallCount - h1.rBase.iCallCount;

				if (hDiff.stAllocationSize != 0 || hDiff.rBase.iCallCount != 0)
				{
					pResult->insert(hDiff);
				}
			}
		}

		// Insert remain buffers
		for (; it1 != pAlloc1->end(); ++it1)
		{
			YXC_HeapAllocRecord hFreed = *it1;
			hFreed.stAllocationSize = -hFreed.stAllocationSize;
			pResult->insert(hFreed);
		}

		for (; it2 != pAlloc2->end(); ++it2)
		{
			YXC_HeapAllocRecord hAlloc = *it2;
			pResult->insert(hAlloc);
		}
	}

	YXC_Status _AllocResultHeapResult(_HDLSet** ppLineSet, SElementPool<yuint32_t>** ppSPool, yuint32_t uMaxCount)
	{
		_HDLSet* pRecSet = (_HDLSet*)::malloc(sizeof(_HDLSet) + sizeof(SElementPool<yuint32_t>));
		_YXC_CHECK_REPORT_NEW_RET(pRecSet != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc for record alloc info failed");

		SElementPool<yuint32_t>* pElePool = (SElementPool<yuint32_t>*)(pRecSet + 1);

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rcPool = pElePool->Create(sizeof(_HDLSetNode), uMaxCount);
		_YXC_CHECK_REPORT_GOTO(rcPool == YXC_ERC_SUCCESS, rcPool, L"Alloc for record alloc info storage failed");

		new (pRecSet) _HDLSet(std::less<YXC_HeapAllocRecord>(), _HDLSetAlloc(pElePool, _YXC_NT_MAX_HEAP_LINE_COUNT));

		*ppLineSet = pRecSet;
		*ppSPool = pElePool;
		return YXC_ERC_SUCCESS;
err_ret:
		free(pRecSet);
		return rcRet;
	}

	void _FreeResultHeapResult(_HDLSet* pLineSet, SElementPool<yuint32_t>* pSPool)
	{
		pSPool->Destroy();
		free(pLineSet);
	}

	void _DumpAllocRecord(HANDLE hProcess, yuint32_t uRecNumber, const YXC_HeapAllocRecord& record, _ModuleInfoX* pModules, yuint32_t uNumModules, FILE* fp)
	{
		fwprintf_s(fp, L"--------------------Heap allocation record %u, Count(%d), size(%ld bytes)---------------\n", uRecNumber,
			record.rBase.iCallCount, record.stAllocationSize);
		fwprintf_s(fp, L"Heap alloc call stack dumped:\n");

		_DumpResCallStack(hProcess, record.rBase, pModules, uNumModules, fp);

		fwprintf_s(fp, L"--------------------Heap allocation record %u------------------------------\n\n", uRecNumber);
	}

#pragma optimize("", off)
	LPVOID WINAPI _HookedHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
	{
		LPVOID pRet = gs_copiedHAlloc(hHeap, dwFlags, dwBytes);

		if (pRet != NULL && (dwFlags & 0xfffffff2) == 0 /* Alloc external flag */)
		{
			CONTEXT context;
			RtlCaptureContext(&context);

			YXX_CritLocker locker(gs_heapLock);
			_AddAllocContext(pRet, dwBytes, &context);
		}

		return (ybyte_t*)pRet;
	}

	BOOL WINAPI _HookedHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID pMem)
	{
		BOOL bRet = gs_copiedHFree(hHeap, dwFlags, pMem);

		if (bRet)
		{
			YXX_CritLocker locker(gs_heapLock);
			_RemoveAllocContext(pMem);
		}

		return bRet;
	}

	LPVOID WINAPI _HookedHeapRealloc(HANDLE hHeap, DWORD dwFlags, LPVOID pMem, SIZE_T dwBytes)
	{
		LPVOID pRet = gs_copiedHRealloc(hHeap, dwFlags, pMem, dwBytes);

		if (pRet)
		{
			CONTEXT context;
			RtlCaptureContext(&context);

			if (pMem != pRet)
			{
				YXX_CritLocker locker(gs_heapLock);
				_RemoveAllocContext(pMem);
				_AddAllocContext(pRet, dwBytes, &context);
			}
			else
			{
				// _ChangeAllocSize(pMem, dwBytes);
			}
		}

		return pRet;
	}
#pragma optimize("", on)
}

namespace YXC_Inner
{
	ybyte_t gs_byResPipeCritLock[sizeof(YXX_Crit)];
	YXX_Crit& gs_resPipeLock = *(YXX_Crit*)gs_byResPipeCritLock;
	YXC_Event gs_autoEvent = NULL;
	YXC_Event gs_endEvent = NULL;
	YXC_HeapAInfo gs_baseHeapInfo = NULL;

	static YXC_Status _StartAutoDetectProcess(yuint32_t uSeconds);

	void _InitHeapSection()
	{
		new (gs_byCritLock) YXX_Crit(4000);
		new (gs_byResPipeCritLock) YXX_Crit(4000);
	}

	void _DeleteHeapSection()
	{
		gs_heapLock.~CriticalSec();
		gs_resPipeLock.~CriticalSec();
	}

	YXC_Status _InitHeapDetect(ybool_t bInDllMain)
	{
		gs_heapLock.Lock();

		if (gs_bHeapDetectionInited)
		{
			gs_heapLock.Unlock();
			return YXC_ERC_SUCCESS;
		}

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rcAlloc = YXC_ERC_UNKNOWN;
		YXC_Status rcFree = YXC_ERC_UNKNOWN;
		YXC_Status rcRealloc = YXC_ERC_UNKNOWN;
		YXC_Status rcMMPtr = YXC_ERC_UNKNOWN, rcMMLine = YXC_ERC_UNKNOWN;
		YXC_Status rcPipe = YXC_ERC_UNKNOWN;

		rcPipe = _InitResDetectSrv(bInDllMain); // gs_hDetectSrv.Create(bInDllMain, _YXC_NT_RES_NAMED_PIPE_INSTANCES);
		_YXC_CHECK_GOTO(rcPipe == YXC_ERC_SUCCESS, rcPipe);

		// 2M ptrs
		rcMMPtr = gs_allocByPtr.Create(sizeof(_HDMapNode), _YXC_NT_MAX_HEAP_PTR_COUNT);
		_YXC_CHECK_GOTO(rcMMPtr == YXC_ERC_SUCCESS, rcMMPtr);

		// 256K allocs
		rcMMLine = gs_allocByLine.Create(sizeof(_HDLSetNode), _YXC_NT_MAX_HEAP_LINE_COUNT);
		_YXC_CHECK_GOTO(rcMMLine == YXC_ERC_SUCCESS, rcMMLine);

		new (gs_pLineSet) _HDLSet(std::less<YXC_HeapAllocRecord>(), _HDLSetAlloc(&gs_allocByLine, _YXC_NT_MAX_HEAP_LINE_COUNT));
		new (gs_pPtrMap) _HDMap(std::less<void*>(), _HDMapAlloc(&gs_allocByPtr, _YXC_NT_MAX_HEAP_PTR_COUNT));

		rcAlloc = _HookFunc(L"ntdll.dll", "RtlAllocateHeap", (FARPROC*)&gs_OldHeapAlloc, gs_copiedHAllocBytes, (FARPROC)_HookedHeapAlloc);
		_YXC_CHECK_GOTO(rcAlloc == YXC_ERC_SUCCESS, rcAlloc);
		rcFree = _HookFunc(L"ntdll.dll", "RtlFreeHeap", (FARPROC*)&gs_OldHeapFree, gs_copiedHFreeBytes, (FARPROC)_HookedHeapFree);
		_YXC_CHECK_GOTO(rcFree == YXC_ERC_SUCCESS, rcFree);
		rcRealloc = _HookFunc(L"ntdll.dll", "RtlReAllocateHeap", (FARPROC*)&gs_OldHeapRealloc, gs_copiedHReallocBytes, (FARPROC)_HookedHeapRealloc);
		_YXC_CHECK_GOTO(rcRealloc == YXC_ERC_SUCCESS, rcRealloc);

		gs_bHeapDetectionInited = TRUE;
		gs_heapLock.Unlock();

		return YXC_ERC_SUCCESS;
err_ret:
		if (rcAlloc == YXC_ERC_SUCCESS) _UnhookFunc((FARPROC)gs_OldHeapAlloc, gs_copiedHAllocBytes);
		if (rcFree == YXC_ERC_SUCCESS) _UnhookFunc((FARPROC)gs_OldHeapFree, gs_copiedHFreeBytes);
		if (rcAlloc == YXC_ERC_SUCCESS) _UnhookFunc((FARPROC)gs_OldHeapRealloc, gs_copiedHReallocBytes);

		if (rcMMPtr == YXC_ERC_SUCCESS) gs_allocByPtr.Destroy();
		if (rcMMLine == YXC_ERC_SUCCESS) gs_allocByLine.Destroy();
		if (rcPipe == YXC_ERC_SUCCESS) gs_hDetectSrv.Close();

		gs_heapLock.Unlock();
		return rcRet;
	}

	void _FiniHeapDetect()
	{
		YXX_CritLocker locker(gs_heapLock);

		if (gs_bHeapDetectionInited)
		{
			_UnhookFunc((FARPROC)gs_OldHeapAlloc, gs_copiedHAllocBytes);
			_UnhookFunc((FARPROC)gs_OldHeapFree, gs_copiedHFreeBytes);
			_UnhookFunc((FARPROC)gs_OldHeapRealloc, gs_copiedHReallocBytes);

			gs_allocByLine.Destroy();
			gs_allocByPtr.Destroy();

			gs_bHeapDetectionInited = FALSE;
			if (gs_autoEvent)
			{
				YXC_EventSet(gs_autoEvent);
				YXC_WaitSingleKObject(gs_endEvent);
				YXC_EventDestroy(gs_autoEvent);
				YXC_EventDestroy(gs_endEvent);
			}

			if (gs_baseHeapInfo != NULL)
			{
				YXC_FreeHeapAllocs(gs_baseHeapInfo);
				gs_baseHeapInfo = NULL;
			}
		}
	}

	BOOL _HeapDllInitHandler(unsigned int uReason)
	{
		LSTATUS rc;
		HKEY hKey;
		switch (uReason)
		{
		case DLL_PROCESS_ATTACH:
			rc = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\YXCLib\\Settings\\Basic", 0, KEY_ALL_ACCESS, &hKey);
			YXC_Inner::_InitHeapSection();
			if (rc == ERROR_SUCCESS)
			{
				DWORD dwHeapDetectEnabled = 0;
				DWORD dwRetType = 0, dwSize = sizeof(DWORD);
				rc = RegQueryValueExW(hKey, L"EnableHeapDetect", NULL, &dwRetType, (LPBYTE)&dwHeapDetectEnabled, &dwSize);

				if (rc == ERROR_SUCCESS && dwRetType == REG_DWORD && dwHeapDetectEnabled != 0)
				{
					YXC_Status erc = YXC_Inner::_InitHeapDetect(TRUE);
					if (erc != YXC_ERC_SUCCESS)
					{
						return erc;
					}

					DWORD dwHeapAutoDetect = 0;
					dwRetType = 0, dwSize = sizeof(DWORD);
					rc = RegQueryValueExW(hKey, L"HeapAutoDetect", NULL, &dwRetType, (LPBYTE)&dwHeapAutoDetect, &dwSize);
					if (rc == ERROR_SUCCESS && dwRetType == REG_DWORD && dwHeapAutoDetect != 0)
					{
						YXC_Status erc = YXC_Inner::_StartAutoDetectProcess(dwHeapAutoDetect);
						return erc == YXC_ERC_SUCCESS;
					}
				}
			}
			break;
		case DLL_PROCESS_DETACH:
			YXC_Inner::_FiniHeapDetect();
			YXC_Inner::_DeleteHeapSection();
			break;
		}

		return TRUE;
	}

	static YXC_Status _StartAutoDetectProcess(yuint32_t uSeconds)
	{
		STARTUPINFOW startupInfo = { sizeof(STARTUPINFOW) };
		PROCESS_INFORMATION psInfo;

		wchar_t szProcess[MAX_PATH];
		wchar_t szCmd[MAX_PATH];
		::GetModuleFileNameW(NULL, szProcess, MAX_PATH);

		if (wcsicmp(PathFindFileNameW(szProcess), L"HeapDetect.exe") != 0) /* Not HeapDetect process */
		{
			PathRemoveFileSpecW(szProcess);
			PathAppendW(szProcess, L"HeapDetect");
			PathAppendW(szProcess, L"HeapDetect.exe");
			swprintf_s(szCmd, L"\"%s\" -px %d %d", szProcess, ::GetCurrentProcessId(), uSeconds);

			BOOL bRet = ::CreateProcessW(szProcess, szCmd, NULL, NULL, FALSE, 0, NULL, NULL,
				&startupInfo, &psInfo);
			if (!bRet)
			{
				MessageBoxW(NULL, L"Create auto detect process failed.", L"YXC Kernel Module", MB_OK|MB_ICONERROR);
				return YXC_ERC_OS;
			}

			::CloseHandle(psInfo.hProcess);
			::CloseHandle(psInfo.hThread);
		}

		return YXC_ERC_SUCCESS;
	}
}

extern "C"
{
	YXC_Status YXC_EnableHeapDetect()
	{
		return _InitHeapDetect(FALSE);
	}

	void YXC_DisableHeapDetect()
	{
		_FiniHeapDetect();
	}

	YXC_Status YXC_DumpProcessHeapAllocs(HANDLE hProcess, const YXC_HeapAInfo allocInfo, const wchar_t* cpszFilename)
	{
		FILE* fp = NULL;
		ybool_t bFileOpend = FALSE;
		ybool_t bSymbolInited = FALSE;
		_ModuleInfoX miBuffer[_YXC_DEF_PE_MODULES], *pModuleInfo = miBuffer;

		std::vector<YXC_HeapAllocRecord> vecSortByBytes;
		vecSortByBytes.reserve(_YXC_NT_MAX_HEAP_LINE_COUNT);

		switch ((yuintptr_t)cpszFilename)
		{
		case YXC_STD_OUT_FILE:
			fp = stdout;
			break;
		case YXC_STD_ERR_FILE:
			fp = stderr;
			break;
		default:
			fp = _wfopen(cpszFilename, L"w");
			bFileOpend = TRUE;
			break;
		}

		_YXC_CHECK_CRT_RET(fp != NULL, L"Open file '%s' failed", cpszFilename);
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		_HDLSet* pRecSet = _HeapAPtr(allocInfo);

		yuint32_t uNumberModules = YXC_ARR_COUNT(miBuffer);
		YXC_Status rc = _InitModuleInfo(hProcess, &pModuleInfo, &uNumberModules);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Initialized module information failed");

		bSymbolInited = ::SymInitialize(hProcess, NULL, TRUE);
		if (!bSymbolInited && ERROR_INVALID_PARAMETER == ::GetLastError())
		{
			::SymCleanup(hProcess);
			bSymbolInited = ::SymInitialize(hProcess, NULL, TRUE);
		}

		_YXC_CHECK_OS_GOTO(bSymbolInited, L"Initialize symbols information failed");

		for (_HDLSet::iterator it = pRecSet->begin(); it != pRecSet->end(); ++it)
		{
			vecSortByBytes.push_back(*it);
		}
		std::sort(vecSortByBytes.begin(), vecSortByBytes.end(), _SortByBytesFunc);

		yuint32_t uRecNumber = 1;
		for (std::vector<YXC_HeapAllocRecord>::iterator it = vecSortByBytes.begin(); it != vecSortByBytes.end(); ++it)
		{
			_DumpAllocRecord(hProcess, uRecNumber++, *it, pModuleInfo, uNumberModules, fp);
		}

		if (pModuleInfo != miBuffer) _FreeModuleInfo(pModuleInfo);
		if (bFileOpend) fclose(fp);
		return YXC_ERC_SUCCESS;
err_ret:
		if (pModuleInfo != miBuffer) _FreeModuleInfo(pModuleInfo);
		if (bSymbolInited) SymCleanup(hProcess);
		if (bFileOpend)	fclose(fp);
		return rcRet;
	}

	YXC_Status YXC_QueryHeapAllocs(YXC_HeapAInfo* pHeapAllocInfo)
	{
		_HDLSet* pRecSet = NULL;
		SElementPool<yuint32_t>* pElePool = NULL;

		YXC_Status rc = _AllocResultHeapResult(&pRecSet, &pElePool, _YXC_NT_MAX_HEAP_LINE_COUNT);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Alloc for HAI result failed");

		rc = _FillResultHeapSet(pRecSet);
		if (rc != YXC_ERC_SUCCESS)
		{
			_FreeResultHeapResult(pRecSet, pElePool);
			_YXC_REPORT_NEW_ERR(rc, L"Diff heap to result set failed");
			return rc;
		}

		*pHeapAllocInfo = _HeapAHdl(pRecSet);
		return YXC_ERC_SUCCESS;
	}

	void YXC_FreeHeapAllocs(YXC_HeapAInfo allocInfo)
	{
		_HDLSet* pRecSet = _HeapAPtr(allocInfo);

		SElementPool<yuint32_t>* pePool = (SElementPool<yuint32_t>*)(pRecSet + 1);

		_FreeResultHeapResult(pRecSet, pePool);
	}

	YXC_Status YXC_DiffHeapAllocs(const YXC_HeapAInfo alloc1, const YXC_HeapAInfo alloc2, YXC_HeapAInfo* pHeapDiff)
	{
		_HDLSet* pRecSet = NULL;
		SElementPool<yuint32_t>* pElePool = NULL;

		YXC_Status rc = _AllocResultHeapResult(&pRecSet, &pElePool, _YXC_NT_MAX_HEAP_LINE_COUNT * 2);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Alloc for HAI result failed");

		_HDLSet* pAlloc1 = _HeapAPtr(alloc1);
		_HDLSet* pAlloc2 = _HeapAPtr(alloc2);
		_HeapDiff(pAlloc1, pAlloc2, pRecSet);

		*pHeapDiff = _HeapAHdl(pRecSet);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_GetProcessHeapAllocs(yuint32_t uProcessId, YXC_HeapAInfo* pHeapAllocInfo)
	{
		YXC_NPipeCli pipeClient = NULL;
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		_HDLSet* pRecSet = NULL;
		SElementPool<yuint32_t>* pElePool = NULL;

		YXC_Status rc = _AllocResultHeapResult(&pRecSet, &pElePool, _YXC_NT_MAX_HEAP_LINE_COUNT);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Alloc for HAI result failed");

		size_t max_records = pRecSet->max_size() - 1;

		wchar_t szPipeName[YXC_MAX_CCH_PATH];
		swprintf_s(szPipeName, L"__yxc_hDetect_xfv_42_%d_ax_fxed", uProcessId);
		rc = YXC_NPipeClientConnect(szPipeName, NULL, 2000, &pipeClient);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Connect to remote pipe failed, process id = %u", uProcessId);

		ybyte_t byCallType = _YXC_RES_REC_HEAP;
		void* pRet = NULL;
		ysize_t stCbRet = 0;

		rc = YXC_NPipeClientCall(pipeClient, &byCallType, sizeof(ybyte_t), &pRet, &stCbRet);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Call remote pipe failed, process id = %u", uProcessId);

		yuint32_t* puStatus = (yuint32_t*)pRet;

		YXC_Status rServiceStatus = (YXC_Status)*puStatus;
		yuint32_t* pNumRecordsOrLen = puStatus + 1;
		yuint32_t uLength = *pNumRecordsOrLen;
		if (rServiceStatus == YXC_ERC_SUCCESS)
		{
			_YXC_CHECK_REPORT_NEW_RET(uLength < max_records, YXC_ERC_BUFFER_NOT_ENOUGH, L"Too big record set, size = %u, max size = %lu",
				uLength, max_records);

			YXC_HeapAllocRecord* pRecords = (YXC_HeapAllocRecord*)(pNumRecordsOrLen + 1);
			for (yuint32_t i = 0; i < uLength; ++i)
			{
				pRecSet->insert(pRecords[i]);
			}
		}
		else
		{
			_YXC_CHECK_REPORT_NEW_GOTO(FALSE, rServiceStatus, L"Remote pipe returned message : %s", (wchar_t*)(pNumRecordsOrLen + 1));
		}

		YXC_NPipeClientClose(pipeClient);
		*pHeapAllocInfo = _HeapAHdl(pRecSet);
		return YXC_ERC_SUCCESS;

err_ret:
		if (pRecSet) _FreeResultHeapResult(pRecSet, pElePool);
		if (pipeClient != NULL) YXC_NPipeClientClose(pipeClient);
		return rcRet;
	}

	YXC_Status YXC_ReadHeapAllocs(const wchar_t* cpszFilename, yuint32_t* puProcessId, YXC_HeapAInfo* pHeapInfo)
	{
		_HDLSet* pRecSet = NULL;
		SElementPool<yuint32_t>* pElePool = NULL;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		FILE* fp = _wfopen(cpszFilename, L"rb");
		_YXC_CHECK_CRT_RET(fp != NULL, L"Open file '%s' to read heap info failed", cpszFilename);

		YXC_Status rc = _AllocResultHeapResult(&pRecSet, &pElePool, _YXC_NT_MAX_HEAP_LINE_COUNT);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Alloc for HAI result failed");

		_ResRecordHeader header;
		size_t nRead = fread(&header, sizeof(_ResRecordHeader), 1, fp);
		_YXC_CHECK_CRT_GOTO(nRead == 1, L"Read process id and alloc count failed, file = '%s'", cpszFilename);
		_YXC_CHECK_REPORT_NEW_GOTO(header.byRecType == _YXC_RES_REC_HEAP, YXC_ERC_INVALID_TYPE, L"Invalid record type %d, not a heap record",
			header.byRecType);

		size_t max_supported = pRecSet->max_size() - 1;
		_YXC_CHECK_REPORT_NEW_GOTO(header.uNumRecords < max_supported, YXC_ERC_INVALID_PARAMETER, L"Too big record set, "
			L"num records = %d, max support = %d, file = '%s'", header.uNumRecords, max_supported, cpszFilename);

		for (yuint32_t i = 0; i < header.uNumRecords; ++i)
		{
			YXC_HeapAllocRecord record;
			nRead = fread(&record, sizeof(YXC_HeapAllocRecord), 1, fp);

			_YXC_CHECK_CRT_GOTO(nRead == 1, L"Read alloc record failed, index = %d, file = '%s'", i, cpszFilename);
			pRecSet->insert(record);
		}

		*puProcessId = header.uProcessId;
		*pHeapInfo = _HeapAHdl(pRecSet);
		fclose(fp);
		return YXC_ERC_SUCCESS;
err_ret:
		if (pRecSet) _FreeResultHeapResult(pRecSet, pElePool);
		fclose(fp);
		return rcRet;
	}

	YXC_Status YXC_WriteHeapAllocs(const wchar_t* cpszFilename, yuint32_t uProcessId, const YXC_HeapAInfo heapInfo)
	{
		FILE* fp = _wfopen(cpszFilename, L"wb");
		_YXC_CHECK_CRT_RET(fp != NULL, L"Open file '%s' to write heap info failed", cpszFilename);

		_HDLSet* pLineSet = _HeapAPtr(heapInfo);
		YXC_Status rcRet = YXC_ERC_SUCCESS;

		_ResRecordHeader header = { uProcessId, pLineSet->size(), _YXC_RES_REC_HEAP };
		size_t written = fwrite(&header, sizeof(_ResRecordHeader), 1, fp);
		_YXC_CHECK_CRT_GOTO(written == 1, L"Write process id and alloc count failed, file = '%s'",cpszFilename);

		yuint32_t uRecordIndex = 0;
		for (_HDLSet::iterator it = pLineSet->begin(); it != pLineSet->end(); ++it)
		{
			written = fwrite(&(*it), sizeof(YXC_HeapAllocRecord), 1, fp);
			_YXC_CHECK_CRT_GOTO(written == 1, L"Write alloc record failed, index = %d, file = '%s'", uRecordIndex, cpszFilename);
			++uRecordIndex;
		}

		fclose(fp);
		return YXC_ERC_SUCCESS;

err_ret:
		fclose(fp);
		return rcRet;
	}

#if YXC_EXPORTS_FLAG == YXC_EXPORTS_DISPATCH_DLL
	BOOL WINAPI DllMain(HANDLE dllHandle, unsigned int reason, void* lpReserved)
	{
		return YXC_Inner::_HeapDllInitHandler(reason);
	}
#endif /* YXC_EXPORTS_FLAG */
}

#else
#error Only support heap detect on windows
#endif /* YXC_PLATFORM_WIN */

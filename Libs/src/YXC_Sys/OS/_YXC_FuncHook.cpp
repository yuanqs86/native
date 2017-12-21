#define __MODULE__ "EK.Base.FuncHook"

#include <YXC_Sys/OS/_YXC_FuncHook.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

#define _YXC_ASM_JMP_SIZE (sizeof(ybyte_t) + sizeof(yuintptr_t))
#define _YXC_ASM_HOOKED_PROC_SIZE (2 * _YXC_ASM_JMP_SIZE + (YXC_X86_MAX_INSTRUCION_LEN - 1))

namespace YXC_Inner
{
	static void _GetInstructionSegmentOffset(ybyte_t* pFunc, ysize_t stSegOff, ysize_t* pstRealOff)
	{
		ysize_t stOffLen = 0;
		while (stOffLen < stSegOff)
		{
			ybyte_t byInsLen;
			_YXC_GetInstructionLen_X86(pFunc, &byInsLen);

			if (byInsLen == 0) // can't parse it
			{
				*pstRealOff = stSegOff;
				return;
			}
			else
			{
				pFunc += byInsLen;
				stOffLen += byInsLen;
			}
		}

		*pstRealOff = stOffLen;
		return;
	}

	YXC_Status _HookFunc(const wchar_t* cpszModule, const char* cpszProcName, FARPROC* pOldProc, ybyte_t* pCopiedProcBuffer, FARPROC pNewProc)
	{
		HMODULE hModule = GetModuleHandleW(cpszModule);

		_YXC_CHECK_OS_RET(hModule != NULL, L"Hook function failed, don't find module, module('%s'), function('%S')", cpszModule, cpszProcName);

		FARPROC pfnOldPtr = GetProcAddress(hModule, cpszProcName);
		_YXC_CHECK_OS_RET(pfnOldPtr != NULL, L"Hook function failed, don't find procedure, module('%s'), function('%S')",
			cpszModule, cpszProcName);

		DWORD dwOldProtect;
		BOOL bRet = VirtualProtectEx(GetCurrentProcess(), pfnOldPtr, _YXC_ASM_JMP_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		_YXC_CHECK_OS_RET(bRet, L"Change old function protect failed, module('%s'), function('%S')", cpszModule, cpszProcName);

		bRet = VirtualProtectEx(GetCurrentProcess(), pCopiedProcBuffer, _YXC_ASM_HOOKED_PROC_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		_YXC_CHECK_OS_RET(bRet, L"Change new function protect failed, module('%s'), function('%S')", cpszModule, cpszProcName);

		ysize_t stRealOff;
		_GetInstructionSegmentOffset((ybyte_t*)pfnOldPtr, _YXC_ASM_JMP_SIZE, &stRealOff);

		ybyte_t* pTempBuffer = pCopiedProcBuffer;
		memcpy(pTempBuffer, pfnOldPtr, stRealOff);
		pTempBuffer += stRealOff;

		*pTempBuffer++ = 0xe9; // jmp
		*(yuintptr_t*)pTempBuffer = (yuintptr_t)pfnOldPtr - (yuintptr_t)pCopiedProcBuffer - _YXC_ASM_JMP_SIZE; // Hook back to old function

		ybyte_t* pNewCtrlBy = (ybyte_t*)pfnOldPtr;

		*pNewCtrlBy++ = 0xe9; // jmp;
		*(yuintptr_t*)pNewCtrlBy = (yuintptr_t)pNewProc - (yuintptr_t)pfnOldPtr - _YXC_ASM_JMP_SIZE;

		*pOldProc = pfnOldPtr;
		return YXC_ERC_SUCCESS;
	}

	void _UnhookFunc(FARPROC pOldProc, const ybyte_t* pNewBuffer)
	{
		DWORD oldProtect;
		BOOL bRet = VirtualProtectEx(GetCurrentProcess(), pOldProc, _YXC_ASM_JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
		if (bRet)
		{
			memcpy((void*)pOldProc, pNewBuffer, _YXC_ASM_JMP_SIZE);
			VirtualProtectEx(GetCurrentProcess(), pOldProc, _YXC_ASM_JMP_SIZE, PAGE_EXECUTE_READ, &oldProtect);
		}
	}
}

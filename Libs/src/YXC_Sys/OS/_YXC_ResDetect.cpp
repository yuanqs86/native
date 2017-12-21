#define __MODULE__ "EK.ResDetect.Service"

#include <YXC_Sys/OS/_YXC_ResDetect.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>

#include <stdio.h>
#include <DbgHelp.h>
#include <Psapi.h>
#include <Shlwapi.h>

namespace YXC_Inner
{
	void _DumpResCallStack(HANDLE hProcess, const YXC_ResourceAlloc& record, _ModuleInfoX* pModules, yuint32_t uNumModules, FILE* fp)
	{
		ybool_t bIsAlloc = record.iCallCount > 0;
		wchar_t c = bIsAlloc ? L'+' : L'-';

		IMAGEHLP_LINE64 line = { sizeof(IMAGEHLP_LINE64) };

		yuint32_t uLineNumber = 0;
		const char* pszFilePath = NULL;
		const wchar_t* pszModule = NULL;
		yuintptr_t upModuleBaseAddr = 0;

		char buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {0};
		PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buf;
		pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		pSymbol->MaxNameLen = MAX_SYM_NAME;

		for (yuint32_t i = 0; i < record.uNumStacks; ++i)
		{
			yuintptr_t upAddr = record.upAllocStacks[i];
			DWORD displacement = 0;

			pszModule = L"Unknown";
			for (yuint32_t i = 0; i < uNumModules; ++i)
			{
				if (upAddr >= pModules[i].upBaseAddr && upAddr < pModules[i].upBaseAddr + pModules[i].stImageSize)
				{
					pszModule = pModules[i].szFilename;
					upModuleBaseAddr = pModules[i].upBaseAddr;
					break;
				}
			}

			ybool_t bHasFile = SymGetLineFromAddr64(hProcess, upAddr, &displacement, &line);
			if (bHasFile)
			{
				uLineNumber = line.LineNumber;
				pszFilePath = line.FileName;
			}

			DWORD64 displacement2 = 0;
			ybool_t bHasName = SymFromAddr(hProcess, upAddr, &displacement2, pSymbol);

			if (bHasFile && bHasName)
			{
				fwprintf_s(fp, L"%c\t%s!%S() + 0x%llu bytes, at %S:%d\n", c, pszModule, pSymbol->Name, upAddr - pSymbol->Address,
					line.FileName, line.LineNumber);
			}
			else if (bHasFile)
			{
				fwprintf_s(fp, L"%c\t%s![0x%p + 0x%p](), at %s:%d\n", c, pszModule, upAddr, line.FileName, line.LineNumber);
			}
			else if (bHasName)
			{
				fwprintf_s(fp, L"%c\t%s!%S()[0x%p + 0x%p] + 0x%llu bytes\n", c, pszModule, pSymbol->Name, upModuleBaseAddr, upAddr - upModuleBaseAddr, upAddr - pSymbol->Address);
			}
			else
			{
				fwprintf_s(fp, L"%c\t%s![0x%p + 0x%p]()\n", c, pszModule, upModuleBaseAddr, upAddr - upModuleBaseAddr);
			}
		}
	}

	YXC_Status _InitModuleInfo(HANDLE hProcess, _ModuleInfoX** ppModules, yuint32_t* puNumModules)
	{
		HMODULE hModules[2048];
		DWORD dwCbNeeded = 0;
		_ModuleInfoX* pModules = *ppModules;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		BOOL bRet = EnumProcessModules(hProcess, hModules, sizeof(hModules), &dwCbNeeded);
		_YXC_CHECK_OS_RET(bRet, L"Enum process modules failed");

		if (bRet)
		{
			DWORD dwNumModules = dwCbNeeded / sizeof(HMODULE);
			*puNumModules = dwNumModules;
			if (dwNumModules < *puNumModules)
			{
				pModules = (_ModuleInfoX*)::calloc(dwNumModules, sizeof(_ModuleInfoX));
				_YXC_CHECK_REPORT_RET(pModules != NULL, YXC_ERC_OUT_OF_MEMORY, L"Query number modules failed");
			}

			wchar_t szFilename[MAX_PATH];
			for (DWORD i = 0; i < dwNumModules; ++i)
			{
				DWORD dwRet = ::GetModuleFileNameExW(hProcess, hModules[i], szFilename, MAX_PATH);

				if (dwRet != 0)
				{
					wchar_t* pszFile = ::PathFindFileNameW(szFilename);
					wchar_t* pszDst = pModules[i].szFilename;
					YXCLib::_AppendStringChkW(pszDst, pszDst + _YXC_PE_MAX_FILE_NAME, pszFile, YXC_STR_NTS);
				}

				MODULEINFO moduleInfo = {0};
				BOOL bRet = ::GetModuleInformation(hProcess, hModules[i], &moduleInfo, sizeof(MODULEINFO));
				if (bRet)
				{
					pModules[i].stImageSize = moduleInfo.SizeOfImage;
					pModules[i].upBaseAddr = (yuintptr_t)moduleInfo.lpBaseOfDll;
				}
			}
		}

		*ppModules = pModules;
		return YXC_ERC_SUCCESS;
	}

	void _FreeModuleInfo(_ModuleInfoX* pModules)
	{
		free(pModules);
	}
}

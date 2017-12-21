#define __MODULE__ "EK.Net.Hook"

#include <hash_set>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>

#include <YXC_Sys/YXC_NetHook.h>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>

namespace
{
	using YXC_Inner::gs_kernelLogger;
	using YXC_Inner::gs_bDoSrvLeakHook;
	using YXCLib::OSGetLastError;
#if YXC_PLATFORM_WIN
	static BOOL gs_bSendHookInited = FALSE;
	static BOOL gs_bHasHook = FALSE;

	typedef DWORD (__stdcall *NtDeviceIoControlFileProc)(
		HANDLE FileHandle,
		HANDLE Event,
		void* ApcRoutine,
		PVOID ApcContext,
		void* IoStatusBlock,
		ULONG IoControlCode,
		PVOID InputBuffer,
		ULONG InputBufferLength,
		PVOID OutputBuffer,
		ULONG OutputBufferLength
	);

#define _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE 32
#define _YXC_SEND_CONTROL_CMD 73759
#define _YXC_RECV_CONTROL_CMD 73751
#define _YXC_MAX_MBI_ARR 2000
#define _YXC_MAX_EXT_LEAK 10

	static NtDeviceIoControlFileProc gs_oldDIOCtrl = NULL;
	static BYTE gs_copiedDIOCtrlBytes[_YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE];
	static NtDeviceIoControlFileProc gs_copiedDIOCtrl = (NtDeviceIoControlFileProc)&gs_copiedDIOCtrlBytes[0];
	static CRITICAL_SECTION gs_crit;
	static LPVOID gs_leakedPos = NULL;
	static void* gs_extLeak[_YXC_MAX_EXT_LEAK];
	static BOOL gs_bNoLeak = FALSE;
	static BOOL gs_bLeakInited = FALSE;

	static stdext::hash_set<socket_t> gs_sockets;

	static BOOL _GetVersionByBuildLab(const wchar_t* pszBuildLab, DWORD* pdwMajorVersion, DWORD* pdwMinorVersion)
	{
		if (wcsstr(pszBuildLab, L"xp") != NULL)
		{
			*pdwMajorVersion = 5;
			*pdwMinorVersion = 1;
		}
		else if (wcsstr(pszBuildLab, L"srv03") != NULL)
		{
			*pdwMajorVersion = 5;
			*pdwMinorVersion = 2;
		}
		else if (wcsstr(pszBuildLab, L"vista") != NULL)
		{
			*pdwMajorVersion = 6;
			*pdwMinorVersion = 0;
		}
		else if (wcsstr(pszBuildLab, L"win7") != NULL)
		{
			*pdwMajorVersion = 6;
			*pdwMinorVersion = 1;
		}
		else
		{
			return FALSE;
		}
		return TRUE;
	}

	static BOOL IsReliableWindowsServer(const OSVERSIONINFOEXW* pOsVersion)
	{
		BOOL bIsWindowsServer = FALSE;
		if (pOsVersion->dwMajorVersion == 5)
		{
			bIsWindowsServer = pOsVersion->dwMinorVersion == 2;
		}
		else
		{
			bIsWindowsServer = pOsVersion->wProductType != VER_NT_WORKSTATION;
		}

		if (bIsWindowsServer)
		{
			return TRUE;
			BOOL bIsWow64Process = FALSE;
			::IsWow64Process(::GetCurrentProcess(), &bIsWow64Process);
			return !bIsWow64Process;
		}

		return FALSE;
	}

	static void FreeAllLeakedSendMemories()
	{
		OSVERSIONINFOEXW osVersion;
		osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
		BOOL bRet = GetVersionExW((OSVERSIONINFOW*)&osVersion);
		if (!bRet) return;
		if (IsReliableWindowsServer(&osVersion))
		{
			return;
		}

		void* baseAddr = NULL;
		while (TRUE)
		{
			MEMORY_BASIC_INFORMATION mbi;
			size_t sQuery = ::VirtualQueryEx(GetCurrentProcess(), baseAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

			if (sQuery == 0) break;

			if (mbi.State == MEM_RESERVE && mbi.Type == MEM_PRIVATE
				&& mbi.RegionSize == 0x1000 && mbi.AllocationBase == mbi.BaseAddress
				&& mbi.Protect == 0 && mbi.AllocationProtect == PAGE_READWRITE)
			{
				VirtualFreeEx(GetCurrentProcess(), mbi.AllocationBase, 0, MEM_RELEASE);
			}
			baseAddr = (char*)mbi.BaseAddress + mbi.RegionSize;
		}
	}

	//static void FreeLeakedVirtualMemory(DWORD dwNumBlocksNew, MEMORY_BASIC_INFORMATION* pNewBlocks,
	//	DWORD dwNumBlocksOld, MEMORY_BASIC_INFORMATION* pOldBlocks)
	//{
	//	for (int i = 0; i < dwNumBlocksNew; ++i)
	//	{
	//		BOOL bFindInOld = FALSE;
	//		for (int j = 0; j < dwNumBlocksOld; ++i)
	//		{
	//			if (pNewBlocks[i].AllocationBase == pOldBlocks[j].AllocationBase)
	//			{
	//				bFindInOld = TRUE;
	//				break;
	//			}
	//		}

	//		if (!bFindInOld)
	//		{
	//			VirtualFreeEx(GetCurrentProcess(), pNewBlocks[i].AllocationBase, 0, MEM_RELEASE);
	//		}
	//	}
	//}
	static void __stdcall QueryExtLeakPos(void* pBasePos, int iMaxExt, int* pExtLeaks, void** ppExtPos)
	{
		int iDetectedExt = 0;
		char* pDetBase = (char*)pBasePos + 0x10000;
		MEMORY_BASIC_INFORMATION mbi;
		while (iDetectedExt < iMaxExt)
		{
			size_t sQuery = ::VirtualQueryEx(GetCurrentProcess(), pDetBase, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
			if (sQuery != 0)
			{
				if (mbi.AllocationBase == NULL)
				{
					break;
				}
				else
				{
					if (mbi.State == MEM_RESERVE && mbi.Type == MEM_PRIVATE && mbi.RegionSize == 0x1000
						&& mbi.AllocationBase == mbi.BaseAddress && mbi.Protect == 0
						&& mbi.AllocationProtect == PAGE_READWRITE && pDetBase == mbi.AllocationBase)
					{
						ppExtPos[iDetectedExt++] = pDetBase;
						pDetBase += 0x10000;
					}
					else
					{
						pDetBase = (char*)mbi.BaseAddress + mbi.RegionSize;
					}
				}
			}
			else
			{
				break;
			}
		}
		*pExtLeaks = iDetectedExt;
	}

	static void QueryLeakPos(LPVOID* pLeakPos, int* pExtLeaks, void** ppExtPos)
	{
		if (*pLeakPos != NULL)
		{
			MEMORY_BASIC_INFORMATION mbi;
			size_t sQuery = ::VirtualQueryEx(GetCurrentProcess(), *pLeakPos, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

			if (sQuery != 0 && mbi.State == MEM_RESERVE && mbi.Type == MEM_PRIVATE
				&& mbi.RegionSize == 0x1000 && mbi.AllocationBase == mbi.BaseAddress
				&& mbi.Protect == 0 && mbi.AllocationProtect == PAGE_READWRITE && mbi.AllocationBase == *pLeakPos)
			{
				QueryExtLeakPos(*pLeakPos, _YXC_MAX_EXT_LEAK, pExtLeaks, ppExtPos);
				return;
			}
			else if (sQuery != 0 && mbi.State == MEM_FREE) // memory not in use, leak will occurred here next time, so just pass here.
			{
				*pLeakPos = NULL;
				return;
			}
		}

		void* baseAddr = NULL;
		while (TRUE)
		{
			MEMORY_BASIC_INFORMATION mbi;
			size_t sQuery = ::VirtualQueryEx(GetCurrentProcess(), baseAddr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

			if (sQuery == 0) return;

			if (mbi.State == MEM_RESERVE && mbi.Type == MEM_PRIVATE
				&& mbi.RegionSize == 0x1000 && mbi.AllocationBase == mbi.BaseAddress
				&& mbi.Protect == 0 && mbi.AllocationProtect == PAGE_READWRITE)
			{
				*pLeakPos = mbi.AllocationBase;
				QueryExtLeakPos(*pLeakPos, _YXC_MAX_EXT_LEAK, pExtLeaks, ppExtPos);
				return;
			}
			baseAddr = (char*)mbi.BaseAddress + mbi.RegionSize;
		}
	}

	static void __stdcall CheckSendLeakedMem()
	{
		void* pLeakedPos = gs_leakedPos;
		int iNumExtLeaks = 0;
		void* pExtLeak[_YXC_MAX_EXT_LEAK] = {0};
		QueryLeakPos(&pLeakedPos, &iNumExtLeaks, pExtLeak);

		if (!gs_bLeakInited)
		{
			gs_bNoLeak = pLeakedPos == NULL;
			gs_bLeakInited = TRUE;
		}

		if (pLeakedPos != NULL)
		{
			VirtualFreeEx(GetCurrentProcess(), pLeakedPos, 0, MEM_RELEASE);
			for (int i = 0; i < iNumExtLeaks; ++i)
			{
				VirtualFreeEx(GetCurrentProcess(), pExtLeak[i], 0, MEM_RELEASE);
			}
		}
		if (pLeakedPos != NULL)
		{
			gs_leakedPos = pLeakedPos;
		}
	}

	static DWORD __stdcall g_NtDeviceIoControlFile(
		HANDLE FileHandle,
		HANDLE Event,
		void* ApcRoutine,
		PVOID ApcContext,
		void* IoStatusBlock,
		ULONG IoControlCode,
		PVOID InputBuffer,
		ULONG InputBufferLength,
		PVOID OutputBuffer,
		ULONG OutputBufferLength
	)
	{
		if (IoControlCode == _YXC_SEND_CONTROL_CMD && !gs_bNoLeak) // fix for virtual memory leak of send
		{
			EnterCriticalSection(&gs_crit);

			if (gs_sockets.find((socket_t)FileHandle) != gs_sockets.end())
			{
				DWORD dwRet = gs_copiedDIOCtrl(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode,
					InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);
				CheckSendLeakedMem();

				LeaveCriticalSection(&gs_crit);
				return dwRet;
			}

			DWORD dwRet = gs_copiedDIOCtrl(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode,
				InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);

			LeaveCriticalSection(&gs_crit);
			return dwRet;
		}
		else
		{
			return gs_copiedDIOCtrl(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode,
				InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);
		}
	}

	static void AddClientSocketHook(socket_t socket)
	{
		if (gs_bHasHook)
		{
			EnterCriticalSection(&gs_crit);

			try
			{
				gs_sockets.insert(socket);
			}
			catch (...)
			{

			}

			LeaveCriticalSection(&gs_crit);
		}
	}

	static void RemoveHookedClientSocket(socket_t socket)
	{
		if (gs_bHasHook)
		{
			EnterCriticalSection(&gs_crit);

			try
			{
				gs_sockets.erase(socket);
			}
			catch (...)
			{

			}

			LeaveCriticalSection(&gs_crit);
		}
	}

	static BOOL HookFuncs()
	{
		HMODULE hModule = GetModuleHandleW(L"ntdll.dll");

		FARPROC pfnDeviceIoControl = GetProcAddress(hModule, "NtDeviceIoControlFile");

		if (pfnDeviceIoControl == NULL) return FALSE;

		gs_oldDIOCtrl = (NtDeviceIoControlFileProc)pfnDeviceIoControl;
		DWORD oldProtect;
		BOOL bRet = VirtualProtectEx(GetCurrentProcess(), pfnDeviceIoControl, _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE,
			PAGE_EXECUTE_READWRITE, &oldProtect);
		if (bRet == FALSE) return FALSE;

		bRet = VirtualProtectEx(GetCurrentProcess(), gs_copiedDIOCtrlBytes, _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE,
			PAGE_EXECUTE_READWRITE, &oldProtect);
		if (bRet == FALSE) return FALSE;

		memcpy(gs_copiedDIOCtrl, pfnDeviceIoControl, _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE);

		BYTE* pNewCtrlBy = (BYTE*)pfnDeviceIoControl;

		*pNewCtrlBy++ = 0xe9; // jmp
		*(yuintptr_t*)pNewCtrlBy = (yuintptr_t)g_NtDeviceIoControlFile - (yuintptr_t)pfnDeviceIoControl - sizeof(ybyte_t) - sizeof(void*);

		return TRUE;
	}

	static void UnhookFuncs()
	{
		DWORD oldProtect;
		BOOL bRet = VirtualProtectEx(GetCurrentProcess(), gs_oldDIOCtrl, _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE,
			PAGE_EXECUTE_READWRITE, &oldProtect);
		if (bRet)
		{
			memcpy((void*)gs_oldDIOCtrl, gs_copiedDIOCtrl, _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE);
			VirtualProtectEx(GetCurrentProcess(), gs_oldDIOCtrl, _YXC_NT_DIO_CONTROL_FILE_PROC_MAX_SIZE,
				PAGE_EXECUTE_READ, &oldProtect);
		}
	}

	static BOOL InitializeSendHook()
	{
#if YXC_IS_32BIT
		BOOL bRet = InterlockedExchange((volatile long*)&gs_bSendHookInited, 1);
		if (!bRet)
		{
			HKEY hOsVerKey;
			LSTATUS rc = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hOsVerKey);
			//rc = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Notepad++", 0, KEY_READ, &hOsVerKey);

			if (rc == ERROR_SUCCESS)
			{
				DWORD dwType;
				wchar_t szBuildLab[50];
				DWORD dwCbStr = sizeof(szBuildLab);
				rc = RegQueryValueExW(hOsVerKey, L"BuildLab", NULL, &dwType, (LPBYTE)szBuildLab, &dwCbStr);
				if (rc == ERROR_SUCCESS && dwType == REG_SZ)
				{
					DWORD dwMajorVersion, dwMinorVersion;
					BOOL bGetVer = _GetVersionByBuildLab(wcslwr(szBuildLab), &dwMajorVersion, &dwMinorVersion);
					if (bGetVer)
					{
						_YXC_KREPORT_INFO(L"Detect windows version %d.%d", dwMajorVersion, dwMinorVersion);
						if (dwMajorVersion == 6) // Only do hook in windows vista / windows 7
						{
							::InitializeCriticalSectionAndSpinCount(&gs_crit, 500);
							BOOL bRet = HookFuncs();

							if (bRet)
							{
								gs_bHasHook = TRUE;
								return bRet;
							}
							return FALSE;
						}
					}
					else
					{
						_YXC_KREPORT_ERR(L"Unable to detect windows version, won't do hook");
					}
				}
			}

			return TRUE;
		}
#else
#pragma message("Don't support send hook on x64 platform.")
#endif /* YXC_IS_32BIT */
		return TRUE;
	}

#endif /* YXC_PLATFORM_WIN */
}

namespace YXC_Inner
{
	void _NetCloseSendHook()
	{
		if (gs_bHasHook)
		{
			DeleteCriticalSection(&gs_crit);
			UnhookFuncs();
		}
	}
}

extern "C"
{
	YXC_Status YXC_NetEnableLeakHook()
	{
		BOOL bRet = InitializeSendHook();

		_YXC_CHECK_OS_RET(bRet, L"Initialize send hook failed");

		return YXC_ERC_SUCCESS;
	}

	void YXC_NetReleaseAllLeakedMem()
	{
		FreeAllLeakedSendMemories();
	}

	void YXC_NetAddToHookList(socket_t socket)
	{
		AddClientSocketHook(socket);
	}

	void YXC_NetRemoveFromHookList(socket_t socket)
	{
		RemoveHookedClientSocket(socket);
	}

	void YXC_SockSetSrvLeakHook(ybool_t bEnableHook)
	{
		gs_bDoSrvLeakHook = bEnableHook;
	}
};

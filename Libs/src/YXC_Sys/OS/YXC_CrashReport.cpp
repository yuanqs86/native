#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/OS/_YXC_WinVer.hpp>
#include <YXC_Sys/YXC_Locker.hpp>

#if YXC_PLATFORM_WIN

#include <DbgHelp.h>
#include <Psapi.h>
#include <Shlwapi.h>
#include <exception>

#if YXC_IS_32BIT
#define YXC_IMAGE_TYPE IMAGE_FILE_MACHINE_I386
#else
#define YXC_IMAGE_TYPE IMAGE_FILE_MACHINE_AMD64
#endif /* YXC_IS_32BIT */

#if !(YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL)
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "DbgHelp.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Version.lib")
#endif /* !(YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL) */

#endif /* YXC_PLATFORM_WIN */

#define _YXC_CRASH_FILE_NAME L"crash_report"
#define _YXC_HPRINTF_BUFFER_SIZE (1 << 16)                // must be at least 2048

#define _YXC_ONEK            (1 << 10)
#define _YXC_64K             (1 << 16)
#define _YXC_ONEM            (1 << 20)
#define _YXC_ONEG            (1 << 30)

#ifdef _T
#undef _T
#endif /* _T */

#define _T(x) L##x

namespace
{
#if YXC_PLATFORM_WIN

	YXX_Crit gs_crit;
	static volatile yuint32_t gs_report_id = 0;

	///////////////////////////////////////////////////////////////////////////////
	// lstrrchr (avoid the C Runtime )
	static wchar_t* lstrrchr(const wchar_t* string, int ch)
	{
		wchar_t *start = (wchar_t*)string;

		while (*string++);                       /* find end of string */
		/* search towards front */
		while (--string != start && *string != (wchar_t)ch);

		if (*string == (TCHAR) ch)                /* char found ? */
		{
			return (TCHAR *)string;
		}
		return NULL;
	}

	// hprintf behaves similarly to printf, with a few vital differences.
	// It uses wvsprintf to do the formatting, which is a system routine,
	// thus avoiding C run time interactions. For similar reasons it
	// uses WriteFile rather than fwrite.
	// The one limitation that this imposes is that wvsprintf, and
	// therefore hprintf, cannot handle floating point numbers.

	// Too many calls to WriteFile can take a long time, causing
	// confusing delays when programs crash. Therefore I implemented
	// a simple buffering scheme for hprintf

	static wchar_t _s_hprintf_buffer[_YXC_HPRINTF_BUFFER_SIZE];   // wvsprintf never prints more than one K.
	static int _s_hprintf_index = 0;

	///////////////////////////////////////////////////////////////////////////////
	// hflush
	static void hflush(HANDLE LogFile)
	{
		if (_s_hprintf_index > 0)
		{
			DWORD NumBytes;
			WriteFile(LogFile, _s_hprintf_buffer, _s_hprintf_index * sizeof(wchar_t), &NumBytes, 0);
			_s_hprintf_index = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	// hprintf
	static void hprintf(HANDLE LogFile, const wchar_t* Format, ...)
	{
		if (_s_hprintf_index > _YXC_HPRINTF_BUFFER_SIZE || _s_hprintf_index < 0)
		{
			_s_hprintf_index = _YXC_HPRINTF_BUFFER_SIZE;
		}

		if (_s_hprintf_index > (1 << 10)) /* Above 1K, flush it. */
		{
			DWORD NumBytes;
			WriteFile(LogFile, _s_hprintf_buffer, _s_hprintf_index * sizeof(wchar_t), &NumBytes, 0);
			_s_hprintf_index = 0;
		}

		va_list arglist;
		va_start( arglist, Format);
		_s_hprintf_index += wvsprintfW(&_s_hprintf_buffer[_s_hprintf_index], Format, arglist);
		va_end( arglist);
	}

	///////////////////////////////////////////////////////////////////////////////
	// FormatTime
	// Format the specified FILETIME to output in a human readable format,
	// without using the C run time.
	static void FormatTime(wchar_t* output, FILETIME TimeToPrint)
	{
		output[0] = _T('\0');
		WORD Date, Time;
		if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) && FileTimeToDosDateTime(&TimeToPrint, &Date, &Time))
		{
			wsprintfW(output, L"%d/%d/%d %02d:%02d:%02d", (Date / 32) & 15, Date & 31, (Date / 512) + 1980,
				(Time >> 11), (Time >> 5) & 0x3F, (Time & 0x1F) * 2);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// DumpCppException
	// Print information about a c++ exception.
	void DumpCppException(HANDLE hLogFile, EXCEPTION_RECORD* Exception)
	{
		__try
		{
			std::exception* pObject = (std::exception*)Exception->ExceptionInformation[1];
			const char* pExcp = pObject->what();
			TCHAR szDebugMessage[1000];
			if (strnicmp(pExcp, "bad alloc", strlen("bad alloc")) == 0)
			{
				DWORD dwAllocSize = *(DWORD*)((void**)pObject + 6);
				wsprintf(szDebugMessage, _T("Alloc for %ld bytes caused a bad alloc.\r\n"),	dwAllocSize);
			}
			else
			{
				wsprintf(szDebugMessage, _T("Caused a c++ exception %S.\r\n"), pExcp);
			}

			hprintf(hLogFile, _T("%s"), szDebugMessage);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			hprintf(hLogFile, _T("Failed to dump this c++ exception, may be not a standard c++ exception."));
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// DumpModuleInfo
	// Print information about a code module (DLL or EXE) such as its size,
	// location, time stamp, etc.
	static BOOL DumpModuleInfo(HANDLE LogFile, HINSTANCE ModuleHandle, int nModuleNo)
	{
		BOOL rc = false;
		wchar_t szModName[MAX_PATH*2] = {0};

		__try
		{
			if (GetModuleFileName(ModuleHandle, szModName, MAX_PATH) > 0)
			{
				// If GetModuleFileName returns greater than zero then this must
				// be a valid code module address. Therefore we can try to walk
				// our way through its structures to find the link time stamp.
				IMAGE_DOS_HEADER *DosHeader = (IMAGE_DOS_HEADER*)ModuleHandle;
				if (IMAGE_DOS_SIGNATURE != DosHeader->e_magic)
				{
					return FALSE;
				}

				IMAGE_NT_HEADERS *NTHeader = (IMAGE_NT_HEADERS*)((CHAR*)DosHeader
					+ DosHeader->e_lfanew);
				if (IMAGE_NT_SIGNATURE != NTHeader->Signature)
				{
					return FALSE;
				}

				// open the code module file so that we can get its file date and size
				HANDLE ModuleFile = CreateFile(szModName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL, 0);

				TCHAR TimeBuffer[100] = {0};

				DWORD FileSize = 0;
				if (ModuleFile != INVALID_HANDLE_VALUE)
				{
					FileSize = GetFileSize(ModuleFile, 0);
					FILETIME LastWriteTime;
					if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime))
					{
						FormatTime(TimeBuffer, LastWriteTime);
					}
					CloseHandle(ModuleFile);
				}
				hprintf(LogFile, L"Module %d\r\n", nModuleNo);
				hprintf(LogFile, L"%s\r\n", szModName);

				ULONGLONG imageBase = NTHeader->OptionalHeader.ImageBase;
				MODULEINFO moduleInfo = {0};
				::GetModuleInformation(GetCurrentProcess(), (HMODULE)ModuleHandle, &moduleInfo, sizeof(MODULEINFO));
				hprintf(LogFile, L"Image Base: 0x%p  Image Size: 0x%08x\r\n", moduleInfo.lpBaseOfDll,
					NTHeader->OptionalHeader.SizeOfImage);
				hprintf(LogFile, L"Checksum:   0x%d  Time Stamp: 0x%08x\r\n", NTHeader->OptionalHeader.CheckSum,
					NTHeader->FileHeader.TimeDateStamp);
				hprintf(LogFile, L"File Size:  %-10d  File Time:  %s\r\n", FileSize, TimeBuffer);

				hprintf(LogFile, L"Version Information:\r\n");

				YXC_Inner::_MiniVersion ver(szModName);
				TCHAR szBuf[200];
				WORD dwBuf[4];

				ver.GetCompanyName(szBuf, sizeof(szBuf)-1);
				hprintf(LogFile, L"    Company:    %s\r\n", szBuf);

				ver.GetProductName(szBuf, sizeof(szBuf)-1);
				hprintf(LogFile, L"    Product:    %s\r\n", szBuf);

				ver.GetFileDescription(szBuf, sizeof(szBuf)-1);
				hprintf(LogFile, L"    FileDesc:   %s\r\n", szBuf);

				ver.GetFileVersion(dwBuf);
				hprintf(LogFile, L"    FileVer:    %d.%d.%d.%d\r\n", dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);

				ver.GetProductVersion(dwBuf);
				hprintf(LogFile, L"    ProdVer:    %d.%d.%d.%d\r\n", dwBuf[0], dwBuf[1], dwBuf[2], dwBuf[3]);

				ver.Release();

				hprintf(LogFile, L"\r\n");

				rc = TRUE;
			}
		}
		// Handle any exceptions by continuing from this point.
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
		return rc;
	}

	///////////////////////////////////////////////////////////////////////////////
	// DumpModuleList
	// Scan memory looking for code modules (DLLs or EXEs). VirtualQuery is used
	// to find all the blocks of address space that were reserved or committed,
	// and ShowModuleInfo will display module information if they are code
	// modules.
	static void DumpModuleList(HANDLE LogFile)
	{
		HMODULE modules[256] = {0};
		DWORD needed = 0;
		BOOL bRet = EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed);
		int numModules = needed / sizeof(HMODULE);

		for (int i = 0; i < numModules; ++i)
		{
			DumpModuleInfo(LogFile, (HINSTANCE)modules[i], i);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// DumpSystemInformation
	// Record information about the user's system, such as processor type, amount
	// of memory, etc.
	static void DumpSystemInformation(HANDLE LogFile)
	{
		FILETIME CurrentTime;
		GetSystemTimeAsFileTime(&CurrentTime);
		wchar_t szTimeBuffer[100];
		FormatTime(szTimeBuffer, CurrentTime);

		hprintf(LogFile, L"Error occurred at %s.\r\n", szTimeBuffer);

		wchar_t szModuleName[MAX_PATH*2] = {0};
		if (GetModuleFileNameW(0, szModuleName, MAX_PATH) <= 0)
		{
			lstrcpyW(szModuleName, L"Unknown");
		}

		wchar_t szUserName[200] = {0};
		DWORD UserNameSize = 200;
		if (!GetUserNameW(szUserName, &UserNameSize))
		{
			lstrcpyW(szUserName, L"Unknown");
		}

		hprintf(LogFile, L"%s, run by %s.\r\n", szModuleName, szUserName);

		// print out operating system
		TCHAR szWinVer[50], szMajorMinorBuild[50];
		int nWinVer;
		YXC_Inner::_GetWinVer(szWinVer, &nWinVer, szMajorMinorBuild);
		hprintf(LogFile, L"Operating system:  %s (%s).\r\n", szWinVer, szMajorMinorBuild);

		SYSTEM_INFO	SystemInfo;
		GetSystemInfo(&SystemInfo);
		hprintf(LogFile, L"%d processor(s), type %d.\r\n", SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);

		MEMORYSTATUS MemInfo;
		MemInfo.dwLength = sizeof(MemInfo);
		GlobalMemoryStatus(&MemInfo);

		// Print out info on memory, rounded up.
		hprintf(LogFile, L"%d%% memory in use.\r\n", MemInfo.dwMemoryLoad);
		hprintf(LogFile, L"%d MBytes physical memory.\r\n", (MemInfo.dwTotalPhys + _YXC_ONEM - 1) / _YXC_ONEM);
		hprintf(LogFile, L"%d MBytes physical memory free.\r\n", (MemInfo.dwAvailPhys + _YXC_ONEM - 1) / _YXC_ONEM);
		hprintf(LogFile, L"%d MBytes paging file.\r\n", (MemInfo.dwTotalPageFile + _YXC_ONEM - 1) / _YXC_ONEM);
		hprintf(LogFile, L"%d MBytes paging file free.\r\n", (MemInfo.dwAvailPageFile + _YXC_ONEM - 1) / _YXC_ONEM);
		hprintf(LogFile, L"%d MBytes user address space.\r\n", (MemInfo.dwTotalVirtual + _YXC_ONEM - 1) / _YXC_ONEM);
		hprintf(LogFile, L"%d MBytes user address space free.\r\n", (MemInfo.dwAvailVirtual + _YXC_ONEM - 1) / _YXC_ONEM);
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetExceptionDescription
	// Translate the exception code into something human readable
	static const wchar_t* GetExceptionDescription(DWORD ExceptionCode)
	{
		struct ExceptionNames
		{
			DWORD ExceptionCode;
			const wchar_t* ExceptionName;
		};

		ExceptionNames ExceptionMap[] =
		{
			{0x40010005, L"a Control-C"},
			{0x40010008, L"a Control-Break"},
			{YXC_BAD_ALLOC_CODE, L"A bad alloc of YXC system"},
			{0x80000002, L"a Datatype Misalignment"},
			{0x80000003, L"a Breakpoint"},
			{0xc0000005, L"an Access Violation"},
			{0xc0000006, L"an In Page Error"},
			{0xc0000017, L"a No Memory"},
			{0xc000001d, L"an Illegal Instruction"},
			{0xc0000025, L"a Noncontinuable Exception"},
			{0xc0000026, L"an Invalid Disposition"},
			{0xc000008c, L"a Array Bounds Exceeded"},
			{0xc000008d, L"a Float Denormal Operand"},
			{0xc000008e, L"a Float Divide by Zero"},
			{0xc000008f, L"a Float Inexact Result"},
			{0xc0000090, L"a Float Invalid Operation"},
			{0xc0000091, L"a Float Overflow"},
			{0xc0000092, L"a Float Stack Check"},
			{0xc0000093, L"a Float Underflow"},
			{0xc0000094, L"an Integer Divide by Zero"},
			{0xc0000095, L"an Integer Overflow"},
			{0xc0000096, L"a Privileged Instruction"},
			{0xc00000fD, L"a Stack Overflow"},
			{0xc0000142, L"a DLL Initialization Failed"},
			{0xe06d7363, L"a Microsoft C++ Exception"},
		};

		for (int i = 0; i < sizeof(ExceptionMap) / sizeof(ExceptionMap[0]); i++)
		{
			if (ExceptionCode == ExceptionMap[i].ExceptionCode)
			{
				return ExceptionMap[i].ExceptionName;
			}
		}

		return L"an Unknown exception type";
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetFilePart
	static wchar_t* GetFilePart(const wchar_t* source)
	{
		TCHAR *result = lstrrchr(source, L'\\');
		if (result)
		{
			result++;
		}
		else
		{
			result = (TCHAR *)source;
		}
		return result;
	}

	///////////////////////////////////////////////////////////////////////////////
	// DumpStack
	static void DumpStack(HANDLE LogFile, yuintptr_t* pStack)
	{
		hprintf(LogFile, L"\r\n\r\nStack:\r\n");

		const int MaxStackDump = 8192;  // Maximum number of DWORDS in stack dumps.
		const int StackColumns = 4;     // Number of columns in stack dump.

		__try
		{
			// Esp contains the bottom of the stack, or at least the bottom of
			// the currently used area.

			CONTEXT context;
			RtlCaptureContext(&context);

			yuintptr_t* pStackTop = (yuintptr_t*)yxcwrap_getstack_top();
			//__asm
			//{
			//	// Load the top (highest address) of the stack from the
			//	// thread information block. It will be found there in
			//	// Win9x and Windows NT.
			//	mov	eax, fs:[4]
			//	mov pStackTop, eax
			//}

			if (pStackTop > pStack + MaxStackDump)
			{
				pStackTop = pStack + MaxStackDump;
			}

			int Count = 0;

			yuintptr_t* pStackStart = pStack;

			int nDwordsPrinted = 0;

			while (pStack + 1 <= pStackTop)
			{
				if ((Count % StackColumns) == 0)
				{
					pStackStart = pStack;
					nDwordsPrinted = 0;
#if YXC_IS_32BIT
					hprintf(LogFile, _T("0x%08x "), pStack);
#else
					hprintf(LogFile, _T("0x%p "), pStack);
#endif /* YXC_IS_32BIT */
				}

				if ((++Count % StackColumns) == 0 || pStack + 2 > pStackTop)
				{
#if YXC_IS_32BIT
					hprintf(LogFile, _T("%08x "), *pStack);
#else
					hprintf(LogFile, _T("0x%08x 0x%08x "), YXC_HI_32BITS(*pStack), YXC_LO_32BITS(*pStack));
#endif /* YXC_IS_32BIT */
					nDwordsPrinted++;

					int n = nDwordsPrinted;
					while (n < 4)
					{
						hprintf(LogFile, _T("         "));
						n++;
					}

					for (int i = 0; i < nDwordsPrinted; i++)
					{
						yuintptr_t dwStack = *pStackStart;
						for (int j = 0; j < sizeof(yuintptr_t); j++)
						{
							char c = (char)(dwStack & 0xFF);
							if (c < 0x20 || c > 0x7E)
								c = '.';

							WCHAR w = (WCHAR)c;
							hprintf(LogFile, _T("%c"), w);

							dwStack = dwStack >> 8;
						}
						pStackStart++;
					}

					hprintf(LogFile, _T("\r\n"));
				}
				else
				{
#if YXC_IS_32BIT
					hprintf(LogFile, _T("%08x "), *pStack);
#else
					hprintf(LogFile, _T("0x%08x 0x%08x "), YXC_HI_32BITS(*pStack), YXC_LO_32BITS(*pStack));
#endif /* YXC_IS_32BIT */
					nDwordsPrinted++;
				}
				pStack++;
			}
			hprintf(LogFile, _T("\r\n"));
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			hprintf(LogFile, _T("Exception encountered during stack dump.\r\n"));
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// DumpRegisters
	static void DumpRegisters(HANDLE LogFile, PCONTEXT Context)
	{
		// Print out the register values in an XP error window compatible format.
#if YXC_IS_32BIT
		hprintf(LogFile, _T("\r\n"));
		hprintf(LogFile, _T("Context:\r\n"));
		hprintf(LogFile, _T("EDI:    0x%08x  ESI: 0x%08x  EAX:   0x%08x\r\n"), Context->Edi, Context->Esi, Context->Eax);
		hprintf(LogFile, _T("EBX:    0x%08x  ECX: 0x%08x  EDX:   0x%08x\r\n"), Context->Ebx, Context->Ecx, Context->Edx);
		hprintf(LogFile, _T("EIP:    0x%08x  EBP: 0x%08x  SegCs: 0x%08x\r\n"), Context->Eip, Context->Ebp, Context->SegCs);
		hprintf(LogFile, _T("EFlags: 0x%08x  ESP: 0x%08x  SegSs: 0x%08x\r\n"), Context->EFlags, Context->Esp, Context->SegSs);
#else
		hprintf(LogFile, _T("\r\n"));
		hprintf(LogFile, _T("Context:\r\n"));
		hprintf(LogFile, _T("RDI:    0x%p  RSI: 0x%p  RAX:   0x%p\r\n"), Context->Rdi, Context->Rsi, Context->Rax);
		hprintf(LogFile, _T("RBX:    0x%p  RCX: 0x%p  RDX:   0x%p\r\n"), Context->Rbx, Context->Rcx, Context->Rdx);
		hprintf(LogFile, _T("RIP:    0x%p  RBP: 0x%p  SegCs: 0x%d\r\n"), Context->Rip, Context->Rbp, Context->SegCs);
		hprintf(LogFile, _T("EFlags: 0x%d  ESP: 0x%p  SegSs: 0x%d\r\n"), Context->EFlags, Context->Rsp, Context->SegSs);
#endif /* YXC_IS_32BIT */
	}

	static void DumpCallStack(HANDLE LogFile, PCONTEXT pContext)
	{
		STACKFRAME64 frame;
		memset(&frame, 0, sizeof(STACKFRAME64));

#if YXC_IS_32BIT
		frame.AddrPC.Offset = pContext->Eip;
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrStack.Offset = pContext->Esp;
		frame.AddrStack.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = pContext->Ebp;
		frame.AddrFrame.Mode = AddrModeFlat;
#else
		frame.AddrPC.Offset = pContext->Rip;
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrStack.Offset = pContext->Rsp;
		frame.AddrStack.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = pContext->Rbp;
		frame.AddrFrame.Mode = AddrModeFlat;
#endif /* YXC_IS_64BIT */

		STACKFRAME64 frame_orginal = frame;
		CONTEXT context = *pContext;

		__try
		{
			HMODULE modules[256] = {0};
			DWORD needed = 0;
			BOOL bRet = EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed);
			int numModules = needed / sizeof(HMODULE);

			wchar_t wszModuleName[MAX_PATH] = L"Unknown";
			wchar_t* pszModuleName = wszModuleName;
			yuintptr_t upBaseAddr = 0;

			hprintf(LogFile, _T("\r\n"));
			hprintf(LogFile, _T("CallStack Dump Started:\r\n"));
#if YXC_IS_32BIT
			while (TRUE)
			{
				if (!StackWalk64(YXC_IMAGE_TYPE, GetCurrentProcess(), GetCurrentThread(), &frame, &context, NULL, NULL, NULL, NULL))
				{
					break;
				}

				if (frame.AddrFrame.Offset == 0) break; // end of stack
				DWORD64 callStack = frame.AddrPC.Offset;

				if (bRet)
				{
					for (int i = 0; i < numModules; ++i)
					{
						MODULEINFO moduleInfo;
						GetModuleInformation(GetCurrentProcess(), modules[i], &moduleInfo, sizeof(MODULEINFO));

						if ((DWORD64)moduleInfo.lpBaseOfDll <= callStack && (DWORD64)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage > callStack)
						{
							GetModuleFileNameW(modules[i], wszModuleName, MAX_PATH);
							pszModuleName = PathFindFileNameW(wszModuleName);
							if (pszModuleName != NULL)
							{
								//pszModuleName = wszModuleName;
								upBaseAddr = (yuintptr_t)moduleInfo.lpBaseOfDll;
							}
							break;
						}
					}
				}

				hprintf(LogFile, _T("%s!0x%x....... (+0x%x)\r\n"), pszModuleName, YXC_LO_32BITS(frame.AddrPC.Offset),
					YXC_LO_32BITS(frame.AddrPC.Offset - upBaseAddr));
			}
#else
			yuintptr_t upAllocStacks[64];
			yuint32_t uRealStacks = ::RtlCaptureStackBackTrace(0, 64, (PVOID*)upAllocStacks, NULL);

			for (yuint32_t x = 0; x < uRealStacks; ++x)
			{
				DWORD64 callStack = upAllocStacks[x];
				for (int i = 0; i < numModules; ++i)
				{
					MODULEINFO moduleInfo;
					GetModuleInformation(GetCurrentProcess(), modules[i], &moduleInfo, sizeof(MODULEINFO));

					if ((DWORD64)moduleInfo.lpBaseOfDll <= callStack && (DWORD64)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage > callStack)
					{
						GetModuleFileNameW(modules[i], wszModuleName, MAX_PATH);
						pszModuleName = PathFindFileNameW(wszModuleName);
						if (pszModuleName != NULL)
						{
							//pszModuleName = wszModuleName;
							upBaseAddr = (yuintptr_t)moduleInfo.lpBaseOfDll;
						}
						break;
					}
				}

				hprintf(LogFile, _T("%s!0x%08x%08x....... (+0x%08x%08x)\r\n"), pszModuleName, YXC_HI_32BITS(callStack),
					YXC_LO_32BITS(callStack), YXC_HI_32BITS(callStack - upBaseAddr),
					YXC_LO_32BITS(callStack - upBaseAddr));
			}

#endif /* YXC_IS_32BIT */
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			hprintf(LogFile, _T("Exception occurred when dump stack"));
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//
	// RecordExceptionInfo
	//
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	int __cdecl _RecordExceptionInfoImpl(HANDLE hLogFile, PEXCEPTION_POINTERS pExceptPtrs, const wchar_t* lpszMessage)
	{
		const int NumCodeBytes = 16;    // Number of code bytes to record.

		// Create a filename to record the error information to.
		// Storing it in the executable directory works well.

		// Print out a blank line to separate this error log from any previous ones
		//hprintf(hLogFile, _T("\r\n"));

		PEXCEPTION_RECORD Exception = pExceptPtrs->ExceptionRecord;
		PCONTEXT          Context   = pExceptPtrs->ContextRecord;

		wchar_t szCrashModulePathName[MAX_PATH*2] = {0};
		wchar_t *pszCrashModuleFileName = _T("Unknown");

		MEMORY_BASIC_INFORMATION MemInfo;

		TCHAR szModuleName[MAX_PATH*2] = {0};
		if (GetModuleFileName(0, szModuleName, MAX_PATH) <= 0)
		{
			lstrcpyW(szModuleName, _T("Unknown"));
		}

		TCHAR *pszFilePart = GetFilePart(szModuleName);

#if YXC_IS_32BIT
		void* instruction = (void*)Context->Eip;
#else
		void* instruction = (void*)Context->Rip;
#endif /* YXC_IS_32BIT */

		// VirtualQuery can be used to get the allocation base associated with a
		// code address, which is the same as the ModuleHandle. This can be used
		// to get the filename of the module that the crash happened in.
		if (VirtualQuery(instruction, &MemInfo, sizeof(MemInfo)) &&
			(GetModuleFileName((HINSTANCE)MemInfo.AllocationBase, szCrashModulePathName, MAX_PATH) > 0))
		{
			pszCrashModuleFileName = GetFilePart(szCrashModulePathName);
		}

		// Print out the beginning of the error log in a Win95 error window
		// compatible format.
#if YXC_IS_32BIT
		hprintf(hLogFile, _T("%s caused %s (0x%08x) \r\nin module %s at %04x:%08x.\r\n\r\n"),
			pszFilePart, GetExceptionDescription(Exception->ExceptionCode),
			Exception->ExceptionCode,
			pszCrashModuleFileName, Context->SegCs, Context->Eip);
#else
		hprintf(hLogFile, _T("%s caused %s (0x%08x) \r\nin module %s at %04x:%p.\r\n\r\n"),
			pszFilePart, GetExceptionDescription(Exception->ExceptionCode),
			Exception->ExceptionCode,
			pszCrashModuleFileName, Context->SegCs, Context->Rip);
#endif /* YXC_IS_32BIT */
		hflush(hLogFile);

		hprintf(hLogFile, _T("Exception handler called in %s.\r\n"), lpszMessage);
		hflush(hLogFile);

		DumpSystemInformation(hLogFile);
		hflush(hLogFile);

		// If the exception was an access violation, print out some additional
		// information, to the error log and the debugger.
		if (Exception->ExceptionCode == STATUS_ACCESS_VIOLATION &&
			Exception->NumberParameters >= 2)
		{
			TCHAR szDebugMessage[1000];
			const TCHAR* readwrite = _T("Read from");
			if (Exception->ExceptionInformation[0])
			{
				readwrite = _T("Write to");
			}

#if YXC_IS_32BIT
			wsprintf(szDebugMessage, _T("%s location %08x caused an access violation.\r\n"),
				readwrite, Exception->ExceptionInformation[1]);
#else
			wsprintf(szDebugMessage, _T("%s location %p caused an access violation.\r\n"),
				readwrite, Exception->ExceptionInformation[1]);
#endif /* YXC_IS_32BIT */


#ifdef	_DEBUG
			// The Visual C++ debugger doesn't actually tell you whether a read
			// or a write caused the access violation, nor does it tell what
			// address was being read or written. So I fixed that.
			OutputDebugString(_T("Exception handler: "));
			OutputDebugString(szDebugMessage);
#endif
			hprintf(hLogFile, _T("%s"), szDebugMessage);
		}
		else if (Exception->ExceptionCode == 0xe06d7363) // A C++ exception
		{
			DumpCppException(hLogFile, Exception);
		}
		else if (Exception->ExceptionCode == YXC_BAD_ALLOC_CODE && Exception->NumberParameters == 1)
		{
			hprintf(hLogFile, _T("Alloc %ld bytes caused a error.\r\n"), Exception->ExceptionInformation[0]);
		}
		hflush(hLogFile);

		DumpRegisters(hLogFile, Context);
		hflush(hLogFile);

		// Print out the bytes of code at the instruction pointer. Since the
		// crash may have been caused by an instruction pointer that was bad,
		// this code needs to be wrapped in an exception handler, in case there
		// is no memory to read. If the dereferencing of code[] fails, the
		// exception handler will print '??'.
		hprintf(hLogFile, _T("\r\nBytes at CS:EIP:\r\n"));
#if YXC_IS_32BIT
		BYTE * code = (BYTE *)Context->Eip;
#else
		BYTE * code = (BYTE *)Context->Rip;
#endif /* YXC_IS_32BIT */
		for (int codebyte = 0; codebyte < NumCodeBytes; codebyte++)
		{
			__try
			{
				hprintf(hLogFile, _T("%02x "), code[codebyte]);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				hprintf(hLogFile, _T("?? "));
			}
		}
		hflush(hLogFile);

		hprintf(hLogFile, _T("\r\n"));
		CONTEXT cont = *pExceptPtrs->ContextRecord;
		DumpCallStack(hLogFile, &cont);

		hprintf(hLogFile, _T("\r\n"));
		hflush(hLogFile);
		//DumpModules(hLogFile, Context);
		// Time to print part or all of the stack to the error log. This allows
		// us to figure out the call stack, parameters, local variables, etc.

		// Esp contains the bottom of the stack, or at least the bottom of
		// the currently used area
#if YXC_IS_32BIT
		yuintptr_t* pStack = (yuintptr_t*)Context->Esp;
#else
		yuintptr_t* pStack = (yuintptr_t*)Context->Rsp;
#endif /* YXC_IS_32BIT */

		DumpStack(hLogFile, pStack);
		hflush(hLogFile);
		DumpModuleList(hLogFile);
		hflush(hLogFile);

		hprintf(hLogFile, _T("\r\n===== [end of %s] =====\r\n"), _YXC_CRASH_FILE_NAME);
		hflush(hLogFile);

		// return the magic value which tells Win32 that this handler didn't
		// actually handle the exception - so that things will proceed as per
		// normal.
		return EXCEPTION_CONTINUE_SEARCH;
	}

	int __cdecl _RecordExceptionInfo(PEXCEPTION_POINTERS pExceptPtrs, const wchar_t* lpszMessage)
	{
		TCHAR szModuleName[MAX_PATH*2] = {0};
		if (GetModuleFileName(0, szModuleName, MAX_PATH) <= 0)
		{
			lstrcpyW(szModuleName, _T("Unknown"));
		}

		TCHAR *pszFilePart = GetFilePart(szModuleName);

		// Extract the file name portion and remove it's file extension
		TCHAR szFileName[MAX_PATH*2];
		lstrcpyW(szFileName, pszFilePart);
		TCHAR *lastperiod = lstrrchr(szFileName, _T('.'));
		if (lastperiod)
		{
			lastperiod[0] = 0;
		}

		yuint32_t iId = YXCLib::Interlocked::Increment(&gs_report_id);

		wchar_t szCrashFileName[MAX_PATH];
		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		wsprintfW(szCrashFileName, L"%s[%d-%d]_%02d_%02d_%02d-%02d_%02d_%02d_%03d-.txt", _YXC_CRASH_FILE_NAME, GetCurrentProcessId(),
			iId, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
		// Replace the executable filename with our error log file name
		lstrcpyW(pszFilePart, szCrashFileName);

		SetLastError(ERROR_SUCCESS);
		HANDLE hLogFile = CreateFileW(szModuleName, GENERIC_WRITE, 0, 0, OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);

		if (hLogFile == INVALID_HANDLE_VALUE)
		{
			OutputDebugStringW(_T("Error creating exception report\r\n"));
			return EXCEPTION_CONTINUE_SEARCH;
		}

		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			BYTE unicodeBytes[2] = { 0xff, 0xfe };
			DWORD dwWritten = 0;
			WriteFile(hLogFile, unicodeBytes, 2, &dwWritten, NULL);
		}

		// Append to the error log
		SetFilePointer(hLogFile, 0, 0, FILE_END);

		__try
		{
			_RecordExceptionInfoImpl(hLogFile, pExceptPtrs, lpszMessage);
		}
		__except (1)
		{
			DWORD dwNumWritten;
			wchar_t szError[] = L"\r\nFailed to write exception information to crash report.";
			hflush(hLogFile);
			::WriteFile(hLogFile, szError, sizeof(szError) - sizeof(wchar_t), &dwNumWritten, NULL);
		}

		CloseHandle(hLogFile);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	typedef LPTOP_LEVEL_EXCEPTION_FILTER (__stdcall *SetExcpFilterFunc)(LPTOP_LEVEL_EXCEPTION_FILTER pFilter);

	typedef PVOID (__stdcall *AddVectoredExceptionHandlerFunc)(ULONG First, PVECTORED_EXCEPTION_HANDLER pHandler);
	AddVectoredExceptionHandlerFunc gs_orginalVEH = AddVectoredExceptionHandler;

	static SetExcpFilterFunc gs_oldSEF = SetUnhandledExceptionFilter;

#if YXC_IS_32BIT
	static ybyte_t gs_oldCtrlBytes[sizeof(ybyte_t) + sizeof(yuintptr_t)] = {0};
#else
	static ybyte_t gs_oldCtrlBytes[14] = {0};
#endif /* YXC_IS_32BIT */
	static volatile yuint32_t gs_bHookEnabled = FALSE;
	static YXC_SysExceptionHandler gs_sysExceptionHandler = NULL;
	static DWORD gs_dwOldProtect = 0;

	LONG WINAPI _CrashExceptionFilter(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
	{
		{
			YXX_CritLocker locker(gs_crit);
			_RecordExceptionInfo(ExceptionInfo, L"Crash report");
		}

		if (gs_sysExceptionHandler != NULL) /* Call back to handler and give up handling. */
		{
			return gs_sysExceptionHandler(ExceptionInfo);
		}
		else
		{
			YXC_SysHandleException(ExceptionInfo);
		}

		// will come here if debugger attached
		return EXCEPTION_CONTINUE_SEARCH;
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI _RedirectedSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER pFilter)
	{
		return NULL;
	}

	PVOID WINAPI _RedirectedAddVectoredExceptionHandler(ULONG First, PVECTORED_EXCEPTION_HANDLER pHandler)
	{
		return NULL;
	}

	static BOOL HookExceptionHandlerFuncs()
	{
		DWORD oldProtect;
		BOOL bRet = VirtualProtectEx(GetCurrentProcess(), gs_oldSEF, sizeof(ybyte_t) + sizeof(yuintptr_t), PAGE_EXECUTE_READWRITE, &gs_dwOldProtect);
		if (bRet == FALSE) return FALSE;

		memcpy(gs_oldCtrlBytes, gs_oldSEF, sizeof(gs_oldCtrlBytes));

		BYTE* pNewCtrlBy = (BYTE*)gs_oldSEF;

#if YXC_IS_32BIT
		*pNewCtrlBy++ = 0xe9; // jmp
		*(yuintptr_t*)pNewCtrlBy = (yuintptr_t)_RedirectedSetUnhandledExceptionFilter - (yuintptr_t)gs_oldSEF - sizeof(ybyte_t) - sizeof(void*);
#else
		*pNewCtrlBy++ = 0x68;
		*(yuint32_t*)pNewCtrlBy = YXC_LO_32BITS((yuintptr_t)_RedirectedSetUnhandledExceptionFilter);
		pNewCtrlBy += 4;
		*pNewCtrlBy++ = 0xC7;
		*pNewCtrlBy++ = 0x44;
		*pNewCtrlBy++ = 0x24;
		*pNewCtrlBy++ = 0x04;
		*(yuint32_t*)pNewCtrlBy = YXC_HI_32BITS((yuintptr_t)_RedirectedSetUnhandledExceptionFilter);
		pNewCtrlBy += 4;
		*pNewCtrlBy = 0xC3;
#endif /* YXC_IS_32BIT */

		return TRUE;
	}

	static void UnhookExceptionHandlerFuncs()
	{
		DWORD oldProtect;
		BOOL bRet = VirtualProtectEx(GetCurrentProcess(), gs_oldSEF, sizeof(gs_oldCtrlBytes),
			PAGE_EXECUTE_READWRITE, &oldProtect);
		if (bRet)
		{
			memcpy((void*)gs_oldSEF, gs_oldCtrlBytes, sizeof(gs_oldCtrlBytes));
			VirtualProtectEx(GetCurrentProcess(), gs_oldSEF, sizeof(gs_oldCtrlBytes), gs_dwOldProtect, &oldProtect);
		}
	}

	__forceinline void GetCallerAddr(yuint32_t* pModuleBaseAddr);
#endif /* YXC_PLATFORM_WIN */
}

namespace YXC_Inner
{
	ybool_t _EnableCrashReport(YXC_SysExceptionHandler handler)
	{
		yuint32_t uOriginal = YXCLib::Interlocked::Exchange(&gs_bHookEnabled, 1);
		if (uOriginal == 0)
		{
			SetUnhandledExceptionFilter(_CrashExceptionFilter);

//#if YXC_IS_64BIT
//#pragma message("Won't hook exception handler in 64 bit windows.")
//#else
			BOOL bRet = HookExceptionHandlerFuncs();
			if (!bRet) /* Failure to do hook here. Return to invalid status. */
			{
				YXCLib::Interlocked::Exchange(&gs_bHookEnabled, 0);
				return FALSE;
			}
//#endif /* YXC_IS_64BIT */
		}
		gs_sysExceptionHandler = handler;
		return TRUE;
	}

	void _DisableCrashReport()
	{
		yuint32_t uOriginal = YXCLib::Interlocked::Exchange(&gs_bHookEnabled, 0);
		if (uOriginal == 1)
		{
			SetUnhandledExceptionFilter(NULL);

//#if YXC_IS_64BIT
//#pragma message("Won't hook exception handler in 64 bit windows.")
//#else
			UnhookExceptionHandlerFuncs();
//#endif /* YXC_IS_64BIT */

			gs_sysExceptionHandler = NULL;
		}
	}
}

extern "C"
{
	void YXC_ReportException(PEXCEPTION_POINTERS pExceptPtrs, const wchar_t* lpszMessage)
	{
		_RecordExceptionInfo(pExceptPtrs, lpszMessage);
	}

	ybool_t YXC_EnableCrashHook()
	{
		return YXC_Inner::_EnableCrashReport(NULL);
	}

	ybool_t YXC_EnableCrashHookWithCallback(YXC_SysExceptionHandler excpHandler)
	{
		return YXC_Inner::_EnableCrashReport(excpHandler);
	}

	void YXC_DisableCrashHook()
	{
		YXC_Inner::_DisableCrashReport();
	}

	void YXC_DisableVectoredException()
	{

	}

	void YXC_RestoreVectoredException()
	{

	}

	void YXC_SysHandleException(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		wchar_t szMessage[500];
		wsprintfW(szMessage, L"应用程序出现异常， 错误码为0x%8x， 请与您的产品供应商联系。", ExceptionInfo->ExceptionRecord->ExceptionCode);
		MessageBoxW(NULL, szMessage, L"YXCLib YXCLib", MB_OK | MB_ICONERROR);

#if _MSC_VER > 1400
		if (!::IsDebuggerPresent())
		{
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}
#endif /* _MSC_VER */
	}
}

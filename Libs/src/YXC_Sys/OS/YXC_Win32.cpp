#include <YXC_Sys/YXC_SysBase.h>
#include <YXC_Sys/YXC_TextEncoding.h>

namespace
{
	__declspec(thread) wchar_t szFunc2[YXC_MAX_CCH_PATH];
}

extern "C"
{
	wchar_t* __efunction(const char* func)
	{
		YXC_TECharToWChar(func, YXC_STR_NTS, szFunc2, YXC_MAX_CCH_PATH - 1, NULL, NULL);

		return szFunc2;
	}
};

namespace
{
	static OSVERSIONINFOEXW _osInfo;
	int _InitOSInfo()
	{
		_osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
		::GetVersionExW((OSVERSIONINFOW*)&_osInfo);
		return 0;
	}

	static int _iInit = _InitOSInfo();
}

extern "C"
{
	const OSVERSIONINFOEXW* YXC_GetOSVersionInfo()
	{
		return &_osInfo;
	}
};

#if _MSC_VER < 1600

namespace
{
	typedef BOOL (__stdcall *SetThreadErrorModeFunc)(DWORD dwNewMode, LPDWORD lpOldMode);
	static SetThreadErrorModeFunc _funcSetThreadErrorMode;
	static BOOL InitThreadErrorModeFunc()
	{
		HMODULE h = GetModuleHandleA("kernel32.dll");

		_funcSetThreadErrorMode = (SetThreadErrorModeFunc)GetProcAddress(h, "SetThreadErrorMode");
		return TRUE;
	}

	static BOOL gs_initErrorMode = InitThreadErrorModeFunc();
}

extern "C"
{
#if YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL
	__declspec(dllexport)
#endif /* YXC_EXPORTS_FLAG & YXC_EXPORTS_FLAG_DLL*/
	BOOL __stdcall SetThreadErrorMode(
		_In_   DWORD dwNewMode,
		_Out_  LPDWORD lpOldMode
	)
	{
		return _funcSetThreadErrorMode(dwNewMode, lpOldMode);
	}
};
#endif

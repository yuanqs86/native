#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_MMInterface.h>
#include <YXC_Sys/YXC_HandleRef.hpp>

namespace
{
	static yuint32_t s_uTlsIndex;
}

extern "C"
{
};

namespace YXC_Inner
{
	BOOL _PNCMPDllInitHandler(unsigned int reason)
	{
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		}

		return TRUE;
	}
}

#if YXC_PLATFORM_WIN

#if YXC_EXPORTS_FLAG == YXC_EXPORTS_DISPATCH_DLL
extern "C"
{
	BOOL WINAPI DllMain(HANDLE dllHandle, unsigned int reason, void* lpReserved)
	{
		return YXC_Inner::_PNCMPDllInitHandler(reason);
	}
};
#endif /* YXC_EXPORTS_FLAG */

static void WINAPI TlsMain(HANDLE dllHandle, DWORD reason, void* lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		s_uTlsIndex = TlsAlloc();
		break;
	case DLL_PROCESS_DETACH:
		::TlsFree(s_uTlsIndex);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	}

	return;
}

// put a pointer in a special segment
#pragma data_seg(".CRT$XLB")
static PIMAGE_TLS_CALLBACK s_thread_callback = TlsMain;
#pragma data_seg()

#ifdef _M_IX86
#pragma comment(linker, "/INCLUDE:__tls_used")
#else
#pragma comment(linker, "/INCLUDE:_tls_used")
#endif /* _M_IX86 */

#endif /* YXC_PLATFORM_WIN */

#define __MODULE__ "EK.Logger.Base"

#include <YXC_Sys/YXC_Logger.h>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>

using namespace YXC_Inner;

namespace
{
    static YXC_Status _LoggerReportW(_LoggerBase* pLogger, YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule,
        const wchar_t* cpszTitle, ybool_t bReportLastError, const wchar_t* cpszFormat, va_list vaList)
    {
#if YCHAR_WCHAR_T
        return pLogger->LogFormat(logLevel, cpszModule, cpszTitle, cpszFormat, vaList, bReportLastError);
#else
        char szMessage[4096], *pszMessage = szMessage;
		ysize_t stCchMessage = 4096;

		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;

		char szModule[256], *pszModule = szModule;
		char szTitle[1024], *pszTitle = szTitle;

		if (cpszModule != NULL)
		{
			ysize_t stCchModule = 256;
            rc = _Msg_Convert(cpszModule, &pszModule, &stCchModule);
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}

		if (cpszTitle != NULL)
		{
			ysize_t stCchTitle = 1024;
            rc = _Msg_Convert(cpszTitle, &pszTitle, &stCchTitle);
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}

        rc = _Msg_Convert(cpszFormat, vaList, &pszMessage, &stCchMessage);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		rc = pLogger->Log(logLevel, cpszModule ? pszModule : NULL, cpszTitle ? pszTitle : NULL, pszMessage, bReportLastError);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

	err_ret:
		if (pszMessage != szMessage) _Log_FreeBuffer(pszMessage);
		if (pszModule != szModule) _Log_FreeBuffer(pszModule);
		if (pszTitle != szTitle) _Log_FreeBuffer(pszTitle);

		return rcRet;
#endif
    }

	static YXC_Status _LoggerReportA(_LoggerBase* pLogger, YXC_LoggerLogLevel logLevel, const char* cpszModule,
		const char* cpszTitle, ybool_t bReportLastError, const char* cpszFormat, va_list vaList)
	{
#if YCHAR_WCHAR_T
        ychar szMessage[4096], *pszMessage = szMessage;
		ysize_t stCchMessage = 4096;

		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;

		ychar szModule[256], *pszModule = szModule;
		ychar szTitle[1024], *pszTitle = szTitle;

		if (cpszModule != NULL)
		{
			ysize_t stCchModule = 256;
            rc = _Msg_Convert(cpszModule, &pszModule, &stCchModule);
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}

		if (cpszTitle != NULL)
		{
			ysize_t stCchTitle = 1024;
            rc = _Msg_Convert(cpszTitle, &pszTitle, &stCchTitle);
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}

        rc = _Msg_Convert(cpszFormat, vaList, &pszMessage, &stCchMessage);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		rc = pLogger->Log(logLevel, cpszModule ? pszModule : NULL, cpszTitle ? pszTitle : NULL, pszMessage, bReportLastError);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

	err_ret:
		if (pszMessage != szMessage) _Log_FreeBuffer(pszMessage);
		if (pszModule != szModule) _Log_FreeBuffer(pszModule);
		if (pszTitle != szTitle) _Log_FreeBuffer(pszTitle);

		return rcRet;
#else
        return pLogger->LogFormat(logLevel, cpszModule, cpszTitle, cpszFormat, vaList, bReportLastError);
#endif /* YCHAR_WCHAR_T */
	}
}

extern "C"
{
	YXC_Status YXC_LoggerCreateW(YXC_LoggerType logType, const YXC_LoggerCreateInfoW* pExtraInfo, YXC_Logger* pLogger)
	{
		_LoggerBase* pLoggerBase = NULL;

		YXC_Status rc = _LoggerBase::CreateLoggerW(logType, pExtraInfo, &pLoggerBase);
		if (rc == YXC_ERC_SUCCESS)
		{
			*pLogger = _LoggerHdl(pLoggerBase);
		}
		return rc;
	}

	YXC_Status YXC_LoggerCreateA(YXC_LoggerType logType, const YXC_LoggerCreateInfoA* pExtraInfo, YXC_Logger* pLogger)
	{
		_LoggerBase* pLoggerBase = NULL;

		YXC_Status rc = _LoggerBase::CreateLoggerA(logType, pExtraInfo, &pLoggerBase);
		if (rc == YXC_ERC_SUCCESS)
		{
			*pLogger = _LoggerHdl(pLoggerBase);
		}
		return rc;
	}

	void YXC_LoggerSetReportLevel(YXC_Logger logger, YXC_LoggerLogLevel minReportLevel)
	{
		_LoggerBase* pLoggerBase = _LoggerPtr(logger);

		pLoggerBase->SetMinLogLevel(minReportLevel);
	}

	void YXC_LoggerClose(YXC_Logger logger)
	{
		_LoggerBase::CloseLogger(_LoggerPtr(logger));
	}

	YXC_Status YXC_LoggerLogW(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule,
		const wchar_t* cpszTitle, const wchar_t* cpszFormat, ...)
	{
		va_list vaList;
		va_start(vaList, cpszFormat);

		YXC_Status rc = YXC_LoggerLogW_V(logger, logLevel, cpszModule, cpszTitle, cpszFormat, vaList);
		va_end(vaList);

		return rc;
	}

	YXC_Status YXC_LoggerErrorW(YXC_Logger logger, const wchar_t* cpszModule,
		const wchar_t* cpszTitle, const wchar_t* cpszFormat, ...)
	{
		va_list vaList;
		va_start(vaList, cpszFormat);

		YXC_Status rc = YXC_LoggerErrorW_V(logger, cpszModule, cpszTitle, cpszFormat, vaList);

		va_end(vaList);
		return rc;
	}

	YXC_Status YXC_LoggerLogA(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const char* cpszModule, const char* cpszTitle,
		const char* cpszFormat, ...)
	{
		va_list vaList;
		va_start(vaList, cpszFormat);

		YXC_Status rc = YXC_LoggerLogA_V(logger, logLevel, cpszModule, cpszTitle, cpszFormat, vaList);
		va_end(vaList);

		return rc;
	}

	YXC_Status YXC_LoggerErrorA(YXC_Logger logger, const char* cpszModule,
		const char* cpszTitle, const char* cpszFormat, ...)
	{
		va_list vaList;
		va_start(vaList, cpszFormat);

		YXC_Status rc = YXC_LoggerErrorA_V(logger, cpszModule, cpszTitle, cpszFormat, vaList);
		va_end(vaList);

		return rc;
	}

	YXC_Status YXC_LoggerLogA_V(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const char* cpszModule, const char* cpszTitle,
		const char* cpszFormat, va_list vaList)
	{
		_LoggerBase* pLogger = _LoggerPtr(logger);
		if (pLogger->CanReport(logLevel))
		{
			YXC_Status rc = _LoggerReportA(pLogger, logLevel, cpszModule, cpszTitle,
				FALSE, cpszFormat, vaList);
			return rc;
		}

		return YXC_ERC_LOG_LOW_LEVEL;
	}

	YXC_Status YXC_LoggerErrorA_V(YXC_Logger logger, const char* cpszModule,
		const char* cpszTitle, const char* cpszFormat, va_list vaList)
	{
		_LoggerBase* pLogger = _LoggerPtr(logger);
		if (pLogger->CanReport(YXC_LOGGER_LL_ERROR))
		{
			YXC_Status rc = _LoggerReportA(pLogger, YXC_LOGGER_LL_ERROR, cpszModule, cpszTitle,
				TRUE, cpszFormat, vaList);
			return rc;
		}

		return YXC_ERC_LOG_LOW_LEVEL;
	}

	YXC_Status YXC_LoggerLogW_V(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule, const wchar_t* cpszTitle,
                              const wchar_t* cpszFormat, va_list vaList)
	{
		_LoggerBase* pLogger = _LoggerPtr(logger);
		if (pLogger->CanReport(logLevel))
		{
			YXC_Status rc = _LoggerReportW(pLogger, logLevel, cpszModule, cpszTitle,
                FALSE, cpszFormat, vaList);
			return rc;
		}

		return YXC_ERC_LOG_LOW_LEVEL;
	}

	YXC_Status YXC_LoggerErrorW_V(YXC_Logger logger, const wchar_t* cpszModule,
                                const wchar_t* cpszTitle, const wchar_t* cpszFormat, va_list vaList)
	{
		_LoggerBase* pLogger = _LoggerPtr(logger);
		if (pLogger->CanReport(YXC_LOGGER_LL_ERROR))
		{
			YXC_Status rc = _LoggerReportW(pLogger, YXC_LOGGER_LL_ERROR, cpszModule, cpszTitle,
                TRUE, cpszFormat, vaList);
			return rc;
		}

		return YXC_ERC_LOG_LOW_LEVEL;
	}

	YXC_Logger YXC_GetKernelXLogger()
	{
		return _LoggerHdl(gs_kernelLogger);
	}
};

#if YXC_PLATFORM_WIN
#include <Shlwapi.h>
namespace YXC_Inner
{
	static BOOL _CreateKLogger()
	{
		HKEY hKey;
		LSTATUS rc = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\YXCLib\\Settings\\Basic", 0, KEY_ALL_ACCESS, &hKey);
		if (rc == ERROR_SUCCESS)
		{
			DWORD dwKernelLoggerLevel = 0;
			DWORD dwRetType = 0, dwSize = sizeof(DWORD);
			rc = RegQueryValueExW(hKey, L"KernelLogLevel", NULL, &dwRetType, (LPBYTE)&dwKernelLoggerLevel, &dwSize);

			if (rc == ERROR_SUCCESS && dwRetType == REG_DWORD)
			{
				YXC_LoggerLogLevel logLevel = (YXC_LoggerLogLevel)dwKernelLoggerLevel;

				wchar_t szPath[MAX_PATH], szAppName[MAX_PATH];

				HMODULE hModuleApp = ::GetModuleHandleW(NULL);
				if (hModuleApp == NULL) return FALSE;

				HMODULE hModule = ::GetModuleHandleW(L"YXC_Sys.dll");
				if (hModule == NULL)
				{
					hModule = hModuleApp;
				}

				DWORD dwRet = GetModuleFileNameW(hModule, szPath, MAX_PATH);
				if (dwRet == 0) return FALSE;
				::PathRemoveFileSpecW(szPath);

				dwRet = GetModuleFileNameW(hModuleApp, szAppName, MAX_PATH);
				if (dwRet == 0) return FALSE;

				wchar_t* pszFile = PathFindFileNameW(szAppName);
				::PathAppendW(szPath, pszFile);
				wcscat_s(szPath, L"-kernel.ylog");

				YXC_LoggerCreateInfoW createInfo = { szPath };
				YXC_Status rc = _LoggerBase::CreateLoggerW(YXC_LOGGER_TYPE_FILE, &createInfo, &gs_kernelLogger);
				if (rc == YXC_ERC_SUCCESS)
				{
					gs_kernelLogger->SetMinLogLevel(logLevel);
				}

				return rc == YXC_ERC_SUCCESS;
			}
		}

		return TRUE;
	}

	BOOL _LogDllInitHandler(unsigned int reason)
	{
		BOOL bRet = TRUE;
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			bRet = _CreateKLogger();
			break;
		case DLL_PROCESS_DETACH:
			if (gs_kernelLogger)
			{
				_LoggerBase::CloseLogger(gs_kernelLogger);
				gs_kernelLogger = NULL;
			}
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		}

		return bRet;
	}
}
#if YXC_EXPORTS_FLAG == YXC_EXPORTS_DISPATCH_DLL
extern "C"
{
	BOOL WINAPI DllMain(HANDLE dllHandle, unsigned int reason, void* lpReserved)
	{
		return YXC_Inner::_LogDllInitHandler(reason);
	}
};
#endif /* YXC_EXPORTS_FLAG */
#endif /* YXC_PLATFORM_WIN */

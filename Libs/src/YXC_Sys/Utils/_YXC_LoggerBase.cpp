#define __MODULE__ "EK.Logger.LoggerBase"

#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>
#include <YXC_Sys/Utils/_YXC_FileLogger.hpp>
#include <YXC_Sys/Utils/_YXC_ConsoleLogger.hpp>
#include <YXC_Sys/Utils/_YXC_DebugLogger.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>
#include <YXC_Sys/YXC_FilePath.h>
#include <stdio.h>

#if YXC_PLATFORM_WIN
#include <Shlwapi.h>
#elif YXC_PLATFORM_UNIX
#include <sys/time.h>
#endif /* YXC_PLATFORM_WIN */

#define _YXC_TIME_FORMAT YC("[%04d-%02d-%02d %02d:%02d:%02d.%03d] ")

namespace
{
	static const ychar* gs_szLogLevel[] = { YC("LVL_VERBO "), YC("LVL_INFO  "), YC("LVL_WARN  "), YC("LVL_ERROR "), YC("LVL_FATAL ") };

	static const ychar* _GetLogLevelStr(YXC_LoggerLogLevel logLevel)
	{
		if (logLevel > YXC_LOGGER_LL_FATAL) logLevel = YXC_LOGGER_LL_FATAL;
		if (logLevel < YXC_LOGGER_LL_VERBOSE) logLevel = YXC_LOGGER_LL_VERBOSE;

		return gs_szLogLevel[(int)logLevel];
	}

	//static const wchar_t gs_szLogFormat_All[] = _YXC_TIME_FORMAT L"%s" _YXC_SIMPLE_LOG_FORMAT;
	//static const wchar_t gs_szLogFormat_Name[] = L"%s" _YXC_SIMPLE_LOG_FORMAT;
	//static const wchar_t gs_szLogFormat_Time[] = _YXC_TIME_FORMAT _YXC_SIMPLE_LOG_FORMAT;
	//static const wchar_t gs_szLogFormat_Simple[] = _YXC_SIMPLE_LOG_FORMAT;
	static const ychar gs_szDetailCh[] = YC("\r\n    DETAILS : ");
	// static const wchar_t* gs_szLogFormats[] = { gs_szLogFormat_Simple, gs_szLogFormat_Time, gs_szLogFormat_Name, gs_szLogFormat_All };

#if YXC_PLATFORM_WIN
	static const ychar gs_lineEnd[] = YC("\r\n");
#else
    static const ychar gs_lineEnd[] = YC("\n");
#endif /* YXC_PLATFORM_WIN */

	static ysize_t _FormatCopy(ychar* pszFormat, ychar* pszEnd, const ychar* pszSrc, ybool_t* pbFullCopied)
	{
		ychar* pszStart = pszFormat;
		if (pbFullCopied != NULL) *pbFullCopied = FALSE;

		while (pszFormat < pszEnd)
		{
			ychar wch = *pszSrc++;
			switch (wch)
			{
			case '\0':
				*pszFormat = wch;
				if (pbFullCopied != NULL) *pbFullCopied = TRUE;
				return pszFormat - pszStart;
			case '%':
				if (pszFormat + 1 >= pszEnd)
				{
					return pszFormat - pszStart;
				}
				// duplicate to '%%'
				*pszFormat++ = '%';
				*pszFormat++ = '%';
				break;
			default:
				*pszFormat++ = wch;
				break;
			}
		}

		return pszFormat - pszStart;
	}

	static size_t _GetBaseFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
		ybool_t bAppendTime, ybool_t bAppendFileName, ychar* pszBaseFormat, ysize_t stCchBufFormat, ybool_t bUseFormat)
	{
		const ychar* cpszLvl = _GetLogLevelStr(logLevel);
		ychar* pszOldBase = pszBaseFormat;
		ychar* pszEnd = pszOldBase + stCchBufFormat;

		if (bAppendTime)
		{
            struct timeval tv;
            struct tm tm_l;
            yxcwrap_gettimeofday(&tv);
#if YXC_PLATFORM_WIN
			time_t tm_seconds = tv.tv_sec;
			localtime_s(&tm_l, &tm_seconds);
#else
            localtime_r(&tv.tv_sec, &tm_l);
#endif /* YXC_PLATFORM_WIN */
			ssize_t iRet = yh_snprintf(pszBaseFormat, pszEnd - pszBaseFormat, _YXC_TIME_FORMAT, tm_l.tm_year + 1900, tm_l.tm_mon + 1, tm_l.tm_mday,
				tm_l.tm_hour, tm_l.tm_min, tm_l.tm_sec, tv.tv_usec / 1000);
			pszBaseFormat += iRet;
		}

		if (bAppendFileName)
		{
			ychar szFileName[YXC_MAX_CCH_PATH];
            YXC_FPathFindModulePath(szFileName, NULL);
            const ychar* pszFileName = YXC_FPathFindFileSpec(szFileName);

			if (bUseFormat)
			{
				ysize_t stFormatted = _FormatCopy(pszBaseFormat, pszEnd, pszFileName, NULL);
				pszBaseFormat += stFormatted;
			}
			else
			{
				YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, pszFileName, YXC_STR_NTS);
			}

			ychar szPid[100];
			yh_sprintf(szPid, YC("[%d] "), YXCLib::OSGetCurrentProcessId());

			YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, szPid, YXC_STR_NTS);
		}

		YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, cpszLvl, YXC_STR_NTS);

		if (cpszModule != NULL)
		{
			YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, YC("<"), YXC_STRING_ARR_LEN(YC("<")));
			if (bUseFormat)
			{
				ysize_t stFormatted = _FormatCopy(pszBaseFormat, pszEnd, cpszModule, NULL);
				pszBaseFormat += stFormatted;
			}
			else
			{
				YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, cpszModule, YXC_STR_NTS);
			}
			YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, YC("> "), YXC_STRING_ARR_LEN(YC("> ")));
		}

		if (cpszTitle != NULL)
		{
			if (bUseFormat)
			{
				ysize_t stFormatted = _FormatCopy(pszBaseFormat, pszEnd, cpszTitle, NULL);
				pszBaseFormat += stFormatted;
			}
			else
			{
				YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, cpszTitle, YXC_STR_NTS);
			}
			YXCLib::_AppendStringChk(pszBaseFormat, pszEnd, gs_szDetailCh, YXC_STRING_ARR_LEN(gs_szDetailCh));
		}

		return pszBaseFormat - pszOldBase;
	}

	static ysize_t _CopyErrorBuffer(ychar* pszBuffer)
	{
		const YXC_Error* pError = ::YXC_GetLastError();
		if (pError == NULL) return 0;

		ysize_t stTotal = 0;

		static const ychar gs_szErrInfo[] = YC("\r\n\tError Information:");
		static const ychar gs_szErrorFormat[] = YC("\r\n\t\tMessage = %s, Module = %s, Src = %d, Code = %lld, File = %s:%d, Function = %s");

		if (pszBuffer != NULL)
		{
			yh_strcpy(pszBuffer, gs_szErrInfo);
			pszBuffer += YXC_STRING_ARR_LEN(gs_szErrInfo);
		}
		stTotal += YXC_STRING_ARR_LEN(gs_szErrInfo);

		while (pError != NULL)
		{
			if (pszBuffer == NULL)
			{
				yssize_t stSize = yh_scprintf(gs_szErrorFormat, pError->wszMsg, pError->wszModule, (int)pError->errSrc, pError->llErrCode,
					pError->wszFile, pError->iLine, pError->wszFunction);
                if (stSize >= 0) stTotal += stSize;
			}
			else
			{
				yssize_t stSize = yh_sprintf(pszBuffer, gs_szErrorFormat, pError->wszMsg, pError->wszModule, (int)pError->errSrc, pError->llErrCode,
					pError->wszFile, pError->iLine, pError->wszFunction);
                if (stSize >= 0)
                {
                    stTotal += stSize;
                    pszBuffer += stSize;
                }
			}

			pError = pError->pInnerError;
		}

		return stTotal;
	}

	static YXC_Status _CreateLoggerPointer(YXC_LoggerType loggerType, YXC_Inner::_LoggerBase** ppLogger)
	{
		switch (loggerType)
		{
		case YXC_LOGGER_TYPE_FILE:
			*ppLogger = (YXC_Inner::_LoggerBase*)malloc(sizeof(YXC_Inner::_FileLogger));
			_YXC_CHECK_REPORT_NEW_RET(*ppLogger != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for FileLogger failed"));
			new (*ppLogger) YXC_Inner::_FileLogger();
			break;
		case YXC_LOGGER_TYPE_CONSOLE:
			*ppLogger = (YXC_Inner::_LoggerBase*)malloc(sizeof(YXC_Inner::_ConsoleLogger));
			_YXC_CHECK_REPORT_NEW_RET(*ppLogger != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for ConsoleLogger failed"));
			new (*ppLogger) YXC_Inner::_ConsoleLogger();
			break;
#if YXC_PLATFORM_WIN
		case YXC_LOGGER_TYPE_DEBUG:
			*ppLogger = (YXC_Inner::_LoggerBase*)malloc(sizeof(YXC_Inner::_DebugLogger));
			_YXC_CHECK_REPORT_NEW_RET(*ppLogger != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for DebugLogger failed"));
			new (*ppLogger) YXC_Inner::_DebugLogger();
			break;
#endif /* YXC_PLATFORM_WIN */
		case YXC_LOGGER_TYPE_DUMMY:
			*ppLogger = (YXC_Inner::_LoggerBase*)malloc(sizeof(YXC_Inner::_DummyLogger));
			_YXC_CHECK_REPORT_NEW_RET(*ppLogger != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for DummyLogger failed"));
			new (*ppLogger) YXC_Inner::_DummyLogger();
			break;
		default:
			_YXC_REPORT_NEW_ERR(YXC_ERC_NOT_SUPPORTED, YC("Invalid logger type specified %d"), (int)loggerType);
			return YXC_ERC_NOT_SUPPORTED;
		}

		return YXC_ERC_SUCCESS;
	}
}

namespace YXC_Inner
{
	YXC_Status _Log_Convert(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle, const ychar* cpszFormat,
		va_list vaList, ybool_t bAppendTime, ybool_t bAppendFileName, ybool_t bReportError, ychar** ppOutBuf, ysize_t* pstCchBuf)
	{
		ychar szRealFormat[4096];
        szRealFormat[4095] = 0;
		_GetBaseFormat(logLevel, cpszTitle, cpszModule, bAppendTime, bAppendFileName, szRealFormat, 4096, TRUE);

		yh_strncat(szRealFormat, cpszFormat, 4095);

		yssize_t iCchNeeded = yh_vscprintf(szRealFormat, vaList);

		ysize_t stCchErr = bReportError ? _CopyErrorBuffer(NULL) : 0;
		ysize_t stCchBufNeeded = iCchNeeded + stCchErr + YXC_STRING_ARR_LEN(gs_lineEnd) + 1;

		if (stCchBufNeeded > *pstCchBuf)
		{
			*ppOutBuf = (ychar*)::malloc(stCchBufNeeded * sizeof(ychar));
			_YXC_CHECK_REPORT_NEW_RET(*ppOutBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer failed, cchSize = %lu"), stCchBufNeeded);
		}
		*pstCchBuf = stCchBufNeeded - 1;

		ychar* pszMem = *ppOutBuf;
		yh_vsnprintf(pszMem, stCchBufNeeded, szRealFormat, vaList);
		pszMem += iCchNeeded;

		if (bReportError)
		{
			_CopyErrorBuffer(pszMem);
			pszMem += stCchErr;
		}

		memcpy(pszMem, gs_lineEnd, sizeof(gs_lineEnd));
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _Log_Convert(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle, const ychar* cpszMessage,
		ybool_t bAppendTime, ybool_t bAppendFileName, ybool_t bReportError, ychar** ppOutBuf, ysize_t* pstCchBuf)
	{
		ychar szBaseFormat[4096];
		size_t iCchFormat = _GetBaseFormat(logLevel, cpszTitle, cpszModule, bAppendTime, bAppendFileName, szBaseFormat, 4096, FALSE);

		size_t strLen = yh_strlen(cpszMessage);
		ysize_t stCchErr = bReportError ? _CopyErrorBuffer(NULL) : 0;
		ysize_t stCchBufNeeded = strLen + iCchFormat + stCchErr + YXC_STRING_ARR_LEN(gs_lineEnd) + 1;

		if (*pstCchBuf < stCchBufNeeded)
		{
			*ppOutBuf = (ychar*)::malloc(stCchBufNeeded * sizeof(ychar));
			_YXC_CHECK_REPORT_NEW_RET(*ppOutBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer failed, cchSize = %lu"), stCchBufNeeded);
		}
		*pstCchBuf = stCchBufNeeded - 1;

		ychar* pszMem = *ppOutBuf;

		memcpy(pszMem, szBaseFormat, iCchFormat * sizeof(ychar));
		pszMem += iCchFormat;

		memcpy(pszMem, cpszMessage, strLen * sizeof(ychar));
		pszMem += strLen;

		if (bReportError)
		{
			_CopyErrorBuffer(pszMem);
			pszMem += stCchErr;
		}

		memcpy(pszMem, gs_lineEnd, sizeof(gs_lineEnd));
		return YXC_ERC_SUCCESS;
	}

#if YCHAR_WCHAR_T
	YXC_Status _Msg_Convert(const char* cpszFormat, va_list vaList, ychar** ppOutBuf, ysize_t* pstCchBuf)
	{
		char chBuf[4096], *pszChBuf = chBuf;
		ychar* pszOldOutBuf = *ppOutBuf;
		yssize_t iCchNeeded = yxcwrap_vscprintf(cpszFormat, vaList);

		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;
        yuint32_t uMax, uCvt;

		if (iCchNeeded + 1 > 4096)
		{
			pszChBuf = (char*)::malloc((iCchNeeded + 1) * sizeof(char));
			_YXC_CHECK_REPORT_NEW_GOTO(pszChBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer(ANSI) failed, cchSize = %d"), (int)iCchNeeded + 1);
		}

		vsprintf(chBuf, cpszFormat, vaList);

        rc = YXC_TECharToWChar(pszChBuf, YXC_STR_NTS, NULL, 0, &uMax, NULL);
        _YXC_CHECK_STATUS_GOTO(rc, YC("Convert to wchar failed"));
		if (uMax >= *pstCchBuf)
		{
			*ppOutBuf = (ychar*)malloc((uMax + 1) * sizeof(ychar));
			*pstCchBuf = uMax;
			_YXC_CHECK_REPORT_NEW_GOTO(*ppOutBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer failed, cchSize = %d"), uMax);
		}

		rc = YXC_TECharToWChar(pszChBuf, YXC_STR_NTS, *ppOutBuf, uMax, &uCvt, NULL);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Convert multi byte to wide char failed"));

		if (pszChBuf != chBuf) free(pszChBuf);
		return YXC_ERC_SUCCESS;
err_ret:
		if (pszChBuf != chBuf) free(pszChBuf);
		if (*ppOutBuf != pszOldOutBuf) free(*ppOutBuf);
		return rcRet;
	}

	YXC_Status _Msg_Convert(const char* cpszMessage, ychar** ppOutBuf, ysize_t* pstCchBuf)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS;
		ychar* pszOldOutBuf = *ppOutBuf;

        yuint32_t uMax, uCvt;
		YXC_Status rc = YXC_TECharToWChar(cpszMessage, YXC_STR_NTS, NULL, 0, &uMax, NULL);
        _YXC_CHECK_STATUS_GOTO(rc, YC("Convert to wchar failed"));
		if (uMax >= *pstCchBuf)
		{
			*ppOutBuf = (ychar*)malloc((uMax + 1) * sizeof(ychar));
			*pstCchBuf = uMax;
			_YXC_CHECK_REPORT_NEW_GOTO(*ppOutBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer failed, cchSize = %d"), uMax);
		}

		rc = YXC_TECharToWChar(cpszMessage, YXC_STR_NTS, *ppOutBuf, uMax, &uCvt, NULL);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Convert multi byte to wide char failed"));

		return YXC_ERC_SUCCESS;
err_ret:
		if (*ppOutBuf != pszOldOutBuf) free(*ppOutBuf);
		return rcRet;
	}
#else

	YXC_Status _Msg_Convert(const wchar_t* cpszFormat, va_list vaList, ychar** ppOutBuf, ysize_t* pstCchBuf)
	{
		wchar_t chBuf[4096], *pszChBuf = chBuf;
		yssize_t iCchNeeded = yxcwrap_vscwprintf(cpszFormat, vaList);
        _YXC_CHECK_OS_RET(iCchNeeded >= 0, YC("Calculate source size failed"));

		if (iCchNeeded + 1 > 4096)
		{
			pszChBuf = (wchar_t*)::malloc((iCchNeeded + 1) * sizeof(wchar_t));
			_YXC_CHECK_REPORT_NEW_RET(pszChBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer(ANSI) failed, cchSize = %d"), (int)iCchNeeded + 1);
		}

		yxcwrap_vswprintf(pszChBuf, cpszFormat, vaList);
		ychar* pszOldOutBuf = *ppOutBuf;

		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;
        yuint32_t uCvt, uMaxChar = 4 * (yuint32_t)iCchNeeded;

        if (*pstCchBuf < uMaxChar)
        {
			*ppOutBuf = (ychar*)malloc((uMaxChar + 1) * sizeof(ychar));
			*pstCchBuf = uMaxChar;
			_YXC_CHECK_REPORT_NEW_GOTO(*ppOutBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer failed, cchSize = %d"), uMaxChar);
        }

		rc = YXC_TEWCharToChar(pszChBuf, YXC_STR_NTS, *ppOutBuf, uMaxChar, &uCvt, NULL);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Convert multi byte to wide char failed"));

		if (pszChBuf != chBuf) free(pszChBuf);

        *pstCchBuf = uCvt;
		return YXC_ERC_SUCCESS;
    err_ret:
		if (pszChBuf != chBuf) free(pszChBuf);
		if (*ppOutBuf != pszOldOutBuf) free(*ppOutBuf);
		return rcRet;
	}

	YXC_Status _Msg_Convert(const wchar_t* cpszMessage, ychar** ppOutBuf, ysize_t* pstCchBuf)
	{
		YXC_Status rcRet = YXC_ERC_SUCCESS, rc;
		ychar* pszOldOutBuf = *ppOutBuf;

        yuint32_t uMsg = (yuint32_t)wcslen(cpszMessage), uMaxChar = 4 * uMsg;
        yuint32_t uCvt;
        if (uMaxChar >= *pstCchBuf)
        {
			*ppOutBuf = (ychar*)malloc((uMaxChar + 1) * sizeof(ychar));
			_YXC_CHECK_REPORT_NEW_GOTO(*ppOutBuf != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for log buffer failed, cchSize = %d"), uMaxChar);
        }

        rc = YXC_TEWCharToChar(cpszMessage, uMsg, pszOldOutBuf, uMaxChar, &uCvt, NULL);
        _YXC_CHECK_STATUS_GOTO(rc, YC("Convert to wchar failed"));

        *pstCchBuf = uCvt;
		return YXC_ERC_SUCCESS;
    err_ret:
		if (*ppOutBuf != pszOldOutBuf) free(*ppOutBuf);
		return rcRet;
	}
#endif /* YCHAR_WCHAR_T */

	void _Log_FreeBuffer(ychar* pszBuffer)
	{
		free(pszBuffer);
	}

	_LoggerBase::_LoggerBase() : _minLogLevel(YXC_LOGGER_LL_INFO)
	{

	}

	_LoggerBase::~_LoggerBase()
	{

	}

	YXC_Status _LoggerBase::CreateLoggerA(YXC_LoggerType loggerType, const YXC_LoggerCreateInfoA* pInfo, _LoggerBase** ppLogger)
	{
		_LoggerBase* pLogger = NULL;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		YXC_Status rc = _CreateLoggerPointer(loggerType, &pLogger);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		rc = pLogger->CreateA(pInfo);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		*ppLogger = pLogger;
		return YXC_ERC_SUCCESS;
err_ret:
		if (pLogger != NULL)
		{
			pLogger->~_LoggerBase();
			free(pLogger);
		}
		return rc;
	}

	YXC_Status _LoggerBase::CreateLoggerW(YXC_LoggerType loggerType, const YXC_LoggerCreateInfoW* pInfo, _LoggerBase** ppLogger)
	{
		_LoggerBase* pLogger = NULL;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		YXC_Status rc = _CreateLoggerPointer(loggerType, &pLogger);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		rc = pLogger->CreateW(pInfo);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		*ppLogger = pLogger;
		return YXC_ERC_SUCCESS;
	err_ret:
		if (pLogger != NULL)
		{
			pLogger->~_LoggerBase();
			free(pLogger);
		}
		return rc;
	}

	void _LoggerBase::CloseLogger(_LoggerBase* pLogger)
	{
		pLogger->~_LoggerBase();

		free(pLogger);
	}

	YXC_Status _LoggerBase::CreateA(const YXC_LoggerCreateInfoA* pInfo)
	{
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _LoggerBase::CreateW(const YXC_LoggerCreateInfoW* pInfo)
	{
		return YXC_ERC_SUCCESS;
	}

	void _LoggerBase::Close()
	{

	}

	_LoggerBase* gs_kernelLogger = NULL;
}

namespace YXC_Inner
{
	_DummyLogger::_DummyLogger() {}

	_DummyLogger::~_DummyLogger() {}

	YXC_Status _DummyLogger::LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle, const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError)
	{
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _DummyLogger::Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle, const ychar* cpszMessage, ybool_t bReportLastError)
	{
		return YXC_ERC_SUCCESS;
	}
}

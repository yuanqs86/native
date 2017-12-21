/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_LOGGER_H__
#define __INC_YXC_SYS_BASE_LOGGER_H__

#include <YXC_Sys/YXC_Sys.h>

#if YXC_PLATFORM_WIN && !defined(__BCOPT__)
#include <vadefs.h>
#endif /* YXC_PLATFORM_WIN */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_Logger);

	typedef enum __YXC_LOGGER_TYPE
	{
		YXC_LOGGER_TYPE_UNKNOWN = 0,
		YXC_LOGGER_TYPE_FILE,
		YXC_LOGGER_TYPE_EVENT_LOG,
		YXC_LOGGER_TYPE_CONSOLE,
		YXC_LOGGER_TYPE_DEBUG,
		YXC_LOGGER_TYPE_DUMMY,
	}YXC_LoggerType;

	typedef enum __YXC_LOGGER_LOG_LEVEL
	{
		YXC_LOGGER_LL_VERBOSE = 0,
		YXC_LOGGER_LL_INFO = 1,
		YXC_LOGGER_LL_WARNING = 2,
		YXC_LOGGER_LL_ERROR = 3,
		YXC_LOGGER_LL_FATAL = 4,
		YXC_LOGGER_LL_MAX = 5 /* If you set this, means don't report any log */,
	}YXC_LoggerLogLevel;

    typedef enum __YXC_LOGGER_LOG_PRIORITY
    {
        YXC_LOGGER_LP_HIGH = 0,
        YXC_LOGGER_LP_NORMAL = 1,
        YXC_LOGGER_LP_LOW = 2,
        YXC_LOGGER_LP_LOOP = 3,
    }YXC_LoggerLogPri;

	typedef union __YXC_LOGGER_CREATE_INFOW
	{
		const wchar_t* cpszFilePath;
	}YXC_LoggerCreateInfoW;

	typedef union __YXC_LOGGER_CREATE_INFOA
	{
		const char* cpszFilePath;
	}YXC_LoggerCreateInfoA;

	YXC_API(YXC_Status) YXC_LoggerCreateW(YXC_LoggerType logType, const YXC_LoggerCreateInfoW* pExtraInfo, YXC_Logger* pLogger);

	YXC_API(YXC_Status) YXC_LoggerCreateA(YXC_LoggerType logType, const YXC_LoggerCreateInfoA* pExtraInfo, YXC_Logger* pLogger);

	YXC_API(void) YXC_LoggerClose(YXC_Logger logger);

	YXC_API(void) YXC_LoggerSetReportLevel(YXC_Logger logger, YXC_LoggerLogLevel minReportLevel);

	// Please use YXC_LoggerLogW if you can instead of YXC_LoggerReportA, since we will convert ANSI to unicode here.
	YXC_API(YXC_Status) YXC_LoggerLogW(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule, const wchar_t* cpszTitle,
		const wchar_t* cpszFormat, ...);

	YXC_API(YXC_Status) YXC_LoggerLogA(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const char* cpszModule, const char* cpszTitle,
		const char* cpszFormat, ...);

    YXC_API(YXC_Status) YXC_LoggerErrorW(YXC_Logger logger, const wchar_t* cpszModule,
		const wchar_t* cpszTitle, const wchar_t* cpszFormat, ...);

	YXC_API(YXC_Status) YXC_LoggerErrorA(YXC_Logger logger, const char* cpszModule,
		const char* cpszTitle, const char* cpszFormat, ...);

	YXC_API(YXC_Status) YXC_LoggerLogW_V(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule, const wchar_t* cpszTitle,
		const wchar_t* cpszFormat, va_list vaList);

	YXC_API(YXC_Status) YXC_LoggerLogA_V(YXC_Logger logger, YXC_LoggerLogLevel logLevel, const char* cpszModule, const char* cpszTitle,
		const char* cpszFormat, va_list vaList);

	YXC_API(YXC_Status) YXC_LoggerErrorW_V(YXC_Logger logger, const wchar_t* cpszModule,
		const wchar_t* cpszTitle, const wchar_t* cpszFormat, va_list vaList);

	YXC_API(YXC_Status) YXC_LoggerErrorA_V(YXC_Logger logger, const char* cpszModule,
        const char* cpszTitle, const char* cpszFormat, va_list vaList);

	YXC_API(YXC_Logger) YXC_GetKernelXLogger();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#if YCHAR_WCHAR_T
#define YXC_LoggerCreate YXC_LoggerCreateW
#define YXC_LoggerLog YXC_LoggerLogW
#define YXC_LoggerLog_V YXC_LoggerLogW_V
#define YXC_LoggerError YXC_LoggerErrorW
#define YXC_LoggerError_V YXC_LoggerErrorW_V
typedef YXC_LoggerCreateInfoW YXC_LoggerCreateInfo;
#else
#define YXC_LoggerCreate YXC_LoggerCreateA
#define YXC_LoggerLog YXC_LoggerLogA
#define YXC_LoggerLog_V YXC_LoggerLogA_V
#define YXC_LoggerError YXC_LoggerErrorA
#define YXC_LoggerError_V YXC_LoggerErrorA_V
typedef YXC_LoggerCreateInfoA YXC_LoggerCreateInfo;
#endif /* YCHAR_WCHAR_T */

#endif /* __INC_YXC_SYS_BASE_LOGGER_H__ */

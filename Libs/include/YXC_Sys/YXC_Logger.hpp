/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_LOGGER_HPP__
#define __INC_YXC_SYS_BASE_LOGGER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_Logger.hpp>
#include <varargs.h>

namespace YXCLib
{
	/* Singleton logger, not inheritable */
	class Logger
	{
	private:
		inline Logger();

		inline ~Logger();

		Logger(const Logger& rhs);

		Logger& operator =(const Logger& rhs);

	public:
		inline YXC_Status LogW(YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule, const wchar_t* cpszTitle,
			const wchar_t* cpszFormat, va_list vaList)
		{
			YXC_Logger logger = Interlocked::ExchangeAddPtr((void* volatile*)&this->_logger, 0);

			if (logger != NULL)
			{
				return YXC_LoggerLogW_V(logger, logLevel, cpszModule, cpszTitle, cpszFormat, vaList);
			}

			return YXC_ERC_INVALID_HANDLE;
		}

		inline YXC_Status LogLastErrorW(const wchar_t* cpszModule, const wchar_t* cpszTitle,
			const wchar_t* cpszFormat, va_list vaList)
		{
			YXC_Logger logger = Interlocked::ExchangeAddPtr((void* volatile*)&this->_logger, 0);

			if (logger != NULL)
			{
				return YXC_LoggerErrorW_V(logger, cpszModule, cpszTitle, cpszFormat, vaList);
			}

			return YXC_ERC_INVALID_HANDLE;
		}

		inline YXC_Status LogA(YXC_LoggerLogLevel logLevel, const char* cpszModule, const char* cpszTitle,
			const char* cpszFormat, va_list vaList)
		{
			YXC_Logger logger = Interlocked::ExchangeAddPtr((void* volatile*)&this->_logger, 0);

			if (logger != NULL)
			{
				return YXC_LoggerReportA_V(logger, logLevel, cpszModule, cpszTitle, cpszFormat, vaList);
			}

			return YXC_ERC_INVALID_HANDLE;
		}

		inline YXC_Status LogLastErrorA(const char* cpszModule, const char* cpszTitle,
			const char* cpszFormat, va_list vaList)
		{
			YXC_Logger logger = Interlocked::ExchangeAddPtr((void* volatile*)&this->_logger, 0);

			if (logger != NULL)
			{
				return YXC_LoggerReportLastErrorA_V(logger, cpszModule, cpszTitle, cpszFormat, vaList);
			}

			return YXC_ERC_INVALID_HANDLE;
		}

		inline YXC_Status LogW(YXC_LoggerLogLevel logLevel, const wchar_t* cpszFormat, va_list vaList)
		{
			return this->LogW(logLevel, NULL, NULL, cpszFormat, vaList);
		}

		inline YXC_Status LogW(YXC_LoggerLogLevel logLevel, const wchar_t* cpszModule, const wchar_t* cpszFormat, va_list vaList)
		{
			return this->LogW(logLevel, cpszModule, NULL, vaList);
		}

	public:
		inline YXC_Status LogInfoW(YXC_LoggerLogLevel logLevel, const wchar_t* cpszFormat, ...)
		{
			va_list vaList;
			va_start(vaList, cpszFormat);

			YXC_Status rc = LogW(YXC_LOGGER_LL_INFO, cpszFormat, vaList);
			va_end(vaList);

			return rc;
		}

		YXC_Status LogMsgA(const char* cpszMessage);

	public:
		static inline Logger* GetInstance();

		static inline YXC_Status InitLogger(YXC_LoggerType loggerType, YXC_LoggerLogLevel minLogLevel);

	private:
		YXC_Logger _logger;

		YXC_Crit _crit;

	private:
		static Logger logger;
	};
}

#endif /* __INC_YXC_SYS_BASE_LOGGER_HPP__ */

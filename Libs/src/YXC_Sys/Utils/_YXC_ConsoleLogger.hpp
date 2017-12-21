#ifndef __INNER_INC_YXC_SYS_BASE_CONSOLE_LOGGER_HPP__
#define __INNER_INC_YXC_SYS_BASE_CONSOLE_LOGGER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>

namespace YXC_Inner
{
	class _ConsoleLogger : public _LoggerBase
	{
	public:
		_ConsoleLogger();

		~_ConsoleLogger();

	public:
		virtual YXC_Status LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError);

		virtual YXC_Status Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszMessage, ybool_t bReportLastError);

	private:
		_ConsoleLogger(const _ConsoleLogger& rhs);

		_ConsoleLogger& operator =(const _ConsoleLogger& rhs);

	private:
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_CONSOLE_LOGGER_HPP__ */

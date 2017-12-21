#ifndef __INNER_INC_YXC_SYS_BASE_DEBUG_LOGGER_HPP__
#define __INNER_INC_YXC_SYS_BASE_DEBUG_LOGGER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>

namespace YXC_Inner
{
	class _DebugLogger : public _LoggerBase
	{
	public:
		_DebugLogger();

		~_DebugLogger();

	public:
		virtual YXC_Status LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError);

		virtual YXC_Status Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszMessage, ybool_t bReportLastError);

	private:
		_DebugLogger(const _DebugLogger& rhs);

		_DebugLogger& operator =(const _DebugLogger& rhs);

	private:
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_DEBUG_LOGGER_HPP__ */

#ifndef __INNER_INC_YXC_SYS_BASE_FILE_LOGGER_HPP__
#define __INNER_INC_YXC_SYS_BASE_FILE_LOGGER_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_TextFile.h>
#include <string>

#define _YXC_LOGGER_BUF_SIZE (1 << 12)

namespace YXC_Inner
{
	class _FileLogger : public _LoggerBase
	{
	public:
		_FileLogger();

		~_FileLogger();

	public:
		virtual YXC_Status LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError);

		virtual YXC_Status Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszMessage, ybool_t bReportLastError);

		virtual void Close();

		virtual YXC_Status CreateA(const YXC_LoggerCreateInfoA* pInfo);

		virtual YXC_Status CreateW(const YXC_LoggerCreateInfoW* pInfo);

        YXC_Status Create(const YXC_LoggerCreateInfo* pInfo);

	private:
		_FileLogger(const _FileLogger& rhs);

		_FileLogger& operator =(const _FileLogger& rhs);

	private:
		YXC_Status _WriteMessage(const ychar* cpszMsg, yuint32_t uCchMsg);

		void _DeleteOldLogs(const ychar* pszDir);

	private:
        YXC_TextFile _txtFile;

		ybool_t _bAppendFileSpec;

		YXX_Crit _crit;

		YXC_FPath _sFilePath;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_FILE_LOGGER_HPP__ */

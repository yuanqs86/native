#define __MODULE__ "EK.Logger.DebugLogger"

#include <stdio.h>

#include <YXC_Sys/Utils/_YXC_DebugLogger.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

namespace YXC_Inner
{
	_DebugLogger::_DebugLogger()
	{

	}

	_DebugLogger::~_DebugLogger()
	{

	}

	YXC_Status _DebugLogger::LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
		const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError)
	{
		ychar szMessageBuffer[_YXC_LOGGER_BUF_SIZE], *pMsgBuf = szMessageBuffer;
		ysize_t stSizeBuf = _YXC_LOGGER_BUF_SIZE;

		YXC_Status rc = _Log_Convert(logLevel, cpszTitle, cpszModule, cpszFormat, vaList, FALSE, TRUE,
			bReportLastError, &pMsgBuf, &stSizeBuf);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Convert log format[%s] failed"), cpszTitle);

		::OutputDebugStringW(pMsgBuf);
		if (pMsgBuf != szMessageBuffer)
		{
			_Log_FreeBuffer(pMsgBuf);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _DebugLogger::Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
		const ychar* cpszMessage, ybool_t bReportLastError)
	{
		ychar szMessageBuffer[_YXC_LOGGER_BUF_SIZE], *pMsgBuf = szMessageBuffer;
		ysize_t stSizeBuf = _YXC_LOGGER_BUF_SIZE;

		YXC_Status rc = _Log_Convert(logLevel, cpszTitle, cpszModule, cpszMessage, FALSE, TRUE,
			bReportLastError, &pMsgBuf, &stSizeBuf);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Convert log format[%s] failed", cpszTitle);

		::OutputDebugStringW(pMsgBuf);
		if (pMsgBuf != szMessageBuffer)
		{
			_Log_FreeBuffer(pMsgBuf);
		}

		return YXC_ERC_SUCCESS;
	}
}

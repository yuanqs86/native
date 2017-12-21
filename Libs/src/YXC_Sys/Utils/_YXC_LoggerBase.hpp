#ifndef __INNER_INC_YXC_SYS_BASE_LOGGER_BASE_HPP__
#define __INNER_INC_YXC_SYS_BASE_LOGGER_BASE_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Logger.h>

#define _YXC_LOGGER_BUF_SIZE (1 << 12)

namespace YXC_Inner
{
	class _LoggerBase
	{
	public:
		_LoggerBase();

		virtual ~_LoggerBase();

	public:
		inline void SetMinLogLevel(YXC_LoggerLogLevel minLogLevel) { this->_minLogLevel = minLogLevel; }

		inline ybool_t CanReport(YXC_LoggerLogLevel logLevel) { return (int)logLevel >= (int)this->_minLogLevel; }

	public:
		virtual YXC_Status CreateA(const YXC_LoggerCreateInfoA* pInfo);

		virtual YXC_Status CreateW(const YXC_LoggerCreateInfoW* pInfo);

		virtual void Close();

		virtual YXC_Status LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError) = 0;

		virtual YXC_Status Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle, const ychar* cpszMessage,
            ybool_t bReportLastError) = 0;

	public:
		static YXC_Status CreateLoggerA(YXC_LoggerType loggerType, const YXC_LoggerCreateInfoA* pInfo, _LoggerBase** ppLogger);

		static YXC_Status CreateLoggerW(YXC_LoggerType loggerType, const YXC_LoggerCreateInfoW* pInfo, _LoggerBase** ppLogger);

		static void CloseLogger(_LoggerBase* pLogger);

	private:
		_LoggerBase(const _LoggerBase& rhs);

		_LoggerBase& operator =(const _LoggerBase& rhs);

		YXC_LoggerLogLevel _minLogLevel;
	};

	YXC_Status _Log_Convert(YXC_LoggerLogLevel logLevel, const ychar* pszTitle, const ychar* cpszModule,
		const ychar* cpszFormat, va_list vaList, ybool_t bAppendTime, ybool_t bAppendFileName,
		ybool_t bReportError, ychar** ppOutBuf, ysize_t* pstOutBufLen);

	YXC_Status _Log_Convert(YXC_LoggerLogLevel logLevel, const ychar* pszTitle, const ychar* cpszModule,
		const ychar* cpszMessage, ybool_t bAppendTime, ybool_t bAppendFileName,
		ybool_t bReportError, ychar** ppOutBuf, ysize_t* pstCchBuf);

#if YCHAR_WCHAR_T
	YXC_Status _Msg_Convert(const char* cpszFormat, va_list vaList, ychar** ppOutBuf, ysize_t* pstCchBuf);

	YXC_Status _Msg_Convert(const char* cpszMessage, ychar** ppOutBuf, ysize_t* pstCchBuf);

#else
	YXC_Status _Msg_Convert(const wchar_t* cpszFormat, va_list vaList, ychar** ppOutBuf, ysize_t* pstCchBuf);

	YXC_Status _Msg_Convert(const wchar_t* cpszMessage, ychar** ppOutBuf, ysize_t* pstCchBuf);

#endif /* YCHAR_WCHAR_T */

	void _Log_FreeBuffer(ychar* pszBuffer);

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_Logger, _LoggerBase, _LoggerPtr, _LoggerHdl);

	extern _LoggerBase* gs_kernelLogger;

	class _DummyLogger : public _LoggerBase
	{
	public:
		_DummyLogger();

		~_DummyLogger();

	public:
		virtual YXC_Status LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError);

		virtual YXC_Status Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
			const ychar* cpszMessage, ybool_t bReportLastError);

	private:
		_DummyLogger(const _DummyLogger& rhs);

		_DummyLogger& operator =(const _DummyLogger& rhs);

	private:
	};
}

#if YXC_PLATFORM_WIN

#define _YXC_KREPORT_ERR(_msg, ...)													\
	do {																			\
		if (YXC_Inner::gs_kernelLogger != NULL) {									\
			YXC_LoggerError((YXC_Logger)YXC_Inner::gs_kernelLogger, __EMODULE__,		\
				NULL, _msg, ##__VA_ARGS__);											\
		}																			\
	} while (0)

#define _YXC_KREPORT_INFO(_msg, ...)													\
	do {																			\
		if (YXC_Inner::gs_kernelLogger != NULL) {									\
			YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_INFO,	\
				__EMODULE__, NULL, _msg, ##__VA_ARGS__);								\
		}																			\
	} while (0)

#define _YXC_KREPORT_VERBOSE(_msg, ...)													\
	do {																				\
		if (YXC_Inner::gs_kernelLogger != NULL) {										\
			YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_VERBOSE,    \
				__EMODULE__, NULL, _msg, ##__VA_ARGS__);									\
		}																				\
	} while (0)

#define _YXC_KREPORT_THREAD_ERR(_msg, ...)											\
	do {																			\
		if (YXC_Inner::gs_kernelLogger != NULL) {									\
			YXC_LoggerError((YXC_Logger)YXC_Inner::gs_kernelLogger, __EMODULE__, NULL, \
                YC("[%d]" _msg), YXCLib::OSGetCurrentThreadId(), ##__VA_ARGS__);       \
		}																			\
	} while (0)

#define _YXC_KREPORT_THREAD_INFO(_msg, ...)											\
	do {																			\
		if (YXC_Inner::gs_kernelLogger != NULL) {									\
			YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_INFO,   \
				__EMODULE__, NULL, YC("[%d]" _msg), YXCLib::OSGetCurrentThreadId(),	\
				##__VA_ARGS__);														\
		}																			\
	} while (0)

#define _YXC_KREPORT_THREAD_VERBOSE(_msg, ...)											\
	do {																				\
		if (YXC_Inner::gs_kernelLogger != NULL) {										\
			YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_VERBOSE,	\
				__EMODULE__, NULL, YC("[%d]") _msg, YXCLib::OSGetCurrentThreadId(),		\
				##__VA_ARGS__);															\
		}																				\
	} while (0)

#else

#define _YXC_KREPORT_ERR(_msg, args...)								\
    do {															\
        if (YXC_Inner::gs_kernelLogger != NULL) {					\
            YXC_LoggerError((YXC_Logger)YXC_Inner::gs_kernelLogger,	\
                __EMODULE__, NULL, _msg, ##args);					\
        }															\
    } while (0)

#define _YXC_KREPORT_INFO(_msg, args...)												\
    do {																			\
        if (YXC_Inner::gs_kernelLogger != NULL) {									\
            YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_INFO,	\
                __EMODULE__, NULL, _msg, ##args);									\
        }																			\
    } while (0)

#define _YXC_KREPORT_VERBOSE(_msg, args...)													\
    do {																					\
        if (YXC_Inner::gs_kernelLogger != NULL) {											\
            YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_VERBOSE,		\
                __EMODULE__, NULL, _msg, ##args);											\
        }																					\
    } while (0)

#define _YXC_KREPORT_THREAD_ERR(_msg, args...)											\
    do {																				\
        if (YXC_Inner::gs_kernelLogger != NULL) {										\
            YXC_LoggerError((YXC_Logger)YXC_Inner::gs_kernelLogger, __EMODULE__,	NULL,   \
                YC("[%d]" _msg), YXCLib::OSGetCurrentThreadId(), ##args);				\
        }																				\
    } while (0)

#define _YXC_KREPORT_THREAD_INFO(_msg, args...)														\
    do {																							\
        if (YXC_Inner::gs_kernelLogger != NULL) {													\
            YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_INFO, __EMODULE__,		\
                NULL, YC("[%d]" _msg), YXCLib::OSGetCurrentThreadId(), ##args);						\
        }																							\
    } while (0)

#define _YXC_KREPORT_THREAD_VERBOSE(_msg, args...)													\
    do {																							\
        if (YXC_Inner::gs_kernelLogger != NULL) {													\
            YXC_LoggerLog((YXC_Logger)YXC_Inner::gs_kernelLogger, YXC_LOGGER_LL_VERBOSE, __EMODULE__,   \
                NULL, YC("[%d]" _msg), YXCLib::OSGetCurrentThreadId(), ##args);						\
        }																							\
    } while (0)


#endif /* YXC_PLATFORM_WIN */

#endif /* __INNER_INC_YXC_SYS_BASE_LOGGER_BASE_HPP__ */

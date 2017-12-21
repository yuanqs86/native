/** \file YXC_Sys.h
 * Core foundation file of YXC system.
 */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_CORE_H__
#define __INC_YXC_SYS_BASE_CORE_H__

#include <YXC_Sys/YXC_SysBase.h>
#include <YXC_Sys/YXC_Types.h>
#include <YXC_Sys/YXC_Platform.h>
#include <YXC_Sys/YXC_ErrorCode.h>
#include <YXC_Sys/YXC_SysCExtend.h>

#define YXC_BASE_ERROR_BUFFER 4096 /* Normal error buffer size */

#ifdef __cplusplus

#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

extern "C"
{
#endif /* __cplusplus */

	/**
	 * \brief An error enumeration witch identifies the error comes from.
	 */
	typedef enum
	{
		YXC_ERROR_SRC_OS, /**< The error comes from operating system. */
		YXC_ERROR_SRC_C_RUNTIME, /**< The error comes from c runtime library. */
		YXC_ERROR_SRC_YXC, /**< The error comes from an EJ module library. */
		YXC_ERROR_SRC_YXC_EXT, /**< The error comes from an EJ external library. */
		YXC_ERROR_SRC_APPLICATION,  /**< The error comes from your application. */
		YXC_ERROR_SRC_3RD_PARTY, /**< The error comes from a 3rd party component. */
		YXC_ERROR_SRC_SQL, /**< The error comes from a sql database operation. */
	}YXC_ErrorSrc;

	/**
	 * \brief A buffer structure with its buffer size and pointer.
	 */
	typedef struct __YXC_BUFFER
	{
		/**
		 * The buffer size of the buffer.
		 */
		ysize_t stBufSize;

		/**
		 * The pointer of the buffer.
		 */
		void* pBuffer;
	}YXC_Buffer;

	/**
	 * \brief A buffer structure with its buffer size and pointer.
	 */
	typedef struct __YXC_BINARY_DATA
	{
		/**
		 * The buffer size of the data.
		 */
		ysize_t stCbBuf;

		/**
		 * The data length of the data.
		 */
		ysize_t stCbData;

		/**
		 * The pointer of the data.
		 */
		void* pBuffer;
	}YXC_BinaryData;

	/**
	 * \brief The error information structure.
	 */
	typedef struct __YXC_ErrorInfo
	{
		YXC_ErrorSrc errSrc; /**< The source of the error. */
		yuint32_t uCategory; /**< The sub category of the error. */
		long long llErrCode; /**< The code of the error. */
		ychar* wszModule; /**< The module in which the error occurred. */
		ychar* wszMsg; /**< The message of the error */
		ychar* wszFunction; /**< The function in which the error occurred. */
		ychar* wszFile; /**< The source file name of the error. */
		int iLine; /**< The source file line number of the error. */

		ysize_t stAdditionalBufSize; /**< The additional buffer size of the error. */
		void* pvAdditional; /**< The additional buffer of the error. */

		const struct __YXC_ErrorInfo* pInnerError; /**< The inner error of the error. */
	}YXC_Error;

	/**
	 * \brief Report an YXC system error to your thread error buffer.
	 * \param[in] bClearPrevErrors Put TRUE here to clear all of the previous errors, otherwise, FALSE.
	 * \param[in] esError          The error code of YXC system to report.
	 * \param[in] cpszModule       Specify the module of the error. Can be NULL.
	 * \param[in] cpszFile         Specify the file of the error. Can be NULL.
	 * \param[in] iLine            Specify the line of the error. If you don't care this, use 0.
	 * \param[in] cpszFunc         Specify the function of the error. Can be NULL.
	 * \param[in] cpszMsgFormat    A c-style format string here. Can be NULL.
	 * \see YXC_ReportError
	 * \see YXC_ReportErrorExFormat
	 * \see YXC_GetLastError
	 * \see YXC_GetLastErrorMessage
	 * \return ::YXC_ERC_SUCCESS for successful, otherwise, failure.
	 * \attention Please never call YXC_GetLastError() or YXC_GetLastErrorMessage() after calling this function, this function won't yield any error messages.
	 */
	YXC_API(YXC_Status) YXC_ReportErrorFormat(
		ybool_t bClearPrevErrors,
		YXC_Status esError,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		const ychar* cpszMsgFormat,
		...
	);

	/**
	 * \brief Report an YXC system error(no format) to your thread error buffer.
	 * \param[in] bClearPrevErrors Put TRUE here to clear all of the previous errors, otherwise, FALSE.
	 * \param[in] esError          The error code of YXC system to report.
	 * \param[in] cpszModule       Specify the module of the error. Can be NULL.
	 * \param[in] cpszFile         Specify the file of the error. Can be NULL.
	 * \param[in] iLine            Specify the line of the error. If you don't care this, use 0.
	 * \param[in] cpszFunc         Specify the function of the error. Can be NULL.
	 * \param[in] cpszMsg          A c-style string here. Can be NULL.
	 * \see YXC_ReportErrorExFormat
	 * \see YXC_ReportErrorFormat
	 * \see YXC_GetLastError
	 * \see YXC_GetLastErrorMessage
	 * \return ::YXC_ERC_SUCCESS for successful, otherwise, failure.
	 * \attention Please never call YXC_GetLastError() or YXC_GetLastErrorMessage() after calling this function, this function won't yield any error messages.
	 */
	YXC_API(YXC_Status) YXC_ReportError(
		ybool_t bClearPrevErrors,
		YXC_Status esError,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		const ychar* cpszMsg
	);

	/**
	 * \brief Report an error of any source to your thread error buffer.
	 * \param[in] bClearPrevErrors      Put TRUE here to clear all of the previous errors, otherwise, FALSE.
	 * \param[in] errSrc                Specify the source of the error.
	 * \param[in] uErrCategory          Specify the sub category of the error.
	 * \param[in] llErrCode             The error code value.
	 * \param[in] cpszModule            Specify the module of the error. Can be NULL.
	 * \param[in] cpszFile              Specify the file of the error. Can be NULL.
	 * \param[in] iLine                 Specify the line of the error. If you don't care this, use 0.
	 * \param[in] cpszFunc              Specify the function of the error. Can be NULL.
	 * \param[in] stAdditionalBufSize   Specify the additional buffer size of this error.
	 * \param[in] pvAdditional          Specify the additional pointer of this error. Can be NULL.
	 * \param[in] cpszMsgFormat         A c-style format string here. Can be NULL.
	 * \see YXC_ReportErrorFormat
	 * \see YXC_ReportErrorEx
	 * \see YXC_GetLastError
	 * \see YXC_GetLastErrorMessage
	 * \return An YXC system code, ::YXC_ERC_SUCCESS for success. You can just ignore this return code.
	 * \attention Please never call YXC_GetLastError() or YXC_GetLastErrorMessage() after calling this function, this function won't yield any error messages.
	 */
	YXC_API(YXC_Status) YXC_ReportErrorExFormat(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uErrCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsgFormat,
		...
	);

	/**
	 * \brief Report an error of any source to your thread error buffer.
	 * \param[in] bClearPrevErrors      Put TRUE here to clear all of the previous errors, otherwise, FALSE.
	 * \param[in] errSrc                Specify the source of the error.
	 * \param[in] uErrCategory          Specify the sub category of the error.
	 * \param[in] llErrCode             The error code value.
	 * \param[in] cpszModule            Specify the module of the error. Can be NULL.
	 * \param[in] cpszFile              Specify the file of the error. Can be NULL.
	 * \param[in] iLine                 Specify the line of the error. If you don't care this, use 0.
	 * \param[in] cpszFunc              Specify the function of the error. Can be NULL.
	 * \param[in] stAdditionalBufSize   Specify the additional buffer size of this error.
	 * \param[in] pvAdditional          Specify the additional pointer of this error. Can be NULL.
	 * \param[in] cpszMsgFormat         A c-style format string here. Can be NULL.
	 * \param[in] vaList				Argument list. Can be NULL.
	 * \see YXC_ReportErrorFormat
	 * \see YXC_ReportErrorExFormat
	 * \see YXC_ReportErrorEx
	 * \see YXC_GetLastError
	 * \see YXC_GetLastErrorMessage
	 * \return An YXC system code, ::YXC_ERC_SUCCESS for success. You can just ignore this return code.
	 * \attention Please never call YXC_GetLastError() or YXC_GetLastErrorMessage() after calling this function, this function won't yield any error messages.
	 */
	YXC_API(YXC_Status) YXC_ReportErrorExFormat_V(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uErrCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsgFormat,
		va_list vaList
	);

	/**
	 * \brief Report an error of any source without format to your thread error buffer.
	 * \param[in] bClearPrevErrors      Put TRUE here to clear all of the previous errors, otherwise, FALSE.
	 * \param[in] errSrc                Specify the source of the error.
	 * \param[in] uErrCategory          Specify the sub category of the error.
	 * \param[in] llErrCode             The error code value.
	 * \param[in] cpszModule            Specify the module of the error. Can be NULL.
	 * \param[in] cpszFile              Specify the file of the error. Can be NULL.
	 * \param[in] iLine                 Specify the line of the error. If you don't care this, use 0.
	 * \param[in] cpszFunc              Specify the function of the error. Can be NULL.
	 * \param[in] stAdditionalBufSize   Specify the additional buffer size of this error.
	 * \param[in] pvAdditional          Specify the additional pointer of this error. Can be NULL.
	 * \param[in] cpszMsg               A c-style string here. Can be NULL.
	 * \see YXC_ReportError
	 * \see YXC_ReportErrorExFormat
	 * \see YXC_GetLastError
	 * \see YXC_GetLastErrorMessage
	 * \return An YXC system code, ::YXC_ERC_SUCCESS for success. You can just ignore this return code.
	 * \attention Please never call YXC_GetLastError() or YXC_GetLastErrorMessage() after calling this function, this function won't yield any error messages.
	 */
	YXC_API(YXC_Status) YXC_ReportErrorEx(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uErrCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsg
	);

	/**
	 * \brief all the errors in your thread buffer.
	 * \return An YXC system code, 0 for success. You can just ignore this return code.
	 * \see YXC_ReportErrorExFormat
	 * \see YXC_ReportErrorFormat
	 * \attention Please never call YXC_GetLastError() or YXC_GetLastErrorMessage() after calling this function, this function won't yield any error messages.
	 */
	YXC_API(YXC_Status) YXC_ClearErrors();

	/**
	 * \brief Get the thread error pointer to use.
	 * \return A pointer to ::YXC_Error structure, NULL for no error.
	 * \see YXC_GetLastErrorMessage
	 */
	YXC_API(const YXC_Error*) YXC_GetLastError();

	/**
	 * \brief Get the thread innermost error pointer to use.
	 * \return A pointer to ::YXC_Error structure, NULL for no error.
	 * \see YXC_GetLastErrorMessage
	 * \see YXC_GetLastError
	 */
	YXC_API(const YXC_Error*) YXC_GetInnermostError();

	/**
	 * \brief Get the os last error code to use.
	 * \return A os error code. 0 for no os error.
	 * \see YXC_GetLastErrorMessage
	 * \see YXC_GetLastError
	 */
	YXC_API(yint64_t) YXC_GetLastOSError();

	/**
	 * \brief Get the thread error message to use.
	 * \return A pointer to a null-terminated string, this pointer will never be NULL.
	 * \see YXC_GetLastError
	 */
	YXC_API(const ychar*) YXC_GetLastErrorMessage();

	/**
	 * \brief Copy the thread error pointer to a new buffer.
	 * \param[in]  stBuffer     Input buffer size for buffer copy.
	 * \param[in]  pBuffer      The input buffer pointer.
	 * \param[out] pstNeeded    Buffer size needed to hold the ::YXC_Error message, can be NULL if you don't care this parameter.
	 * \return A pointer to ::YXC_Error structure, NULL for copy failed or no error. If there is no error in system error buffer,
	 *     in this case, this function will fill the pstNeeded parameter(if not NULL) by zero.
	 * \see YXC_GetLastError
	 */
	YXC_API(const YXC_Error*) YXC_CopyLastError(ysize_t stBuffer, ybyte_t* pBuffer, ysize_t* pstNeeded);

	YXC_API(void) YXC_FatalAssert(const ychar* szMessage, ...);

	YXC_API(void) YXC_FatalAssertSys(const ychar* szMessage, ...);

	YXC_API(void) YXC_FatalAssert_V(const ychar* szMessage, va_list vaList);

	YXC_API(void) YXC_FatalAssertSys_V(const ychar* szMessage, va_list vaList);

	YXC_API(ysize_t) YXC_FormatError(const YXC_Error* errorInfo, ychar* buffer, ysize_t ccBuffer);

#if YXC_PLATFORM_WIN
	YXC_API(void) YXC_ReportException(PEXCEPTION_POINTERS pExceptPtrs, const wchar_t* lpszMessage);

	YXC_API(int) YXC_NewHandler(size_t allocSize);

	YXC_API(ybool_t) YXC_EnableCrashHook();

	typedef LONG (*YXC_SysExceptionHandler)(PEXCEPTION_POINTERS pExceptPtrs);

	YXC_API(ybool_t) YXC_EnableCrashHookWithCallback(YXC_SysExceptionHandler excpHandler);

	YXC_API(void) YXC_DisableCrashHook();

	YXC_API(void) YXC_DisableVectoredException();

	YXC_API(void) YXC_RestoreVectoredException();

	YXC_API(void) YXC_SysHandleException(PEXCEPTION_POINTERS pExceptPtrs);

#endif /* YXC_PLATFORM_WIN32 */

	/**
	 * \brief Get the string length of a basic ansi-string.
	 * \param[in] bStr     An input ansi-string.
	 * \return The length, in characters, without the null-terminating character of the parameter \b bStr.
	 * \see YXC_GetLastError
	 */
	YXC_INLINE ysize_t YXC_BStringLenA(YXC_BStringA bStr)
	{
		return bStr.stCchStr == YXC_STR_NTS ? strlen(bStr.pStr) : bStr.stCchStr;
	}

	/**
	 * \brief Get the string length of a basic unicode-string.
	 * \param[in] bStr     An input unicode-string.
	 * \return The length, in characters, without the null-terminating character of the parameter \b bStr.
	 * \see YXC_GetLastError
	 */
	YXC_INLINE ysize_t YXC_BStringLenW(YXC_BStringW bStr)
	{
		return bStr.stCchStr == YXC_STR_NTS ? wcslen(bStr.pStr) : bStr.stCchStr;
	}

	/**
	 * \brief Get the string length of a basic unicode-string.
	 * \param[in] bStr     An input unicode-string.
	 * \return The length, in characters, without the null-terminating character of the parameter \b bStr.
	 * \see YXC_GetLastError
	 */
	YXC_INLINE ysize_t YXC_BStringLen(YXC_BString bStr)
	{
		return bStr.stCchStr == YXC_STR_NTS ? yh_strlen(bStr.pStr) : bStr.stCchStr;
	}

	YXC_INLINE YXC_BBStringA YXC_InitBBStringBufA(char* pszBuf, yuint32_t uCchBuf)
	{
		YXC_BBStringA bsRet = { pszBuf, 0, uCchBuf };
		return bsRet;
	}

	YXC_INLINE YXC_BBStringW YXC_InitBBStringBufW(wchar_t* pszBuf, yuint32_t uCchBuf)
	{
		YXC_BBStringW bsRet = { pszBuf, 0, uCchBuf };
		return bsRet;
	}

	YXC_INLINE YXC_BBString YXC_InitBBStringBuf(ychar* pszBuf, yuint32_t uCchBuf)
	{
		YXC_BBString bsRet = { pszBuf, 0, uCchBuf };
		return bsRet;
	}

	YXC_API(void) YXC_GuidToGuidStrA(const YXC_Guid* guid, YXC_GuidStrA guidStr);
	YXC_API(void) YXC_GuidStrToGuidA(const YXC_GuidStrA guidStr, YXC_Guid* guid);
	YXC_API(void) YXC_GuidToGuidStrW(const YXC_Guid* guid, YXC_GuidStrW guidStr);
	YXC_API(void) YXC_GuidStrToGuidW(const YXC_GuidStrW guidStr, YXC_Guid* guid);
	YXC_API(const char*) YXC_GuidStrTLS(const YXC_Guid* guid);

#if YCHAR_WCHAR_T
#define YXC_GuidStrToGuid YXC_GuidStrToGuidW
#define YXC_GuidToGuidStr YXC_GuidToGuidStrW
#else
#define YXC_GuidStrToGuid YXC_GuidStrToGuidA
#define YXC_GuidToGuidStr YXC_GuidToGuidStrA
#endif /* YCHAR_WCHAR_T */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_CORE_H__ */

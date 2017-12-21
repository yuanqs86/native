/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_TYPES_H__
#define __INC_YXC_SYS_BASE_TYPES_H__

#include <YXC_Sys/YXC_Platform.h>
#include <YXC_Sys/YXC_Characters.h>
#include <limits.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef int YXC_Status;

	typedef unsigned char ybool_t;

	typedef char yint8_t;
	typedef unsigned char yuint8_t;
	typedef short yint16_t;
	typedef unsigned short yuint16_t;
	typedef int yint32_t;
	typedef unsigned int yuint32_t;
	typedef long long yint64_t;
	typedef unsigned long long yuint64_t;
	typedef float efloat_t;
	typedef double edouble_t;

	typedef yuint8_t ybyte_t;
	typedef yint8_t ysbyte_t;

#if YXC_IS_64BIT
	typedef long long yintptr_t;
	typedef unsigned long long yuintptr_t;

	typedef long long yssize_t;
	typedef unsigned long long ysize_t;
#else
	typedef int yintptr_t;
	typedef unsigned int yuintptr_t;

	typedef int yssize_t;
	typedef unsigned int ysize_t;
#endif /* YXC_IS_64BIT */

#if YXC_IS_64BIT
#define YXC_SIZE_MAX ULLONG_MAX
#define YXC_SSIZE_MAX LLONG_MAX
#define YXC_SSIZE_MIN LLONG_MIN
#else
#define YXC_SIZE_MAX UINT_MAX
#define YXC_SSIZE_MAX INT_MAX
#define YXC_SSIZE_MIN INT_MIN
#endif /* YXC_IS_64BIT */

	typedef char YXC_GuidStrA[36 + 1];
	typedef wchar_t YXC_GuidStrW[36 + 1];
	typedef struct __YXC_BASIC_BUFFERED_STRINGA
	{
		char* pStr;
		ysize_t stCchStr;
		ysize_t stCchBufStr;
	}YXC_BBStringA;

	typedef struct __YXC_BASIC_BUFFERED_STRINGW
	{
		wchar_t* pStr;
		ysize_t stCchStr;
		ysize_t stCchBufStr;
	}YXC_BBStringW;

	/**
	 * \brief Represents an ansi-string. Its length is determined by member \b stCchStr.
	 */
	typedef struct __YXC_BASIC_STRINGA
	{
		char* pStr; /**< The pointer of this string. */
		yssize_t stCchStr; /**< The length, in characters, of this string. To make a null-terminated string, set this member to ::YXC_STR_NTS. */
	}YXC_BStringA;

	/**
	 * \brief Represents an unicode-string. Its length is determined by member \b stCchStr.
	 */
	typedef struct __YXC_BASIC_STRINGW
	{
		wchar_t* pStr; /**< The pointer of this string. */
		yssize_t stCchStr; /**< The length, in characters, of this string. To make a null-terminated string, set this member to ::YXC_STR_NTS. */
	}YXC_BStringW;

	typedef struct __YXC_SINGLE_LIST_NODE
	{
		struct __YXC_SINGLE_LIST_NODE* pNext;
	}YXC_SLNode;

	/**
	 * \brief Represents date information and time information.
	 */
	typedef struct __YXC_DATE_TIME
	{
		yint16_t uYears; /**< The year of time, minus number for BC. times. */
		yuint16_t uMonths; /**< The month of time. */
		yuint16_t uDays; /**< The day of time. */
		yuint16_t uHours; /**< The hour of the time. */
		yuint16_t uMinutes; /**< The minutes of the time. */
		yuint16_t uSeconds; /**< The seconds of the time. */
		yuint32_t uFraction; /**< The fractions of the time. */
	}YXC_DateTime;

	/**
	 * \brief Represents date information.
	 */
	typedef struct __YXC_DATE
	{
		yint16_t uYears; /**< The year of time, minus number for BC. times. */
		yuint16_t uMonths; /**< The month of time. */
		yuint16_t uDays; /**< The day of time. */
	}YXC_Date;

	/**
	 * \brief Represents time information.
	 */
	typedef struct __YXC_TIME
	{
		yuint16_t uHours; /**< The hour of the time. */
		yuint16_t uMinutes; /**< The minutes of the time. */
		yuint16_t uSeconds; /**< The seconds of the time. */
		yuint32_t uFraction; /**< The fractions of the time. */
	}YXC_Time;

#if YCHAR_WCHAR_T
    typedef YXC_BStringW YXC_BString;
	typedef YXC_BBStringW YXC_BBString;
	typedef YXC_GuidStrW YXC_GuidStr;

#else
    typedef YXC_BStringA YXC_BString;
	typedef YXC_BBStringA YXC_BBString;
	typedef YXC_GuidStrA YXC_GuidStr;
#endif /* YCHAR_WCHAR_T */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_TYPES_H__ */

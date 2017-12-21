/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_STRING_UTILITIES_HPP__
#define __INC_YXC_SYS_BASE_STRING_UTILITIES_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <string>
#pragma warning (push)
#pragma warning (disable: 4996)

namespace YXCLib
{
	YXC_API(ybool_t) _AppendStringChkW(wchar_t*& pszBuf, wchar_t* pszEnd, const wchar_t* szStr, yint32_t iStrLen);

	YXC_API(ybool_t) _AppendStringChkA(char*& pszBuf, char* pszEnd, const char* szStr, yint32_t iStrLen);

    YXC_INLINE ybool_t _AppendStringChk(ychar*& pszBuf, ychar* pszEnd, const ychar* szStr, yint32_t iStrLen)
    {
#if YCHAR_WCHAR_T
        return _AppendStringChkW(pszBuf, pszEnd, szStr, iStrLen);
#else
        return _AppendStringChkA(pszBuf, pszEnd, szStr, iStrLen);
#endif /* YCHAR_WCHAR_T */
    }

	template <typename Ch>
	struct _ChTraits
	{
		typedef std::string _CStrType;

		static const char COMMA = ',';
		static const char SHARP = '#';
		static const char DOLLAR = '$';
		static const char COLON = ':';
		static const char SEMICOLON = ';';
		static const char OPENING_BRACKET = '[';
		static const char CLOSING_BRACKET = ']';
		static const char DOUBLE_QUOTE = '\"';
		static const char EQUALSIGN = '=';
		static const char ENTER = '\r';
		static const char RETURN = '\n';
		static const char SPACE = ' ';
		static const char TAB = '\t';

#if YXC_PLATFORM_WIN
		static const int CC_LINE_END = 2;
#else
		static const int CC_LINE_END = 1;
#endif /* YXC_PLATFORM_WIN */
		static const char* LINE_END;

		static inline char* strchr(char* pStr, char ch)
		{
			return ::strchr(pStr, ch);
		}

		static inline const char* strchr(const char* pStr, char ch)
		{
			return ::strchr(pStr, ch);
		}

		static inline int stricmp(const char* pStr1, const char* pStr2)
		{
#if YXC_PLATFORM_WIN
			return ::stricmp(pStr1, pStr2);
#elif YXC_PLATFORM_UNIX
            return ::strcasecmp(pStr1, pStr2);
#endif /* YXC_PLATFORM_WIN */
		}

		static inline size_t strlen(const char* pStr)
		{
			return ::strlen(pStr);
		}

		static inline char* strncpy(char* dst, const char* src, size_t count)
		{
			return ::strncpy(dst, src, count);
		}
	};

	template <>
	struct _ChTraits<wchar_t>
	{
		typedef std::wstring _CStrType;

		static const wchar_t COMMA = L',';
		static const wchar_t SHARP = L'#';
		static const wchar_t DOLLAR = L'$';
		static const wchar_t COLON = L':';
		static const wchar_t SEMICOLON = L';';
		static const wchar_t OPENING_BRACKET = L'[';
		static const wchar_t CLOSING_BRACKET = L']';
		static const wchar_t DOUBLE_QUOTE = L'\"';
		static const wchar_t EQUALSIGN = L'=';
		static const wchar_t ENTER = L'\r';
		static const wchar_t RETURN = L'\n';
		static const wchar_t SPACE = L' ';
		static const wchar_t TAB = L'\t';

#if YXC_PLATFORM_WIN
		static const int CC_LINE_END = 2;
#else
		static const int CC_LINE_END = 1;
#endif /* YXC_PLATFORM_WIN */
		static const wchar_t* LINE_END;

		static inline wchar_t* strchr(wchar_t* pCh, wchar_t ch)
		{
			return ::wcschr(pCh, ch);
		}

		static inline const wchar_t* strchr(const wchar_t* pCh, wchar_t ch)
		{
			return ::wcschr(pCh, ch);
		}

		static inline int stricmp(const wchar_t* pStr1, const wchar_t* pStr2)
		{
#if YXC_PLATFORM_WIN
#if !defined(__BCOPT__)
			return ::wcsicmp(pStr1, pStr2);
#else
			return ::wcscmp(pStr1, pStr2);
#endif /* __BCOPT__ */

#elif YXC_PLATFORM_UNIX
#if YXC_PLATFORM_ANDROID
			return ::wcscmp(pStr1, pStr2);
#else
            return ::wcscasecmp(pStr1, pStr2);
#endif /* YXC_PLATFORM_ANDROID */
#endif /* YXC_PLATFORM_WIN */
		}

		static inline size_t strlen(const wchar_t* pStr)
		{
			return ::wcslen(pStr);
		}

		static inline wchar_t* strncpy(wchar_t* dst, const wchar_t* src, size_t count)
		{
			return ::wcsncpy(dst, src, count);
		}

		static inline int fputc(wchar_t ch, FILE* fp)
		{
			return ::fputwc(ch, fp);
		}

		static inline int fputs(const wchar_t* str, FILE* fp)
		{
			return ::fputws(str, fp);
		}

		static inline wchar_t* fgets(wchar_t* str, int count, FILE* fp)
		{
			return ::fgetws(str, count, fp);
		}
	};

	template <typename Ch>
	struct NString
	{
		ybool_t isNull;
		typename _ChTraits<Ch>::_CStrType val;
	};

	template <typename Ch>
	static inline NString<Ch> CreateNString(const Ch* pszVal)
	{
		NString<Ch> ret;
		if (pszVal != NULL)
		{
			ret.val = pszVal;
			ret.isNull = FALSE;
		}
		else
		{
			ret.isNull = TRUE;
		}
		return ret;
	}

	template <typename Ch>
	static inline NString<Ch> CreateNString(ybool_t hasStr, const typename _ChTraits<Ch>::_CStrType& strVal)
	{
		NString<Ch> ret = { hasStr, strVal };
		return ret;
	}

	template <typename C>
	ybool_t _AppendStringChkT(C*& pszBuf, C* pszEnd, const C* szStr, yint32_t iStrLen)
	{
		ybool_t bFullAppend = FALSE;
		if (iStrLen == YXC_STR_NTS)
		{
			for (; pszBuf < pszEnd; ++pszBuf)
			{
				C wch = *szStr++;
				if ((*pszBuf = wch) == 0)
				{
					bFullAppend = TRUE;
					break;
				}
			}

			if (!bFullAppend)
			{
				*(pszBuf - 1) = 0; // Fill with terminate character.
			}
			return bFullAppend;
		}
		else
		{
			if (pszBuf < pszEnd)
			{
				yuint32_t uCchCpy = std::min<yuint32_t>((yuint32_t)(pszEnd - pszBuf - 1), iStrLen);
				memcpy(pszBuf, szStr, uCchCpy * sizeof(C));
				pszBuf[uCchCpy] = 0;

				pszBuf += uCchCpy;

				return iStrLen == uCchCpy;
			}
			return FALSE;
		}
	}

}
#pragma warning (pop)

#endif /* __INC_YXC_SYS_BASE_STRING_UTILITIES_HPP__ */

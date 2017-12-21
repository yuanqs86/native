/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_TEXT_ENCODING_H__
#define __INC_YXC_SYS_BASE_TEXT_ENCODING_H__

#include <YXC_Sys/YXC_Sys.h>

extern "C"
{
	typedef enum __YXC_TEXT_ENCODING
	{
		YXC_TENCODING_DEFAULT = 0, /* Default encoding, using same encoding of source file or ANSI. */
		YXC_TENCODING_ANSI, /* ANSI encoding. */
		YXC_TENCODING_UTF16_LE, /* UTF-16 little endian. */
		YXC_TENCODING_UTF16_BE, /* UTF-16 big endian. */
		YXC_TENCODING_UTF32_LE, /* UTF-16 little endian. */
		YXC_TENCODING_UTF32_BE, /* UTF-16 big endian. */
		YXC_TENCODING_UTF8, /* UTF8 encoding. */
		YXC_TENCODING_UTF8_WITH_BOM, /* UTF8 encoding with BOM. */
        YXC_TENCODING_CHAR, /* Default char encoding. */
        YXC_TENCODING_WCHAR, /* Default wchar_t encoding. */

#if YCHAR_WCHAR_T
		YXC_TENCODING_ECHAR = YXC_TENCODING_WCHAR, /* Default ychar encoding. */
#else
		YXC_TENCODING_ECHAR = YXC_TENCODING_CHAR
#endif /* YCHAR_WCHAR_T */
	}YXC_TEncoding;

	YXC_API(YXC_Status) YXC_TEUTF8ToChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, char* pszDst, yuint32_t uCcDstStr,
		yuint32_t* puConverted, yuint32_t* puCcUsed);

	YXC_API(YXC_Status) YXC_TECharToUTF8(const char* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst, yuint32_t uCcDstStr,
		yuint32_t* puConverted, yuint32_t* puCcUsed);

    YXC_API(YXC_Status) YXC_TEncodingConvert(YXC_TEncoding encSrc, YXC_TEncoding encDst, const void* pszSrc, yint32_t iCcSrcStr,
        void* pszDst, yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puUsed);

    YXC_API(YXC_Status) YXC_TECharToWChar(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst, yuint32_t uCcDstStr,
        yuint32_t* puConverted, yuint32_t* puCcUsed);

    YXC_API(YXC_Status) YXC_TEWCharToChar(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst, yuint32_t uCcDstStr,
        yuint32_t* puConverted, yuint32_t* puCcUsed);

    YXC_API(YXC_Status) YXC_TEUTF8ToWChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst, yuint32_t uCcDstStr,
        yuint32_t* puConverted, yuint32_t* puCcUsed);

	YXC_API(YXC_Status) YXC_TEWCharToUTF8(const wchar_t* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst, yuint32_t uCcDstStr,
        yuint32_t* puConverted, yuint32_t* puCcUsed);

	YXC_API(YXC_Status) YXC_TECharCopy(const char* pszSrc, yint32_t iCcSrcStr, char* pszDst, yuint32_t uCcDstStr,
		yuint32_t* puConverted, yuint32_t* puCcUsed);

	YXC_API(YXC_Status) YXC_TEWCharCopy(const wchar_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst, yuint32_t uCcDstStr,
		yuint32_t* puConverted, yuint32_t* puCcUsed);
};

#if YCHAR_WCHAR_T
#define YXC_TEECharToWChar(s, si, d, di, pc, pu) YXC_TEWCharCopy(s, si, d, di, pc, pu)
#define YXC_TEECharToChar(s, si, d, di, pc, pu) YXC_TEWCharToChar(s, si, d, di, pc, pu)
#define YXC_TEECharToUTF8(s, si, d, di, pc, pu) YXC_TEWCharToUTF8(s, si, d, di, pc, pu)

#define YXC_TEWCharToEChar(s, si, d, di, pc, pu) YXC_TEWCharCopy(s, si, d, di, pc, pu)
#define YXC_TECharToEChar(s, si, d, di, pc, pu) YXC_TECharToWChar(s, si, d, di, pc, pu)
#define YXC_TEUTF8ToEChar(s, si, d, di, pc, pu) YXC_TEUTF8ToWChar(s, si, d, di, pc, pu)
#else
#define YXC_TEECharToWChar(s, si, d, di, pc, pu) YXC_TECharToWChar(s, si, d, di, pc, pu)
#define YXC_TEECharToChar(s, si, d, di, pc, pu) YXC_TECharCopy(s, si, d, di, pc, pu)
#define YXC_TEECharToUTF8(s, si, d, di, pc, pu) YXC_TECharToUTF8(s, si, d, di, pc, pu)

#define YXC_TEWCharToEChar(s, si, d, di, pc, pu) YXC_TEWCharToChar(s, si, d, di, pc, pu)
#define YXC_TECharToEChar(s, si, d, di, pc, pu) YXC_TECharCopy(s, si, d, di, pc, pu)
#define YXC_TEUTF8ToEChar(s, si, d, di, pc, pu) YXC_TEUTF8ToChar(s, si, d, di, pc, pu)
#endif /* YCHAR_WCHAR_T */

#endif /* __INC_YXC_SYS_BASE_TEXT_ENCODING_H__ */

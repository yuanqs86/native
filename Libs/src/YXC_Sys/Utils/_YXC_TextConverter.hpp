#ifndef __INNER_INC_YXC_SYS_BASE_TEXT_CONVERTER_HPP__
#define __INNER_INC_YXC_SYS_BASE_TEXT_CONVERTER_HPP__

#include <YXC_Sys/OS/_YXC_FileBase.hpp>
#include <YXC_Sys/YXC_TextFile.h>
#include <YXC_Sys/YXC_Locker.hpp>

namespace YXC_Inner
{
	typedef void (*_TextConvertProc)(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted);

	typedef void (*_TextConvertLineProc)(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd);

	static const yuint32_t TENCODING_MAX = 6;
    extern YXC_TEncoding gs_TEncWChar; //= YXC_TENCODING_UTF32_BE;
    extern YXC_TEncoding gs_TEncChar; //= YXC_TENCODING_UTF8;
	static inline yuint32_t _TEncodingToIndex(YXC_TEncoding enc)
	{
		switch (enc)
		{
		case YXC_TENCODING_ANSI:
			return 0;
		case YXC_TENCODING_UTF8:
			return 1;
		case YXC_TENCODING_UTF16_LE:
			return 2;
		case YXC_TENCODING_UTF16_BE:
			return 3;
        case YXC_TENCODING_UTF32_LE:
            return 4;
        case YXC_TENCODING_UTF32_BE:
            return 5;
        case YXC_TENCODING_CHAR:
            return _TEncodingToIndex(gs_TEncChar);
        case YXC_TENCODING_WCHAR:
            return _TEncodingToIndex(gs_TEncWChar);
		default:
			return (yuint32_t)-1;
		}
	}

	static inline void _GetTEncodingInfo(YXC_TEncoding enc, yuint32_t* puIndex, yuint32_t* uChBytes)
	{
		switch (enc)
		{
		case YXC_TENCODING_ANSI:
			*uChBytes = sizeof(char);
			*puIndex = 0;
			break;
		case YXC_TENCODING_UTF8:
			*uChBytes = sizeof(char);
			*puIndex = 1;
			break;
		case YXC_TENCODING_UTF16_LE:
			*uChBytes = sizeof(yuint16_t);
			*puIndex = 2;
			break;
		case YXC_TENCODING_UTF16_BE:
			*uChBytes = sizeof(yuint16_t);
			*puIndex = 3;
			break;
        case YXC_TENCODING_UTF32_LE:
            *uChBytes = sizeof(yuint32_t);
            *puIndex = 4;
            break;
        case YXC_TENCODING_UTF32_BE:
            *uChBytes = sizeof(yuint32_t);
            *puIndex = 5;
            break;
        case YXC_TENCODING_CHAR:
            return _GetTEncodingInfo(gs_TEncChar, puIndex, uChBytes);
        case YXC_TENCODING_WCHAR:
            return _GetTEncodingInfo(gs_TEncWChar, puIndex, uChBytes);
        default:
            break;
		}
	}

	typedef _TextConvertProc _TCvtArr[TENCODING_MAX];
	typedef _TextConvertLineProc _TCvtLineArr[TENCODING_MAX];

	//inline YXC_TEncoding _IndexToTEncoding(yuint32_t uIndex)
	//{
	//	switch (enc)
	//	{
	//	case YXC_TENCODING_ANSI:
	//		return 0;
	//	case YXC_TENCODING_UTF8:
	//		return 1;
	//	case YXC_TENCODING_UTF16_LE:
	//		return 2;
	//	case YXC_TENCODING_UTF16_BE:
	//		return 3;
	//	default:
	//		return (yuint32_t)-1;
	//	}
	//}

	class _TextConverter
	{
	public:
		static YXC_Status FillConverterFuncs(YXC_TEncoding encoding, _TCvtArr& pfnCvtReadArr, _TCvtArr& pfnCvtWriteArr,
			_TCvtLineArr& pfnConvertLineArr);

		static ybool_t WCharToChar(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed);

		static ybool_t CharToWChar(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed);

		static ybool_t WCharToUTF8(const wchar_t* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed);

		static ybool_t UTF8ToWChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed);

		static ybool_t UTF8ToChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed);

		static ybool_t CharToUTF8(const char* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed);

		static ybool_t TEncodingConvert(YXC_TEncoding encSrc, YXC_TEncoding encDst, const void* pszSrc, yint32_t iCcSrcStr, void* pszDst,
			yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puUsed);

		static inline ybool_t WCharToChar(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
			yuint32_t uCcDstStr)
		{
			yuint32_t uCcUsed, uCcConverted;
			return WCharToChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uCcUsed, &uCcConverted);
		}

		static inline ybool_t CharToWChar(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
			yuint32_t uCcDstStr)
		{
			yuint32_t uCcUsed, uCcConverted;
			return CharToWChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uCcUsed, &uCcConverted);
		}

		static inline ybool_t WCharToUTF8(const wchar_t* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
			yuint32_t uCcDstStr)
		{
			yuint32_t uCcUsed, uCcConverted;
			return WCharToUTF8(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uCcUsed, &uCcConverted);
		}

		static inline ybool_t UTF8ToWChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
			yuint32_t uCcDstStr)
		{
			yuint32_t uCcUsed, uCcConverted;
			return UTF8ToWChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uCcUsed, &uCcConverted);
		}

		static inline ybool_t UTF8ToChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
			yuint32_t uCcDstStr)
		{
			yuint32_t uCcUsed, uCcConverted;
			return UTF8ToChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uCcUsed, &uCcConverted);
		}

		static inline ybool_t CharToUTF8(const char* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
			yuint32_t uCcDstStr)
		{
			yuint32_t uCcUsed, uCcConverted;
			return CharToUTF8(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uCcUsed, &uCcConverted);
		}

		static int InitConverter();

	private:
		static _TextConvertProc _pfnCvtArr[TENCODING_MAX][TENCODING_MAX];
		static _TextConvertLineProc _pfnCvtLineArr[TENCODING_MAX][TENCODING_MAX];
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_TEXT_CONVERTER_HPP__ */

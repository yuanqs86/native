#define __MODULE__ "EK.Text.Converter"

#include <YXC_Sys/Utils/_YXC_TextConverter.hpp>
#include <YXC_Sys/YXC_StrCvtMBS.hpp>
#include <YXC_Sys/YXC_StrCvtWCS.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <algorithm>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>

using namespace YXCLib;

namespace YXC_Inner
{
#if YXC_PLATFORM_WIN
    YXC_TEncoding gs_TEncChar = YXC_TENCODING_ANSI;
    YXC_TEncoding gs_TEncWChar = YXC_TENCODING_UTF16_LE;
#elif YXC_PLATFORM_APPLE
    YXC_TEncoding gs_TEncChar = YXC_TENCODING_UTF8;
    YXC_TEncoding gs_TEncWChar = YXC_TENCODING_UTF32_BE;
#elif YXC_PLATFORM_UNIX
    YXC_TEncoding gs_TEncChar = YXC_TENCODING_UTF8;
    YXC_TEncoding gs_TEncWChar = YXC_TENCODING_UTF32_LE;
#else
#error "Don't support platform"
#endif /* YXC_PLATFORM_WIN */

	template <yuint16_t u32Type>
	static void _UTF32StrCopy(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_UTF32Utils<u32Type>::CopyString(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted);
	}

	template <yuint16_t u32Type>
	static void _UTF32StrCopyLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		_UTF32Utils<u32Type>::CopyLine(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted, pbLineEnd);
	}

	template <yuint16_t u32Type>
	static void _UTF32StrConvert(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_UTF32Utils<u32Type>::ReverseStringOrder(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
            puCcUsed, puCcConverted);
	}

	template <yuint16_t u32Type>
	static void _UTF32StrConvertLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		_UTF32Utils<u32Type>::ReverseStringLine(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
            puCcUsed, puCcConverted, pbLineEnd);
	}

	template <yuint16_t u32Type, ybool_t bIsUTF8>
	static void _UTF32StrToMultiByte(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_UTF32Utils<u32Type>::template ConvertToMultiByte<bIsUTF8>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted);
	}

	template <yuint16_t u32Type, ybool_t bIsUTF8>
	static void _UTF32StrToMultiByteLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		return _UTF32Utils<u32Type>::template ConvertToMultiByteLine<bIsUTF8>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
            puCcUsed, puCcConverted, pbLineEnd);
	}

	template <yuint16_t u16Type>
	static void _WideStrCopy(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_UTF16Utils<u16Type>::CopyString(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted);
	}

	template <yuint16_t u16Type>
	static void _WideStrCopyLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		_UTF16Utils<u16Type>::CopyLine(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
			puCcUsed, puCcConverted, pbLineEnd);
	}

	template <yuint16_t u16Type>
	static void _WideStrConvert(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_UTF16Utils<u16Type>::ReverseStringOrder(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
			puCcUsed, puCcConverted);
	}

	template <yuint16_t u16Type>
	static void _WideStrConvertLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		_UTF16Utils<u16Type>::ReverseStringLine(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
			puCcUsed, puCcConverted, pbLineEnd);
	}

	template <yuint16_t u16Type, ybool_t bIsUTF8>
	static void _WideStrToMultiByte(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_UTF16Utils<u16Type>::template ConvertToMultiByte<bIsUTF8>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted);
	}

	template <yuint16_t u16Type, ybool_t bIsUTF8>
	static void _WideStrToMultiByteLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		return _UTF16Utils<u16Type>::template ConvertToMultiByteLine<bIsUTF8>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
			puCcUsed, puCcConverted, pbLineEnd);
	}

	template <ybool_t bIsUTF8>
	static void _MultiByteStrCopy(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		_MultiByteUtils<bIsUTF8>::CopyString(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted);
	}

	template <ybool_t bIsUTF8>
	static void _MultiByteStrCopyLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		_MultiByteUtils<bIsUTF8>::CopyLine(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed,
			puCcConverted, pbLineEnd);
	}

	template <ybool_t bIsUTF8>
	static void _MultiByteStrConvert(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		typedef _MultiByteImpl<!bIsUTF8> CvtType;
		_MultiByteUtils<bIsUTF8>::template ConvertT<CvtType>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed, puCcConverted);
	}

	template <ybool_t bIsUTF8>
	static void _MultiByteStrConvertLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		typedef _MultiByteImpl<!bIsUTF8> CvtType;
		_MultiByteUtils<bIsUTF8>::template ConvertTLine<CvtType>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr, puCcUsed,
			puCcConverted, pbLineEnd);
    }

	template <ybool_t bIsUTF8, yuint16_t u16Type>
	static void _MultiByteStrToWide(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		typedef _UTF16Impl2<u16Type> CvtType;
		return _MultiByteUtils<bIsUTF8>::template ConvertT<CvtType>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
			puCcUsed, puCcConverted);
	}

	template <ybool_t bIsUTF8, yuint16_t u16Type>
	static void _MultiByteStrToWideLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
		yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		typedef _UTF16Impl2<u16Type> CvtType;
		_MultiByteUtils<bIsUTF8>::template ConvertTLine<CvtType>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
			puCcUsed, puCcConverted, pbLineEnd);
	}

	template <ybool_t bIsUTF8, yuint16_t u32Type>
	static void _MultiByteStrToUTF32(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
                                    yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
	{
		typedef _UTF32Impl2<u32Type> CvtType;
		return _MultiByteUtils<bIsUTF8>::template ConvertT<CvtType>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
            puCcUsed, puCcConverted);
	}

	template <ybool_t bIsUTF8, yuint16_t u32Type>
	static void _MultiByteStrToUTF32Line(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
                                        yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
	{
		typedef _UTF32Impl2<u32Type> CvtType;
		_MultiByteUtils<bIsUTF8>::template ConvertTLine<CvtType>(pSrcStr, uCcSrcStr, pDstStr, uCcDstStr,
            puCcUsed, puCcConverted, pbLineEnd);
	}

	int _TextConverter::InitConverter()
	{
		yuint32_t uIxAnsi = _TEncodingToIndex(YXC_TENCODING_ANSI);
		yuint32_t uIxUTF8 = _TEncodingToIndex(YXC_TENCODING_UTF8);
		yuint32_t uIxUTF16LE = _TEncodingToIndex(YXC_TENCODING_UTF16_LE);
		yuint32_t uIxUTF16BE = _TEncodingToIndex(YXC_TENCODING_UTF16_BE);
        yuint32_t uIxUTF32LE = _TEncodingToIndex(YXC_TENCODING_UTF32_LE);
		yuint32_t uIxUTF32BE = _TEncodingToIndex(YXC_TENCODING_UTF32_BE);

		if (YXC_Inner::gs_bIsLittleEndianCPU)
		{
			_pfnCvtArr[uIxAnsi][uIxAnsi] = _MultiByteStrCopy<FALSE>;
			_pfnCvtArr[uIxAnsi][uIxUTF8] = _MultiByteStrConvert<FALSE>;
			_pfnCvtArr[uIxAnsi][uIxUTF16LE] = _MultiByteStrToWide<FALSE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxAnsi][uIxUTF16BE] = _MultiByteStrToWide<FALSE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtArr[uIxAnsi][uIxUTF32LE] = _MultiByteStrToUTF32<FALSE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxAnsi][uIxUTF32BE] = _MultiByteStrToUTF32<FALSE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtArr[uIxUTF8][uIxAnsi] = _MultiByteStrConvert<TRUE>;
			_pfnCvtArr[uIxUTF8][uIxUTF8] = _MultiByteStrCopy<TRUE>;
			_pfnCvtArr[uIxUTF8][uIxUTF16LE] = _MultiByteStrToWide<TRUE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxUTF8][uIxUTF16BE] = _MultiByteStrToWide<TRUE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtArr[uIxUTF8][uIxUTF32LE] = _MultiByteStrToUTF32<TRUE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxUTF8][uIxUTF32BE] = _MultiByteStrToUTF32<TRUE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtArr[uIxUTF16LE][uIxAnsi] = _WideStrToMultiByte<_UNI_CONVERT_ULE_LE, FALSE>;
			_pfnCvtArr[uIxUTF16LE][uIxUTF8] = _WideStrToMultiByte<_UNI_CONVERT_ULE_LE, TRUE>;
			_pfnCvtArr[uIxUTF16LE][uIxUTF16LE] = _WideStrCopy<_UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxUTF16LE][uIxUTF16BE] = _WideStrConvert<_UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxUTF16BE][uIxAnsi] = _WideStrToMultiByte<_UNI_CONVERT_UBE_LE, FALSE>;
			_pfnCvtArr[uIxUTF16BE][uIxUTF8] = _WideStrToMultiByte<_UNI_CONVERT_UBE_LE, TRUE>;
			_pfnCvtArr[uIxUTF16BE][uIxUTF16LE] = _WideStrConvert<_UNI_CONVERT_UBE_LE>;
			_pfnCvtArr[uIxUTF16BE][uIxUTF16BE] = _WideStrCopy<_UNI_CONVERT_UBE_LE>;

			_pfnCvtArr[uIxUTF32LE][uIxAnsi] = _UTF32StrToMultiByte<_UNI_CONVERT_ULE_LE, FALSE>;
			_pfnCvtArr[uIxUTF32LE][uIxUTF8] = _UTF32StrToMultiByte<_UNI_CONVERT_ULE_LE, TRUE>;
			_pfnCvtArr[uIxUTF32LE][uIxUTF32LE] = _UTF32StrCopy<_UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxUTF32LE][uIxUTF32BE] = _UTF32StrConvert<_UNI_CONVERT_ULE_LE>;
			_pfnCvtArr[uIxUTF32BE][uIxAnsi] = _UTF32StrToMultiByte<_UNI_CONVERT_UBE_LE, FALSE>;
			_pfnCvtArr[uIxUTF32BE][uIxUTF8] = _UTF32StrToMultiByte<_UNI_CONVERT_UBE_LE, TRUE>;
			_pfnCvtArr[uIxUTF32BE][uIxUTF32LE] = _UTF32StrCopy<_UNI_CONVERT_UBE_LE>;
			_pfnCvtArr[uIxUTF32BE][uIxUTF32BE] = _UTF32StrConvert<_UNI_CONVERT_UBE_LE>;

			_pfnCvtLineArr[uIxAnsi][uIxAnsi] = _MultiByteStrCopyLine<FALSE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF8] = _MultiByteStrConvertLine<FALSE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF16LE] = _MultiByteStrToWideLine<FALSE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF16BE] = _MultiByteStrToWideLine<FALSE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF32LE] = _MultiByteStrToUTF32Line<FALSE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF32BE] = _MultiByteStrToUTF32Line<FALSE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtLineArr[uIxUTF8][uIxAnsi] = _MultiByteStrConvertLine<TRUE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF8] = _MultiByteStrCopyLine<TRUE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF16LE] = _MultiByteStrToWideLine<TRUE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF16BE] = _MultiByteStrToWideLine<TRUE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF32LE] = _MultiByteStrToUTF32Line<TRUE, _UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF32BE] = _MultiByteStrToUTF32Line<TRUE, _UNI_CONVERT_UBE_LE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxAnsi] = _WideStrToMultiByteLine<_UNI_CONVERT_ULE_LE, FALSE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxUTF8] = _WideStrToMultiByteLine<_UNI_CONVERT_ULE_LE, TRUE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxUTF16LE] = _WideStrCopyLine<_UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxUTF16BE] = _WideStrConvertLine<_UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxAnsi] = _WideStrToMultiByteLine<_UNI_CONVERT_UBE_LE, FALSE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxUTF8] = _WideStrToMultiByteLine<_UNI_CONVERT_UBE_LE, TRUE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxUTF16LE] = _WideStrConvertLine<_UNI_CONVERT_UBE_LE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxUTF16BE] = _WideStrCopyLine<_UNI_CONVERT_UBE_LE>;

			_pfnCvtLineArr[uIxUTF32LE][uIxAnsi] = _UTF32StrToMultiByteLine<_UNI_CONVERT_ULE_LE, FALSE>;
			_pfnCvtLineArr[uIxUTF32LE][uIxUTF8] = _UTF32StrToMultiByteLine<_UNI_CONVERT_ULE_LE, TRUE>;
			_pfnCvtLineArr[uIxUTF32LE][uIxUTF32LE] = _UTF32StrCopyLine<_UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxUTF32LE][uIxUTF32BE] = _UTF32StrConvertLine<_UNI_CONVERT_ULE_LE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxAnsi] = _UTF32StrToMultiByteLine<_UNI_CONVERT_UBE_LE, FALSE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxUTF8] = _UTF32StrToMultiByteLine<_UNI_CONVERT_UBE_LE, TRUE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxUTF32LE] = _UTF32StrCopyLine<_UNI_CONVERT_UBE_LE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxUTF32BE] = _UTF32StrConvertLine<_UNI_CONVERT_UBE_LE>;
		}
		else
		{
			_pfnCvtArr[uIxAnsi][uIxAnsi] = _MultiByteStrCopy<FALSE>;
			_pfnCvtArr[uIxAnsi][uIxUTF8] = _MultiByteStrConvert<FALSE>;
			_pfnCvtArr[uIxAnsi][uIxUTF16LE] = _MultiByteStrToWide<FALSE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxAnsi][uIxUTF16BE] = _MultiByteStrToWide<FALSE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtArr[uIxAnsi][uIxUTF32LE] = _MultiByteStrToUTF32<FALSE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxAnsi][uIxUTF32BE] = _MultiByteStrToUTF32<FALSE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtArr[uIxUTF8][uIxAnsi] = _MultiByteStrConvert<TRUE>;
			_pfnCvtArr[uIxUTF8][uIxUTF8] = _MultiByteStrCopy<TRUE>;
			_pfnCvtArr[uIxUTF8][uIxUTF16LE] = _MultiByteStrToWide<TRUE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxUTF8][uIxUTF16BE] = _MultiByteStrToWide<TRUE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtArr[uIxUTF8][uIxUTF32LE] = _MultiByteStrToUTF32<TRUE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxUTF8][uIxUTF32BE] = _MultiByteStrToUTF32<TRUE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtArr[uIxUTF16LE][uIxAnsi] = _WideStrToMultiByte<_UNI_CONVERT_ULE_BE, FALSE>;
			_pfnCvtArr[uIxUTF16LE][uIxUTF8] = _WideStrToMultiByte<_UNI_CONVERT_ULE_BE, TRUE>;
			_pfnCvtArr[uIxUTF16LE][uIxUTF16LE] = _WideStrCopy<_UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxUTF16LE][uIxUTF16BE] = _WideStrConvert<_UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxUTF16BE][uIxAnsi] = _WideStrToMultiByte<_UNI_CONVERT_UBE_BE, FALSE>;
			_pfnCvtArr[uIxUTF16BE][uIxUTF8] = _WideStrToMultiByte<_UNI_CONVERT_UBE_BE, TRUE>;
			_pfnCvtArr[uIxUTF16BE][uIxUTF16LE] = _WideStrConvert<_UNI_CONVERT_UBE_BE>;
			_pfnCvtArr[uIxUTF16BE][uIxUTF16BE] = _WideStrCopy<_UNI_CONVERT_UBE_BE>;

			_pfnCvtArr[uIxUTF32LE][uIxAnsi] = _UTF32StrToMultiByte<_UNI_CONVERT_ULE_BE, FALSE>;
			_pfnCvtArr[uIxUTF32LE][uIxUTF8] = _UTF32StrToMultiByte<_UNI_CONVERT_ULE_BE, TRUE>;
			_pfnCvtArr[uIxUTF32LE][uIxUTF32LE] = _UTF32StrCopy<_UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxUTF32LE][uIxUTF32BE] = _UTF32StrConvert<_UNI_CONVERT_ULE_BE>;
			_pfnCvtArr[uIxUTF32BE][uIxAnsi] = _UTF32StrToMultiByte<_UNI_CONVERT_UBE_BE, FALSE>;
			_pfnCvtArr[uIxUTF32BE][uIxUTF8] = _UTF32StrToMultiByte<_UNI_CONVERT_UBE_BE, TRUE>;
			_pfnCvtArr[uIxUTF32BE][uIxUTF32LE] = _UTF32StrCopy<_UNI_CONVERT_UBE_BE>;
			_pfnCvtArr[uIxUTF32BE][uIxUTF32BE] = _UTF32StrConvert<_UNI_CONVERT_UBE_BE>;

			_pfnCvtLineArr[uIxAnsi][uIxAnsi] = _MultiByteStrCopyLine<FALSE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF8] = _MultiByteStrConvertLine<FALSE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF16LE] = _MultiByteStrToWideLine<FALSE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF16BE] = _MultiByteStrToWideLine<FALSE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF32LE] = _MultiByteStrToUTF32Line<FALSE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxAnsi][uIxUTF32BE] = _MultiByteStrToUTF32Line<FALSE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtLineArr[uIxUTF8][uIxAnsi] = _MultiByteStrConvertLine<TRUE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF8] = _MultiByteStrCopyLine<TRUE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF16LE] = _MultiByteStrToWideLine<TRUE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF16BE] = _MultiByteStrToWideLine<TRUE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF32LE] = _MultiByteStrToUTF32Line<TRUE, _UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxUTF8][uIxUTF32BE] = _MultiByteStrToUTF32Line<TRUE, _UNI_CONVERT_UBE_BE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxAnsi] = _WideStrToMultiByteLine<_UNI_CONVERT_ULE_BE, FALSE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxUTF8] = _WideStrToMultiByteLine<_UNI_CONVERT_ULE_BE, TRUE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxUTF16LE] = _WideStrCopyLine<_UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxUTF16LE][uIxUTF16BE] = _WideStrConvertLine<_UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxAnsi] = _WideStrToMultiByteLine<_UNI_CONVERT_UBE_BE, FALSE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxUTF8] = _WideStrToMultiByteLine<_UNI_CONVERT_UBE_BE, TRUE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxUTF16LE] = _WideStrConvertLine<_UNI_CONVERT_UBE_BE>;
			_pfnCvtLineArr[uIxUTF16BE][uIxUTF16BE] = _WideStrCopyLine<_UNI_CONVERT_UBE_BE>;

			_pfnCvtLineArr[uIxUTF32LE][uIxAnsi] = _UTF32StrToMultiByteLine<_UNI_CONVERT_ULE_BE, FALSE>;
			_pfnCvtLineArr[uIxUTF32LE][uIxUTF8] = _UTF32StrToMultiByteLine<_UNI_CONVERT_ULE_BE, TRUE>;
			_pfnCvtLineArr[uIxUTF32LE][uIxUTF32LE] = _UTF32StrCopyLine<_UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxUTF32LE][uIxUTF32BE] = _UTF32StrConvertLine<_UNI_CONVERT_ULE_BE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxAnsi] = _UTF32StrToMultiByteLine<_UNI_CONVERT_UBE_BE, FALSE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxUTF8] = _UTF32StrToMultiByteLine<_UNI_CONVERT_UBE_BE, TRUE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxUTF32LE] = _UTF32StrCopyLine<_UNI_CONVERT_UBE_BE>;
			_pfnCvtLineArr[uIxUTF32BE][uIxUTF32BE] = _UTF32StrConvertLine<_UNI_CONVERT_UBE_BE>;
		}
		return 0;
	}

	YXC_Status _TextConverter::FillConverterFuncs(YXC_TEncoding encoding, _TCvtArr& pfnCvtReadArr, _TCvtArr& pfnCvtWriteArr,
		_TCvtLineArr& pfnConvertLineArr)
	{
		yuint32_t uIndex = _TEncodingToIndex(encoding);
		_YXC_CHECK_REPORT_NEW_RET(uIndex < TENCODING_MAX, YXC_ERC_INVALID_PARAMETER, YC("Invalid encoding convert type(%d)"),
			encoding);

		for (yuint32_t i = 0; i < TENCODING_MAX; ++i)
		{
			pfnCvtReadArr[i] = _pfnCvtArr[uIndex][i];
			pfnCvtWriteArr[i] = _pfnCvtArr[i][uIndex];
			pfnConvertLineArr[i] = _pfnCvtLineArr[uIndex][i];
		}

		return YXC_ERC_SUCCESS;
	}

	ybool_t _TextConverter::CharToWChar(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uCcUsed, uCcConverted;

		yuint32_t uIxChar = _TEncodingToIndex(gs_TEncChar);
		yuint32_t uIxWChar = _TEncodingToIndex(gs_TEncWChar);

		yuint32_t uCcSrcStr = iCcSrcStr == YXC_STR_NTS ? (yuint32_t)strlen(pszSrc) : (yuint32_t)iCcSrcStr;
		_pfnCvtArr[uIxChar][uIxWChar]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puCcUsed = uCcUsed;
		*puCcConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	ybool_t _TextConverter::WCharToChar(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uCcUsed, uCcConverted;

		yuint32_t uIxWChar = _TEncodingToIndex(gs_TEncWChar);
		yuint32_t uIxChar = _TEncodingToIndex(gs_TEncChar);

		yuint32_t uCcSrcStr = iCcSrcStr == YXC_STR_NTS ? (yuint32_t)wcslen(pszSrc) : (yuint32_t)iCcSrcStr;
		_pfnCvtArr[uIxWChar][uIxChar]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puCcUsed = uCcUsed;
		*puCcConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	ybool_t _TextConverter::CharToUTF8(const char* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uCcUsed, uCcConverted;
		yuint32_t uIxChar = _TEncodingToIndex(gs_TEncChar);
		yuint32_t uIxUTF8 = _TEncodingToIndex(YXC_TENCODING_UTF8);

		yuint32_t uCcSrcStr = iCcSrcStr == YXC_STR_NTS ? (yuint32_t)strlen(pszSrc) : (yuint32_t)iCcSrcStr;
		_pfnCvtArr[uIxChar][uIxUTF8]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puCcUsed = uCcUsed;
		*puCcConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	ybool_t _TextConverter::UTF8ToChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uCcUsed, uCcConverted;
		yuint32_t uIxChar = _TEncodingToIndex(gs_TEncChar);
		yuint32_t uIxUTF8 = _TEncodingToIndex(YXC_TENCODING_UTF8);

		yuint32_t uCcSrcStr = iCcSrcStr == YXC_STR_NTS ? (yuint32_t)strlen((char*)pszSrc) : (yuint32_t)iCcSrcStr;
		_pfnCvtArr[uIxUTF8][uIxChar]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puCcUsed = uCcUsed;
		*puCcConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	ybool_t _TextConverter::UTF8ToWChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uCcUsed, uCcConverted;
		yuint32_t uIxUTF8 = _TEncodingToIndex(YXC_TENCODING_UTF8);
		yuint32_t uIxWChar = _TEncodingToIndex(gs_TEncWChar);

		yuint32_t uCcSrcStr = iCcSrcStr == YXC_STR_NTS ? (yuint32_t)strlen((char*)pszSrc) : (yuint32_t)iCcSrcStr;
		_pfnCvtArr[uIxUTF8][uIxWChar]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puCcUsed = uCcUsed;
		*puCcConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	ybool_t _TextConverter::WCharToUTF8(const wchar_t* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puCcConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uCcUsed, uCcConverted;
		yuint32_t uIxUTF8 = _TEncodingToIndex(YXC_TENCODING_UTF8);
		yuint32_t uIxWChar = _TEncodingToIndex(gs_TEncWChar);

		yuint32_t uCcSrcStr = iCcSrcStr == YXC_STR_NTS ? (yuint32_t)wcslen(pszSrc) : (yuint32_t)iCcSrcStr;
		_pfnCvtArr[uIxWChar][uIxUTF8]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puCcUsed = uCcUsed;
		*puCcConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	ybool_t _TextConverter::TEncodingConvert(YXC_TEncoding encSrc, YXC_TEncoding encDst, const void* pszSrc, yint32_t iCcSrcStr, void* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puUsed)
	{
		yuint32_t uCcUsed, uCcConverted;
		yuint32_t uIxSrc = _TEncodingToIndex(encSrc);
		yuint32_t uIxDst = _TEncodingToIndex(encDst);

		yuint32_t uCcSrcStr = iCcSrcStr;
		_pfnCvtArr[uIxSrc][uIxDst]((const ybyte_t*)pszSrc, uCcSrcStr, (ybyte_t*)pszDst, uCcDstStr,
			&uCcUsed, &uCcConverted);

		*puUsed = uCcUsed;
		*puConverted = uCcConverted;
		return uCcUsed == uCcSrcStr;
	}

	_TextConvertProc _TextConverter::_pfnCvtArr[TENCODING_MAX][TENCODING_MAX];
	_TextConvertLineProc _TextConverter::_pfnCvtLineArr[TENCODING_MAX][TENCODING_MAX];

    static int gs_cvtInit = _TextConverter::InitConverter();
}

using YXC_Inner::_TextConverter;

extern "C"
{
//	YXC_Status YXC_TEUTF8ToUTF16(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
//		yuint32_t uCcDstStr, yuint32_t* puConverted)
//	{
//		ybool_t bRet = _TextConverter::UTF8ToHostUTF16(pszSrc, iCcSrcStr, pszDst, uCcDstStr, puConverted);
//		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));
//
//		return YXC_ERC_SUCCESS;
//	}
//
//	YXC_Status YXC_TEUTF16ToUTF8(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
//		yuint32_t uCcDstStr, yuint32_t* puConverted)
//	{
//		ybool_t bRet = _TextConverter::HostUTF16ToUTF8(pszSrc, iCcSrcStr, pszDst, uCcDstStr, puConverted);
//		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));
//
//		return YXC_ERC_SUCCESS;
//	}
//
//	YXC_Status YXC_TEUTF16ToAnsi(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
//		yuint32_t uCcDstStr, yuint32_t* puConverted)
//	{
//		ybool_t bRet = _TextConverter::HostUTF16ToAnsi(pszSrc, iCcSrcStr, pszDst, uCcDstStr, puConverted);
//		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));
//
//		return YXC_ERC_SUCCESS;
//	}
//
//	YXC_Status YXC_TEAnsiToUTF16(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
//		yuint32_t uCcDstStr, yuint32_t* puConverted)
//	{
//		ybool_t bRet = _TextConverter::AnsiToHostUTF16(pszSrc, iCcSrcStr, pszDst, uCcDstStr, puConverted);
//		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));
//
//		return YXC_ERC_SUCCESS;
//	}

	static yuint32_t YXC_TEncodingGetCount(YXC_TEncoding encSrc, YXC_TEncoding encDst, const void* pszSrc, yuint32_t uSrcLen, yuint32_t uSize)
	{
		ybyte_t* byTemp = (ybyte_t*)alloca(sizeof(yint32_t) * (1024 + 1)); /* 2048 characters one time. */

		yuint32_t uTotalConverted = 0, uTotalUsed = 0;
		const ybyte_t* bySrc = (const ybyte_t*)pszSrc;

		while (uTotalUsed < uSrcLen)
		{
			yuint32_t uUsed = 0, uConverted = 0;
			YXC_TEncodingConvert(encSrc, encDst, bySrc + uTotalUsed, uSrcLen - uTotalUsed, byTemp, 1024 * sizeof(yint32_t) / uSize,
				&uConverted, &uUsed);

			uTotalUsed += uUsed;
			uTotalConverted += uConverted;
		}

		return uTotalConverted;
	}

	YXC_Status YXC_TEUTF8ToChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		if (pszDst == NULL)
		{
			yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? strlen((const char*)pszSrc) : iCcSrcStr;
			*puConverted = YXC_TEncodingGetCount(YXC_TENCODING_UTF8, YXC_TENCODING_CHAR, pszSrc, uSrcLen, sizeof(char));
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}

		yuint32_t uConverted, uUsed;
		ybool_t bRet = _TextConverter::UTF8ToChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uConverted, &uUsed);
		if (puConverted) *puConverted = uConverted;
		if (puCcUsed) *puCcUsed = uUsed;
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_TECharToUTF8(const char* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		if (pszDst == NULL)
		{
			yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? strlen(pszSrc) : iCcSrcStr;
			*puConverted = YXC_TEncodingGetCount(YXC_TENCODING_CHAR, YXC_TENCODING_UTF8, pszSrc, uSrcLen, sizeof(yuint8_t));
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}
		yuint32_t uConverted, uUsed;
		ybool_t bRet = _TextConverter::CharToUTF8(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uConverted, &uUsed);
		if (puConverted) *puConverted = uConverted;
		if (puCcUsed) *puCcUsed = uUsed;
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}

    YXC_Status YXC_TEWCharToChar(const wchar_t* pszSrc, yint32_t iCcSrcStr, char* pszDst,
    	yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		if (pszDst == NULL)
		{
			yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? wcslen(pszSrc) : iCcSrcStr;
			*puConverted = YXC_TEncodingGetCount(YXC_TENCODING_WCHAR, YXC_TENCODING_CHAR, pszSrc, uSrcLen, sizeof(char));
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}
		yuint32_t uConverted, uUsed;
		ybool_t bRet = _TextConverter::WCharToChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uConverted, &uUsed);
		if (puConverted) *puConverted = uConverted;
		if (puCcUsed) *puCcUsed = uUsed;
    	_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

    	return YXC_ERC_SUCCESS;
    }

    YXC_Status YXC_TECharToWChar(const char* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
    	yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		if (pszDst == NULL)
		{
			yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? strlen(pszSrc) : iCcSrcStr;
			*puConverted = YXC_TEncodingGetCount(YXC_TENCODING_CHAR, YXC_TENCODING_WCHAR, pszSrc, uSrcLen, sizeof(wchar_t));
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}

		yuint32_t uConverted, uUsed;
		ybool_t bRet = _TextConverter::CharToWChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uConverted, &uUsed);
		if (puConverted) *puConverted = uConverted;
		if (puCcUsed) *puCcUsed = uUsed;
    	_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

    	return YXC_ERC_SUCCESS;
    }

	YXC_Status YXC_TEUTF8ToWChar(const yuint8_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst,
                              yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		if (pszDst == NULL)
		{
			yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? strlen((const char*)pszSrc) : iCcSrcStr;
			*puConverted = YXC_TEncodingGetCount(YXC_TENCODING_UTF8, YXC_TENCODING_WCHAR, pszSrc, uSrcLen, sizeof(wchar_t));
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}
		yuint32_t uConverted, uUsed;
		ybool_t bRet = _TextConverter::UTF8ToWChar(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uConverted, &uUsed);
		if (puConverted) *puConverted = uConverted;
		if (puCcUsed) *puCcUsed = uUsed;
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_TECharCopy(const char* pszSrc, yint32_t iCcSrcStr, char* pszDst, yuint32_t uCcDstStr,
		yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? strlen(pszSrc) : iCcSrcStr;
		if (pszDst == NULL)
		{
			*puConverted = uSrcLen;
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}

		yuint32_t uCpy = YXCLib::TMin<yuint32_t>(uSrcLen, uCcDstStr);
		memcpy(pszDst, pszSrc, uCpy * sizeof(char));
		pszDst[uCpy] = 0;
		if (puConverted) *puConverted = uCpy;
		if (puCcUsed) *puCcUsed = uCpy;
		_YXC_CHECK_REPORT_NEW_RET(uCpy == uSrcLen, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_TEWCharCopy(const wchar_t* pszSrc, yint32_t iCcSrcStr, wchar_t* pszDst, yuint32_t uCcDstStr,
		yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? wcslen(pszSrc) : iCcSrcStr;
		if (pszDst == NULL)
		{
			*puConverted = uSrcLen;
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}

		yuint32_t uCpy = YXCLib::TMin<yuint32_t>(uSrcLen, uCcDstStr);
		memcpy(pszDst, pszSrc, uCpy * sizeof(wchar_t));
		pszDst[uCpy] = 0;
		if (puConverted) *puConverted = uCpy;
		if (puCcUsed) *puCcUsed = uCpy;
		_YXC_CHECK_REPORT_NEW_RET(uCpy == uSrcLen, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_TEWCharToUTF8(const wchar_t* pszSrc, yint32_t iCcSrcStr, yuint8_t* pszDst,
                              yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puCcUsed)
	{
		if (pszDst == NULL)
		{
			yuint32_t uSrcLen = iCcSrcStr == YXC_STR_NTS ? wcslen(pszSrc) : iCcSrcStr;
			*puConverted = YXC_TEncodingGetCount(YXC_TENCODING_WCHAR, YXC_TENCODING_UTF8, pszSrc, uSrcLen, sizeof(yuint8_t));
			if (puCcUsed) *puCcUsed = uSrcLen;
			return YXC_ERC_SUCCESS;
		}
		yuint32_t uConverted, uUsed;
		ybool_t bRet = _TextConverter::WCharToUTF8(pszSrc, iCcSrcStr, pszDst, uCcDstStr, &uConverted, &uUsed);
		if (puConverted) *puConverted = uConverted;
		if (puCcUsed) *puCcUsed = uUsed;
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_TEncodingConvert(YXC_TEncoding encSrc, YXC_TEncoding encDst, const void* pszSrc, yint32_t iCcSrcStr, void* pszDst,
		yuint32_t uCcDstStr, yuint32_t* puConverted, yuint32_t* puUsed)
	{
		ybool_t bRet = _TextConverter::TEncodingConvert(encSrc, encDst, pszSrc, iCcSrcStr, pszDst,
			uCcDstStr, puConverted, puUsed);
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));

		return YXC_ERC_SUCCESS;
	}
};

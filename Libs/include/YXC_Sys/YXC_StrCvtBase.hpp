/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_STRING_CONVERT_BASE_HPP__
#define __INC_YXC_SYS_BASE_STRING_CONVERT_BASE_HPP__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_EndianReverse.hpp>
#include <string>

#define _YXC_UNMAPPED_CH '?'
#define _YXC_MAPPED_CH_COUNT 65536

namespace YXCLib
{
	static const yuint32_t _MAX_UTF8_CH_BYTES = 4;
	static const yuint32_t _MAX_ANSI_CH_BYTES = 2;

	static const yuint32_t _UNI_CONVERT_ULE_LE = 0;
	static const yuint32_t _UNI_CONVERT_ULE_BE = 1;
	static const yuint32_t _UNI_CONVERT_UBE_BE = 2;
	static const yuint32_t _UNI_CONVERT_UBE_LE = 3;

	struct _UTF16CvtFlag
	{
		yuint16_t wFlagAnd;
		yuint16_t wFlagDiag;
		yuint16_t wReturn;
		yuint16_t wEnter;
	};

	struct _UTF32CvtFlag
	{
		yuint32_t wReturn;
		yuint32_t wEnter;
	};

#pragma pack(push, 1)
	struct _UTF8ChInfo {
		ybyte_t uNumBytes;
		ybyte_t utf8Bytes[3];
	};

	struct _AnsiChInfo {
		ybyte_t uNumBytes;
		ybyte_t ansiBytes[2];
	};

	extern _UTF8ChInfo g_UTF8ChInfoArr[_YXC_MAPPED_CH_COUNT];
	extern _AnsiChInfo g_AnsiChInfoArr[_YXC_MAPPED_CH_COUNT];
	extern yuint16_t g_AnsiChRevInfoArr[_YXC_MAPPED_CH_COUNT];
#pragma pack(pop)

	template <typename T1, typename T2>
	static inline void _RemoveLineEnter(T1*& pszDst, const T2* pszSrc, yuint32_t& uIndex, yuint32_t uEnterVal)
	{
		if (uIndex > 0 && pszSrc[uIndex - 1] == uEnterVal)
		{
			--pszDst;
		}
		++uIndex;
	}

	template <yuint32_t u16Type>
	class _UTF16Impl
	{
	public:
		static _UTF16CvtFlag GetCvtFlags()
		{
			_UTF16CvtFlag flag = { 0xfc00, 0xdc00, '\n', '\r' };
			return flag;
		}

		static inline yuint16_t CvtCh(yuint16_t wCh1)
		{
			return wCh1;
		}

		static yuint32_t MergeChAdv(yuint16_t wCh1, yuint16_t wCh2)
		{
			yuint16_t uLO16Bits = wCh1 & 0x03ff;
			yuint16_t uHI16Bits = wCh2 & 0x03ff;

			return (uLO16Bits & 0x03ff) | ((uHI16Bits & 0x03ff) << 10);
		}

		static void CvtChAdv(yuint32_t ch, yuint16_t* pCh1, yuint16_t* pCh2)
		{
			yuint16_t wchL = (ch & 0x3ff) | 0xfc00;
			yuint16_t wchH = (ch >> 10) | 0xf800;

			*pCh1 = wchL;
			*pCh2 = wchH;
		}
	};

	template <>
	class _UTF16Impl<_UNI_CONVERT_ULE_BE>
	{
	public:
		static _UTF16CvtFlag GetCvtFlags()
		{
			_UTF16CvtFlag flag = { 0x00fc, 0x00dc, _U16Reverse('\n'), _U16Reverse('\r') };
			return flag;
		}

		static yuint16_t CvtCh(yuint16_t wCh1)
		{
			return _U16Reverse(wCh1);
		}

		static yuint32_t MergeChAdv(yuint16_t wCh1, yuint16_t wCh2)
		{
			yuint16_t uLO16Bits = _U16Reverse(wCh1) & 0x03ff;
			yuint16_t uHI16Bits = _U16Reverse(wCh2) & 0x03ff;

			return (uLO16Bits & 0x03ff) | ((uHI16Bits & 0x03ff) << 10);
		}

		static void CvtChAdv(yuint32_t ch, yuint16_t* pCh1, yuint16_t* pCh2)
		{
			yuint16_t wchL = _U16Reverse((ch & 0x3ff) | 0xfc00);
			yuint16_t wchH = _U16Reverse((ch >> 10) | 0xf800);

			*pCh1 = wchH;
			*pCh2 = wchL;
		}
	};

	template <>
	class _UTF16Impl<_UNI_CONVERT_UBE_BE>
	{
	public:
		static _UTF16CvtFlag GetCvtFlags()
		{
			_UTF16CvtFlag flag = { 0xfc00, 0xd800, '\n', '\r' };
			return flag;
		}

		static yuint16_t CvtCh(yuint16_t wCh1)
		{
			return wCh1;
		}

		static yuint32_t MergeChAdv(yuint16_t wCh1, yuint16_t wCh2)
		{
			yuint16_t uHI16Bits = wCh1 & 0x03ff;
			yuint16_t uLO16Bits = wCh2 & 0x03ff;

			return (uLO16Bits & 0x03ff) | ((uHI16Bits & 0x03ff) << 10);
		}

		static void CvtChAdv(yuint32_t ch, yuint16_t* pCh1, yuint16_t* pCh2)
		{
			yuint16_t wchL = (ch & 0x3ff) | 0xfc00;
			yuint16_t wchH = (ch >> 10) | 0xf800;

			*pCh1 = wchH;
			*pCh2 = wchL;
		}
	};

	template <>
	class _UTF16Impl<_UNI_CONVERT_UBE_LE>
	{
	public:
		static _UTF16CvtFlag GetCvtFlags()
		{
			_UTF16CvtFlag flag = { 0x00fc, 0x00d8, _U16Reverse('\n'), _U16Reverse('\r') };
			return flag;
		}

		static yuint16_t CvtCh(yuint16_t wCh1)
		{
			return _U16Reverse(wCh1);
		}

		static yuint32_t MergeChAdv(yuint16_t wCh1, yuint16_t wCh2)
		{
			yuint16_t uHI16Bits = _U16Reverse(wCh1) & 0x03ff;
			yuint16_t uLO16Bits = _U16Reverse(wCh2) & 0x03ff;

			return (uLO16Bits & 0x03ff) | ((uHI16Bits & 0x03ff) << 10);
		}

		static void CvtChAdv(yuint32_t ch, yuint16_t* pCh1, yuint16_t* pCh2)
		{
			yuint16_t wchL = _U16Reverse((ch & 0x3ff) | 0xfc00);
			yuint16_t wchH = _U16Reverse((ch >> 10) | 0xf800);

			*pCh1 = wchL;
			*pCh2 = wchH;
		}
	};

	template <yuint32_t u16Type>
	class _UTF16Impl2
	{
	public:
		typedef yuint16_t Ch;

		static ybool_t Convert(yuint16_t ch, yuint16_t*& pDstStr, yuint16_t* pDstEnd)
		{
			if (pDstStr < pDstEnd)
			{
				*pDstStr++ = _UTF16Impl<u16Type>::CvtCh(ch);
				return TRUE;
			}

			return FALSE;
		}

		static ybool_t ConvertAdv(yuint32_t ch, yuint16_t*& pDstStr, yuint16_t* pDstEnd)
		{
			if (pDstStr + 2 <= pDstEnd)
			{
				_UTF16Impl<u16Type>::CvtChAdv(ch, pDstStr, pDstStr + 1);
				return TRUE;
			}

			return FALSE;
		}
	};

	template <ybool_t bIsUTF8>
	class _MultiByteImpl
	{
	public:
		typedef ybyte_t Ch;

		static const yuint32_t ADV_CH_VAL = 0xf8;
		/* UTF8 Fix size */
		static inline void FixSize(const ybyte_t* pszSrc, yuint32_t& uCcMaxStr)
		{
			const ybyte_t* pszSrcLast = pszSrc, *pszSrcEnd = pszSrc + uCcMaxStr;
			if (uCcMaxStr >= _MAX_UTF8_CH_BYTES)
			{
				pszSrcLast = pszSrc + uCcMaxStr - _MAX_UTF8_CH_BYTES;
			}

			while (pszSrcLast < pszSrcEnd)
			{
				ybyte_t chVal = *pszSrcLast;
				if (chVal < 0xc0) /* Normal char, 0b0xxxxxx */
				{
					++pszSrcLast;
				}
				else if (chVal < 0xe0) /* 2 bytes char, 0b110xxxxx */
				{
					if (pszSrcLast + 2 > pszSrcEnd) /* No enough memory to contain this char. */
					{
						break;
					}
					pszSrcLast += 2;
				}
				else if (chVal < 0xf0) /* 3 bytes char, 0b1110xxxx */
				{
					if (pszSrcLast + 3 > pszSrcEnd) /* No enough memory to contain this char. */
					{
						break;
					}
					pszSrcLast += 3;
				}
				else /* 4 bytes char, 0b11110xxx */
				{
					if (pszSrcLast + 4 > pszSrcEnd) /* No enough memory to contain this char. */
					{
						break;
					}
					pszSrcLast += 4;
				}
			}

			uCcMaxStr = (yuint32_t)(pszSrcLast - pszSrc);
		}

		static ybool_t Convert(yuint16_t ch, ybyte_t*& pDstStr, ybyte_t* pDstEnd)
		{
			_UTF8ChInfo chInfo = g_UTF8ChInfoArr[ch];
			if (pDstStr + chInfo.uNumBytes <= pDstEnd)
			{
				/* Drop loop procedure for efficiency. */
				if (chInfo.uNumBytes == 1)
				{
					*pDstStr++ = chInfo.utf8Bytes[0];
				}
				else if (chInfo.uNumBytes == 2)
				{
					*pDstStr++ = chInfo.utf8Bytes[0];
					*pDstStr++ = chInfo.utf8Bytes[1];
				}
				else
				{
					*pDstStr++ = chInfo.utf8Bytes[0];
					*pDstStr++ = chInfo.utf8Bytes[1];
					*pDstStr++ = chInfo.utf8Bytes[2];
				}
				return TRUE;
			}

			return FALSE;
		}

		static ybool_t ConvertAdv(yuint32_t uChVal, ybyte_t*& pDstStr, ybyte_t* pDstEnd)
		{
			if (pDstStr + 4 <= pDstEnd)
			{
				pDstStr[0] = 0xf8 + ((uChVal >> 18) & 0x07);
				pDstStr[1] = 0x80 + ((uChVal >> 12) & 0x3f);
				pDstStr[2] = 0x80 + ((uChVal >> 6) & 0x3f);
				pDstStr[3] = 0x80 + (uChVal & 0x3f);
				pDstStr += 4;
				return TRUE;
			}

			return FALSE;
		}

		static yuint16_t ReadCh(ybyte_t byCh1, const ybyte_t* pSrcStr, yuint32_t& uIndex)
		{
			if (byCh1 < 0xc0)
			{
				return byCh1;
			}
			else if (byCh1 < 0xe0)
			{
				ybyte_t byHighVal = byCh1 & 0x1f;
				ybyte_t byLowVal = pSrcStr[++uIndex] & 0x3f;
				yuint16_t uRet = (byHighVal << 6) | byLowVal;
				return uRet;
			}
			else
			{
				ybyte_t byHighVal = byCh1 & 0x0f;
				ybyte_t byMidVal = pSrcStr[++uIndex] & 0x3f;
				ybyte_t byLowVal = pSrcStr[++uIndex] & 0x3f;
				yuint16_t uRet = (byHighVal << 12) | (byMidVal << 6) | byLowVal;
				return uRet;
			}
		}

		static yuint32_t ReadChAdv(ybyte_t byCh1, const ybyte_t* pSrcStr, yuint32_t& uIndex)
		{
			ybyte_t byVal4 = byCh1 & 0x07;
			ybyte_t byVal3 = pSrcStr[++uIndex] & 0x3f;
			ybyte_t byVal2 = pSrcStr[++uIndex] & 0x3f;
			ybyte_t byVal1 = pSrcStr[++uIndex] & 0x3f;
			yuint32_t uRet = (byVal4 << 18) | (byVal3 << 12) | (byVal2 << 6) | byVal1;
			return uRet;
		}
	};

	template <>
	class _MultiByteImpl<FALSE>
	{
	public:
		typedef ybyte_t Ch;

		static const yuint32_t ADV_CH_VAL = 0x100;
		/* Ansi Fix size */
		static inline void FixSize(const ybyte_t* pszSrc, yuint32_t& uCcMaxStr)
		{
			const ybyte_t* pszSrcLast = pszSrc, *pszSrcEnd = pszSrc + uCcMaxStr;
			if (uCcMaxStr >= _MAX_ANSI_CH_BYTES)
			{
				pszSrcLast = pszSrc + uCcMaxStr - _MAX_ANSI_CH_BYTES;
			}

			while (pszSrcLast < pszSrcEnd)
			{
				ybyte_t chVal = *pszSrcLast;
				if (chVal < 0x80) /* Normal char, 0b0xxxxxx */
				{
					++pszSrcLast;
				}
				else /* 2 bytes char, DBCS leaded */
				{
					if (pszSrcLast + 2 > pszSrcEnd) /* No enough memory to contain this char. */
					{
						break;
					}
					pszSrcLast += 2;
				}
			}

			uCcMaxStr = (yuint32_t)(pszSrcLast - pszSrc);
		}

		static ybool_t Convert(yuint16_t ch, ybyte_t*& pDstStr, ybyte_t* pDstEnd)
		{
			_AnsiChInfo chInfo = g_AnsiChInfoArr[ch];
			if (pDstStr + chInfo.uNumBytes <= pDstEnd)
			{
				if (chInfo.uNumBytes == 1) /* Drop loop procedure for efficiency. */
				{
					*pDstStr++ = chInfo.ansiBytes[0];
				}
				else
				{
					*pDstStr++ = chInfo.ansiBytes[0];
					*pDstStr++ = chInfo.ansiBytes[1];
				}
				return TRUE;
			}

			return FALSE;
		}

		static ybool_t ConvertAdv(yuint32_t uChVal, ybyte_t*& pDstStr, ybyte_t* pDstEnd)
		{
			if (pDstStr < pDstEnd)
			{
				*pDstStr++ = _YXC_UNMAPPED_CH; /* Ansi : Can't map this character. */
				return TRUE;
			}

			return FALSE;
		}

		static yuint16_t ReadCh(ybyte_t byCh1, const ybyte_t* pSrcStr, yuint32_t& uIndex)
		{
			if (byCh1 < 0x80)
			{
				return byCh1;
			}
			else
			{
				ybyte_t byLowVal = pSrcStr[++uIndex];
				yuint16_t uAnsiChVal = (byCh1 << 8) | byLowVal;
				return g_AnsiChRevInfoArr[uAnsiChVal];
			}
		}

		static yuint32_t ReadChAdv(ybyte_t byCh1, const ybyte_t* pSrcStr, yuint32_t& uIndex)
		{
			++uIndex;
			return _YXC_UNMAPPED_CH;
		}
	};


	template <yuint32_t u32Type>
	class _UTF32Impl
	{
	public:
		static _UTF32CvtFlag GetCvtFlags()
		{
			_UTF32CvtFlag flag = { '\n', '\r' };
			return flag;
		}

		static inline yuint32_t CvtCh(yuint32_t wCh1)
		{
			return wCh1;
		}
	};

	template <>
	class _UTF32Impl<_UNI_CONVERT_ULE_BE>
	{
	public:
		static _UTF32CvtFlag GetCvtFlags()
		{
			_UTF32CvtFlag flag = { _U32Reverse('\n'), _U32Reverse('\r') };
			return flag;
		}

		static yuint32_t CvtCh(yuint32_t wCh1)
		{
			return _U32Reverse(wCh1);
		}
	};

	template <>
	class _UTF32Impl<_UNI_CONVERT_UBE_LE>
	{
	public:
		static _UTF32CvtFlag GetCvtFlags()
		{
			_UTF32CvtFlag flag = { _U32Reverse('\n'), _U32Reverse('\r') };
			return flag;
		}

		static yuint32_t CvtCh(yuint32_t wCh1)
		{
			return _U32Reverse(wCh1);
		}
	};

	template <yuint32_t u32Type>
	class _UTF32Impl2
	{
	public:
		typedef yuint32_t Ch;

		static ybool_t Convert(yuint16_t ch, yuint32_t*& pDstStr, yuint32_t* pDstEnd)
		{
			if (pDstStr < pDstEnd)
			{
				*pDstStr++ = _UTF32Impl<u32Type>::CvtCh(ch);
				return TRUE;
			}

			return FALSE;
		}

		static ybool_t ConvertAdv(yuint32_t ch, yuint32_t*& pDstStr, yuint32_t* pDstEnd)
		{
			if (pDstStr < pDstEnd)
			{
				*pDstStr++ = _UTF32Impl<u32Type>::CvtCh(ch);
				return TRUE;
			}

			return FALSE;
		}
	};
}

#endif /* __INC_YXC_SYS_BASE_STRING_CONVERT_BASE_HPP__ */

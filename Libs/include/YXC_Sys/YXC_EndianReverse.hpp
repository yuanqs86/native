/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_ENDIAN_REVERSE_HPP__
#define __INC_YXC_SYS_BASE_ENDIAN_REVERSE_HPP__

#define _YXC_BYTE_NUM_BITS 8

#include <YXC_Sys/YXC_Sys.h>
#include <algorithm>

namespace YXCLib
{
	inline yuint16_t _U16Reverse(yuint16_t u16Val)
	{
		return ((u16Val & 0xff) << 8) | ((u16Val & 0xff00) >> 8);
	}

	inline yuint32_t _U32Reverse(yuint32_t u32Val)
	{
		return ((u32Val & 0x000000ff) << 24) | ((u32Val & 0x0000ff00) << 8)
			| ((u32Val & 0x00ff0000) >> 8) | ((u32Val & 0xff000000) >> 24);
	}

	inline yuint64_t _U64Reverse(yuint64_t u64Val)
	{
#if YXC_IS_64BIT
		return ((u64Val & 0x00000000000000ffULL) << 56) | ((u64Val & 0x000000000000ff00ULL) << 40)
			| ((u64Val & 0x0000000000ff0000ULL) << 24) | ((u64Val & 0x00000000ff000000ULL) << 8)
			| ((u64Val & 0x000000ff00000000ULL) >> 8) | ((u64Val & 0x0000ff0000000000ULL) >> 24)
			| ((u64Val & 0x00ff000000000000ULL) >> 40) | ((u64Val & 0xff00000000000000ULL) >> 56);
#else
		yuint32_t uVal0 = u64Val & 0x00000000ffffffffULL;
		yuint32_t uVal1 = (u64Val & 0xffffffff00000000ULL) >> 32;

		return ((yuint64_t)_U32Reverse(uVal0) << 32) | _U32Reverse(uVal1);
#endif /* YXC_IS_64BIT */
	}

	template <typename T>
	struct _DigitalLimit
	{
		static const ybyte_t MAX_VAL = 0x01;
	};

	template <>
	struct _DigitalLimit<yuint16_t>
	{
		static const yuint16_t MAX_VAL = 0xffff;
	};

	template <>
	struct _DigitalLimit<yuint32_t>
	{
		static const yuint32_t MAX_VAL = 0xffffffff;
	};

	template <>
	struct _DigitalLimit<yuint64_t>
	{
		static const yuint64_t MAX_VAL = 0xffffffffffffffffULL;
	};

	template <typename T>
	inline T _UTGetSegment(T val, ybyte_t byStart, ybyte_t byLen)
	{
		T uVal1 = val >> byStart;
		T uValFlag = ~(_DigitalLimit<T>::MAX_VAL << byLen);

		return uVal1 & uValFlag;
	}

	template <typename T>
	inline T _UTReverseBE(T utVal, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		T utResult = 0;
		ybyte_t* pbySrc = (ybyte_t*)&utVal;

		ybyte_t byNumBytesOff = 0;
		ybyte_t byBitsOff = 0;
		ybyte_t bySegOffset = 0;

		for (yuint32_t i = 0; i < uNumBitSegments; ++i)
		{
			T utSubResult = 0;

			ybyte_t bySegLen = pSegsLen[i];
			ybyte_t bySegCut = 0;

			while (bySegCut < bySegLen)
			{
				ybyte_t byBytesRemain = _YXC_BYTE_NUM_BITS - byBitsOff;
				ybyte_t byCutout = YXCLib::TMin<ybyte_t>(bySegLen - bySegCut, byBytesRemain);
				for (ybyte_t j = 0; j < byCutout; ++j)
				{
					ybyte_t bitVal = (pbySrc[byNumBytesOff] >> (byBytesRemain - 1 - j)) & 0x01;
					utSubResult |= bitVal << (bySegLen - bySegCut - 1 - j);
				}

				bySegCut += byCutout;
				byBitsOff += byCutout;
				if (byBitsOff == _YXC_BYTE_NUM_BITS)
				{
					/* Goto fill next byte */
					++byNumBytesOff;
					byBitsOff = 0;

					if (byNumBytesOff > sizeof(T))
					{
						break;
					}
				}
			}

			utResult |= utSubResult << bySegOffset;
			bySegOffset += bySegLen;
		}
		return utResult;
	}

	template <typename T>
	inline T _UTReverseLE(T utVal, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		ybyte_t byNumBytesOff = 0;
		ybyte_t byBitsOff = 0;

		ybyte_t byDst[sizeof(T)] = {0};
		ybyte_t bySegOffset = 0;

		for (yuint32_t i = 0; i < uNumBitSegments; ++i)
		{
			ybyte_t bySegLen = pSegsLen[i];

			/* Get Segment value */
			T utSegmentVal = _UTGetSegment(utVal, bySegOffset, bySegLen);
			ybyte_t bySegCut = 0;

			/* Continue fill segment */
			while (bySegCut < bySegLen)
			{
				ybyte_t byBytesRemain = _YXC_BYTE_NUM_BITS - byBitsOff;
				ybyte_t byCutout = YXCLib::TMin<ybyte_t>(bySegLen - bySegCut, byBytesRemain);
				/* fill the remain of byte */
				for (ybyte_t j = 0; j < byCutout; ++j)
				{
					/* Reverse the byte order and put the bit into dst byte. */
					ybyte_t bitVal = (utSegmentVal >> (bySegLen - bySegCut - 1 - j)) & 0x01;
					byDst[byNumBytesOff] |= bitVal << (byBytesRemain - 1 - j);
				}

				bySegCut += byCutout;
				byBitsOff += byCutout;
				if (byBitsOff == _YXC_BYTE_NUM_BITS)
				{
					/* Goto fill next byte */
					++byNumBytesOff;
					byBitsOff = 0;

					if (byNumBytesOff > sizeof(byDst))
					{
						break;
					}
				}
			}

			bySegOffset += bySegLen;
		}

		return *(T*)byDst;
	}

	inline void _ReverseStringByteOrderW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch)
	{
		ysize_t i = 0;
		while (i < stCch)
		{
#if YXC_PLATFORM_WIN
			pszDst[i] = _U16Reverse(pszSrc[i]);
#elif YXC_PLATFORM_UNIX
            pszDst[i] = _U32Reverse(pszSrc[i]);
#endif /* YXC_PLATFORM_WIN */
            ++i;
		}

		pszDst[stCch] = L'\0';
	}

	inline ysize_t _ReverseStringByteOrderW(ychar* pszString, yssize_t stCchStr)
	{
		if (stCchStr != YXC_STR_NTS)
		{
			yssize_t i = 0;
			while (i < stCchStr)
			{
#if YXC_PLATFORM_WIN
                pszString[i] = _U16Reverse(pszString[i]);
#elif YXC_PLATFORM_UNIX
                pszString[i] = _U32Reverse(pszString[i]);
#endif /* YXC_PLATFORM_WIN */
			}

			return stCchStr;
		}
		else
		{
			stCchStr = 0;
			while (TRUE)
			{
				wchar_t wch = *pszString;
#if YXC_PLATFORM_WIN
                *pszString++ = _U16Reverse(wch);
#elif YXC_PLATFORM_UNIX
                *pszString++ = _U32Reverse(wch);
#endif /* YXC_PLATFORM_WIN */
				++stCchStr;

				if (wch == L'\0') break;
			}

			return stCchStr;
		}
	}
}

#endif /* __INC_YXC_SYS_BASE_ENDIAN_REVERSE_HPP__ */

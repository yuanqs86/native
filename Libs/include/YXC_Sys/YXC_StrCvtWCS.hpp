/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_STRING_CONVERT_WCS16_HPP__
#define __INC_YXC_SYS_BASE_STRING_CONVERT_WCS16_HPP__

#include <YXC_Sys/YXC_StrCvtBase.hpp>

namespace YXCLib
{
	static ybool_t _IsIncompleteUniCh(yuint16_t ch, yuint16_t flagAnd, yuint16_t flagDiag)
	{
		if ((ch & flagAnd) == flagDiag) /* First ch byte, ignore and drop. */
		{
			return TRUE;
		}
		return FALSE;
	}

	static void _Ch_UTF16RevertByteOrder(const yuint16_t* pszSrc, yuint32_t uCcStr, yuint16_t* pszDst,
		yuint16_t chFlagAnd, yuint16_t chFlagDiag)
	{
		const yuint16_t* pszSrcEnd = pszSrc + uCcStr;
		for (const yuint16_t* p = pszSrc; p < pszSrcEnd; ++p, ++pszDst)
		{
			if (_IsIncompleteUniCh(*p, chFlagAnd, chFlagDiag))
			{
				pszDst[0] = _U16Reverse(p[1]);
				pszDst[1] = _U16Reverse(p[0]);
				++pszDst;
				++p;
			}
			else
			{
				*pszDst = _U16Reverse(*p);
			}
		}

		pszDst[uCcStr] = 0;
	}

	static void _Ch_UTF16FixSize(const yuint16_t* pszSrc, yuint32_t& uCcMaxStr, yuint16_t chFlagAnd, yuint16_t chFlagDiag)
	{
		if (uCcMaxStr > 0)
		{
			yuint16_t chLast = pszSrc[uCcMaxStr - 1];
			if (_IsIncompleteUniCh(chLast, chFlagAnd, chFlagDiag))
			{
				--uCcMaxStr;
			}
		}
	}

	template <yuint32_t u16Type>
	class _UTF16Utils
	{
	public:
		static void CopyString(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
			yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint16_t* pszSrc = (const yuint16_t*)pSrcStr;
			yuint16_t* pszDst = (yuint16_t*)pDstStr;

			_UTF16CvtFlag iFlag = _UTF16Impl<u16Type>::GetCvtFlags();
			_Ch_UTF16FixSize(pszSrc, uCcMaxConvert, iFlag.wFlagAnd, iFlag.wFlagDiag);

			memcpy(pDstStr, pSrcStr, sizeof(yuint32_t) * uCcMaxConvert);
			pszDst[uCcMaxConvert] = 0;
			*puCcUsed = *puCcConverted = uCcMaxConvert;
		}

		static void CopyLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
			yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint16_t* pszSrc = (const yuint16_t*)pSrcStr;
			yuint16_t* pszDst = (yuint16_t*)pDstStr;

			_UTF16CvtFlag iFlag = _UTF16Impl<u16Type>::GetCvtFlags();
			_Ch_UTF16FixSize(pszSrc, uCcMaxConvert, iFlag.wFlagAnd, iFlag.wFlagDiag);
			*pbLineEnd = FALSE;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcMaxConvert; ++uIndex)
			{
				yuint16_t wSrc = pszSrc[uIndex];
				if (wSrc == iFlag.wReturn)
				{
					*pbLineEnd = TRUE;
					_RemoveLineEnter(pszDst, pszSrc, uIndex, iFlag.wEnter);
					break;
				}

				*pszDst++ = wSrc;
			}

			*pszDst = 0;
			*puCcUsed = uIndex;
			*puCcConverted = (yuint32_t)(pszDst - (yuint16_t*)pDstStr);
		}

		static void ReverseStringOrder(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
			yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint16_t* pszSrc = (const yuint16_t*)pSrcStr;

			_UTF16CvtFlag iFlag = _UTF16Impl<u16Type>::GetCvtFlags();
			_Ch_UTF16FixSize(pszSrc, uCcMaxConvert, iFlag.wFlagAnd, iFlag.wFlagDiag);
			_Ch_UTF16RevertByteOrder(pszSrc, uCcMaxConvert, (yuint16_t*)pDstStr, iFlag.wFlagAnd, iFlag.wFlagDiag);

			*puCcUsed = *puCcConverted = uCcMaxConvert;
		}

		static void ReverseStringLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
			yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint16_t* pszSrc = (const yuint16_t*)pSrcStr;
			yuint16_t* pszDst = (yuint16_t*)pDstStr;

			_UTF16CvtFlag iFlag = _UTF16Impl<u16Type>::GetCvtFlags();
			_Ch_UTF16FixSize(pszSrc, uCcMaxConvert, iFlag.wFlagAnd, iFlag.wFlagDiag);
			*pbLineEnd = FALSE;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcMaxConvert; ++uIndex)
			{
				yuint16_t wCh1 = pszSrc[uIndex];
				if (_IsIncompleteUniCh(wCh1, iFlag.wFlagAnd, iFlag.wFlagDiag))
				{
					*pszDst++ = _U16Reverse(pszSrc[uIndex + 1]);
					*pszDst++ = _U16Reverse(wCh1);
					++uIndex;
				}
				else
				{
					if (wCh1 == iFlag.wReturn)
					{
						*pbLineEnd = TRUE;
						_RemoveLineEnter(pszDst, pszSrc, uIndex, iFlag.wEnter);
						break;
					}
					*pszDst++ = _U16Reverse(wCh1);
				}
			}

			*pszDst = 0;
			*puCcUsed = uIndex;
			*puCcConverted = (yuint32_t)(pszDst - (yuint16_t*)pDstStr);
		}

        template <ybool_t bIsUTF8>
		static void ConvertToMultiByte(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
			yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			const yuint16_t* pszSrc = (const yuint16_t*)pSrcStr;
			_UTF16CvtFlag iFlag = _UTF16Impl<u16Type>::GetCvtFlags();
			_Ch_UTF16FixSize(pszSrc, uCcSrcStr, iFlag.wFlagAnd, iFlag.wFlagDiag);

			ybyte_t* pszDstCur = pDstStr, *pszDstEnd = pszDstCur + uCcDstStr;

			yuint32_t uIndex = 0;
			for (uIndex = 0; uIndex < uCcSrcStr; ++uIndex)
			{
				yuint16_t wCh = pszSrc[uIndex];
				if ((wCh & iFlag.wFlagAnd) == iFlag.wFlagDiag) /* Expand string */
				{
					yuint16_t wch2 = pszSrc[uIndex + 1]; /* Don't check character and only merge real char value. */

					yuint32_t uChVal = _UTF16Impl<u16Type>::MergeChAdv(wCh, wch2);
					ybool_t bConverted = _MultiByteImpl<bIsUTF8>::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
					if (!bConverted)
					{
						break;
					}

					++uIndex;
				}
				else /* Simple string. */
				{
					yuint16_t wChVal = _UTF16Impl<u16Type>::CvtCh(wCh);

					ybool_t bConverted2 = _MultiByteImpl<bIsUTF8>::Convert(wChVal, pszDstCur, pszDstEnd);
					if (!bConverted2)
					{
						break;
					}
				}
			}

			*puCcUsed = uIndex;
			yuint32_t uCcConverted = (yuint32_t)(pszDstCur - pDstStr);
			pDstStr[uCcConverted] = 0; /* Terminate this string. */
			*puCcConverted = uCcConverted;
		}

		template <ybool_t bIsUTF8>
		static void ConvertToMultiByteLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
			yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			const yuint16_t* pszSrc = (const yuint16_t*)pSrcStr;
			_UTF16CvtFlag iFlag = _UTF16Impl<u16Type>::GetCvtFlags();
			_Ch_UTF16FixSize(pszSrc, uCcSrcStr, iFlag.wFlagAnd, iFlag.wFlagDiag);
			*pbLineEnd = FALSE;

			ybyte_t* pszDstCur = pDstStr, *pszDstEnd = pszDstCur + uCcDstStr;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcSrcStr; ++uIndex)
			{
				yuint16_t wCh = pszSrc[uIndex];
				if ((wCh & iFlag.wFlagAnd) == iFlag.wFlagDiag) /* Expand string */
				{
					yuint16_t wch2 = pszSrc[uIndex + 1]; /* Don't check character and only merge real char value. */

					yuint32_t uChVal = _UTF16Impl<u16Type>::MergeChAdv(wCh, wch2);
					ybool_t bConverted = _MultiByteImpl<bIsUTF8>::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
					if (!bConverted)
					{
						break;
					}

					++uIndex;
				}
				else /* Simple string. */
				{
					yuint16_t wChVal = _UTF16Impl<u16Type>::CvtCh(wCh);
					if (wChVal == L'\n') /* Return. */
					{
						*pbLineEnd = TRUE;
						_RemoveLineEnter(pszDstCur, pszSrc, uIndex, iFlag.wEnter);
						break;
					}
					else
					{
						ybool_t bConverted2 = _MultiByteImpl<bIsUTF8>::Convert(wChVal, pszDstCur, pszDstEnd);
						if (!bConverted2)
						{
							break;
						}
					}
				}
			}

			*puCcUsed = uIndex;
			yuint32_t uCcConverted = (yuint32_t)(pszDstCur - pDstStr);
			pDstStr[uCcConverted] = 0; /* Terminate this string. */
			*puCcConverted = uCcConverted;
		}
	};

	static void _Ch_UTF32RevertByteOrder(const yuint32_t* pszSrc, yuint32_t uCcStr, yuint32_t* pszDst)
	{
		const yuint32_t* pszSrcEnd = pszSrc + uCcStr;
		for (const yuint32_t* p = pszSrc; p < pszSrcEnd; ++p, ++pszDst)
		{
            *pszDst = _U32Reverse(*p);
		}

		pszDst[uCcStr] = 0;
	}

	template <yuint32_t u32Type>
	class _UTF32Utils
	{
	public:
		static void CopyString(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
                               yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			yuint32_t* pszDst = (yuint32_t*)pDstStr;

			memcpy(pDstStr, pSrcStr, sizeof(yuint32_t) * uCcMaxConvert);
			pszDst[uCcMaxConvert] = 0;
			*puCcUsed = *puCcConverted = uCcMaxConvert;
		}

		static void CopyLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
                             yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint32_t* pszSrc = (const yuint32_t*)pSrcStr;
			yuint32_t* pszDst = (yuint32_t*)pDstStr;
			_UTF32CvtFlag iFlag = _UTF32Impl<u32Type>::GetCvtFlags();

			*pbLineEnd = FALSE;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcMaxConvert; ++uIndex)
			{
				yuint32_t wSrc = pszSrc[uIndex];
				if (wSrc == iFlag.wReturn)
				{
					*pbLineEnd = TRUE;
					_RemoveLineEnter(pszDst, pszSrc, uIndex, iFlag.wEnter);
					break;
				}

				*pszDst++ = wSrc;
			}

			*pszDst = 0;
			*puCcUsed = uIndex;
			*puCcConverted = (yuint32_t)(pszDst - (yuint32_t*)pDstStr);
		}

		static void ReverseStringOrder(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
                                       yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint32_t* pszSrc = (const yuint32_t*)pSrcStr;

			_Ch_UTF32RevertByteOrder(pszSrc, uCcMaxConvert, (yuint32_t*)pDstStr);

			*puCcUsed = *puCcConverted = uCcMaxConvert;
		}

		static void ReverseStringLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
                                      yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			const yuint32_t* pszSrc = (const yuint32_t*)pSrcStr;
			yuint32_t* pszDst = (yuint32_t*)pDstStr;
			_UTF32CvtFlag iFlag = _UTF32Impl<u32Type>::GetCvtFlags();

			*pbLineEnd = FALSE;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcMaxConvert; ++uIndex)
			{
				yuint32_t wCh1 = pszSrc[uIndex];
                if (wCh1 == iFlag.wReturn)
                {
                    *pbLineEnd = TRUE;
                    _RemoveLineEnter(pszDst, pszSrc, uIndex, iFlag.wEnter);
                    break;
                }
                *pszDst++ = _U32Reverse(wCh1);
			}

			*pszDst = 0;
			*puCcUsed = uIndex;
			*puCcConverted = (yuint32_t)(pszDst - (yuint32_t*)pDstStr);
		}

        template <ybool_t bIsUTF8>
		static void ConvertToMultiByte(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
                                       yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			const yuint32_t* pszSrc = (const yuint32_t*)pSrcStr;

			ybyte_t* pszDstCur = pDstStr, *pszDstEnd = pszDstCur + uCcDstStr;

			yuint32_t uIndex = 0;
			for (uIndex = 0; uIndex < uCcSrcStr; ++uIndex)
			{
                yuint32_t uChVal = _UTF32Impl<u32Type>::CvtCh(pszSrc[uIndex]);
                if (uChVal >= 0x10000)
                {
                    ybool_t bConverted = _MultiByteImpl<bIsUTF8>::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
                    if (!bConverted)
                    {
                        break;
                    }
                }
                else
                {
                    ybool_t bConverted = _MultiByteImpl<bIsUTF8>::Convert(uChVal, pszDstCur, pszDstEnd);
                    if (!bConverted)
                    {
                        break;
                    }
                }
			}

			*puCcUsed = uIndex;
			yuint32_t uCcConverted = (yuint32_t)(pszDstCur - pDstStr);
			pDstStr[uCcConverted] = 0; /* Terminate this string. */
			*puCcConverted = uCcConverted;
		}

		template <ybool_t bIsUTF8>
		static void ConvertToMultiByteLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr,
                                           yuint32_t uCcDstStr, yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			const yuint32_t* pszSrc = (const yuint32_t*)pSrcStr;
			_UTF32CvtFlag iFlag = _UTF32Impl<u32Type>::GetCvtFlags();
			*pbLineEnd = FALSE;

			ybyte_t* pszDstCur = pDstStr, *pszDstEnd = pszDstCur + uCcDstStr;

			yuint32_t uIndex = 0;
			for (uIndex = 0; uIndex < uCcSrcStr; ++uIndex)
			{
                yuint32_t uChVal = _UTF32Impl<u32Type>::CvtCh(pszSrc[uIndex]);
                if (uChVal >= 0x10000)
                {
                    ybool_t bConverted = _MultiByteImpl<bIsUTF8>::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
                    if (!bConverted)
                    {
                        break;
                    }
                }
                else
                {
					if (uChVal == '\n') /* Return. */
					{
						*pbLineEnd = TRUE;
						_RemoveLineEnter(pszDstCur, pszSrc, uIndex, iFlag.wEnter);
						break;
					}
					else
					{
						ybool_t bConverted2 = _MultiByteImpl<bIsUTF8>::Convert(uChVal, pszDstCur, pszDstEnd);
						if (!bConverted2)
						{
							break;
						}
					}
                    ybool_t bConverted = _MultiByteImpl<bIsUTF8>::Convert(uChVal, pszDstCur, pszDstEnd);
                    if (!bConverted)
                    {
                        break;
                    }
                }
                ybool_t bConverted = _MultiByteImpl<bIsUTF8>::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
                if (!bConverted)
                {
                    break;
                }
			}

			*puCcUsed = uIndex;
			yuint32_t uCcConverted = (yuint32_t)(pszDstCur - pDstStr);
			pDstStr[uCcConverted] = 0; /* Terminate this string. */
			*puCcConverted = uCcConverted;
		}
	};
}

#endif /* __INC_YXC_SYS_BASE_STRING_CONVERT_WCS16_HPP__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_STRING_CONVERT_MBS_HPP__
#define __INC_YXC_SYS_BASE_STRING_CONVERT_MBS_HPP__

#include <YXC_Sys/YXC_StrCvtBase.hpp>

namespace YXCLib
{
	template <ybool_t bIsUTF8>
	class _MultiByteUtils
	{
	public:
		static void CopyString(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
			yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);

			_MultiByteImpl<bIsUTF8>::FixSize(pSrcStr, uCcMaxConvert);

			memcpy(pDstStr, pSrcStr, sizeof(ybyte_t) * uCcMaxConvert);
			pDstStr[uCcMaxConvert] = 0;
			*puCcUsed = *puCcConverted = uCcMaxConvert;
		}

		static void CopyLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
			yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			yuint32_t uCcMaxConvert = YXCLib::TMin(uCcSrcStr, uCcDstStr);
			ybyte_t* pDstCur = pDstStr;

			_MultiByteImpl<bIsUTF8>::FixSize(pSrcStr, uCcMaxConvert);
			*pbLineEnd = FALSE;

			yuint32_t uIndex = 0;
			for (;uIndex < uCcMaxConvert; ++uIndex)
			{
				ybyte_t byStr = pSrcStr[uIndex];
				if (byStr == '\n')
				{
					*pbLineEnd = TRUE;
					_RemoveLineEnter(pDstCur, pSrcStr, uIndex, '\r');
					break;
				}
				*pDstCur++ = byStr;
			}
			*pDstCur = 0;
			*puCcUsed = uIndex;
			*puCcConverted = (yuint32_t)(pDstCur - pDstStr);
		}

		template <typename Cvt>
		static void ConvertT(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
            yuint32_t* puCcUsed, yuint32_t* puCcConverted)
		{
			_MultiByteImpl<bIsUTF8>::FixSize(pSrcStr, uCcSrcStr);
			typename Cvt::Ch* pszDst = (typename Cvt::Ch*)pDstStr;
			typename Cvt::Ch* pszDstCur = pszDst, *pszDstEnd = pszDstCur + uCcDstStr;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcSrcStr; ++uIndex)
			{
				yuint32_t uTempIndex = uIndex;
				ybyte_t byFirst = pSrcStr[uTempIndex];

				if (byFirst <= _MultiByteImpl<bIsUTF8>::ADV_CH_VAL)
				{
					yuint16_t uChVal = _MultiByteImpl<bIsUTF8>::ReadCh(byFirst, pSrcStr, uTempIndex);
					ybool_t bConverted = Cvt::Convert(uChVal, pszDstCur, pszDstEnd);
					if (!bConverted)
					{
						break;
					}
				}
				else
				{
					yuint32_t uChVal = _MultiByteImpl<bIsUTF8>::ReadChAdv(byFirst, pSrcStr, uTempIndex);
					ybool_t bConverted = Cvt::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
					if (!bConverted)
					{
						break;
					}
				}
				uIndex = uTempIndex;
			}

			*puCcUsed = uIndex;
			yuint32_t uCcConverted = (yuint32_t)(pszDstCur - pszDst);
			pszDst[uCcConverted] = 0; /* Terminate this string. */
			*puCcConverted = uCcConverted;
		}

		template <typename Cvt>
		static void ConvertTLine(const ybyte_t* pSrcStr, yuint32_t uCcSrcStr, ybyte_t* pDstStr, yuint32_t uCcDstStr,
            yuint32_t* puCcUsed, yuint32_t* puCcConverted, ybool_t* pbLineEnd)
		{
			_MultiByteImpl<bIsUTF8>::FixSize(pSrcStr, uCcSrcStr);
			typename Cvt::Ch* pszDst = (typename Cvt::Ch*)pDstStr;
			typename Cvt::Ch* pszDstCur = pszDst, *pszDstEnd = pszDstCur + uCcDstStr;
			*pbLineEnd = FALSE;

			yuint32_t uIndex = 0;
			for (; uIndex < uCcSrcStr; ++uIndex)
			{
				yuint32_t uTempIndex = uIndex;
				ybyte_t byFirst = pSrcStr[uTempIndex];

				if (byFirst <= _MultiByteImpl<bIsUTF8>::ADV_CH_VAL)
				{
					yuint16_t uChVal = _MultiByteImpl<bIsUTF8>::ReadCh(byFirst, pSrcStr, uTempIndex);
					if (uChVal == '\n')
					{
						*pbLineEnd = TRUE;
						_RemoveLineEnter(pszDstCur, pSrcStr, uIndex, '\r');
						break;
					}
					ybool_t bConverted = Cvt::Convert(uChVal, pszDstCur, pszDstEnd);
					if (!bConverted)
					{
						break;
					}
				}
				else
				{
					yuint32_t uChVal = _MultiByteImpl<bIsUTF8>::ReadChAdv(byFirst, pSrcStr, uTempIndex);
					ybool_t bConverted = Cvt::ConvertAdv(uChVal, pszDstCur, pszDstEnd);
					if (!bConverted)
					{
						break;
					}
				}
				uIndex = uTempIndex;
			}

			*puCcUsed = uIndex;
			yuint32_t uCcConverted = (yuint32_t)(pszDstCur - pszDst);
			pszDst[uCcConverted] = 0; /* Terminate this string. */
			*puCcConverted = uCcConverted;
		}
	};
}

#endif /* __INC_YXC_SYS_BASE_STRING_CONVERT_MBS_HPP__ */

#include <YXC_Sys/YXC_NetMarshal.h>
#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/Net/_YXC_NetMarshal.hpp>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>

extern "C"
{
	ybool_t YXC_NetNeedMarshalNumber()
	{
		return YXC_Inner::gs_bIsLittleEndianCPU;
	}

	yuint16_t YXC_NetMarshalUInt16(yuint16_t u16Val)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetMarshalUInt16(u16Val);
		}

		return u16Val;
	}

	yuint32_t YXC_NetMarshalUInt32(yuint32_t u32Val)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetMarshalUInt32(u32Val);
		}

		return u32Val;
	}

	yuint64_t YXC_NetMarshalUInt64(yuint64_t u64Val)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetMarshalUInt64(u64Val);
		}

		return u64Val;
	}

	yuint16_t YXC_NetUnmarshalUInt16(yuint16_t u16Val)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetUnmarshalUInt16(u16Val);
		}

		return u16Val;
	}

	yuint32_t YXC_NetUnmarshalUInt32(yuint32_t u32Val)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetUnmarshalUInt32(u32Val);
		}

		return u32Val;
	}

	yuint64_t YXC_NetUnmarshalUInt64(yuint64_t u64Val)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetUnmarshalUInt64(u64Val);
		}

		return u64Val;
	}

	void YXC_NetMarshalStringW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			YXC_Inner::_NetMarshalStringW(pszDst, pszSrc, stCch);
		}
		else
		{
			wcsncpy(pszDst, pszSrc, stCch);
			pszDst[stCch] = 0;
		}
	}

	void YXC_NetMarshalStringSelfW(wchar_t* pszStr, ysize_t stCch)
	{
		YXC_NetMarshalStringW(pszStr, pszStr, stCch);
	}

	void YXC_NetUnmarshalStringW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			YXC_Inner::_NetUnmarshalStringW(pszDst, pszSrc, stCch);
		}
		else
		{
			wcsncpy(pszDst, pszSrc, stCch);
			pszDst[stCch] = 0;
		}
	}

	void YXC_NetUnmarshalStringSelfW(wchar_t* pszStr, ysize_t stCch)
	{
		YXC_NetUnmarshalStringW(pszStr, pszStr, stCch);
	}

	yuint64_t YXC_NetMarshalUInt64Bits(yuint64_t u64Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetMarshalUInt64Bits(u64Val, uNumBitSegments, pSegsLen);
		}
		return u64Val;
	}

	yuint64_t YXC_NetUnmarshalUInt64Bits(yuint64_t u64Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetUnmarshalUInt64Bits(u64Val, uNumBitSegments, pSegsLen);
		}
		return u64Val;
	}

	yuint32_t YXC_NetMarshalUInt32Bits(yuint32_t u32Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetMarshalUInt32Bits(u32Val, uNumBitSegments, pSegsLen);
		}
		return u32Val;
	}

	yuint32_t YXC_NetUnmarshalUInt32Bits(yuint32_t u32Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		if (YXC_NetNeedMarshalNumber())
		{
			return YXC_Inner::_NetUnmarshalUInt32Bits(u32Val, uNumBitSegments, pSegsLen);
		}
		return u32Val;
	}

	void YXC_NetMarshalGuid(YXC_Guid* guid)
	{
		guid->Data1 = YXC_NetMarshalUInt32(guid->Data1);
		guid->Data2 = YXC_NetMarshalUInt16(guid->Data2);
		guid->Data3 = YXC_NetMarshalUInt16(guid->Data3);
	}

	void YXC_NetUnmarshalGuid(YXC_Guid* guid)
	{
		guid->Data1 = YXC_NetUnmarshalUInt32(guid->Data1);
		guid->Data2 = YXC_NetUnmarshalUInt16(guid->Data2);
		guid->Data3 = YXC_NetUnmarshalUInt16(guid->Data3);
	}
};

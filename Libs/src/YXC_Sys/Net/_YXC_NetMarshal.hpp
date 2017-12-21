#ifndef __INNER_INC_YXC_SYS_BASE_NET_MARSHAL_HPP__
#define __INNER_INC_YXC_SYS_BASE_NET_MARSHAL_HPP__

#include <YXC_Sys/YXC_EndianReverse.hpp>

namespace YXC_Inner
{
	inline yuint16_t _NetMarshalUInt16(yuint16_t u16Val)
	{
		return YXCLib::_U16Reverse(u16Val);
	}

	inline yuint32_t _NetMarshalUInt32(yuint32_t u32Val)
	{
		return YXCLib::_U32Reverse(u32Val);
	}

	inline yuint64_t _NetMarshalUInt64(yuint64_t u64Val)
	{
		return YXCLib::_U64Reverse(u64Val);
	}

	yuint32_t _NetMarshalUInt32Bits(yuint32_t u32Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		return YXCLib::_UTReverseBE(u32Val, uNumBitSegments, pSegsLen);
	}

	yuint64_t _NetMarshalUInt64Bits(yuint64_t u64Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		return YXCLib::_UTReverseBE(u64Val, uNumBitSegments, pSegsLen);
	}

	inline yuint16_t _NetUnmarshalUInt16(yuint16_t u16Marshalled)
	{
		return YXCLib::_U16Reverse(u16Marshalled);
	}

	inline yuint32_t _NetUnmarshalUInt32(yuint32_t u32Marshalled)
	{
		return YXCLib::_U32Reverse(u32Marshalled);
	}

	inline yuint64_t _NetUnmarshalUInt64(yuint64_t u64Marshalled)
	{
		return YXCLib::_U64Reverse(u64Marshalled);
	}

	yuint32_t _NetUnmarshalUInt32Bits(yuint32_t u32Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		return YXCLib::_UTReverseLE(u32Val, uNumBitSegments, pSegsLen);
	}

	yuint64_t _NetUnmarshalUInt64Bits(yuint64_t u64Val, yuint32_t uNumBitSegments, const ybyte_t* pSegsLen)
	{
		return YXCLib::_UTReverseLE(u64Val, uNumBitSegments, pSegsLen);
	}

	inline void _NetMarshalStringW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch)
	{
		YXCLib::_ReverseStringByteOrderW(pszDst, pszSrc, stCch);
	}

	inline void _NetUnmarshalStringW(wchar_t* pszDst, const wchar_t* pszSrc, ysize_t stCch)
	{
		YXCLib::_ReverseStringByteOrderW(pszDst, pszSrc, stCch);
	}
}

#endif /* __INNER_INC_YXC_SYS_BASE_NET_MARSHAL_HPP__ */

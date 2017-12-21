#define YXC_MACROS_NO_MIN_MAX

#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_StrCvtBase.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <algorithm>

namespace YXCLib
{
	ybool_t _AppendStringChkW(ychar*& pszBuf, ychar* pszEnd, const ychar* szStr, yint32_t iStrLen)
	{
		return _AppendStringChkT<ychar>(pszBuf, pszEnd, szStr, iStrLen);
	}

	ybool_t _AppendStringChkA(char*& pszBuf, char* pszEnd, const char* szStr, yint32_t iStrLen)
	{
		return _AppendStringChkT<char>(pszBuf, pszEnd, szStr, iStrLen);
	}

	static void _InitUTF8StrBuffers()
	{
		/* Init UTF8 buffers. */
		for (yuint32_t i = 0; i < 0x80; ++i) /* 0 ~ 7 bits. */
		{
			_UTF8ChInfo& chInfo = g_UTF8ChInfoArr[i];

			chInfo.uNumBytes = 1;
			chInfo.utf8Bytes[0] = i;
		}

		for (yuint32_t i = 0x80; i < 0x800; ++i) /* 8 ~ 11 bits. */
		{
			_UTF8ChInfo& chInfo = g_UTF8ChInfoArr[i];

			chInfo.uNumBytes = 2;
			chInfo.utf8Bytes[0] = (i >> 6) | 0xc0;
			chInfo.utf8Bytes[1] = (i & 0x3f) | 0x80;
		}

		for (yuint32_t i = 0x800; i < 0x10000; ++i)
		{
			_UTF8ChInfo& chInfo = g_UTF8ChInfoArr[i];

			chInfo.uNumBytes = 3;
			chInfo.utf8Bytes[0] = (i >> 12) | 0xe0;
			chInfo.utf8Bytes[1] = ((i >> 6) & 0x3f) | 0x80;
			chInfo.utf8Bytes[2] = (i & 0x3f) | 0x80;
		}
	}

	static void _InitAnsiStrRevMap(ychar* pszWch, ybyte_t* pszCh, yuint32_t uCceWideStr)
	{
		ybyte_t* pChIter = pszCh;
		for (yuint32_t i = 1; i < _YXC_MAPPED_CH_COUNT; ++i)
		{
			yuint16_t uHiBits = YXC_HI_8BITS(i), uLoBits = YXC_LO_8BITS(i);
			if (uHiBits <= 0x80 || uHiBits == 0xff) /* Single byte */
			{
				if (uLoBits > 0x80 && uLoBits != 0xff)
				{
					*pChIter++ = _YXC_UNMAPPED_CH; /* Invalid character */
				}
				else
				{
					*pChIter++ = uLoBits;
				}
			}
			else /* DBCS */
			{
				if (uLoBits != 0)
				{
					*pChIter++ = uHiBits;
					*pChIter++ = uLoBits;
				}
				else
				{
					*pChIter++ = _YXC_UNMAPPED_CH;
				}
			}
		}
		*pChIter = 0;

#if YXC_PLATFORM_WIN
		yuint32_t iTrans = MultiByteToWideChar(CP_ACP, 0, (char*)pszCh, pChIter - pszCh, pszWch,
			uCceWideStr * sizeof(ychar));
		g_AnsiChRevInfoArr[0] = 0;
		if (iTrans > 0)
		{
			--iTrans; /* Cch used. */
			for (yuint32_t uUsed = 0, uCurrentCh = 1;
				uUsed < iTrans && uCurrentCh < _YXC_MAPPED_CH_COUNT;
				++uUsed, ++uCurrentCh)
			{
				g_AnsiChRevInfoArr[uCurrentCh] = pszWch[uUsed];
			}
		}
#endif /* YXC_PLATFORM_WIN */
	}

	static void _InitAnsiStrMap(ychar* pszWch, ybyte_t* pszCh, yuint32_t uCceMultiByte)
	{
		for (yuint32_t i = 1; i < _YXC_MAPPED_CH_COUNT; ++i)
		{
			pszWch[i - 1] = i;
		}
		pszWch[_YXC_MAPPED_CH_COUNT - 1] = 0;

#if YXC_PLATFORM_WIN
		yuint32_t iTrans = WideCharToMultiByte(CP_ACP, 0, pszWch, _YXC_MAPPED_CH_COUNT - 1, (char*)pszCh,
			uCceMultiByte * sizeof(char), NULL, NULL);
		if (iTrans > 0)
		{
			--iTrans; /* Cch used. */
			for (yuint32_t uUsed = 0, uCurrentCh = 1;
				uUsed < iTrans && uCurrentCh < _YXC_MAPPED_CH_COUNT;
				++uUsed, ++uCurrentCh)
			{
				_AnsiChInfo& chInfo = g_AnsiChInfoArr[uCurrentCh];
				ybyte_t byCh = pszCh[uUsed];
				if (byCh <= 0x80 || byCh == 0xff)
				{
					chInfo.uNumBytes = 1;
					chInfo.ansiBytes[0] = byCh;
				}
				else
				{
					if (uUsed + 1 < iTrans)
					{
						chInfo.uNumBytes = 2;
						chInfo.ansiBytes[0] = pszCh[uUsed];
						chInfo.ansiBytes[1] = pszCh[uUsed + 1];
						++uUsed;
					}
				}
			}
		}
#else
#endif /* YXC_PLATFORM_WIN */
	}

	static void _InitAnsiStrBuffers()
	{
		yuint32_t uBufWCh = _YXC_MAPPED_CH_COUNT * 2;
		yuint32_t uBufCh = _YXC_MAPPED_CH_COUNT * 2;

		ybyte_t* pMem = (ybyte_t*)malloc(sizeof(ychar) * uBufWCh + sizeof(ybyte_t) * uBufCh);
		ybyte_t* pszCh = pMem;
		ychar* pszWch = (ychar*)(pszCh + uBufCh);
		YXCLib::HandleRef<void*> resMem((void*)pMem, free);

		_InitAnsiStrMap(pszWch, pszCh, (yuint32_t)(uBufCh / sizeof(char)));
		_InitAnsiStrRevMap(pszWch, pszCh, uBufWCh);
	}

	static yuint32_t _InitStrCvtBuffers()
	{
		_InitUTF8StrBuffers();
		_InitAnsiStrBuffers();
		return 0;
	}

	_UTF8ChInfo g_UTF8ChInfoArr[_YXC_MAPPED_CH_COUNT];
	_AnsiChInfo g_AnsiChInfoArr[_YXC_MAPPED_CH_COUNT];
	yuint16_t g_AnsiChRevInfoArr[_YXC_MAPPED_CH_COUNT];

	static yuint32_t gs_uInited = _InitStrCvtBuffers();

#if YXC_PLATFORM_WIN
	template <>
	const char* _ChTraits<char>::LINE_END = "\r\n";

	const wchar_t* _ChTraits<wchar_t>::LINE_END = L"\r\n";
#else
	template <>
	const char* _ChTraits<char>::LINE_END = "\n";

	const wchar_t* _ChTraits<wchar_t>::LINE_END = L"\n";
#endif /* YXC_PLATFORM_WIN */
}

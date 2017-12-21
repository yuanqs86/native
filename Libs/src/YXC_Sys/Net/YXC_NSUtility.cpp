#define __MODULE__ "YXC.DevUtils.NSUtility"
#include <YXC_Sys/YXC_NSUtility.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_NetMarshal.h>
#include <YXC_Sys/YXC_TextEncoding.h>

namespace YXCLib
{
    static void NSReadUTF8String(IMemoryStream& is, yuint8_t** ppszUTF8, yuint32_t* puCcUTF8, const char* pszName)
    {
		char szLenDesc[50] = {0}, *pszEnd = szLenDesc + 50, *pszBuf = szLenDesc;
		_AppendStringChkA(pszBuf, pszEnd, pszName, YXC_STR_NTS);
		_AppendStringChkA(pszBuf, pszEnd, "Len", YXC_STRING_ARR_LEN("Len"));

		yuint32_t uCcStrReal = YXC_NetUnmarshalUInt32(is.ReadT<yuint32_t>(szLenDesc));
        yuint8_t* pszUTF8 = (yuint8_t*)(is.BufferPtr() + is.GetPosition());

        ysize_t uNextPos = is.GetPosition() + (uCcStrReal + 1) * sizeof(yuint8_t);
        is.SetPosition(uNextPos);

        *ppszUTF8 = pszUTF8;
        *puCcUTF8 = uCcStrReal;
    }

    //static void NSWriteUTF8String(OMemoryStream& is, yuint8_t** ppszUTF8, yuint32_t* puCcUTF8, const char* pszName)
    //{

    //}

	void NSReadStringW(IMemoryStream& is, yuint32_t stCcStr, wchar_t* pszStr, yuint32_t* puCcStr, const char* pszName)
	{
        yuint32_t uCcUTF8;
        yuint8_t* pszUTF8;

        NSReadUTF8String(is, &pszUTF8, &uCcUTF8, pszName);

		yuint32_t uLen = 0, uUsed = 0, uTotal = (yuint32_t)strlen((char*)pszUTF8);
		while (uUsed < uTotal)
		{
			wchar_t buf[256 + 1];
			yuint32_t uConverted = 0, uNowUsed = 0;
			YXC_TEncodingConvert(YXC_TENCODING_UTF8, YXC_TENCODING_WCHAR, pszUTF8 + uUsed, uTotal - uUsed,
				buf, 256, &uConverted, &uNowUsed);
			uUsed += uNowUsed;
			uLen += uConverted;

			if (uConverted == 0) break;
		}

        *puCcStr = uLen;

        YXC_TEUTF8ToWChar(pszUTF8, uCcUTF8, pszStr, stCcStr, NULL, NULL);
	}

    void NSReadStringA(IMemoryStream& is, yuint32_t stCcStr, char* pszStr, yuint32_t* puCcStr, const char* pszName)
    {
        yuint32_t uCcUTF8;
        yuint8_t* pszUTF8;

        NSReadUTF8String(is, &pszUTF8, &uCcUTF8, pszName);


		yuint32_t uLen = 0, uUsed = 0, uTotal = (yuint32_t)strlen((char*)pszUTF8);
		while (uUsed < uTotal)
		{
			char buf[256 + 1];
			yuint32_t uConverted = 0, uNowUsed = 0;
			YXC_TEncodingConvert(YXC_TENCODING_UTF8, YXC_TENCODING_CHAR, pszUTF8 + uUsed, uTotal - uUsed,
				buf, 256, &uConverted, &uNowUsed);
			uUsed += uNowUsed;
			uLen += uConverted;

			if (uConverted == 0) break;
		}

        *puCcStr = uLen;

        YXC_TEUTF8ToChar(pszUTF8, uCcUTF8, pszStr, stCcStr, NULL, NULL);
    }

	YXC_Status NSReadEmpStringW(IMemoryStream& is, YMAlloc& emp, wchar_t** ppszStr, yuint32_t* puCcStr, const char* pszName)
	{
        yuint32_t uCcUTF8;
        yuint8_t* pszUTF8;

        NSReadUTF8String(is, &pszUTF8, &uCcUTF8, pszName);

        _YCHK_EAL_STRW_R2(*ppszStr, emp, uCcUTF8);

        yuint32_t uRealSize;
        YXC_Status rc = YXC_TEUTF8ToWChar(pszUTF8, uCcUTF8, *ppszStr, uCcUTF8, &uRealSize, NULL);
        if (rc != YXC_ERC_SUCCESS)
        {
            emp.Free(*ppszStr);
            return rc;
        }

        *puCcStr = uRealSize;
        return YXC_ERC_SUCCESS;
	}

	YXC_Status NSReadEmpStringA(IMemoryStream& is, YMAlloc& emp, char** ppszStr, yuint32_t* puCcStr, const char* pszName)
	{
        yuint32_t uCcUTF8;
        yuint8_t* pszUTF8;

        NSReadUTF8String(is, &pszUTF8, &uCcUTF8, pszName);

        _YCHK_EAL_STRA_R2(*ppszStr, emp, uCcUTF8);

        yuint32_t uRealSize;
        YXC_Status rc = YXC_TEUTF8ToChar(pszUTF8, uCcUTF8, *ppszStr, uCcUTF8, &uRealSize, NULL);
        if (rc != YXC_ERC_SUCCESS)
        {
            emp.Free(*ppszStr);
            return rc;
        }

        *puCcStr = uRealSize;
        return YXC_ERC_SUCCESS;
	}

	void NSReadGuid(IMemoryStream& is, YXC_Guid* guid, const char* pszName)
	{
		guid->Data1 = YXC_NetUnmarshalUInt32(is.ReadT<yuint32_t>("Data1"));
		guid->Data2 = YXC_NetUnmarshalUInt16(is.ReadT<yuint16_t>("Data2"));
		guid->Data3 = YXC_NetUnmarshalUInt16(is.ReadT<yuint16_t>("Data3"));
		for (int i = 0; i < YXC_ARR_COUNT(guid->Data4); ++i)
		{
			guid->Data4[i] = is.ReadT<ybyte_t>("Data4");
		}
	}

	YXC_Status NSReadEmpBinary(IMemoryStream& is, YMAlloc& emp, void** ppBinData, yuint32_t* puCbData, const char* pszName)
	{
		char szLenDesc[50] = {0}, *pszEnd = szLenDesc + 50, *pszBuf = szLenDesc;
		_AppendStringChkA(pszBuf, pszEnd, pszName, YXC_STR_NTS);
		_AppendStringChkA(pszBuf, pszEnd, "Len", YXC_STRING_ARR_LEN("Len"));

		yuint32_t uCcBinary = YXC_NetUnmarshalUInt32(is.ReadT<yuint32_t>(szLenDesc));

		void* pBinData = emp.Alloc(uCcBinary);
		_YXC_CHECK_REPORT_RET(pBinData != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Failed to alloc member '%@'"), pszName);

		is.Read(pBinData, uCcBinary, pszName);
		*puCbData = uCcBinary;
		*ppBinData = pBinData;
		return YXC_ERC_SUCCESS;
	}

	void NSWriteBinary(OMemoryStream& os, const void* pBinData, yuint32_t uCbBin, const char* pszName)
	{
		char szLenDesc[50] = {0}, *pszEnd = szLenDesc + 50, *pszBuf = szLenDesc;
		_AppendStringChkA(pszBuf, pszEnd, pszName, YXC_STR_NTS);
		_AppendStringChkA(pszBuf, pszEnd, "Len", YXC_STRING_ARR_LEN("Len"));

		os.WriteT(YXC_NetMarshalUInt32(uCbBin), szLenDesc);
		os.Write(pBinData, uCbBin, pszName);
	}

	void NSWriteGuid(OMemoryStream& os, const YXC_Guid* guid, const char* pszName)
	{
		os.WriteT(YXC_NetMarshalUInt32(guid->Data1), "Data1");
		os.WriteT(YXC_NetMarshalUInt16(guid->Data2), "Data2");
		os.WriteT(YXC_NetMarshalUInt16(guid->Data3), "Data3");
		for (int i = 0; i < YXC_ARR_COUNT(guid->Data4); ++i)
		{
			os.WriteT(guid->Data4[i], "Data4");
		}
	}

    void NSWriteStringA(OMemoryStream& os, const char* pszStr, const char* pszName)
    {
		char szLenDesc[50] = {0}, *pszEnd = szLenDesc + 50, *pszBuf = szLenDesc;
		_AppendStringChkA(pszBuf, pszEnd, pszName, YXC_STR_NTS);
		_AppendStringChkA(pszBuf, pszEnd, "Len", YXC_STRING_ARR_LEN("Len"));

		if (pszStr != NULL)
		{
            yuint32_t uLen = 0, uUsed = 0, uTotal = (yuint32_t)strlen(pszStr);

			while (uUsed < uTotal)
            {
				yuint8_t buf[256 + 1];
				yuint32_t uConverted = 0, uNowUsed = 0;
				YXC_TEncodingConvert(YXC_TENCODING_CHAR, YXC_TENCODING_UTF8, pszStr + uUsed, uTotal - uUsed,
					buf, 256, &uConverted, &uNowUsed);
				uUsed += uNowUsed;
				uLen += uConverted;

				if (uConverted == 0) break;
			}

			yuint32_t uDelta = (uLen + 1) * sizeof(yuint8_t) + sizeof(yuint32_t);
			os.SetPosition(os.GetPosition() + uDelta);

            yuint32_t* pLength = (yuint32_t*)(os.BufferPtr() + os.GetPosition() - uDelta);
            yuint8_t* pUTF8 = (yuint8_t*)(pLength + 1);

            yuint32_t val = YXC_NetMarshalUInt32(uLen);
            memcpy(pLength, &val, sizeof(yuint32_t));

            YXC_TECharToUTF8(pszStr, YXC_STR_NTS, pUTF8, uLen, NULL, NULL);
		}
		else
		{
			os.WriteT(YXC_NetMarshalUInt32(0), szLenDesc);
			os.Write("", sizeof(""), pszName);
		}
    }

	void NSWriteStringW(OMemoryStream& os, const wchar_t* pszStr, const char* pszName)
	{
		char szLenDesc[50] = {0}, *pszEnd = szLenDesc + 50, *pszBuf = szLenDesc;
		_AppendStringChkA(pszBuf, pszEnd, pszName, YXC_STR_NTS);
		_AppendStringChkA(pszBuf, pszEnd, "Len", YXC_STRING_ARR_LEN("Len"));

		if (pszStr != NULL)
		{
			yuint32_t uLen = 0, uUsed = 0, uTotal = (yuint32_t)wcslen(pszStr);
			while (uUsed < uTotal)
			{
				yuint8_t buf[256 + 1];
				yuint32_t uConverted = 0, uNowUsed = 0;

				YXC_TEncodingConvert(YXC_TENCODING_WCHAR, YXC_TENCODING_UTF8, pszStr + uUsed, uTotal - uUsed,
					buf, 256, &uConverted, &uNowUsed);
				uUsed += uNowUsed;
				uLen += uConverted;

				if (uConverted == 0) break;
			}

			yuint32_t uDelta = (uLen + 1) * sizeof(yuint8_t) + sizeof(yuint32_t);
			os.SetPosition(os.GetPosition() + uDelta);

            yuint32_t* pLength = (yuint32_t*)(os.BufferPtr() + os.GetPosition() - uDelta);
            yuint8_t* pUTF8 = (yuint8_t*)(pLength + 1);

            yuint32_t val = YXC_NetMarshalUInt32(uLen);
            memcpy(pLength, &val, sizeof(yuint32_t));

            YXC_TEWCharToUTF8(pszStr, YXC_STR_NTS, pUTF8, uLen, NULL, NULL);
		}
		else
		{
			os.WriteT(YXC_NetMarshalUInt32(0), szLenDesc);
			os.Write("", sizeof(""), pszName);
		}
	}
}

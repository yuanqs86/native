#define __MODULE__ "EK.TextFile"

#include <YXC_Sys/YXC_TextFile.h>
#include <YXC_Sys/Utils/_YXC_TextFile.hpp>
#include <YXC_Sys/Utils/_YXC_TextConverter.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_StrCvtMBS.hpp>
#include <stdio.h>

using namespace YXC_InnerF;
using namespace YXC_Inner;

extern "C"
{
    YXC_Status YXC_TextFileCreate(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile, YXC_TEncoding reqEncoding,
        YXC_TEncoding* pEncoding, YXC_TextFile* pFile)
    {
		_YCHK_MAL_R1(pTxtFile, _TextFile);
		new (pTxtFile) _TextFile();

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = pTxtFile->CreateText(pszPath, fMode, fAccess, bOSFile, reqEncoding, pEncoding);
		_YXC_CHECK_RC_GOTO(rc);

		*pFile = _TFileHdl(pTxtFile);
		return YXC_ERC_SUCCESS;
    err_ret:
		YXCLib::TDelete(pTxtFile);
		return rcRet;
    }

	YXC_Status YXC_TextFileCreateA(const char* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile, YXC_TEncoding reqEncoding,
		YXC_TEncoding* pEncoding, YXC_TextFile* pFile)
	{
#if YCHAR_WCHAR_T
		YXC_FPath fPath;
		ybool_t bFullConvert = _TextConverter::CharToWChar(pszPath, YXC_STR_NTS, fPath, YXC_MAX_CCH_PATH - 1);
		_YXC_CHECK_REPORT_NEW_RET(bFullConvert, YXC_ERC_DATA_TRUNCATED, YC("Path was truncated"));

		return YXC_TextFileCreateW(fPath, fMode, fAccess, bOSFile, reqEncoding, pEncoding, pFile);
#else
		return YXC_TextFileCreate(pszPath, fMode, fAccess, bOSFile, reqEncoding, pEncoding, pFile);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status YXC_TextFileCreateW(const wchar_t* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile, YXC_TEncoding reqEncoding,
		YXC_TEncoding* pEncoding, YXC_TextFile* pFile)
	{
#if YCHAR_WCHAR_T
		return YXC_TextFileCreate(pszPath, fMode, fAccess, bOSFile, reqEncoding, pEncoding, pFile);
#else
		YXC_FPath fPath;
		ybool_t bFullConvert = _TextConverter::WCharToChar(pszPath, YXC_STR_NTS, fPath, YXC_MAX_CCH_PATH - 1);
		_YXC_CHECK_REPORT_NEW_RET(bFullConvert, YXC_ERC_DATA_TRUNCATED, YC("Path was truncated"));

		return YXC_TextFileCreateA(fPath, fMode, fAccess, bOSFile, reqEncoding, pEncoding, pFile);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status YXC_TextFileWrite(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcWrite, const void* pStr)
	{
		_TextFile* pTxtFile = _TFilePtr(file);
		return pTxtFile->WriteText(enc, uCcWrite, pStr);
	}

	YXC_Status YXC_TextFileWriteLine(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcLine, const void* pLine)
	{
		_TextFile* pTxtFile = _TFilePtr(file);
		return pTxtFile->WriteLine(enc, uCcLine, pLine);
	}

	YXC_Status YXC_TextFileRead(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcBuf, void* pStr, yuint32_t* puCcToRead)
	{
		_TextFile* pTxtFile = _TFilePtr(file);
		return pTxtFile->ReadText(enc, uCcBuf, pStr, puCcToRead);
	}

	YXC_Status YXC_TextFileReadLine(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcLineBuf, void* pLine, yuint32_t* puCcLine)
	{
		_TextFile* pTxtFile = _TFilePtr(file);
		return pTxtFile->ReadLine(enc, uCcLineBuf, pLine, puCcLine);
	}
};

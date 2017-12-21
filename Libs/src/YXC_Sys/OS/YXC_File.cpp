#define __MODULE__ "EK.File"

#include <YXC_Sys/YXC_File.h>
#include <YXC_Sys/OS/_YXC_FileBase.hpp>
#include <YXC_Sys/OS/_YXC_OSFile.hpp>
#include <YXC_Sys/OS/_YXC_StdCFile.hpp>
#include <YXC_Sys/Utils/_YXC_TextConverter.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <new>

using namespace YXC_InnerF;
using namespace YXC_Inner;

namespace
{
	YXC_Status _AllocFilePtr(ybool_t bOSFile, _FileBase** ppFilePtr)
	{
		if (bOSFile)
		{
			_YCHK_MAL_R2(*ppFilePtr, _OSFile);
			new (*ppFilePtr) _OSFile();
		}
		else
		{
			_YCHK_MAL_R2(*ppFilePtr, _StdCFile);
			new (*ppFilePtr) _StdCFile();
		}

		return YXC_ERC_SUCCESS;
	}
}

extern "C"
{
    YXC_Status YXC_FileCreate(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccessMode, ybool_t bOSFile, YXC_File* pFile)
    {
		_FileBase* pFilePtr;
		YXC_Status rc = _AllocFilePtr(bOSFile, &pFilePtr);
		_YXC_CHECK_RC_RET(rc);

		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		rc = pFilePtr->Create(pszPath, fMode, fAccessMode);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to create file '%s'"), pszPath);

		*pFile = _FileHdl(pFilePtr);
		return YXC_ERC_SUCCESS;
    err_ret:
		pFilePtr->~_FileBase();
		free(pFilePtr);
		return rcRet;
    }

	YXC_Status YXC_FileCreateA(const char* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccessMode, ybool_t bOSFile, YXC_File* pFile)
	{
#if YCHAR_WCHAR_T
		YXC_FPath fPath;
		ybool_t bRet = _TextConverter::CharToWChar(pszPath, YXC_STR_NTS, fPath, YXC_STRING_ARR_LEN(fPath));
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("path(%s) was truncated"), pszPath);

		return YXC_FileCreate(fPath, fMode, fAccessMode, bOSFile, pFile);
#else
        return YXC_FileCreate(pszPath, fMode, fAccessMode, bOSFile, pFile);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status YXC_FileCreateW(const wchar_t* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccessMode, ybool_t bOSFile, YXC_File* pFile)
	{
#if YCHAR_WCHAR_T
        return YXC_FileCreate(pszPath, fMode, fAccessMode, bOSFile, pFile);
#else
		YXC_FPath fPath;
		ybool_t bRet = _TextConverter::WCharToChar(pszPath, YXC_STR_NTS, fPath, YXC_STRING_ARR_LEN(fPath));
		_YXC_CHECK_REPORT_NEW_RET(bRet, YXC_ERC_DATA_TRUNCATED, YC("path(%s) was truncated"), pszPath);

		return YXC_FileCreate(fPath, fMode, fAccessMode, bOSFile, pFile);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status YXC_FileRead(YXC_File file, yuint32_t uCbToRead, void* pData, yuint32_t* puCbRead)
	{
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->Read(uCbToRead, pData, puCbRead);
	}

	YXC_Status YXC_FileWrite(YXC_File file, yuint32_t uCbToWrite, const void* pData, yuint32_t* puCbWritten)
	{
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->Write(uCbToWrite, pData, puCbWritten);
	}

	void YXC_FileClose(YXC_File file)
	{
		_FileBase* fPtr = _FilePtr(file);
		fPtr->Close();
		fPtr->~_FileBase();

		free(fPtr);
	}

	void YXC_FileGetPath(YXC_File file, YXC_FPath* pFPath)
	{
		_FileBase* fPtr = _FilePtr(file);
		const ychar* fPath = fPtr->GetPath();
		yh_strcpy(*pFPath, fPath);
	}

	YXC_Status YXC_FileSeek(YXC_File file, YXC_FSeekMode fSeekMode, yint64_t i64SeekTo, yint64_t* pi64CurPos)
	{
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->Seek(fSeekMode, i64SeekTo, pi64CurPos);
	}

	YXC_Status YXC_FileFlush(YXC_File file)
	{
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->Flush();
	}

    YXC_Status YXC_FileLock(YXC_File file)
    {
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->Lock(0, 0);
    }

    YXC_Status YXC_FileUnlock(YXC_File file)
    {
        _FileBase* fPtr = _FilePtr(file);
        return fPtr->Unlock(0, 0);
    }

	YXC_Status YXC_FileGetSize(YXC_File file, yuint64_t* pu64FileSize)
	{
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->GetFileSize(pu64FileSize);
	}

	YXC_Status YXC_FileGetTimes(YXC_File file, yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
	{
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->GetFileTime(pu64Modified, pu64Created, pu64LastAccess);
	}

    YXC_Status YXC_FileSetTimes(YXC_File file, yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
    {
		_FileBase* fPtr = _FilePtr(file);
		return fPtr->SetFileTime(pu64Modified, pu64Created, pu64LastAccess);
    }
};

#define __MODULE__ "EK.File.StdC"
#include <YXC_Sys/OS/_YXC_StdCFile.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_FilePath.h>

#if YXC_PLATFORM_WIN
#include <sys/utime.h>
#include <io.h>
#endif /* YXC_PLATFORM_WIN */

namespace YXC_InnerF
{
	void _StdCFile::Close()
	{
		fclose(this->_fp);
	}

	YXC_Status _StdCFile::Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access)
	{
		ychar szMode[5];
		YXC_Status rc = _GetFOpenFlags(szMode, fMode, access);
		_YXC_CHECK_RC_RET(rc);

		YXC_FPath exactedPath;
		YXC_FPathCopy(exactedPath, pszPath);
		YXC_FPathExact(exactedPath);

		FILE* fp = yh_fopen(exactedPath, szMode);
		if (fMode == YXC_FOPEN_CREATE_NEW)
		{
			if (fp != NULL)
			{
				fclose(fp);
				_YXC_REPORT_OS_ERR(YC("File('%s') is already existing"), exactedPath);
				return YXC_ERC_OS;
			}

			return this->Create(exactedPath, YXC_FOPEN_CREATE_ALWAYS, access);
		}
		else
		{
			_YXC_CHECK_OS_RET(fp != NULL, YC("Failed to open file('%s':%s)"), exactedPath, szMode);
		}

		this->_fp = fp;
		yh_strncpy(this->_fPath, exactedPath, YXC_MAX_CCH_PATH);
		this->_fPath[YXC_MAX_CCH_PATH] = 0;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _StdCFile::Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead)
	{
		yuint32_t uRead = (yuint32_t)fread(pData, 1, uCbToRead, this->_fp);
		if (uRead == 0)
		{
			if (feof(this->_fp)) /* End of file, no data to read. */
			{
				*puRead = 0;
				return YXC_ERC_SUCCESS;
			}
			_YXC_REPORT_OS_ERR(YC("Failed to read file('%s', %u)"), this->_fPath, uCbToRead);
			return YXC_ERC_OS;
		}

		*puRead = uRead;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _StdCFile::Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten)
	{
		yuint32_t uWritten = (yuint32_t)fwrite(pData, 1, uCbToWrite, this->_fp);
		_YXC_CHECK_OS_RET(uWritten > 0, YC("Failed to write file('%s', %u)"), this->_fPath, uCbToWrite);

		*puWritten = uWritten;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _StdCFile::Flush()
	{
		int iRet = ::fflush(this->_fp);
		_YXC_CHECK_OS_RET(iRet != EOF, YC("Failed to flush file('%s') buffers"), this->_fPath);

		return YXC_ERC_SUCCESS;
	}

    YXC_Status _StdCFile::GetFileSize(yuint64_t* pu64FSize)
    {
        return _FileBase::GetFileSize(pu64FSize);
    }

    YXC_Status _StdCFile::Lock(yuint64_t u64Start, yuint64_t uLockSize)
    {
#if YXC_PLATFORM_UNIX
        flockfile(this->_fp);
        return YXC_ERC_SUCCESS;
#else
		_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Not support file locking"));
#endif /* YXC_PLATFORM_UNIX */
    }

    YXC_Status _StdCFile::Unlock(yuint64_t u64Start, yuint64_t uLockSize)
	{
#if YXC_PLATFORM_UNIX
        funlockfile(this->_fp);
		return YXC_ERC_SUCCESS;
#else
	_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Not support file locking"));
#endif /* YXC_PLATFORM_UNIX */
    }

	YXC_Status _StdCFile::Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64CurPos)
	{
		int iSeekMode = SEEK_SET;
		if (seekMode == YXC_FSEEK_END)
		{
			iSeekMode = SEEK_END;
		}
		else if (seekMode == YXC_FSEEK_CUR)
		{
			iSeekMode = SEEK_CUR;
		}
#if YXC_PLATFORM_WIN
		int iSeekRet = _fseeki64(this->_fp, i64SeekPos, iSeekMode);
#elif YXC_PLATFORM_UNIX
        int iSeekRet = fseeko(this->_fp, i64SeekPos, iSeekMode);
#endif /* YXC_PLATFORM_UNIX */

		_YXC_CHECK_OS_RET(iSeekRet == 0, YC("Failed to seek file('%s':%d) to %lld"), this->_fPath, iSeekMode, i64SeekPos);

		if (pi64CurPos)
		{
#if YXC_PLATFORM_WIN
			long long llPos = _ftelli64(this->_fp);
#elif YXC_PLATFORM_UNIX
            long long llPos = ftello(this->_fp);
#endif /* YXC_PLATFORM_UNIX */

			_YXC_CHECK_OS_RET(llPos != -1, YC("Failed to tell file('%s') position"), this->_fPath);
			*pi64CurPos = llPos;
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _StdCFile::_GetFOpenFlags(ychar* pszFlag, YXC_FOpenMode fMode, YXC_FileAccess access)
	{
		if (!(access & YXC_FACCESS_WRITE))
		{
			_YXC_CHECK_REPORT_NEW_RET((access & YXC_FACCESS_READ) && fMode == YXC_FOPEN_OPEN_EXISTING,
				YXC_ERC_INVALID_PARAMETER, YC("Invalid file mode(%d-%d) open parameter"), fMode, access);
			if ((access & YXC_FACCESS_READ) && fMode == YXC_FOPEN_OPEN_EXISTING)
			{
				yh_strcpy(pszFlag, YC("rb"));
				return YXC_ERC_SUCCESS;
			}
		}
		else
		{
			if (fMode == YXC_FOPEN_OPEN_EXISTING || fMode == YXC_FOPEN_CREATE_NEW)
			{
				yh_strcpy(pszFlag, YC("rb+"));
				return YXC_ERC_SUCCESS;
			}
			else if (fMode == YXC_FOPEN_OPEN_ALWAYS || fMode == YXC_FOPEN_APPEND)
			{
				yh_strcpy(pszFlag, YC("ab"));
				if (access & YXC_FACCESS_READ)
				{
					yh_strcat(pszFlag, YC("+"));
				}
				return YXC_ERC_SUCCESS;
			}
			else if (fMode == YXC_FOPEN_CREATE_ALWAYS || fMode == YXC_FOPEN_CREATE_NEW)
			{
				yh_strcpy(pszFlag, YC("wb"));
				if (access & YXC_FACCESS_READ)
				{
					yh_strcat(pszFlag, YC("+"));
				}
				return YXC_ERC_SUCCESS;
			}
		}
		_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_PARAMETER, YC("Invalid file mode(%d-%d) open parameter"), fMode, access);
	}
}

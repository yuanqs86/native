#define __MODULE__ "EK.File.OS"
#include <YXC_Sys/OS/_YXC_OSFile.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_FilePath.h>

#if YXC_PLATFORM_UNIX
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#elif YXC_PLATFORM_WIN
#include <ShlObj.h>
#include <Shlwapi.h>
#endif /* YXC_PLATFORM_UNIX */

namespace YXC_InnerF
{
#if YXC_PLATFORM_WIN
	void _OSFile::Close()
	{
		CloseHandle(this->_h);
	}

	YXC_Status _OSFile::Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access)
	{
		DWORD dwAccess, dwShare, dwCreateDisposition;
		_FileAccessCvt(access, &dwAccess, &dwShare);
		_FileModeCvt(fMode, &dwCreateDisposition);

		YXC_FPath exactedPath;
		YXC_FPathCopy(exactedPath, pszPath);
		YXC_FPathExact(exactedPath);

		DWORD dwAttribute = FILE_ATTRIBUTE_NORMAL;
		if (access & YXC_FACCESS_DIR)
		{
			BOOL bMakeDir = fMode != YXC_FOPEN_OPEN_EXISTING;
			YXC_Status mdRet;
			if (bMakeDir)
			{
				mdRet = YXC_FPathCreateDir(exactedPath, YXC_KOBJATTR_ALL_ACCESS, FALSE);
				_YXC_CHECK_REPORT_RET(mdRet == YXC_ERC_SUCCESS || mdRet == YXC_ERC_ALREADY_EXISTS, mdRet,
					YC("Failed calling to YXC_FPathCreateDir"));
				_YXC_CHECK_REPORT_RET(mdRet == YXC_ERC_SUCCESS || fMode != YXC_FOPEN_CREATE_NEW, mdRet,
					YC("Directory already exists"));
			}

			this->_h = CreateFileW(exactedPath, dwAccess, dwShare, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS
				| FILE_ATTRIBUTE_DIRECTORY, NULL);
			_YXC_CHECK_OS_RET(this->_h != INVALID_HANDLE_VALUE, YC("Failed to open os dir('%s')"), exactedPath);
		}
		else
		{
			this->_h = CreateFileW(exactedPath, dwAccess, dwShare, NULL, dwCreateDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
			_YXC_CHECK_OS_RET(this->_h != INVALID_HANDLE_VALUE, YC("Failed to create os file('%s')"), exactedPath);
		}

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		if (fMode == YXC_FOPEN_APPEND)
		{
			YXC_Status rc = this->Seek(YXC_FSEEK_END, 0, NULL);
			_YXC_CHECK_STATUS_GOTO(rc, YC("Seek os file('%s') for appending failed"), exactedPath);
		}

		yh_strncpy(this->_fPath, exactedPath, YXC_MAX_CCH_PATH);
		this->_fPath[YXC_MAX_CCH_PATH] = 0;
		return YXC_ERC_SUCCESS;
err_ret:
		CloseHandle(this->_h);
		return rcRet;
	}

	YXC_Status _OSFile::GetFileSize(yuint64_t* pu64FSize)
	{
		DWORD dwHigh;
		DWORD dwLow = ::GetFileSize(this->_h, &dwHigh);

		if (dwLow == INVALID_FILE_SIZE && GetLastError() != 0) /* Error when get file size. */
		{
			_YXC_REPORT_OS_ERR(YC("Failed to get file('%s') size"), this->_fPath);
			return YXC_ERC_OS;
		}

		*pu64FSize = YXC_MK64BITS_32(dwHigh, dwLow);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::GetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
	{
		FILETIME fAccess, fWrite, fCreate;
		BOOL bRet = ::GetFileTime(this->_h, &fCreate, &fAccess, &fWrite);
		_YXC_CHECK_OS_RET(bRet, YC("Failed to get file('%s') time"), this->_fPath);

		if (pu64Modified) *pu64Modified = YXCLib::_FileTimeToGTime(fWrite);
		if (pu64Created) *pu64Created = YXCLib::_FileTimeToGTime(fCreate);
		if (pu64LastAccess) *pu64LastAccess = YXCLib::_FileTimeToGTime(fAccess);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::SetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
	{
		FILETIME fAccess, fWrite, fCreate;
		FILETIME* pfAccess = NULL, *pfWrite = NULL, *pfCreate = NULL;

		if (pu64Modified)
		{
			fWrite = YXCLib::_GTimeToFileTime(*pu64Modified);
			pfWrite = &fWrite;
		}

		if (pu64Created)
		{
			fCreate = YXCLib::_GTimeToFileTime(*pu64Created);
			pfCreate = &fCreate;
		}

		if (pu64LastAccess)
		{
			fAccess = YXCLib::_GTimeToFileTime(*pu64LastAccess);
			pfAccess = &fAccess;
		}

		BOOL bRet = ::SetFileTime(this->_h, pfCreate, pfAccess, pfWrite);
		_YXC_CHECK_OS_RET(bRet, YC("Failed to set file('%s') time"), this->_fPath);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Lock(yuint64_t uLockStart, yuint64_t uLockSize)
	{
		OVERLAPPED ol = {0};
		ol.Offset = YXC_LO_32BITS(uLockStart);
		ol.OffsetHigh = YXC_HI_32BITS(uLockStart);

		BOOL bRet = ::LockFileEx(this->_h, LOCKFILE_EXCLUSIVE_LOCK, 0, YXC_LO_32BITS(uLockSize), YXC_HI_32BITS(uLockSize), &ol);
		_YXC_CHECK_OS_RET(bRet, YC("Lock file '%s' failed"), this->_fPath);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Unlock(yuint64_t uUnlockStart, yuint64_t uLockSize)
	{
		BOOL bRet = ::UnlockFile(this->_h, YXC_LO_32BITS(uUnlockStart), YXC_HI_32BITS(uUnlockStart),
			YXC_LO_32BITS(uLockSize), YXC_HI_32BITS(uLockSize));
		_YXC_CHECK_OS_RET(bRet, YC("Unlock file '%s' failed"), this->_fPath);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead)
	{
		BOOL bRet = ::ReadFile(this->_h, pData, uCbToRead, (LPDWORD)puRead, NULL);
		_YXC_CHECK_OS_RET(bRet, YC("Failed to read file('%s', %u)"), this->_fPath, uCbToRead);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten)
	{
		BOOL bRet = ::WriteFile(this->_h, pData, uCbToWrite, (LPDWORD)puWritten, NULL);
		_YXC_CHECK_OS_RET(bRet, YC("Failed to write file('%s', %u)"), this->_fPath, uCbToWrite);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Flush()
	{
		BOOL bRet = ::FlushFileBuffers(this->_h);
		_YXC_CHECK_OS_RET(bRet, YC("Failed to flush file('%s') buffers"), this->_fPath);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64CurPos)
	{
		DWORD dwSeek = (DWORD)seekMode - 1;
		LARGE_INTEGER liSeek;
		liSeek.QuadPart = i64SeekPos;
		BOOL bRet = ::SetFilePointerEx(this->_h, liSeek, (LARGE_INTEGER*)pi64CurPos, dwSeek);
		_YXC_CHECK_OS_RET(bRet, YC("Failed to seek file('%s':%d) to %lld"), seekMode, i64SeekPos);

		return YXC_ERC_SUCCESS;
	}

	void _OSFile::_FileAccessCvt(YXC_FileAccess fAccess, DWORD* pdwAccess, DWORD* pdwFileShare)
	{
		DWORD dwAccess = 0, dwFileShare = 0;
		if (fAccess & YXC_FACCESS_READ)
		{
			dwAccess |= GENERIC_READ;
		}

		if (fAccess & YXC_FACCESS_WRITE)
		{
			dwAccess |= GENERIC_WRITE;
		}

		if (fAccess & YXC_FACCESS_SHARED_READ)
		{
			dwFileShare |= FILE_SHARE_READ;
		}

		if (fAccess & YXC_FACCESS_SHARED_WRITE)
		{
			dwFileShare |= FILE_SHARE_WRITE;
			dwFileShare |= FILE_SHARE_DELETE;
		}

		if (fAccess & YXC_FACCESS_QUERY_INFO)
		{
			dwAccess |= FILE_READ_ATTRIBUTES;
		}

		*pdwAccess = dwAccess;
		*pdwFileShare = dwFileShare;
	}

	void _OSFile::_FileModeCvt(YXC_FOpenMode fMode, DWORD* pdwDisposition)
	{
		switch (fMode)
		{
		case YXC_FOPEN_CREATE_ALWAYS:
			*pdwDisposition = CREATE_ALWAYS;
			break;
		case YXC_FOPEN_CREATE_NEW:
			*pdwDisposition = CREATE_NEW;
			break;
		case YXC_FOPEN_APPEND:
		case YXC_FOPEN_OPEN_ALWAYS:
			*pdwDisposition = OPEN_ALWAYS;
			break;
		case YXC_FOPEN_OPEN_EXISTING:
		default:
			*pdwDisposition = OPEN_EXISTING;
			break;
		}
	}
#elif YXC_PLATFORM_UNIX

	void _OSFile::Close()
	{
        close(this->_fd);
	}

	YXC_Status _OSFile::Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access)
	{
		int flag;
		_FileAccessAndModeCvt(access, fMode, &flag);

		YXC_FPath exactedPath;
		YXC_FPathCopy(exactedPath, pszPath);
		YXC_FPathExact(exactedPath);

        this->_fd = open(exactedPath, flag, 0777);
		_YXC_CHECK_OS_RET(this->_fd >= 0, YC("Failed to create os file('%s')"), exactedPath);

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
//		if (fMode == YXC_FOPEN_APPEND)
//		{
//			YXC_Status rc = this->Seek(YXC_FSEEK_END, 0, NULL);
//			_YXC_CHECK_STATUS_GOTO(rc, YC("Seek os file('%s') for appending failed"), exactedPath);
//		}

		yh_strncpy(this->_fPath, exactedPath, YXC_MAX_CCH_PATH - 1);
		this->_fPath[YXC_MAX_CCH_PATH - 1] = 0;
		return YXC_ERC_SUCCESS;
    err_ret:
		close(this->_fd);
		return rcRet;
	}

    YXC_Status _OSFile::Lock(yuint64_t uLockStart, yuint64_t uLockSize)
    {
        struct flock fl;
        fl.l_start = uLockStart;
        fl.l_len = uLockSize;
        fl.l_type = F_WRLCK;
        fl.l_pid = getpid();
        fl.l_whence = SEEK_SET;

        int ret = fcntl(this->_fd, F_SETLKW, &fl);
        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to lock with file(%lld, %lld)"), uLockStart, uLockSize);

        return YXC_ERC_SUCCESS;
    }

    YXC_Status _OSFile::Unlock(yuint64_t uLockStart, yuint64_t uLockSize)
    {
        struct flock fl;
        fl.l_start = uLockStart;
        fl.l_len = uLockSize;
        fl.l_type = F_UNLCK;
        fl.l_pid = getpid();
        fl.l_whence = SEEK_SET;

        int ret = fcntl(this->_fd, F_SETLK, &fl);
        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to unlock with file(%lld, %lld)"), uLockStart, uLockSize);

        return YXC_ERC_SUCCESS;
    }

	YXC_Status _OSFile::GetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
	{
        struct stat st;
		int iStat = fstat(this->_fd, &st);
		_YXC_CHECK_CRT_RET(iStat == 0, YC("Failed to get file('%s') time flag"), this->_fPath);

		if (pu64Modified) *pu64Modified = st.st_mtime;
		if (pu64Created) *pu64Created = st.st_ctime;
		if (pu64LastAccess) *pu64LastAccess = st.st_atime;
		return YXC_ERC_SUCCESS;
	}

    YXC_Status _OSFile::GetFileSize(yuint64_t* puFSize)
    {
        struct stat st;
        int iStat = fstat(this->_fd, &st);
		_YXC_CHECK_CRT_RET(iStat == 0, YC("Failed to get file('%s') size flag"), this->_fPath);

        *puFSize = st.st_size;
        return YXC_ERC_SUCCESS;
    }

	YXC_Status _OSFile::Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead)
	{
		ssize_t ret = read(this->_fd, pData, uCbToRead);
		_YXC_CHECK_OS_RET(ret >= 0, YC("Failed to read file('%s', %u)"), this->_fPath, uCbToRead);

        *puRead = (yuint32_t)ret;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten)
	{
		ssize_t ret = write(this->_fd, pData, uCbToWrite);
		_YXC_CHECK_OS_RET(ret >= 0, YC("Failed to write file('%s', %u)"), this->_fPath, uCbToWrite);

        *puWritten = (yuint32_t)ret;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Flush()
	{
#if YXC_PLATFORM_APPLE
		int iRet = fls(this->_fd);
#else
		int iRet = fsync(this->_fd);
#endif /* YXC_PLATFORM_APPLE */
		_YXC_CHECK_OS_RET(iRet == 0, YC("Failed to flush file('%s') buffers"), this->_fPath);


		return YXC_ERC_SUCCESS;
	}

	YXC_Status _OSFile::Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64CurPos)
	{
		yuint32_t uSeek = (yuint32_t)seekMode - 1;
        off_t ret = lseek(this->_fd, i64SeekPos, uSeek);
		_YXC_CHECK_OS_RET(ret >= 0, YC("Failed to seek file('%s':%d) to %lld"), this->_fPath, seekMode, i64SeekPos);

        if (pi64CurPos) *pi64CurPos = ret;
		return YXC_ERC_SUCCESS;
	}

	void _OSFile::_FileAccessAndModeCvt(YXC_FileAccess fAccess, YXC_FOpenMode fMode, int* pflags)
	{
        int flags = 0;
		if (fAccess & YXC_FACCESS_READ)
		{
            if (fAccess & YXC_FACCESS_WRITE)
            {
                flags |= O_RDWR;
            }
            else
            {
                flags |= O_RDONLY;
            }
		}
        else if (fAccess & YXC_FACCESS_WRITE)
        {
            flags |= O_WRONLY;
        }

		switch (fMode)
		{
            case YXC_FOPEN_CREATE_ALWAYS:
                flags |= O_CREAT | O_TRUNC;
                break;
            case YXC_FOPEN_CREATE_NEW:
                flags |= O_CREAT | O_EXCL;
                break;
            case YXC_FOPEN_APPEND:
                flags |= O_CREAT;
                break;
            case YXC_FOPEN_OPEN_ALWAYS:
                flags |= O_CREAT;
                break;
            case YXC_FOPEN_OPEN_EXISTING:
            default:
                break;
		}

        *pflags = flags;
	}
#endif /* YXC_PLATFORM_WIN */
}

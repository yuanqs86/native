#define __MODULE__ "EK.OS.Utilities"

#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_FilePath.h>
#include <sys/stat.h>

namespace YXCLib
{
#if YXC_PLATFORM_WIN
	OSSharedMemory::OSSharedMemory() : _hFileMapping(NULL)
	{

	}

	OSSharedMemory::~OSSharedMemory()
	{
		this->Close();
	}

	YXC_Status OSSharedMemory::CreateA(const char* pszName, yuint64_t u64ShmSize, ybool_t* pbCreatedNew)
	{
		this->_hFileMapping = ::CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, YXC_HI_32BITS(u64ShmSize),
			YXC_LO_32BITS(u64ShmSize), pszName);

		_YXC_CHECK_OS_RET(this->_hFileMapping != NULL, L"Create file mapping '%S'(%d, %d) failed", pszName, YXC_HI_32BITS(u64ShmSize),
			YXC_LO_32BITS(u64ShmSize));

		*pbCreatedNew = ::GetLastError() != ERROR_ALREADY_EXISTS;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status OSSharedMemory::CreateW(const wchar_t* pszName, yuint64_t u64ShmSize, ybool_t* pbCreatedNew)
	{
		this->_hFileMapping = ::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, YXC_HI_32BITS(u64ShmSize),
			YXC_LO_32BITS(u64ShmSize), pszName);

		_YXC_CHECK_OS_RET(this->_hFileMapping != NULL, L"Create file mapping '%s'(%d, %d) failed", pszName, YXC_HI_32BITS(u64ShmSize),
			YXC_LO_32BITS(u64ShmSize));

		*pbCreatedNew = ::GetLastError() != ERROR_ALREADY_EXISTS;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status OSSharedMemory::OpenW(const wchar_t* pszName)
	{
		this->_hFileMapping = ::OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, pszName);
		_YXC_CHECK_OS_RET(this->_hFileMapping != NULL, L"Open file mapping '%s' failed", pszName);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status OSSharedMemory::Map(yuint64_t u64FileOffset, ysize_t stMapSize, void** ppRetMapped)
	{
		*ppRetMapped = ::MapViewOfFile(this->_hFileMapping, FILE_MAP_ALL_ACCESS, YXC_HI_32BITS(u64FileOffset), YXC_LO_32BITS(u64FileOffset), stMapSize);

		_YXC_CHECK_OS_RET(*ppRetMapped != NULL, L"Map memory(%llu-%lu) failed", u64FileOffset, stMapSize);
		return YXC_ERC_SUCCESS;
	}

	void OSSharedMemory::Unmap(void* pMappedPtr)
	{
		::UnmapViewOfFile(pMappedPtr);
	}

	YXC_Status OSSharedMemory::Close()
	{
		if (this->_hFileMapping)
		{
			BOOL bRet = ::CloseHandle(this->_hFileMapping);
			_YXC_CHECK_OS_RET(bRet, L"Close file mapping handle(%d) failed");

			this->_hFileMapping = NULL;
		}
		return YXC_ERC_SUCCESS;
	}

#else
// #error "Don't support OS shared memory"
#endif /* YXC_PLATFORM_WIN */

    OSFileFinder::OSFileFinder() : _bIsDir(FALSE)
    {
#if YXC_PLATFORM_UNIX
        this->_pDir = NULL;
#else
        this->_hFinder = INVALID_HANDLE_VALUE;
#endif /* YXC_PLATFORM_UNIX */
    }

    OSFileFinder::~OSFileFinder()
    {
        this->Close();
    }

    YXC_Status OSFileFinder::Open(const ychar* fPath)
    {
        yuint32_t uFlags;
        YXC_Status rc = YXC_FPathAttribute(fPath, &uFlags);
        _YXC_CHECK_RC_RET(rc);

        if (uFlags & YXC_FPATH_ATTR_DIR) /* Is directory. */
        {
            this->_bIsDir = TRUE;
        }

        YXC_FPathCopy(this->_fPath, fPath);
        return YXC_ERC_SUCCESS;
    }

#if YXC_PLATFORM_WIN
	void OSFileFinder::Close()
	{
		if (this->_hFinder != INVALID_HANDLE_VALUE)
		{
			FindClose(this->_hFinder);
			this->_hFinder = INVALID_HANDLE_VALUE;
		}
	}

	YXC_Status OSFileFinder::SelfAttr(OSFileAttr* attr)
	{
		struct _stat64 st;
		int ret = _wstat64(this->_fPath, &st);
		_YXC_CHECK_OS_RET(ret == 0, YC("Failed to find path('%s') attributes"), this->_fPath);

		attr->bIsDir = (st.st_mode & S_IFDIR) != 0;
		attr->fileSize = attr->bIsDir ? 0 : st.st_size;
		attr->createTime = st.st_ctime;
		attr->accessTime = st.st_atime;
		attr->modifyTime = st.st_mtime;
		YXC_FPathCopy(attr->osFilePath, YC(""));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status OSFileFinder::Next(OSFileAttr* attr)
	{
		WIN32_FIND_DATAW fData;
		if (this->_hFinder == INVALID_HANDLE_VALUE)
		{
			wchar_t szFound[YXC_MAX_CCH_PATH] = {0};
			YXC_FPathCombine(szFound, this->_fPath, YC("\\*.*"));
			this->_hFinder = FindFirstFileW(szFound, &fData);
			_YXC_CHECK_REPORT_NEW_RET(this->_hFinder != INVALID_HANDLE_VALUE, YXC_ERC_NO_DATA, YC("Failed to find first file"));
		}
		else
		{
			BOOL bRet = FindNextFileW(this->_hFinder, &fData);
			if (!bRet)
			{
				DWORD dwLastError = GetLastError();
				_YXC_CHECK_OS_RET(dwLastError == ERROR_NO_MORE_FILES, YC("Find next file failed"));
				_YXC_REPORT_NEW_RET(YXC_ERC_NO_DATA, YC("No more files can be found"));
			}
		}

		if (wcscmp(fData.cFileName, L"..") == 0 || wcscmp(fData.cFileName, L".") == 0) /* Relative path, recursive. */
		{
			return this->Next(attr);
		}

		attr->bIsDir = fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		attr->fileSize = YXC_MK64BITS_32(fData.nFileSizeHigh, fData.nFileSizeLow);
		attr->accessTime = YXCLib::_FileTimeToGTime(fData.ftLastAccessTime);
		attr->createTime = YXCLib::_FileTimeToGTime(fData.ftCreationTime);
		attr->modifyTime = YXCLib::_FileTimeToGTime(fData.ftLastWriteTime);
		YXC_FPathCopy(attr->osFilePath, fData.cFileName);
		return YXC_ERC_SUCCESS;
	}
#else
    void OSFileFinder::Close()
    {
        if (this->_pDir != NULL)
        {
            closedir(this->_pDir);
            this->_pDir = NULL;
        }
    }

    YXC_Status OSFileFinder::SelfAttr(OSFileAttr* attr)
    {
        struct stat st;
        int ret = stat(this->_fPath, &st);
        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to find path('%s') attributes"), this->_fPath);

        attr->bIsDir = S_ISDIR(st.st_mode);
        attr->fileSize = attr->bIsDir ? 0 : st.st_size;
        attr->createTime = st.st_ctime;
        attr->accessTime = st.st_atime;
        attr->modifyTime = st.st_mtime;
        YXC_FPathCopy(attr->osFilePath, this->_fPath);

        return YXC_ERC_SUCCESS;
    }

    YXC_Status OSFileFinder::Next(OSFileAttr* attr)
    {
        if (this->_pDir == NULL)
        {
            this->_pDir = opendir(this->_fPath);
            _YXC_CHECK_OS_RET(this->_pDir != NULL, YC("Failed to open directory('%s')"), this->_fPath);
        }

        errno = 0; /* reset err code value. */
        dirent* de = readdir(this->_pDir);
        _YXC_CHECK_OS_RET(errno == 0, YC("Failed to read the directory('%s')"), this->_fPath);
        _YXC_CHECK_REPORT_NEW_RET(de != NULL, YXC_ERC_NO_DATA, YC("Directory end reached."));

        struct stat st;
        int ret = stat(de->d_name, &st);
        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to find path('%s') attributes"), de->d_name);

        attr->bIsDir = S_ISDIR(st.st_mode);
        attr->fileSize = attr->bIsDir ? 0 : st.st_size;
        attr->createTime = st.st_ctime;
        attr->accessTime = st.st_atime;
        attr->modifyTime = st.st_mtime;
        YXC_FPathCopy(attr->osFilePath, de->d_name);

        return YXC_ERC_SUCCESS;
    }
#endif /* YXC_PLATFORM_WIN */
}

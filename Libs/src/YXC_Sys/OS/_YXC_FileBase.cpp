#define __MODULE__ "EK.File.Base"

#include <YXC_Sys/OS/_YXC_FileBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <sys/stat.h>

#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#endif /* YXC_PLATFORM_WIN */

#if YXC_PLATFORM_UNIX
#include <sys/time.h>
#endif /* YXC_PLATFORM_UNIX */

namespace YXC_InnerF
{
	YXC_Status _FileBase::GetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
	{
#if !YCHAR_WCHAR_T
		struct stat st;
		int iStat = stat(this->_fPath, &st);
		_YXC_CHECK_CRT_RET(iStat == 0, YC("Failed to get file('%s') time flag"), this->_fPath);
#else
		//struct stat st;
		struct _stat64 st;
		int iStat = _wstat64(this->_fPath, &st);
		_YXC_CHECK_CRT_RET(iStat == 0, YC("Failed to get file('%s') time flag"), this->_fPath);
#endif /* YCHAR_WCHAR_T */

		*pu64Modified = st.st_mtime;
		*pu64Created = st.st_ctime;
		*pu64LastAccess = st.st_atime;
		return YXC_ERC_SUCCESS;
	}

    YXC_Status _FileBase::SetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess)
    {
#if YXC_PLATFORM_UNIX
		struct timeval tm[2];

        if (pu64Modified)
        {
            tm[0].tv_sec = *pu64Modified / 1000;
            tm[1].tv_sec = *pu64Modified / 1000;
        }
        else if (pu64LastAccess)
        {
            tm[0].tv_sec = *pu64LastAccess / 1000;
            tm[1].tv_sec = *pu64LastAccess / 1000;
        }
        int ret = utimes(this->_fPath, tm);
        _YXC_CHECK_CRT_RET(ret == 0, YC("Failed to call utimes %s"), this->_fPath);
#else
		_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Not supported"));
#endif /* YXC_PLATFORM_UNIX */

        return YXC_ERC_SUCCESS;
    }

    YXC_Status _FileBase::GetFileSize(yuint64_t* puFSize)
	{
#if !YCHAR_WCHAR_T
		struct stat st;
		int iStat = stat(this->_fPath, &st);
		_YXC_CHECK_CRT_RET(iStat == 0, YC("Failed to get file('%s') size flag"), this->_fPath);
#else
		//struct stat st;
		struct _stat64 st;
		int iStat = _wstat64(this->_fPath, &st);
		_YXC_CHECK_CRT_RET(iStat == 0, YC("Failed to get file('%s') size flag"), this->_fPath);
#endif /* YCHAR_WCHAR_T */

        *puFSize = st.st_size;
        return YXC_ERC_SUCCESS;
    }
}

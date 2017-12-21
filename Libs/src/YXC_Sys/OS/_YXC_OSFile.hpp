#ifndef __INNER_INC_YXC_SYS_BASE_OS_FILE_HPP__
#define __INNER_INC_YXC_SYS_BASE_OS_FILE_HPP__

#include <YXC_Sys/YXC_File.h>
#include <YXC_Sys/OS/_YXC_FileBase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXC_InnerF
{
	class _OSFile : public _FileBase
	{
	public:
		virtual YXC_Status Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access);

		virtual void Close();

		virtual YXC_Status Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead);

		virtual YXC_Status Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten);

		virtual YXC_Status Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64CurPos);

		virtual YXC_Status Flush();

        virtual YXC_Status Lock(yuint64_t uLockStart, yuint64_t uLockSize);

        virtual YXC_Status Unlock(yuint64_t uUnlockStart, yuint64_t uLockSize);

		virtual YXC_Status GetFileSize(yuint64_t* pu64FSize);

		virtual YXC_Status GetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess);

#if YXC_PLATFORM_WIN
		virtual YXC_Status SetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess);
#endif /* YXC_PLATFORM_WIN */

#if YXC_PLATFORM_WIN
	protected:
		HANDLE _h;

	protected:
		static void _FileAccessCvt(YXC_FileAccess fAccess, DWORD* pdwAccess, DWORD* pdwFileShare);

		static void _FileModeCvt(YXC_FOpenMode fMode, DWORD* pdwDisposition);

#elif YXC_PLATFORM_UNIX
    protected:
        int _fd;

    protected:
        static void _FileAccessAndModeCvt(YXC_FileAccess fAccess, YXC_FOpenMode fMode, int* pflags);
#endif /* YXC_PLATFORM_WIN */
	};

}

#endif /* __INNER_INC_YXC_SYS_BASE_OS_FILE_HPP__ */

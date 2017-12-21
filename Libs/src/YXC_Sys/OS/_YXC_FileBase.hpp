#ifndef __INNER_INC_YXC_SYS_BASE_FILE_HPP__
#define __INNER_INC_YXC_SYS_BASE_FILE_HPP__

#include <YXC_Sys/YXC_File.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXC_InnerF
{
	class _FileBase
	{
	public:
		virtual YXC_Status Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access) = 0;

		virtual void Close() = 0;

		virtual YXC_Status Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead) = 0;

		virtual YXC_Status Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten) = 0;

		virtual YXC_Status Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* iRealPos) = 0;

		virtual YXC_Status Flush() = 0;

        virtual YXC_Status Lock(yuint64_t uLockStart, yuint64_t uLockSize) = 0;

        virtual YXC_Status Unlock(yuint64_t uUnlockStart, yuint64_t uLockSize) = 0;

		virtual YXC_Status GetFileSize(yuint64_t* pu64FSize);

		virtual YXC_Status GetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess);

		virtual YXC_Status SetFileTime(yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess);

	public:
		inline const ychar* GetPath() const { return this->_fPath; }

	protected:
		YXC_FPath _fPath;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_File, _FileBase, _FilePtr, _FileHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_FILE_HPP__ */

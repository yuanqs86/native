#ifndef __INNER_INC_YXC_SYS_BASE_STDC_FILE_HPP__
#define __INNER_INC_YXC_SYS_BASE_STDC_FILE_HPP__

#include <YXC_Sys/OS/_YXC_FileBase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <stdio.h>

namespace YXC_InnerF
{
	class _StdCFile : public _FileBase
	{
	public:
		virtual YXC_Status Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access);

		virtual void Close();

		virtual YXC_Status Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead);

		virtual YXC_Status Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten);

		virtual YXC_Status Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64CurPos);

        virtual YXC_Status Lock(yuint64_t uLockStart, yuint64_t uLockSize);

        virtual YXC_Status Unlock(yuint64_t uUnlockStart, yuint64_t uLockSize);

		virtual YXC_Status Flush();

		virtual YXC_Status GetFileSize(yuint64_t* pu64FSize);

	protected:
		FILE* _fp;

	protected:
		static YXC_Status _GetFOpenFlags(ychar* pszFlag, YXC_FOpenMode fMode, YXC_FileAccess access);
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_STDC_FILE_HPP__ */

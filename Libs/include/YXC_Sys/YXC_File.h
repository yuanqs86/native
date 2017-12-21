/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_FILE_H__
#define __INC_YXC_SYS_BASE_FILE_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef enum __YXC_FILE_OPEN_MODE
	{
		YXC_FOPEN_UNKNOWN = 0,
		YXC_FOPEN_CREATE_NEW,
		YXC_FOPEN_APPEND,
		YXC_FOPEN_CREATE_ALWAYS,
		YXC_FOPEN_OPEN_ALWAYS,
		YXC_FOPEN_OPEN_EXISTING,
	}YXC_FOpenMode;

	typedef enum __YXC_FILE_ACCESS_MODE
	{
		YXC_FACCESS_NONE = 0,
		YXC_FACCESS_WRITE = 0x1,
		YXC_FACCESS_READ = 0x2,
		YXC_FACCESS_READ_WRITE = 0x3,
		YXC_FACCESS_SHARED_WRITE = 0x4,
		YXC_FACCESS_GEN_WRITE = 0x5,
		YXC_FACCESS_SHARED_READ = 0x8,
		YXC_FACCESS_GEN_READ = 0xa,
		YXC_FACCESS_GEN_ALL = 0xf,
		YXC_FACCESS_QUERY_INFO = 0x10,
		YXC_FACCESS_DIR = 0x20,
	}YXC_FileAccess;

	typedef enum __YXC_FILE_SEEK_MODE
	{
		YXC_FSEEK_NONE = 0,
		YXC_FSEEK_BEGIN,
		YXC_FSEEK_CUR,
		YXC_FSEEK_END
	}YXC_FSeekMode;

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_File);

	YXC_API(YXC_Status) YXC_FileCreateA(const char* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccessMode, ybool_t bOSFile, YXC_File* pFile);

	YXC_API(YXC_Status) YXC_FileCreateW(const wchar_t* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccessMode, ybool_t bOSFile, YXC_File* pFile);

    YXC_API(YXC_Status) YXC_FileCreate(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccessMode, ybool_t bOSFile, YXC_File* pFile);

	YXC_API(YXC_Status) YXC_FileRead(YXC_File file, yuint32_t uCbToRead, void* pData, yuint32_t* puCbRead);

	YXC_API(YXC_Status) YXC_FileWrite(YXC_File file, yuint32_t uCbToWrite, const void* pData, yuint32_t* puCbWritten);

    YXC_API(YXC_Status) YXC_FileLock(YXC_File file);

    YXC_API(YXC_Status) YXC_FileUnlock(YXC_File file);

	YXC_API(void) YXC_FileGetPath(YXC_File file, YXC_FPath* pFPath);

	YXC_API(YXC_Status) YXC_FileSeek(YXC_File file, YXC_FSeekMode fSeekMode, yint64_t i64SeekTo, yint64_t* pi64CurPos);

	YXC_API(YXC_Status) YXC_FileFlush(YXC_File file);

	YXC_API(YXC_Status) YXC_FileGetSize(YXC_File file, yuint64_t* pu64FileSize);

	YXC_API(YXC_Status) YXC_FileGetTimes(YXC_File file, yuint64_t* pu64Modified, yuint64_t* pu64Created,
		yuint64_t* pu64LastAccess);

    YXC_API(YXC_Status) YXC_FileSetTimes(YXC_File file, yuint64_t* pu64Modified, yuint64_t* pu64Created, yuint64_t* pu64LastAccess);

	YXC_API(void) YXC_FileClose(YXC_File file);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_FILE_H__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_TEXT_FILE_H__
#define __INC_YXC_SYS_BASE_TEXT_FILE_H__

#include <YXC_Sys/YXC_File.h>
#include <YXC_Sys/YXC_TextEncoding.h>

extern "C"
{
	YXC_DECLARE_INHERIT_HANDLE(YXC_TextFile, YXC_File);

    YXC_API(YXC_Status) YXC_TextFileCreate(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile, YXC_TEncoding reqEncoding,
                                YXC_TEncoding* pEncoding, YXC_TextFile* pFile);

	YXC_API(YXC_Status) YXC_TextFileCreateA(const char* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile,
		YXC_TEncoding reqEncoding, YXC_TEncoding* pEncoding, YXC_TextFile* pFile);

	YXC_API(YXC_Status) YXC_TextFileCreateW(const wchar_t* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile,
		YXC_TEncoding reqEncoding, YXC_TEncoding* pEncoding, YXC_TextFile* pFile);

	YXC_API(YXC_Status) YXC_TextFileRead(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcBuf, void* pStr, yuint32_t* puCcToRead);

	YXC_API(YXC_Status) YXC_TextFileReadLine(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcLineBuf, void* pLine, yuint32_t* puCcLine);

	YXC_API(YXC_Status) YXC_TextFileWrite(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcWrite, const void* pStr);

	YXC_API(YXC_Status) YXC_TextFileWriteLine(YXC_TextFile file, YXC_TEncoding enc, yuint32_t uCcLine, const void* pLine);
};

#endif /* __INC_YXC_SYS_BASE_TEXT_FILE_H__ */

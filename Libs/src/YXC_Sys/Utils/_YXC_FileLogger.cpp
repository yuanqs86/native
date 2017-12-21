#define __MODULE__ "EK.Logger.FileLogger"

#include <stdio.h>
#include <time.h>

#include <YXC_Sys/Utils/_YXC_FileLogger.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_FilePath.h>
#include <YXC_Sys/YXC_TextEncoding.h>

#if YXC_PLATFORM_WIN
#include <WinSock2.h>
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
	_FileLogger::_FileLogger() : _crit(1000)
	{

	}

	_FileLogger::~_FileLogger()
	{

	}

	YXC_Status _FileLogger::LogFormat(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
		const ychar* cpszFormat, va_list vaList, ybool_t bReportLastError)
	{
		ychar szMessageBuffer[_YXC_LOGGER_BUF_SIZE], *pMsgBuf = szMessageBuffer;
		ysize_t stSizeBuf = _YXC_LOGGER_BUF_SIZE;

		YXC_Status rc = _Log_Convert(logLevel, cpszTitle, cpszModule, cpszFormat, vaList, TRUE, this->_bAppendFileSpec,
			bReportLastError, &pMsgBuf, &stSizeBuf);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Convert log format[%s] failed"), cpszTitle);

		rc = this->_WriteMessage(pMsgBuf, (yuint32_t)stSizeBuf);
		if (pMsgBuf != szMessageBuffer)
		{
			_Log_FreeBuffer(pMsgBuf);
		}
		return rc;
	}

	YXC_Status _FileLogger::Log(YXC_LoggerLogLevel logLevel, const ychar* cpszModule, const ychar* cpszTitle,
		const ychar* cpszMessage, ybool_t bReportLastError)
	{
		ychar szMessageBuffer[_YXC_LOGGER_BUF_SIZE], *pMsgBuf = szMessageBuffer;

		ysize_t stSizeBuf = _YXC_LOGGER_BUF_SIZE;

		YXC_Status rc = _Log_Convert(logLevel, cpszTitle, cpszModule, cpszMessage, TRUE, this->_bAppendFileSpec,
			bReportLastError, &pMsgBuf, &stSizeBuf);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Convert log format[%s] failed"), cpszTitle);

		rc = this->_WriteMessage(pMsgBuf, (yuint32_t)stSizeBuf);
		if (pMsgBuf != szMessageBuffer)
		{
			_Log_FreeBuffer(pMsgBuf);
		}
		return rc;
	}

    YXC_Status _FileLogger::Create(const YXC_LoggerCreateInfo* pInfo)
    {
		ychar szDirPath[YXC_MAX_CCH_PATH], *pszPath = NULL;
        YXC_Status rc;
		if (pInfo == NULL || pInfo->cpszFilePath == NULL)
		{
			ychar szPath[YXC_MAX_CCH_PATH], szFilename[YXC_MAX_CCH_PATH];

			YXC_FPathFindModulePath(szFilename, NULL);
			const ychar* pszFilePart = YXC_FPathFindFileSpec(szFilename);

            timeval tm_day;
            yxcwrap_gettimeofday(&tm_day);

			time_t time_seconds = tm_day.tv_sec;
            tm* tm = localtime(&time_seconds);

            rc = YXC_FPathFindSpecDir(szDirPath, YXC_FPATH_SDIR_APPDATA);
            YXC_FPathAppend(szDirPath, YC("YXCLib"));
            YXC_FPathAppend(szDirPath, YC("Logs"));
            YXC_FPathAppend(szDirPath, pszFilePart);

			rc = YXC_FPathCreateDir(szDirPath, NULL, TRUE);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS || rc == YXC_ERC_ALREADY_EXISTS, rc, YC("Failed to create log path('%s')"), szDirPath);

			ychar szPostFix[100];
			yh_sprintf(szPostFix, YC("%04d-%02d-%02d %02d:%02d:%02d:%03d.ylog"), tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                       tm->tm_hour, tm->tm_min, tm->tm_sec, tm_day.tv_usec / 1000);

			YXC_FPathCombine(szPath, szDirPath, szPostFix);
			pszPath = szPath;
			this->_bAppendFileSpec = FALSE;
		}
		else
		{
			pszPath = (ychar*)pInfo->cpszFilePath;
			this->_bAppendFileSpec = TRUE;

			YXC_FPathCopy(szDirPath, pInfo->cpszFilePath);
			YXC_FPathRemoveFileSpec(szDirPath);
		}

		this->_DeleteOldLogs(szDirPath);

        YXC_TEncoding tEnc;
        rc = YXC_TextFileCreate(pszPath, YXC_FOPEN_APPEND, YXC_FACCESS_GEN_ALL, TRUE, YXC_TENCODING_UTF8, &tEnc, &this->_txtFile);
        _YXC_CHECK_RC_RET(rc);

        YXC_FPathCopy(this->_sFilePath, pszPath);

//		this->_sFilePath.assign(pszPath);
//		this->_hLogFile = h;
		return YXC_ERC_SUCCESS;
    }

	YXC_Status _FileLogger::CreateA(const YXC_LoggerCreateInfoA* pInfo)
	{
#if YCHAR_WCHAR_T
		if (pInfo != NULL && pInfo->cpszFilePath != NULL)
		{
			const char* cpszPath = pInfo->cpszFilePath;

			ychar szPath[YXC_MAX_CCH_PATH] = {0};
			YXC_TECharToWChar(cpszPath, YXC_STR_NTS, szPath, YXC_MAX_CCH_PATH - 1, NULL, NULL);

			YXC_LoggerCreateInfo info;
			info.cpszFilePath = szPath;
			return this->Create(&info);
		}
		else
		{
			return this->Create(NULL);
		}
#else
        return this->Create(pInfo);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status _FileLogger::CreateW(const YXC_LoggerCreateInfoW* pInfo)
	{
#if YCHAR_WCHAR_T
        return this->Create(pInfo);
#else
		if (pInfo != NULL && pInfo->cpszFilePath != NULL)
		{
			const wchar_t* cpszPath = pInfo->cpszFilePath;

			ychar szPath[YXC_MAX_CCH_PATH] = {0};
			YXC_TEWCharToChar(cpszPath, YXC_STR_NTS, szPath, YXC_MAX_CCH_PATH - 1, NULL, NULL);

			YXC_LoggerCreateInfo info;
			info.cpszFilePath = szPath;
			return this->Create(&info);
		}
		else
		{
			return this->Create(NULL);
		}
#endif /* YCHAR_WCHAR_T */
	}

	void _FileLogger::Close()
	{
        YXC_FileClose(this->_txtFile);
	}

	YXC_Status _FileLogger::_WriteMessage(const ychar* cpszMsg, yuint32_t uCchBuffer)
	{
        YXX_CritLocker locker(this->_crit);

        YXC_Status rc = YXC_FileLock(this->_txtFile);
        _YXC_CHECK_RC_RET(rc);

        YXCLib::HandleRef<YXC_TextFile> tfDtor(this->_txtFile, (YXCLib::HandleRef<YXC_TextFile>::DestroyFunc)YXC_FileUnlock);

        rc = YXC_FileSeek(this->_txtFile, YXC_FSEEK_END, 0, NULL);
        _YXC_CHECK_RC_RET(rc);

        rc = YXC_TextFileWrite(this->_txtFile, YXC_TENCODING_ECHAR, uCchBuffer, cpszMsg);
        _YXC_CHECK_RC_RET(rc);

        return YXC_ERC_SUCCESS;
	}

    void _FileLogger::_DeleteOldLogs(const ychar* pszDir)
    {
        yuint32_t uLogDeleteSecs = 720 * 60 * 60;

        YXCLib::OSFileFinder fFinder;
        YXC_Status rc = fFinder.Open(pszDir);
        if (rc != YXC_ERC_SUCCESS) return;

        yuint64_t time_cur = time(NULL);
        while (TRUE)
        {
            YXCLib::OSFileAttr attr;
            rc = fFinder.Next(&attr);

            if (rc != YXC_ERC_SUCCESS) break;

			const ychar* extenstion = YXC_FPathFindExtension(attr.osFilePath);

            if (attr.modifyTime + uLogDeleteSecs < time_cur && yh_stricmp(extenstion, YC(".ylog")) == 0) /* is an old log to delete. */
            {
				YXC_FPath fReal;
				YXC_FPathCombine(fReal, pszDir, attr.osFilePath);
                YXC_FPathDelete(fReal, TRUE);
            }
        }
    }
}

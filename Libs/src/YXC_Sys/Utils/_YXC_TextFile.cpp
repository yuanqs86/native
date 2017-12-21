#define __MODULE__ "EK.File.Text"
#include <YXC_Sys/Utils/_YXC_TextFile.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/Utils/_YXC_TextConverter.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/OS/_YXC_OSFile.hpp>
#include <YXC_Sys/OS/_YXC_StdCFile.hpp>
#include <YXC_Sys/Net/_YXC_NetCommon.hpp>
#include <new>
#include <YXC_Sys/YXC_FilePath.h>

using namespace YXC_Inner;

namespace YXC_InnerF
{
	YXC_Status _TextFile::Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access)
	{
		return this->_pFileImpl->Create(pszPath, fMode, access);
	}

	void _TextFile::Close()
	{
		this->_pFileImpl->Close();
		this->_pFileImpl->~_FileBase();
		if (this->_pFileBuf)
		{
			free(this->_pFileBuf);
		}
		free(this->_pFileImpl);
	}

	YXC_Status _TextFile::Flush()
	{
		return this->_pFileImpl->Flush();
	}

    YXC_Status _TextFile::Lock(yuint64_t uLockStart, yuint64_t uLockSize)
    {
        return this->_pFileImpl->Lock(uLockStart, uLockSize);
    }

    YXC_Status _TextFile::Unlock(yuint64_t uLockStart, yuint64_t uLockSize)
    {
        return this->_pFileImpl->Unlock(uLockStart, uLockSize);
    }

	YXC_Status _TextFile::GetFileSize(yuint64_t* pu64FSize)
	{
		YXX_CritLocker locker(this->_crit);
		YXC_Status rc = this->_pFileImpl->GetFileSize(pu64FSize);
		_YXC_CHECK_RC_RET(rc);

		*pu64FSize -= this->_uCbHeader;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _TextFile::Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64RealPos)
	{
		YXX_CritLocker locker(this->_crit);
		YXC_Status rc;
		yint64_t iRealPos;

		if (seekMode == YXC_FSEEK_CUR)
		{
			i64SeekPos = i64SeekPos + this->_u64FilePos - this->_uCcAvailable * this->_uChBytes;
			rc = this->_pFileImpl->Seek(YXC_FSEEK_BEGIN, i64SeekPos + this->_uCbHeader, &iRealPos);
		}
		else if (seekMode == YXC_FSEEK_BEGIN)
		{
			rc = this->_pFileImpl->Seek(seekMode, i64SeekPos + this->_uCbHeader, &iRealPos);
		}
		else
		{
			rc = this->_pFileImpl->Seek(seekMode, i64SeekPos, &iRealPos);
		}
		_YXC_CHECK_RC_RETP(rc);

		this->_u64FilePos = iRealPos - this->_uCbHeader;
		if (pi64RealPos) *pi64RealPos = this->_u64FilePos;
		this->_uCcAvailable = 0; /* Invalid buffer status after seek. */
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _TextFile::Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead)
	{
		YXX_CritLocker locker(this->_crit);
		return this->_pFileImpl->Read(uCbToRead, pData, puRead);
	}

	YXC_Status _TextFile::Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten)
	{
		YXX_CritLocker locker(this->_crit);
		return this->_pFileImpl->Write(uCbToWrite, pData, puWritten);
	}

	YXC_Status _TextFile::ReadText(YXC_TEncoding enc, yuint32_t uCcToRead, void* pStr, yuint32_t* puCcToRead)
	{
        if (enc == YXC_TENCODING_CHAR) enc = gs_TEncChar;
        else if (enc == YXC_TENCODING_WCHAR) enc = gs_TEncWChar;

		yuint32_t uChBytes = this->_uChBytes, uIndex = this->_uEncodingIndex;
		_GetTEncodingInfo(enc, &uIndex, &uChBytes);

		YXX_CritLocker locker(this->_crit);
		return this->_ReadText(this->_pfnCvtReadProc[uIndex], uChBytes, uCcToRead, (ybyte_t*)pStr, puCcToRead);
	}

	YXC_Status _TextFile::ReadLine(YXC_TEncoding enc, yuint32_t uCcToRead, void* pStr, yuint32_t* puCcToRead)
	{
        if (enc == YXC_TENCODING_CHAR) enc = gs_TEncChar;
        else if (enc == YXC_TENCODING_WCHAR) enc = gs_TEncWChar;

		yuint32_t uChBytes = this->_uChBytes, uIndex = this->_uEncodingIndex;
		_GetTEncodingInfo(enc, &uIndex, &uChBytes);

		YXX_CritLocker locker(this->_crit);
		return this->_ReadTextLine(this->_pfnConvertLineProc[uIndex], uChBytes, uCcToRead, (ybyte_t*)pStr, puCcToRead);
	}

	YXC_Status _TextFile::WriteText(YXC_TEncoding enc, yuint32_t uCcWrite, const void* pStr)
	{
        if (enc == YXC_TENCODING_CHAR) enc = gs_TEncChar;
        else if (enc == YXC_TENCODING_WCHAR) enc = gs_TEncWChar;

		yuint32_t uChBytes = this->_uChBytes, uIndex = this->_uEncodingIndex;
		_GetTEncodingInfo(enc, &uIndex, &uChBytes);

		YXX_CritLocker locker(this->_crit);
		return this->_WriteText(this->_pfnCvtWriteProc[uIndex], uChBytes, uCcWrite, (const ybyte_t*)pStr);
	}

	YXC_Status _TextFile::WriteLine(YXC_TEncoding enc, yuint32_t uCcLine, const void* pLine)
	{
        if (enc == YXC_TENCODING_CHAR) enc = gs_TEncChar;
        else if (enc == YXC_TENCODING_WCHAR) enc = gs_TEncWChar;

		yuint32_t uChBytes = this->_uChBytes, uIndex = this->_uEncodingIndex;
		_GetTEncodingInfo(enc, &uIndex, &uChBytes);

		YXX_CritLocker locker(this->_crit);
		YXC_Status rc = this->_WriteText(this->_pfnCvtWriteProc[uIndex], uChBytes, uCcLine, (const ybyte_t*)pLine);
		_YXC_CHECK_RC_RET(rc);

		/* Append line end with ansi. */
		const char* pszLineEnd = YXCLib::_ChTraits<char>::LINE_END;
		yuint32_t uIxAnsi = _TEncodingToIndex(YXC_TENCODING_ANSI);
		rc = this->_WriteText(this->_pfnCvtWriteProc[uIxAnsi], sizeof(char), strlen(pszLineEnd), (const ybyte_t*)pszLineEnd);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	/* Don't use virtual function in this function, not initialized yet. */
	YXC_Status _TextFile::CreateText(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile,
		YXC_TEncoding reqEncoding, YXC_TEncoding* pEncoding)
	{
		YXC_File file = NULL;
		YXC_Status rc = YXC_FileCreate(pszPath, fMode, fAccess, bOSFile, &file);
		_YXC_CHECK_RC_RETP(rc);

        if (reqEncoding == YXC_TENCODING_CHAR) reqEncoding = gs_TEncChar;
        else if (reqEncoding == YXC_TENCODING_WCHAR) reqEncoding = gs_TEncWChar;

		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		yuint32_t uRealHeaderBytes = 0;
		yint64_t i64FilePos = 0;
		if (fMode == YXC_FOPEN_CREATE_ALWAYS || fMode == YXC_FOPEN_CREATE_NEW) /* Force write new encoding. */
		{
			rc = this->_WriteAndSetEncoding(file, reqEncoding, &uRealHeaderBytes);
			_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to write file('%s') encoding(%d) headers"), pszPath, reqEncoding);
		}
		else /* Read only!. */
		{
			ybyte_t byHeader[4];
			yuint32_t uCbRead;
			YXC_Status rcRead = YXC_FileRead(file, 4, byHeader, &uCbRead);
			_YXC_CHECK_STATUS_GOTO(rcRead, YC("Failed to read file('%s') header"), pszPath);

			if (uCbRead == 0 && (fAccess & YXC_FACCESS_WRITE)) /* Empty file and with write access, Convert to newly encoding. */
			{
				rc = this->_WriteAndSetEncoding(file, reqEncoding, &uRealHeaderBytes);
				_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to write file('%s') encoding(%d) headers"), pszPath, reqEncoding);
			}
			else
			{
				this->_ParseAndSetEncoding(reqEncoding, uCbRead, byHeader, &uRealHeaderBytes);
				if (fMode == YXC_FOPEN_APPEND)
				{
					rc = YXC_FileSeek(file, YXC_FSEEK_END, 0, &i64FilePos);
					i64FilePos -= uRealHeaderBytes;
				}
				else
				{
					rc = YXC_FileSeek(file, YXC_FSEEK_BEGIN, uRealHeaderBytes, NULL);
				}
				_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to seek file('%s') to req pos"), pszPath);
			}
		}

		rc = _TextConverter::FillConverterFuncs(this->_encoding, this->_pfnCvtReadProc, this->_pfnCvtWriteProc,
			this->_pfnConvertLineProc);
		_YXC_CHECK_STATUS_GOTO(rc, YC("Failed to fill converter functions with encoding(%d)"), this->_encoding);
		if (pEncoding)
		{
			*pEncoding = this->_encoding;
		}
		YXC_FPathCopy(this->_fPath, pszPath);
		this->_pFileImpl = _FilePtr(file);
		this->_uCbHeader = uRealHeaderBytes;
		this->_u64FilePos = i64FilePos;
		this->_uCcOff = 0;
		this->_uCcAvailable = 0;
		this->_pFileBuf = NULL;
		return YXC_ERC_SUCCESS;
err_ret:
		YXC_FileClose(file);
		return rcRet;
	}

	YXC_Status _TextFile::_ReadBasicBytes()
	{
		if (this->_pFileBuf == NULL)
		{
			_YCHK_MAL_ARR_R2(this->_pFileBuf, ybyte_t, DEF_TEXT_BUF * 2);
			this->_pWriteBuf = this->_pFileBuf + DEF_TEXT_BUF;
		}

		yuint32_t uSkipBytes = 0;
		if (this->_uCcAvailable > 0)
		{
			uSkipBytes = this->_uCcAvailable * this->_uChBytes;

			if (this->_uCcOff > 0)
			{
				memmove(this->_pFileBuf, this->_pFileBuf + this->_uCcOff * this->_uChBytes,
					uSkipBytes);
			}
		}
		this->_uCcOff = 0;
		yuint32_t uNumBytesRead;
		YXC_Status rc = this->_pFileImpl->Read(DEF_TEXT_BUF - uSkipBytes, this->_pFileBuf + uSkipBytes, &uNumBytesRead);
		_YXC_CHECK_STATUS_RET(rc, YC("Read basic bytes(%u) failed"), DEF_TEXT_BUF - uSkipBytes);
		_YXC_CHECK_REPORT_NEW_RET(uNumBytesRead > 0, YXC_ERC_EOF, YC("End of file('%s') arrived"), this->_fPath);

		this->_u64FilePos += uNumBytesRead;
		this->_uCcAvailable += uNumBytesRead / this->_uChBytes;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _TextFile::_ReadText(_TextConvertProc pfnConvert, yuint32_t uCvtChBytes, yuint32_t uCcToRead, ybyte_t* pPtr, yuint32_t* puCcToRead)
	{
		yint64_t iOldPos = this->_u64FilePos + this->_uCcOff * this->_uChBytes;
		yuint32_t uTotalConverted = 0;
		YXC_Status rcRet = YXC_ERC_UNKNOWN, rc;
		ybyte_t* pPtrCvt = pPtr;

		if (this->_uCcAvailable == 0)
		{
			rc = this->_ReadBasicBytes();
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}

		while (TRUE)
		{
			yuint32_t uDataUsed;
			yuint32_t uConverted;
			pfnConvert(this->_pFileBuf + this->_uCcOff * this->_uChBytes, this->_uCcAvailable, pPtrCvt,
				uCcToRead - uTotalConverted, &uDataUsed, &uConverted);

			uTotalConverted += uConverted;
			this->_uCcOff += uDataUsed;
			this->_uCcAvailable -= uDataUsed;
			pPtrCvt += uConverted * uCvtChBytes;

			if (uTotalConverted == uCcToRead) /* Reach limit num bytes or line ended. */
			{
				*puCcToRead = uTotalConverted;
				return YXC_ERC_SUCCESS;
			}

			rc = this->_ReadBasicBytes();
			if (rc == YXC_ERC_EOF) /* End of stream now. */
			{
				*puCcToRead = uTotalConverted;
				return YXC_ERC_SUCCESS;
			}
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}
err_ret:
		this->_u64FilePos = iOldPos;
		this->_pFileImpl->Seek(YXC_FSEEK_BEGIN, iOldPos, NULL);
		this->_uCcAvailable = 0;
		return rcRet;
	}

	YXC_Status _TextFile::_ReadTextLine(_TextConvertLineProc pfnConvert, yuint32_t uCvtChBytes, yuint32_t uCcToRead, ybyte_t* pPtr, yuint32_t* puCcToRead)
	{
		yint64_t iOldPos = this->_u64FilePos + this->_uCcOff * this->_uChBytes;
		yuint32_t uTotalConverted = 0;
		YXC_Status rcRet = YXC_ERC_UNKNOWN, rc;
		ybyte_t* pPtrCvt = pPtr;

		if (this->_uCcAvailable == 0)
		{
			rc = this->_ReadBasicBytes();
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}

		while (TRUE)
		{
			yuint32_t uDataUsed;
			yuint32_t uConverted;
			ybool_t bLineEnded = FALSE;
			pfnConvert(this->_pFileBuf + this->_uCcOff * this->_uChBytes, this->_uCcAvailable, pPtrCvt,
				uCcToRead - uTotalConverted, &uDataUsed, &uConverted, &bLineEnded);

			uTotalConverted += uConverted;
			this->_uCcOff += uDataUsed;
			this->_uCcAvailable -= uDataUsed;
			pPtrCvt += uConverted * uCvtChBytes;

			if (bLineEnded) /* Line ended. */
			{
				*puCcToRead = uTotalConverted;
				return YXC_ERC_SUCCESS;
			}
			else if (uTotalConverted == uCcToRead) /* Reach limit num bytes. */
			{
				*puCcToRead = uTotalConverted;
				_YXC_REPORT_NEW_RET(YXC_ERC_BUFFER_NOT_ENOUGH, YC("Not enough buffer to hold line"));
			}

			rc = this->_ReadBasicBytes();
			if (rc == YXC_ERC_EOF) /* End of stream now. */
			{
				*puCcToRead = uTotalConverted;
				return YXC_ERC_SUCCESS;
			}
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);
		}
err_ret:
		this->_u64FilePos = iOldPos;
		this->_pFileImpl->Seek(YXC_FSEEK_BEGIN, iOldPos, NULL);
		this->_uCcAvailable = 0;
		return rcRet;
	}

	YXC_Status _TextFile::_WriteText(YXC_Inner::_TextConvertProc pfnConvert, yuint32_t uCvtChBytes, yuint32_t uCcToWrite, const ybyte_t* pPtr)
	{
		if (this->_pFileBuf == NULL)
		{
			_YCHK_MAL_ARR_R2(this->_pFileBuf, ybyte_t, DEF_TEXT_BUF * 2);
			this->_pWriteBuf = this->_pFileBuf + DEF_TEXT_BUF;
		}

		yuint32_t uTotalUsed = 0;
		const ybyte_t* pPtrCvt = pPtr;
		this->_uCcAvailable = 0;

		while (uTotalUsed < uCcToWrite)
		{
			yuint32_t uDataUsed;
			yuint32_t uConverted;
			pfnConvert(pPtrCvt, uCcToWrite - uTotalUsed, this->_pWriteBuf, DEF_TEXT_BUF / this->_uChBytes,
				&uDataUsed, &uConverted);

			uTotalUsed += uDataUsed;
			pPtrCvt += uConverted * uCvtChBytes;

			yuint32_t uWritten = 0;
			YXC_Status rc = this->_pFileImpl->Write(uConverted * this->_uChBytes, this->_pWriteBuf, &uWritten);
			_YXC_CHECK_STATUS_RET(rc, YC("Failed to write content(l:%d) to file('%s')"), uConverted * this->_uChBytes,
				this->_fPath);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _TextFile::_WriteAndSetEncoding(YXC_File file, YXC_TEncoding encoding, yuint32_t* puHeaderBytes)
	{
		const void* pBytes = NULL;
		yuint32_t uCbToWrite = 0;
		switch (encoding)
		{
		case YXC_TENCODING_UTF8_WITH_BOM:
			uCbToWrite = sizeof(UTF8_BOM_BYTES);
			pBytes = UTF8_BOM_BYTES;
			this->_uChBytes = sizeof(char);
			break;
		case YXC_TENCODING_UTF16_BE:
			uCbToWrite = sizeof(UTF16_BE_BYTES);
			pBytes = UTF16_BE_BYTES;
			this->_uChBytes = sizeof(yuint16_t);
			break;
		case YXC_TENCODING_UTF16_LE:
			uCbToWrite = sizeof(UTF16_LE_BYTES);
			pBytes = UTF16_LE_BYTES;
			this->_uChBytes = sizeof(yuint16_t);
			break;
        case YXC_TENCODING_UTF32_BE:
            uCbToWrite = sizeof(UTF32_BE_BYTES);
            pBytes = UTF32_BE_BYTES;
            this->_uChBytes = sizeof(yuint32_t);
            break;
        case YXC_TENCODING_UTF32_LE:
            uCbToWrite = sizeof(UTF32_LE_BYTES);
            pBytes = UTF32_LE_BYTES;
            this->_uChBytes = sizeof(yuint32_t);
            break;
		case YXC_TENCODING_UTF8:
        case YXC_TENCODING_ANSI:
		default:
			this->_uChBytes = sizeof(char);
			break;
		}

		*puHeaderBytes = uCbToWrite;

		if (uCbToWrite > 0)
		{
			yuint32_t uWritten;
			YXC_Status rcWrite = YXC_FileWrite(file, uCbToWrite, pBytes, &uWritten);
			_YXC_CHECK_STATUS_RET(rcWrite, YC("Failed to write header bytes(%u)"), uCbToWrite);
			_YXC_CHECK_REPORT_NEW_RET(uWritten == uCbToWrite, YXC_ERC_IO_INCOMPLETED,
				YC("Failed to write header bytes(%u) completely"), uWritten);
		}

		this->_encoding = encoding;
		if (encoding == YXC_TENCODING_UTF8_WITH_BOM)
		{
			this->_encoding = YXC_TENCODING_UTF8;
		}

		if (encoding == YXC_TENCODING_DEFAULT)
		{
			this->_encoding = gs_TEncChar;
		}

		this->_uEncodingIndex = _TEncodingToIndex(this->_encoding);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _TextFile::_ParseAndSetEncoding(YXC_TEncoding reqEncoding, yuint32_t uCbHeader, ybyte_t* pbyHeader,
		yuint32_t* puHeaderBytes)
	{
        if (uCbHeader >= 4)
        {
            if (memcmp(pbyHeader, UTF32_LE_BYTES, sizeof(UTF32_LE_BYTES)) == 0)
            {
                this->_encoding = YXC_TENCODING_UTF32_LE;
                this->_uChBytes = sizeof(yuint32_t);
                *puHeaderBytes = 4;
                return YXC_ERC_SUCCESS;
            }
            else if (memcmp(pbyHeader, UTF32_BE_BYTES, sizeof(UTF32_BE_BYTES)) == 0)
            {
                this->_encoding = YXC_TENCODING_UTF32_BE;
                this->_uChBytes = sizeof(yuint32_t);
                *puHeaderBytes = 4;
                return YXC_ERC_SUCCESS;
            }
        }

		if (uCbHeader >= 3 && memcmp(pbyHeader, UTF8_BOM_BYTES, sizeof(UTF8_BOM_BYTES)) == 0) /* UTF-8 */
		{
			this->_encoding = YXC_TENCODING_UTF8;
			this->_uChBytes = sizeof(char);
			*puHeaderBytes = 3;
			return YXC_ERC_SUCCESS;
		}

		if (uCbHeader >= 2) /* Unicode? */
		{
			if (memcmp(pbyHeader, UTF16_LE_BYTES, sizeof(UTF16_LE_BYTES)) == 0) /* Unicode LE */
			{
				this->_encoding = YXC_TENCODING_UTF16_LE;
				this->_uChBytes = sizeof(yuint16_t);
				*puHeaderBytes = 2;
				return YXC_ERC_SUCCESS;
			}
			else if (memcmp(pbyHeader, UTF16_BE_BYTES, sizeof(UTF16_BE_BYTES)) == 0) /* Unicode BE */
			{
				this->_encoding = YXC_TENCODING_UTF16_BE;
				this->_uChBytes = sizeof(yuint16_t);
				*puHeaderBytes = 2;
				return YXC_ERC_SUCCESS;
			}
		}

		this->_encoding = gs_TEncChar;
		if (reqEncoding == YXC_TENCODING_UTF8)
		{
			this->_encoding = YXC_TENCODING_UTF8;
		}
		this->_uEncodingIndex = _TEncodingToIndex(this->_encoding);
		this->_uChBytes = sizeof(char);
		*puHeaderBytes = 0;
		return YXC_ERC_SUCCESS;
	}
}

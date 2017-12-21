#ifndef __INNER_INC_YXC_SYS_BASE_TEXT_FILE_HPP__
#define __INNER_INC_YXC_SYS_BASE_TEXT_FILE_HPP__

#include <YXC_Sys/OS/_YXC_FileBase.hpp>
#include <YXC_Sys/YXC_TextFile.h>
#include <YXC_Sys/YXC_Locker.hpp>
#include <YXC_Sys/Utils/_YXC_TextConverter.hpp>

namespace YXC_InnerF
{
	class _TextFile : public _FileBase
	{
	public:
		virtual YXC_Status Create(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess access);

		virtual void Close();

		virtual YXC_Status Read(yuint32_t uCbToRead, void* pData, yuint32_t* puRead);

		virtual YXC_Status Write(yuint32_t uCbToWrite, const void* pData, yuint32_t* puWritten);

		virtual YXC_Status Seek(YXC_FSeekMode seekMode, yint64_t i64SeekPos, yint64_t* pi64RealPos);

		virtual YXC_Status Flush();

        virtual YXC_Status Lock(yuint64_t uLockStart, yuint64_t uLockSize);

        virtual YXC_Status Unlock(yuint64_t uUnlockStart, yuint64_t uLockSize);

		virtual YXC_Status GetFileSize(yuint64_t* pu64FSize);

		YXC_Status CreateText(const ychar* pszPath, YXC_FOpenMode fMode, YXC_FileAccess fAccess, ybool_t bOSFile,
			YXC_TEncoding reqEncoding, YXC_TEncoding* pEncoding);

		YXC_Status ReadText(YXC_TEncoding enc, yuint32_t uCcToRead, void* pStr, yuint32_t* puCcToRead);

		YXC_Status ReadLine(YXC_TEncoding enc, yuint32_t uCcLineBuf, void* pLine, yuint32_t* puCcLine);

		YXC_Status WriteText(YXC_TEncoding enc, yuint32_t uCcWrite, const void* pStr);

		YXC_Status WriteLine(YXC_TEncoding enc, yuint32_t uCcLine, const void* pLine);

	private:
		YXC_Status _WriteAndSetEncoding(YXC_File file, YXC_TEncoding encoding, yuint32_t* puHeaderBytes);

		YXC_Status _ParseAndSetEncoding(YXC_TEncoding reqEncoding, yuint32_t uCbHeader, ybyte_t* pbyHeader,
			yuint32_t* puHeaderBytes);

		YXC_Status _ReadBasicBytes();

		YXC_Status _ReadText(YXC_Inner::_TextConvertProc pfnConvert, yuint32_t uCvtChBytes, yuint32_t uCcToRead, ybyte_t* pPtr, yuint32_t* puCcToRead);

		YXC_Status _ReadTextLine(YXC_Inner::_TextConvertLineProc pfnCvtLine, yuint32_t uCvtChBytes, yuint32_t uCcToRead, ybyte_t* pPtr, yuint32_t* puCcToRead);

		YXC_Status _WriteText(YXC_Inner::_TextConvertProc pfnConvert, yuint32_t uCvtChBytes, yuint32_t uCcToWrite, const ybyte_t* pPtr);

	private:
		_FileBase* _pFileImpl;

		yuint32_t _uCbHeader;
		YXC_TEncoding _encoding;
		yuint32_t _uEncodingIndex;
		yuint32_t _uChBytes;

		YXX_Crit _crit;

		ybyte_t* _pFileBuf;
		ybyte_t* _pWriteBuf;
		yuint32_t _uCcOff;
		yuint32_t _uCcAvailable;
		yuint64_t _u64FilePos;

		YXC_Inner::_TextConvertProc _pfnCvtWriteProc[YXC_Inner::TENCODING_MAX];
		YXC_Inner::_TextConvertProc _pfnCvtReadProc[YXC_Inner::TENCODING_MAX];
		YXC_Inner::_TextConvertLineProc _pfnConvertLineProc[YXC_Inner::TENCODING_MAX];
	};

	static const yuint32_t DEF_TEXT_BUF = 1 << 17; /* 128K buffered. */
	static const ybyte_t UTF8_BOM_BYTES[3] = { 0xEF, 0xBB, 0xBF };
	static const ybyte_t UTF16_LE_BYTES[2] = { 0xFF, 0xFE };
	static const ybyte_t UTF16_BE_BYTES[2] = { 0xFE, 0xFF };
	static const ybyte_t UTF32_LE_BYTES[4] = { 0xFF, 0xFE, 0x00, 0x00 };
	static const ybyte_t UTF32_BE_BYTES[4] = { 0x00, 0x00, 0xFE, 0xFF };

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_TextFile, _TextFile, _TFilePtr, _TFileHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_TEXT_FILE_HPP__ */

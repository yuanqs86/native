#define __MODULE__ "EK.Profile"

#include <YXC_Sys/Utils/_YXC_ProfileImpl.hpp>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_MMInterface.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

using namespace YXCLib;
using namespace YXC_InnerProfile;
namespace
{
	template <typename Ch>
	class _FileUtilsImplT
	{
	public:
		static const YXC_TEncoding DEFAULT_ENCODING = YXC_TENCODING_CHAR;
		static YXC_Status OpenFileForRead(const Ch* pszPath, YXC_TextFile* pFile)
		{
			return YXC_TextFileCreateA(pszPath, YXC_FOPEN_OPEN_EXISTING, YXC_FACCESS_GEN_READ, FALSE, DEFAULT_ENCODING,
				NULL, pFile);
		}

		static YXC_Status OpenFileForWrite(const Ch* pszPath, YXC_TextFile* pFile)
		{
			return YXC_TextFileCreateA(pszPath, YXC_FOPEN_CREATE_ALWAYS, (YXC_FileAccess)(YXC_FACCESS_WRITE | YXC_FACCESS_SHARED_READ),
				FALSE, DEFAULT_ENCODING, NULL, pFile);
		}

		static YXC_Status OpenFileForQuery(const Ch* pszPath, YXC_File* pFile)
		{
			YXC_FileAccess fAccess = (YXC_FileAccess)(YXC_FACCESS_QUERY_INFO | YXC_FACCESS_SHARED_READ | YXC_FACCESS_SHARED_WRITE);
			return YXC_FileCreateA(pszPath, YXC_FOPEN_OPEN_EXISTING, fAccess, TRUE, pFile);
		}
	};

	template <>
	class _FileUtilsImplT<wchar_t>
	{
	public:
		static const YXC_TEncoding DEFAULT_ENCODING = YXC_TENCODING_WCHAR;
		static YXC_Status OpenFileForRead(const wchar_t* pszPath, YXC_TextFile* pFile)
		{
			return YXC_TextFileCreateW(pszPath, YXC_FOPEN_OPEN_EXISTING, YXC_FACCESS_GEN_READ, FALSE, DEFAULT_ENCODING,
				NULL, pFile);
		}

		static YXC_Status OpenFileForWrite(const wchar_t* pszPath, YXC_TextFile* pFile)
		{
			return YXC_TextFileCreateW(pszPath, YXC_FOPEN_CREATE_ALWAYS, (YXC_FileAccess)(YXC_FACCESS_WRITE | YXC_FACCESS_SHARED_READ),
				FALSE, DEFAULT_ENCODING, NULL, pFile);
		}

		static YXC_Status OpenFileForQuery(const wchar_t* pszPath, YXC_File* pFile)
		{
			YXC_FileAccess fAccess = (YXC_FileAccess)(YXC_FACCESS_QUERY_INFO | YXC_FACCESS_SHARED_READ | YXC_FACCESS_SHARED_WRITE);
			return YXC_FileCreateW(pszPath, YXC_FOPEN_OPEN_EXISTING, fAccess, TRUE, pFile);
		}
	};

	template <typename Ch>
	class _FileUtils
	{
	public:
		static YXC_Status WriteLine(YXC_TextFile tFile, yint32_t iCcLine, const Ch* pCh)
		{
			yuint32_t uCcLine = iCcLine == YXC_STR_NTS ? (yuint32_t)_ChTraits<Ch>::strlen(pCh) : iCcLine;
			return YXC_TextFileWriteLine(tFile, _FileUtilsImplT<Ch>::DEFAULT_ENCODING, uCcLine, pCh);
		}

		static YXC_Status WriteLine(YXC_BBString* pBuf, yint32_t iCcLine, const Ch* pCh)
		{
			YXC_BBString& buf = *pBuf;
			yuint32_t uCcLine = iCcLine == YXC_STR_NTS ? (yuint32_t)_ChTraits<Ch>::strlen(pCh) : iCcLine;

			yuint32_t uCcBuf = buf.stCchStr + uCcLine + _ChTraits<Ch>::CC_LINE_END;
			if (uCcBuf > buf.stCchBufStr)
			{
				ysize_t stCcBuf = (buf.stCchBufStr + 1) * sizeof(Ch);
				YXC_Status rc = YXC_MMCExpandBuffer((void**)&buf.pStr, &stCcBuf, (uCcBuf + 1) * sizeof(Ch));
				_YXC_CHECK_RC_RETP(rc);

				buf.stCchBufStr = stCcBuf / sizeof(Ch) - 1;
			}
			memcpy(buf.pStr + buf.stCchStr, pCh, uCcLine * sizeof(Ch));
			memcpy(buf.pStr + buf.stCchStr + uCcLine, _ChTraits<Ch>::LINE_END, _ChTraits<Ch>::CC_LINE_END * sizeof(Ch));
			buf.stCchStr = uCcBuf;
			buf.pStr[uCcBuf] = 0;

			return YXC_ERC_SUCCESS;
		}

		static YXC_Status WriteText(YXC_TextFile tFile, yint32_t iCcStr, const Ch* pStr)
		{
			yuint32_t uCcStr = iCcStr == YXC_STR_NTS ? (yuint32_t)_ChTraits<Ch>::strlen(pStr) : iCcStr;
			return YXC_TextFileWrite(tFile, _FileUtilsImplT<Ch>::DEFAULT_ENCODING, uCcStr, pStr);
		}

		static YXC_Status WriteCh(YXC_TextFile tFile, Ch ch)
		{
			return YXC_TextFileWrite(tFile, _FileUtilsImplT<Ch>::DEFAULT_ENCODING, 1, &ch);
		}

		static YXC_Status ReadLine(YXC_TextFile tFile, yuint32_t uCcBuf, Ch* pBuf, yuint32_t* puCcRead)
		{
			return YXC_TextFileReadLine(tFile, _FileUtilsImplT<Ch>::DEFAULT_ENCODING, uCcBuf, pBuf, puCcRead);
		}
	};

	template <typename Ch>
	class _ProfileImplHelper
	{
	public:
		typedef typename YXCLib::_ChTraits<Ch> traits;
		typedef typename YXCLib::_ChTraits<Ch>::_CStrType string;

		static string TrimQuotes(const string& str)
		{
			int len = (int)str.length();
			if (len <= 1) return str;

			string ret(len, 0);
			const Ch* ptrStart = str.c_str(), *ptrEnd = ptrStart + len - 1;
			Ch* ptrDst = &ret[0], *ptrOrigin = ptrDst;
			if (*ptrStart == traits::DOUBLE_QUOTE && *ptrEnd == traits::DOUBLE_QUOTE)
			{
				++ptrStart;
				--ptrEnd;
			}
			for (const Ch* p = ptrStart; p <= ptrEnd; ++p)
			{
				//if (*p == traits::DOUBLE_QUOTE && *(p + 1) == traits::DOUBLE_QUOTE)
				//{
				//	*ptrDst++ = *p++;
				//}
				//else
				{
					*ptrDst++ = *p;
				}
			}
			*ptrDst++ = 0;
			return string(ptrOrigin);
		}

		static string TrimString(const string& str)
		{
			size_t startIndex = 0;
			for (typename string::const_iterator iBegin = str.begin(); iBegin != str.end(); ++iBegin)
			{
				if (*iBegin != traits::SPACE && *iBegin != traits::TAB && *iBegin != traits::RETURN && *iBegin != traits::ENTER) break;
				++startIndex;
			}
			if (startIndex >= str.length()) return string();
			size_t endIndex = str.length() - 1;
			for (typename string::const_reverse_iterator iBegin = str.rbegin(); iBegin != str.rend(); ++iBegin)
			{
				if (*iBegin != traits::SPACE && *iBegin != traits::TAB && *iBegin != traits::RETURN && *iBegin != traits::ENTER) break;
				--endIndex;
			}
			return str.substr(startIndex, endIndex - startIndex + 1);
		}

		static const Ch* SearchCommentPtr(const Ch* str)
		{
			ybool_t inWord = FALSE;
			const Ch* pRet = str;
			while (*pRet != 0)
			{
				if (!inWord && (*pRet == traits::SEMICOLON || *pRet == traits::SHARP))
				{
					return pRet;
				}
				if (*pRet == traits::DOUBLE_QUOTE)
				{
					if (!inWord)
					{
						inWord = TRUE;
					}
					else
					{
						inWord = *(pRet + 1) == traits::DOUBLE_QUOTE;
						if (inWord) ++pRet;
					}
				}
				++pRet;
			}
			return NULL;
		}

		static ybool_t ParseProfileSection(const string& trimmed, string& secName, string& postComment, ybool_t& hasComment)
		{
			if (*trimmed.begin() == traits::OPENING_BRACKET)
			{
				const Ch* first = trimmed.c_str();
				const Ch* secEnd = traits::strchr(first, traits::CLOSING_BRACKET);
				if (secEnd == NULL) return FALSE;
				secName = trimmed.substr(1, secEnd - first - 1);
				const Ch* comment = SearchCommentPtr(secEnd);
				hasComment = comment != NULL;
				if (hasComment)
				{
					postComment = trimmed.substr(comment - first + 1);
				}
				return TRUE;
			}
			return FALSE;
		}

		static ybool_t ParseProfileString(const string& trimmed, string& oKey, NString<Ch>& oVal,
			NString<Ch>& oCom)
		{
			Ch first = trimmed[0];
			if (first == traits::SEMICOLON || first == traits::SHARP)
			{
				oCom.val = trimmed.substr(1, trimmed.length() - 1);
				oCom.isNull = FALSE;
				return TRUE;
			}
			else
			{
				const Ch* str = trimmed.c_str();
				const Ch* pSEql = traits::strchr(str, '=');
				const Ch* pCom = NULL; // No comment ptr. SearchCommentPtr(pSEql != NULL ? pSEql + 1 : str);

				oVal.isNull = pSEql == NULL;
				oCom.isNull = pCom == NULL;

				if (pCom != NULL) oCom.val = trimmed.substr(pCom - str + 1);

				if (pSEql != NULL)
				{
					oKey = TrimString(trimmed.substr(0, pSEql - str));
					if (pCom != NULL)
					{
						oVal.val = TrimString(trimmed.substr(pSEql - str + 1, pCom - pSEql - 1));
					}
					else
					{
						oVal.val = TrimString(trimmed.substr(pSEql - str + 1));
					}
				}
				else
				{
					if (pCom != NULL)
					{
						oKey = TrimString(trimmed.substr(0, pCom - str));
					}
					else oKey = trimmed;
				}
				return FALSE;
			}
		}
	};

	static const yuint32_t DEFAULT_CC_BUF = 16;
}

namespace YXC_InnerProfile
{
	_ProfileProp::_ProfileProp(ybool_t bIsAutoSync) : _bIsAutoSync(bIsAutoSync) {}

	_ProfileProp::~_ProfileProp() {}

	void _ProfileProp::Lock() {}

	void _ProfileProp::Unlock() {}

	_TSafeProfileProp::_TSafeProfileProp(ybool_t bIsAutoSync) : _ProfileProp(bIsAutoSync),
		_crit(1000)
	{

	}

	_TSafeProfileProp::~_TSafeProfileProp() {}

	void _TSafeProfileProp::Lock()
	{
		this->_crit.Lock();
	}

	void _TSafeProfileProp::Unlock()
	{
		this->_crit.Unlock();
	}
}

namespace YXC_InnerProfile
{
	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::DefCopy(const Ch* pszDefRet, yuint32_t uCchRet, Ch* buf, yuint32_t* pNeeded)
	{
		yuint32_t uCchStr = 0;
		if (pszDefRet != NULL) uCchStr = (yuint32_t)traits::strlen(pszDefRet);

		if (buf)
		{
			traits::strncpy(buf, pszDefRet, uCchRet);
			buf[uCchRet - 1] = '\0';
		}

		_YXC_REPORT_NEW_RET(YXC_ERC_KEY_NOT_FOUND, YC("Key not found, use default value."));
	}

	template <typename Ch>
	_Profile<Ch>::ProfileSection::ProfileSection(const string& name) : m_name(name),
		m_comments(), m_lines()
	{

	}

	template <typename Ch>
	_Profile<Ch>::ProfileSection::~ProfileSection()
	{
		this->Clear();
	}

	template <typename Ch>
	void _Profile<Ch>::ProfileSection::Clear()
	{
		for (LINE_ITER i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			delete *i;
		}
		m_lines.clear();
	}

	template <typename Ch>
	_ProfileLine<Ch>* _Profile<Ch>::ProfileSection::_Find(const Ch* key) const
	{
		for (LINE_CITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
		{
			if (traits::stricmp((*pi)->key.c_str(), key) == 0) return *pi;
		}
		return NULL;
	}

	template <typename Ch>
	_ProfileLine<Ch>* _Profile<Ch>::ProfileSection::_FindOrCreate(const string& key)
	{
		_ProfileLine<Ch>* pLine = this->_Find(key.c_str());
		if (pLine == NULL)
		{
			typename std::auto_ptr<_PLine> pAutoLine(new _PLine(key));
			m_lines.push_back(pAutoLine.get());
			pLine = pAutoLine.release();
		}
		return pLine;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::Set(const string& key, const YXCLib::NString<Ch>& val)
	{
		try
		{
			string trimmed = _ProfileImplHelper<Ch>::TrimString(key);
			_YXC_CHECK_REPORT_NEW_RET(trimmed.length() > 0, YXC_ERC_INVALID_PARAMETER, YC("Empty profile key is not supported"));

			_PLine* line = this->_FindOrCreate(trimmed.c_str());
			line->val = val;
			return YXC_ERC_SUCCESS;
		}
		catch (const std::exception& e)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), e.what());
		}
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::SetVal(const Ch* key, const Ch* val)
	{
		try
		{
			string trimmed = _ProfileImplHelper<Ch>::TrimString(key);
			_YXC_CHECK_REPORT_NEW_RET(trimmed.length() > 0, YXC_ERC_INVALID_PARAMETER, YC("Empty profile key is not supported"));

			_PLine* line = this->_FindOrCreate(trimmed.c_str());
			line->val = YXCLib::CreateNString<Ch>(val);
			return YXC_ERC_SUCCESS;
		}
		catch (const std::exception& e)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), e.what());
		}
	}

	//template <typename Ch>
	//YXC_Status _Profile<Ch>::ProfileSection::InsertVal(const Ch* key, const Ch* val, const Ch* insertBefore)
	//{
	//	if (insertBefore != NULL)
	//	{
	//		for (LINE_ITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
	//		{
	//			if (_ChTraits<Ch>::stricmp((*pi)->key.c_str(), insertBefore) == 0)
	//			{
	//				std::auto_ptr<_PLine> pAutoLine(new _PLine(key));
	//				this->m_lines.insert(pi, pAutoLine.get());
	//				_PLine* pLine = pAutoLine.release();
	//				pLine->val = CreateNString<Ch>(val);
	//				return YXC_ERC_SUCCESS;
	//			}
	//		}
	//	}
	//	return this->SetVal(key, val);
	//}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::Get(const Ch* key, const Ch* defRet, yuint32_t uCchBuf, Ch* buf, yuint32_t* pCchNeeded) const
	{
		_ProfileLine<Ch>* line = this->_Find(key);
		if (line == NULL || line->val.isNull)
		{
			return DefCopy(defRet, uCchBuf, buf, pCchNeeded);
		}

		yuint32_t uCchStr = (yuint32_t)line->val.val.length();
		if (pCchNeeded) *pCchNeeded = uCchStr;

		_YXC_CHECK_REPORT_NEW_RET(uCchBuf >= uCchStr, YXC_ERC_BUFFER_NOT_ENOUGH, YC("Expected buffer(%d), actual(%d)"), uCchStr, uCchBuf);

		if (buf)
		{
			memcpy(buf, line->val.val.c_str(), (uCchStr + 1) * sizeof(Ch));
		}
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::GetKeys(yuint32_t uCchBuf, Ch* pBuf, yuint32_t* puCchNeeded) const
	{
		yuint32_t uCchTotal = 0;
		for (LINE_CITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
		{
			_PLine* pLine = *pi;
			uCchTotal += (yuint32_t)(pLine->key.length() + 1); // for the end char.
		}
		if (m_lines.empty()) ++uCchTotal;

		if (puCchNeeded) *puCchNeeded = uCchTotal;
		_YXC_CHECK_REPORT_NEW_RET(uCchBuf >= uCchTotal, YXC_ERC_BUFFER_NOT_ENOUGH, YC("Expected buffer(%d), actual(%d)"),
			uCchTotal, uCchBuf);

		yuint32_t uOffset = 0;
		for (LINE_CITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
		{
			_PLine* pLine = *pi;
			yuint32_t uCchStr = (yuint32_t)pLine->key.length();
			memcpy(pBuf + uOffset, pLine->key.c_str(), (uCchStr + 1) * sizeof(Ch));
			uOffset += uCchStr + 1;
		}

		if (pBuf)
		{
			pBuf[uOffset] = 0;
			if (m_lines.empty()) pBuf[uOffset + 1] = 0;
		}
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::GetAll(yuint32_t uCchBuf, Ch* buf, yuint32_t* puCchNeeded) const
	{
		yuint32_t uCchTotal = 0;
		for (LINE_CITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
		{
			_PLine* pLine = *pi;
			uCchTotal += (yuint32_t)(pLine->key.length() + 1); // for the end char.
			if (!pLine->val.isNull) uCchTotal += (yuint32_t)pLine->val.val.length() + 1; // for the '=' char.
		}
		if (m_lines.empty()) ++uCchTotal;

		if (puCchNeeded) *puCchNeeded = uCchTotal;
		_YXC_CHECK_REPORT_NEW_RET(uCchBuf >= uCchTotal, YXC_ERC_BUFFER_NOT_ENOUGH, YC("Expected buffer(%d), actual(%d)"),
			uCchTotal, uCchBuf);

		yuint32_t uOffset = 0;
		for (LINE_CITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
		{
			_PLine* pLine = *pi;
			yuint32_t uCchStr = (yuint32_t)pLine->key.length();
			memcpy(buf + uOffset, pLine->key.c_str(), uCchStr * sizeof(Ch));
			if (!pLine->val.isNull)
			{
				buf[uOffset + uCchStr] = '='; // fill '=' char.
				uOffset += uCchStr + 1;
				uCchStr = (yuint32_t)pLine->val.val.length();
				memcpy(buf + uOffset, pLine->val.val.c_str(), uCchStr * sizeof(Ch));
			}
			buf[uOffset + uCchStr] = 0; // fill end char.
			uOffset += uCchStr + 1;
		}

		if (buf)
		{
			buf[uOffset] = 0;
			if (m_lines.empty()) buf[uOffset + 1] = 0;
		}
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::Delete(const Ch* key)
	{
		try
		{
			string trimmed = _ProfileImplHelper<Ch>::TrimString(key);
			for (LINE_ITER pi = m_lines.begin(); pi != m_lines.end(); ++pi)
			{
				if (_ChTraits<Ch>::stricmp((*pi)->key.c_str(), trimmed.c_str()) == 0)
				{
					delete *pi;
					m_lines.erase(pi);
					return YXC_ERC_SUCCESS;
				}
			}
		}
		catch (const std::exception& eh)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), eh.what());
		}

		_YXC_REPORT_NEW_RET(YXC_ERC_KEY_NOT_FOUND, YC("Can't find key to delete"));
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::ProfileSection::SetSection(const Ch* strSet)
	{
		const Ch* p = strSet, *context = NULL;
		try
		{
			string trimmed, key;
			std::vector<string> preCom;
			NString val, comment;
			while (*p != '\0')
			{
				trimmed = _ProfileImplHelper<Ch>::TrimString(p);
				context = traits::strchr(strSet, '\0');
				p = context + 1;
				if (_ProfileImplHelper<Ch>::ParseProfileString(trimmed, key, val, comment))
				{
					preCom.push_back(comment.val);
				}
				else
				{
					this->Set(key, val);
					if (!preCom.empty())
					{
						_PLine* line = this->_Find(key.c_str());
						line->comments = preCom;
						preCom.clear();
					}
				}
			}
			return YXC_ERC_SUCCESS;
		}
		catch (const std::exception& eh)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), eh.what());
		}
	}

	template <typename Ch>
	_Profile<Ch>::_Profile(ybool_t bAutoSync, ybool_t bThreadSafe) : m_sections(), m_lastComm(), m_readBuf(NULL),
		m_uCcReadBuf(0), m_prop(NULL), m_u64FTime(0), m_file(NULL)
	{
		if (bThreadSafe)
		{
			m_prop = new _ProfileProp(bAutoSync);
		}
		else
		{
			m_prop = new _ProfileProp(bAutoSync);
		}
		m_pfPath[0] = 0;
	}

	template <typename Ch>
	_Profile<Ch>::~_Profile()
	{
		this->Destroy();

		if (m_prop != NULL)
		{
			delete m_prop;
		}

		if (m_file != NULL)
		{
			YXC_FileClose(m_file);
		}
	}

	template <typename Ch>
	template <typename Source>
	YXC_Status _Profile<Ch>::_WriteToProfile(Source src) const
	{
		string tmpStr;
		for (SEC_CITER i = this->m_sections.begin(); i != this->m_sections.end(); ++i)
		{
			ProfileSection* pSection = *i;
			for (COMMENT_ITER iComm = pSection->m_comments.begin(); iComm != pSection->m_comments.end(); ++iComm)
			{
				tmpStr.assign(1, traits::SEMICOLON);
				tmpStr.append(*iComm);
				_FileUtils<Ch>::WriteLine(src, (yuint32_t)tmpStr.length(), tmpStr.c_str());
			}

			tmpStr.assign(1, traits::OPENING_BRACKET);
			tmpStr.append(pSection->GetName());
			tmpStr.append(1, traits::CLOSING_BRACKET);
			_FileUtils<Ch>::WriteLine(src, (yuint32_t)tmpStr.length(), tmpStr.c_str());

			for (LINE_CITER j = (*i)->m_lines.begin(); j != (*i)->m_lines.end(); ++j)
			{
				_PLine* ptrLine = *j;
				for (COMMENT_CITER iComm = ptrLine->comments.begin(); iComm != ptrLine->comments.end(); ++iComm)
				{
					tmpStr.assign(1, traits::SEMICOLON);
					tmpStr.append(*iComm);
					_FileUtils<Ch>::WriteLine(src, (yuint32_t)tmpStr.length(), tmpStr.c_str());
				}
				tmpStr.assign(ptrLine->key.c_str());
				if (!ptrLine->val.isNull)
				{
					tmpStr.append(1, traits::SPACE);
					tmpStr.append(1, traits::EQUALSIGN);
					tmpStr.append(1, traits::SPACE);
					tmpStr.append(ptrLine->val.val);
				}
				_FileUtils<Ch>::WriteLine(src, (yuint32_t)tmpStr.length(), tmpStr.c_str());
			}
		}
		for (COMMENT_CITER iComm = m_lastComm.begin(); iComm != m_lastComm.end(); ++iComm)
		{
			tmpStr.assign(1, traits::SEMICOLON);
			tmpStr.append(*iComm);
			_FileUtils<Ch>::WriteLine(src, (yuint32_t)tmpStr.length(), tmpStr.c_str());
		}
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::_ReadLine(const Ch*& ch, Ch*& pszBuf, yuint32_t& uCchBuf)
	{
		const Ch* pos = _ChTraits<Ch>::strchr(ch, _ChTraits<Ch>::RETURN);
		yuint32_t uLen = pos - ch + 1;
		if (pos == NULL)
		{
			uLen = _ChTraits<Ch>::strlen(ch);
			_YXC_REPORT_NEW_RET(YXC_ERC_EOF, YC("End of string profile"));
		}

		if (pszBuf == NULL)
		{
			_YCHK_MAL_ARR_R2(pszBuf, Ch, DEFAULT_CC_BUF + 1);
			uCchBuf = DEFAULT_CC_BUF;
		}

		YXC_Status rc;
		while (uCchBuf < uLen)
		{
			rc = this->_InflateReadBuffer(pszBuf, uCchBuf);
			_YXC_CHECK_RC_RETP(rc);
		}

		memcpy(pszBuf, ch, uLen * sizeof(Ch));
		ch += uLen;
		pszBuf[uLen] = 0;
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::_ReadLine(YXC_TextFile tFile, Ch*& pszBuf, yuint32_t& uCcBuf)
	{
		if (pszBuf == NULL)
		{
			_YCHK_MAL_ARR_R2(pszBuf, Ch, DEFAULT_CC_BUF + 1);
			uCcBuf = DEFAULT_CC_BUF;
		}

		yuint32_t uCcRead;
		yint64_t i64Offset;
		YXC_Status rc = YXC_FileSeek(tFile, YXC_FSEEK_CUR, 0, &i64Offset);
		_YXC_CHECK_RC_RET(rc);

		while (TRUE)
		{
			rc = _FileUtils<Ch>::ReadLine(tFile, uCcBuf, pszBuf, &uCcRead);
			if (rc == YXC_ERC_BUFFER_NOT_ENOUGH)
			{
				rc = this->_InflateReadBuffer(pszBuf, uCcBuf);
				_YXC_CHECK_RC_RET(rc);
				rc = YXC_FileSeek(tFile, YXC_FSEEK_BEGIN, i64Offset, NULL);
				_YXC_CHECK_RC_RET(rc);
				continue; /* Redo read. */
			}
			else
			{
				return rc;
			}
		}
	}

	template <typename Ch>
	template <typename Source>
	YXC_Status _Profile<Ch>::_ReadProfile(Source src)
	{
		string temp1, trimmed, splitted1, splitted2;
		ProfileSection* pSection = NULL;
		_PLine* pLine = NULL;

		std::vector<string> tempComments;
		while (TRUE)
		{
			YXC_Status rc = this->_ReadLine(src, m_readBuf, m_uCcReadBuf);
			if (rc == YXC_ERC_EOF)
			{
				break;
			}
			_YXC_CHECK_STATUS_RET(rc, YC("Failed to read profile line"));

			temp1.assign(m_readBuf);
			trimmed = _ProfileImplHelper<Ch>::TrimString(temp1);
			if (trimmed.empty()) continue;

			ybool_t hasPostComment = FALSE;
			if (_ProfileImplHelper<Ch>::ParseProfileSection(trimmed, splitted1, splitted2, hasPostComment))
			{
				std::auto_ptr<ProfileSection> pAutoSection(new ProfileSection(splitted1.c_str()));
				this->m_sections.push_back(pAutoSection.get());
				pSection = pAutoSection.release();
				if (tempComments.size() != 0)
				{
					pSection->m_comments = tempComments;
					tempComments.clear();
				}
				pLine = NULL;
				continue;
			}

			NString val, postCom;
			ybool_t isCommentLine = _ProfileImplHelper<Ch>::ParseProfileString(trimmed, splitted1, val, postCom);
			if (isCommentLine)
			{
				tempComments.push_back(postCom.val);
			}
			else
			{
				_YXC_CHECK_REPORT_NEW_RET(pSection != NULL, YXC_ERC_INVALID_FILE_FORMAT, YC("Invalid line read, not in a section"));
				if (!val.isNull) val.val = _ProfileImplHelper<Ch>::TrimQuotes(val.val);
				pSection->Set(splitted1, val);
				pLine = pSection->_Find(splitted1.c_str());
				if (tempComments.size() != 0)
				{
					if (pLine != NULL)
					{
						swap(pLine->comments, tempComments);
					}
					tempComments.clear();
				}
			}
		}
		m_lastComm = tempComments;
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::_InflateReadBuffer(Ch*& pszBuf, yuint32_t& uCcBuf)
	{
		yuint32_t uCcNewBuf = uCcBuf * 4;
		_YCHK_MAL_ARR_R1(pszNewBuf, Ch, uCcNewBuf + 1);

		free(pszBuf);
		pszBuf = pszNewBuf;
		uCcBuf = uCcNewBuf;
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	typename _Profile<Ch>::ProfileSection* _Profile<Ch>::_FindSection(const Ch* secName)
	{
		for (SEC_ITER si = m_sections.begin(); si != m_sections.end(); ++si)
		{
			if (traits::stricmp(secName, (*si)->GetName().c_str()) == 0)
			{
				return *si;
			}
		}
		return NULL;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::_FindOrCreateSection(const Ch* pszSection, typename _Profile<Ch>::ProfileSection** ppSection)
	{
		ProfileSection* section = this->_FindSection(pszSection);
		if (section == NULL)
		{
			try
			{
				string trimmed = _ProfileImplHelper<Ch>::TrimString(pszSection);
				_YXC_CHECK_REPORT_NEW_RET(trimmed.length() > 0, YXC_ERC_INVALID_PARAMETER, YC("Empty section name is not allowed"));
				std::auto_ptr<ProfileSection> pAutoSection(new ProfileSection(trimmed));
				this->m_sections.push_back(pAutoSection.get());
				section = pAutoSection.release();
			}
			catch (const std::exception& ex)
			{
				_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), ex.what());
			}
		}

		*ppSection = section;
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::_CheckForModifications()
	{
		if (m_prop->IsAutoSync())
		{
			if (m_file != NULL) /* Has a file to watch. */
			{
				yuint64_t u64FTime;
				YXC_Status rc = YXC_FileGetTimes(m_file, &u64FTime, NULL, NULL);
				_YXC_CHECK_RC_RET(rc);

				if (u64FTime != m_u64FTime) /* Reload. */
				{
					this->Clear(FALSE);

					YXC_TextFile tFile;
					YXC_Status rc = _FileUtilsImplT<Ch>::OpenFileForRead(m_pfPath, &tFile);
					_YXC_CHECK_RC_RET(rc);
					YXCLib::HandleRef<YXC_File> resHolder(tFile, YXC_FileClose);

					rc = this->_ReadProfile(tFile);
					_YXC_CHECK_RC_RET(rc);

					m_u64FTime = u64FTime;
				}
			}
		}
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::LoadContent(const Ch* content)
	{
		this->m_file = NULL;
		this->m_pfPath[0] = 0;
		YXC_Status rc = this->_ReadProfile(content);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::Load(const Ch* path)
	{
		this->Clear(TRUE);

		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_File fQuery;
		YXC_Status rc = _FileUtilsImplT<Ch>::OpenFileForQuery(path, &fQuery);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXC_File> fQueryRes(fQuery, YXC_FileClose);
		yuint64_t u64FTime;
		rc = YXC_FileGetTimes(fQuery, &u64FTime, NULL, NULL);
		_YXC_CHECK_RC_RET(rc);

		try
		{
			YXC_TextFile tFile;
			YXC_Status rc = _FileUtilsImplT<Ch>::OpenFileForRead(path, &tFile);
			_YXC_CHECK_RC_RET(rc);

			YXCLib::HandleRef<YXC_File> tFileRes(tFile, YXC_FileClose);
			rc = this->_ReadProfile(tFile);
			_YXC_CHECK_RC_RET(rc);

			this->m_u64FTime = u64FTime;
			if (m_file)
			{
				YXC_FileClose(m_file);
			}
			m_file = fQuery;
			traits::strncpy(m_pfPath, path, YXC_MAX_CCH_PATH);
			m_pfPath[YXC_MAX_CCH_PATH - 1] = 0;
			fQueryRes.Detach();
			return rc;
		}
		catch (const std::exception& ceh)
		{
			this->Clear(FALSE);
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), ceh.what());
		}
	}


	template <typename Ch>
	YXC_Status _Profile<Ch>::Save(const Ch* path) const
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		if (path == NULL)
		{
			yuint32_t uCcLen = (yuint32_t)traits::strlen(m_pfPath);
			_YXC_CHECK_REPORT_NEW_RET(uCcLen > 0, YXC_ERC_NEED_SAVEAS, YC("Invalid save due to profile is not bound to a path"));
			path = m_pfPath;
		}

		YXC_TextFile tFile;
		YXC_Status rc = _FileUtilsImplT<Ch>::OpenFileForWrite(path, &tFile);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXC_File> resHolder(tFile, YXC_FileClose);
		try
		{
			rc = this->_WriteToProfile(tFile);
			return rc;
		}
		catch (const std::exception& ceh)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), ceh.what());
		}
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::SaveContent(Ch** ppszContent, yuint32_t* puCcContent) const
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);

		try
		{
			YXC_BBString bbString = { 0 };
			YXC_Status rc = this->_WriteToProfile(&bbString);

			YXCLib::HandleRef<void*> bbString_res(bbString.pStr, YXC_MMCFreeData);
			_YXC_CHECK_RC_RETP(rc);

			*ppszContent = (Ch*)bbString_res.Detach();
			*puCcContent = bbString.stCchStr;

			return YXC_ERC_SUCCESS;
		}
		catch (const std::exception& ceh)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), ceh.what());
		}
	}

	template <typename Ch>
	void _Profile<Ch>::Clear(ybool_t bClearPath)
	{
		for (SEC_ITER i = m_sections.begin(); i != m_sections.end(); ++i)
		{
			delete *i;
		}
		m_sections.clear();
		m_lastComm.clear();

		if (bClearPath) m_pfPath[0] = 0;
	}

	template <typename Ch>
	void _Profile<Ch>::Destroy()
	{
		this->Clear(TRUE);
		if (m_readBuf)
		{
			free(m_readBuf);
			m_readBuf = NULL;
		}
		m_uCcReadBuf = 0;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::GetSectionNames(yuint32_t uCchRet, Ch* strRet, yuint32_t* puCchNeeded)
	{
		yuint32_t uCchTotal = 0;
		for (SEC_ITER si = m_sections.begin(); si != m_sections.end(); ++si)
		{
			uCchTotal += (yuint32_t)((*si)->GetName().length() + 1);
		}

		if (puCchNeeded != NULL) *puCchNeeded = uCchTotal;

		_YXC_CHECK_REPORT_NEW_RET(uCchRet >= uCchTotal, YXC_ERC_BUFFER_NOT_ENOUGH, YC("Expected buffer(%d), actual(%d)"),
			uCchTotal, uCchRet);

		for (SEC_ITER si = m_sections.begin(); si != m_sections.end(); ++si)
		{
			yuint32_t uCcCopy = (yuint32_t)((*si)->GetName().length() + 1);
			memcpy(strRet, (*si)->GetName().c_str(), sizeof(Ch) * uCcCopy);
			strRet += uCcCopy;
		}
		*strRet = 0;
		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::GetSectionKeys(const Ch* pszSection, yuint32_t uCchRet, Ch* pszRet, yuint32_t* puCchNeeded)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section = this->_FindSection(pszSection);
		_YXC_CHECK_REPORT_NEW_RET(section != NULL, YXC_ERC_KEY_NOT_FOUND, YC("Failed to find section"));

		return section->GetKeys(uCchRet, pszRet, puCchNeeded);
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::GetSection(const Ch* pszSection, yuint32_t uCchRet, Ch* pszRet, yuint32_t* puCchNeeded)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section = this->_FindSection(pszSection);
		_YXC_CHECK_REPORT_NEW_RET(section != NULL, YXC_ERC_KEY_NOT_FOUND, YC("Failed to find section"));

		return section->GetAll(uCchRet, pszRet, puCchNeeded);
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::AddSection(const Ch* pszSection)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section;
		rc = this->_FindOrCreateSection(pszSection, &section);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::SetSection(const Ch* pszSection, const Ch* pszSet)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section;
		rc = this->_FindOrCreateSection(pszSection, &section);
		_YXC_CHECK_RC_RET(rc);

		return section->SetSection(pszSet);
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::DeleteSection(const Ch* pszSection)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		try
		{
			string trimmed = _ProfileImplHelper<Ch>::TrimString(pszSection);
			for (SEC_ITER si = m_sections.begin(); si != m_sections.end(); ++si)
			{
				if (traits::stricmp(trimmed.c_str(), (*si)->GetName().c_str()))
				{
					delete *si;
					m_sections.erase(si);
					return YXC_ERC_SUCCESS;
				}
			}
		}
		catch (const std::exception& eh)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%ss"), eh.what());
		}
		_YXC_REPORT_NEW_RET(YXC_ERC_KEY_NOT_FOUND, YC("Section is not found"));
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::GetString(const Ch* pszSection, const Ch* pszKey, const Ch* pszDef, yuint32_t uCchRet,
		Ch* pszRet, yuint32_t* puCchNeeded)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section = this->_FindSection(pszSection);
		if (section == NULL)
		{
			return ProfileSection::DefCopy(pszDef, uCchRet, pszRet, puCchNeeded);
		}

		return section->Get(pszKey, pszDef, uCchRet, pszRet, puCchNeeded);
	}

	template <typename Ch>
	YXC_Status _Profile<Ch>::SetString(const Ch* pszSection, const Ch* pszKey, const Ch* pszVal)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section;
		rc = this->_FindOrCreateSection(pszSection, &section);
		_YXC_CHECK_STATUS_RET(rc, YC("Failed to create section"));

		return section->SetVal(pszKey, pszVal);
	}

	//template <typename Ch>
	//YXC_Status _Profile<Ch>::InsertString(const Ch* pszSection, const Ch* pszKey, const Ch* pszVal, const Ch* pszkeyBefore)
	//{
	//	YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
	//	YXC_Status rc = this->_CheckForModifications();
	//	_YXC_CHECK_RC_RET(rc);

	//	ProfileSection* section;
	//	rc = this->_FindOrCreateSection(pszSection, &section);
	//	_YXC_CHECK_STATUS_RET(rc, L"Failed to create section");

	//	return section->InsertVal(pszKey, pszVal, pszkeyBefore);
	//}

	template <typename Ch>
	YXC_Status _Profile<Ch>::DeleteString(const Ch* pszSection, const Ch* pszKey)
	{
		YXCLib::Locker<_ProfileProp> locker(*this->m_prop);
		YXC_Status rc = this->_CheckForModifications();
		_YXC_CHECK_RC_RET(rc);

		ProfileSection* section = this->_FindSection(pszSection);
		_YXC_CHECK_REPORT_NEW_RET(section != NULL, YXC_ERC_KEY_NOT_FOUND, YC("Failed to find section"));

		return section->Delete(pszKey);
	}

	template class _Profile<char>;
	template class _Profile<wchar_t>;
}

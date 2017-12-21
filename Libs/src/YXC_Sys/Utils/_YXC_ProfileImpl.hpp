#ifndef __INNER_INC_YXC_SYS_BASE_PROFILE_HPP__
#define __INNER_INC_YXC_SYS_BASE_PROFILE_HPP__

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include <YXC_Sys/YXC_Profile.h>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_TextFile.h>
#include <YXC_Sys/YXC_Locker.hpp>

namespace YXC_InnerProfile
{
	class _ProfileProp
	{
	public:
		_ProfileProp(ybool_t bIsAutoSync);

		virtual ~_ProfileProp();

	public:
		virtual void Lock();

		virtual void Unlock();

		inline ybool_t IsAutoSync() const { return this->_bIsAutoSync; }

	private:
		ybool_t _bIsAutoSync;
	};

	class _TSafeProfileProp : public _ProfileProp
	{
	public:
		_TSafeProfileProp(ybool_t bIsAutoSync);

		virtual ~_TSafeProfileProp();

	public:
		virtual void Lock();

		virtual void Unlock();

	private:
		YXX_Crit _crit;
	};

	template <typename Ch>
	struct _ProfileLine
	{
		typedef typename YXCLib::_ChTraits<Ch> traits;
		typedef typename YXCLib::_ChTraits<Ch>::_CStrType string;

		string key;
		YXCLib::NString<Ch> val;
		std::vector<string> comments;

		_ProfileLine(const string& key) : key(key), comments()
		{
			this->val.isNull = TRUE;
		}
	};

	template <typename Ch>
	class _Profile
	{
	public:
		typedef typename YXCLib::_ChTraits<Ch> traits;
		typedef typename YXCLib::_ChTraits<Ch>::_CStrType string;
		typedef YXCLib::NString<Ch> NString;
	private:
		typedef YXC_InnerProfile::_ProfileLine<Ch> _PLine;

	public:
		_Profile(ybool_t bAutoSync, ybool_t bThreadSafe);

		~_Profile();
	public:
		YXC_Status Load(const Ch* path);
		YXC_Status LoadContent(const Ch* content);
		YXC_Status Save(const Ch* path) const;
		YXC_Status SaveContent(Ch** ppszContent, yuint32_t* puCcContent) const;
		void Destroy();
		void Clear(ybool_t bClearPath);
	public:
		YXC_Status GetSectionNames(yuint32_t uCchRet, Ch* strRet, yuint32_t* puCchNeeded);
		YXC_Status GetSectionKeys(const Ch* pszSection, yuint32_t uCchRet, Ch* pszRet, yuint32_t* puCchNeeded);
		YXC_Status GetSection(const Ch* pszSection, yuint32_t uCchRet, Ch* pszRet, yuint32_t* puCchNeeded);
		YXC_Status SetSection(const Ch* pszSection, const Ch* pszSet);
		YXC_Status AddSection(const Ch* pszSection);
		YXC_Status DeleteSection(const Ch* pszSection);
		YXC_Status GetString(const Ch* pszSection, const Ch* pszKey, const Ch* pszDef, yuint32_t uCchRet, Ch* pszRet, yuint32_t* puCchNeeded);
		YXC_Status SetString(const Ch* pszSection, const Ch* pszKey, const Ch* pszVal);
		// YXC_Status InsertString(const Ch* pszSection, const Ch* pszKey, const Ch* pszVal, const Ch* pszkeyBefore);
		YXC_Status DeleteString(const Ch* pszSection, const Ch* pszKey);
	private:
		class ProfileSection
		{
		public:
			ProfileSection(const string& name);
			~ProfileSection();
		public:
			YXC_Status SetSection(const Ch* strSet);
			YXC_Status Set(const string& key, const NString& val);
			YXC_Status SetVal(const Ch* key, const Ch* val);
			// YXC_Status InsertVal(const Ch* key, const Ch* val, const Ch* insertBefore);
			YXC_Status Delete(const Ch* key);
			void Clear();

			YXC_Status GetKeys(yuint32_t uCchRet, Ch* buf, yuint32_t* puCchNeeded) const;
			YXC_Status GetAll(yuint32_t uCchRet, Ch* buf, yuint32_t* puCchNeeded) const;
			YXC_Status Get(const Ch* key, const Ch* defRet, yuint32_t uCchRet, Ch* buf, yuint32_t* puCchNeeded) const;
		private:
			_PLine* _Find(const Ch* key) const;
			inline ybool_t _Contains(const Ch* key) const { return this->_Find(key) != NULL; }
			_PLine* _FindOrCreate(const string& key);
		public:
			inline const string& GetName() const { return m_name; }
			static YXC_Status DefCopy(const Ch* pszDefRet, yuint32_t uCchRet, Ch* buf, yuint32_t* pNeeded);

		private:
			std::vector<_ProfileLine<Ch>*> m_lines;
			string m_name;
			std::vector<string> m_comments;

			friend class _Profile;
		};

	private:
		ProfileSection* _FindSection(const Ch* pszSection);
		YXC_Status _FindOrCreateSection(const Ch* pszSection, ProfileSection** ppSection);

		template <typename Source>
		YXC_Status _WriteToProfile(Source src) const;

		template <typename Source>
		YXC_Status _ReadProfile(Source src);

		YXC_Status _ReadLine(YXC_TextFile file, Ch*& pszBuf, yuint32_t& uCchBuf);
		YXC_Status _ReadLine(const Ch*& ch, Ch*& pszBuf, yuint32_t& uCchBuf);
		YXC_Status _InflateReadBuffer(Ch*& pszBuf, yuint32_t& uCcBuf);
		YXC_Status _CheckForModifications();
	private:
		std::vector<ProfileSection*> m_sections;
		std::vector<string> m_lastComm;
		Ch* m_readBuf;
		yuint32_t m_uCcReadBuf;
		_ProfileProp* m_prop;
		YXC_File m_file;
		Ch m_pfPath[YXC_MAX_CCH_PATH];
		yuint64_t m_u64FTime;

	private:
		typedef typename std::vector<ProfileSection*>::const_iterator SEC_CITER;
		typedef typename std::vector<string>::const_iterator COMMENT_CITER;
		typedef typename std::vector<_ProfileLine<Ch>*>::const_iterator LINE_CITER;

		typedef typename std::vector<ProfileSection*>::iterator SEC_ITER;
		typedef typename std::vector<string>::iterator COMMENT_ITER;
		typedef typename std::vector<_ProfileLine<Ch>*>::iterator LINE_ITER;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_ProfileW, _Profile<wchar_t>, _PfPtrW, _PfHdlW);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_ProfileA, _Profile<char>, _PfPtrA, _PfHdlA);
}

#endif /* __INNER_INC_YXC_SYS_BASE_PROFILE_HPP__ */

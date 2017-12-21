/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_IN_MEMORY_STREAM_H__
#define __INC_YXC_SYS_BASE_IN_MEMORY_STREAM_H__

#include <YXC_Sys/YXC_MemoryStream.hpp>
#include <string>
#include <typeinfo>

namespace YXCLib
{
	class YXC_CLASS IMemoryStream : virtual public MemoryStream
	{
	public:
		IMemoryStream(const byte* buf_ptr, size_t buf_len);

		~IMemoryStream();

	public:
		IMemoryStream& Read(void* ptr, size_t len, const char* member = NULL);

		IMemoryStream& ReadString(std::string& str, const char* member = NULL);

		IMemoryStream& ReadString(std::wstring& wstr, const char* member = NULL);

		IMemoryStream& ReadString(char* str, size_t& len, size_t buf_len, const char* member = NULL);

		IMemoryStream& ReadString(wchar_t* wstr, size_t& len, size_t buf_len, const char* member = NULL);

		template <typename T>
		T ReadT(const char* member = NULL)
		{
			STATIC_CHECK_EXPRESSION( IS_ONLY_DATA_TYPE(T) );
			T retVal;
			this->Read((void*)&retVal, sizeof(T), member);
			return retVal;
		}

	protected:
		IMemoryStream& ReadUnchecked(void* ptr, size_t len) throw();

		IMemoryStream& ReadUnchecked(std::string& str) throw();

		IMemoryStream& ReadUnchecked(std::wstring& wstr) throw();

	protected:
		IMemoryStream& operator =(const IMemoryStream& other);

		IMemoryStream(const IMemoryStream& other);

	private:
	};
}


template <typename T>
static YXCLib::IMemoryStream& operator >>(YXCLib::IMemoryStream& is, T& value)
{
	STATIC_CHECK_EXPRESSION( IS_ONLY_DATA_TYPE(T) );

	return is.Read((void*)&value, sizeof(T), NULL);
}

#if !YXC_GNU_C
template <>
static YXCLib::IMemoryStream& operator >>(YXCLib::IMemoryStream& is, std::string& str)
{
	return is.ReadString(str);
}

template <>
static YXCLib::IMemoryStream& operator >>(YXCLib::IMemoryStream& is, std::wstring& wstr)
{
	return is.ReadString(wstr);
}
#else

template <>
YXCLib::IMemoryStream& operator >>(YXCLib::IMemoryStream& is, std::string& str)
{
	return is.ReadString(str);
}

template <>
YXCLib::IMemoryStream& operator >>(YXCLib::IMemoryStream& is, std::wstring& wstr)
{
	return is.ReadString(wstr);
}

#endif /* YXC_PLATFORM_WIN */

#endif /* __INC_YXC_SYS_BASE_IN_MEMORY_STREAM_H__ */

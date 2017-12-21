/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_OUT_MEMORY_STREAM_H__
#define __INC_YXC_SYS_BASE_OUT_MEMORY_STREAM_H__

#include <YXC_Sys/YXC_MemoryStream.hpp>

#ifdef __cplusplus

#include <string>

namespace YXCLib
{
	class YXC_CLASS OMemoryStream : virtual public MemoryStream
	{
	protected :
		int _manage_mem;

	public :
		OMemoryStream(size_t buf_len = 0);

		OMemoryStream(byte* buf_ptr, size_t buf_len);

		~OMemoryStream();
	public :
		OMemoryStream& Write(const void* buf, size_t len, const char* member = NULL);

		OMemoryStream& WriteString(const char* str, size_t len, const char* member = NULL);

		OMemoryStream& WriteString(const wchar_t* str, size_t len, const char* member = NULL);

		OMemoryStream& WriteString(const std::string& str, const char* member = NULL);

		OMemoryStream& WriteString(const std::wstring& str, const char* member = NULL);

		template <typename T>
		OMemoryStream& WriteT(T value, const char* member = NULL)
		{
			STATIC_CHECK_EXPRESSION( IS_ONLY_DATA_TYPE(T) );

			return this->Write((const void*)&value, sizeof(T), member);
		}

	protected :
		OMemoryStream& WriteUnchecked(const void* buf, size_t len) throw();

		OMemoryStream& WriteUnchecked(const std::string& str) throw();

		OMemoryStream& WriteUnchecked(const std::wstring& str) throw();

	protected :
		void _PromiseMemory(size_t mem_size);

		void _CheckValidRange(size_t cur_off, size_t need_len, size_t max_len, const char* member);

	protected:
		OMemoryStream& operator =(const OMemoryStream& other);

		OMemoryStream(const OMemoryStream& other);

	private:
		size_t _def_buf_len;
		//template <typename T>
		//friend OMemoryStream& operator <<(OMemoryStream& os, const T& value);
	};
}

template <typename T>
static inline YXCLib::OMemoryStream& operator <<(YXCLib::OMemoryStream& os, const T& val)
{
	STATIC_CHECK_EXPRESSION( IS_ONLY_DATA_TYPE(T) );

	return os.Write((const void*)&val, sizeof(T));
}

#if !YXC_GNU_C
template <>
static inline YXCLib::OMemoryStream& operator <<(YXCLib::OMemoryStream& os, const std::string& str)
{
	return os.WriteString(str);
}

template <>
static inline YXCLib::OMemoryStream& operator <<(YXCLib::OMemoryStream& os, const std::wstring& str)
{
	return os.WriteString(str);
}
#else

template <>
inline YXCLib::OMemoryStream& operator <<(YXCLib::OMemoryStream& os, const std::string& str)
{
	return os.WriteString(str);
}

template <>
inline YXCLib::OMemoryStream& operator <<(YXCLib::OMemoryStream& os, const std::wstring& str)
{
	return os.WriteString(str);
}

#endif /* YXC_PLATFORM_WIN */

#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_OUT_MEMORY_STREAM_H__ */

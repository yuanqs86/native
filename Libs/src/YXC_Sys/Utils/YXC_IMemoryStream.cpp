#include <YXC_Sys/YXC_IMemoryStream.hpp>

namespace YXCLib
{
	IMemoryStream::IMemoryStream(const byte* buf_ptr, size_t buf_len)
		: MemoryStream(const_cast<byte*>(buf_ptr), buf_len)
	{
	}

	IMemoryStream::~IMemoryStream() {}

	IMemoryStream& IMemoryStream::Read(void* ptr, size_t len, const char* member)
	{
		_CheckValidRange(this->_off, len, this->_buf_len, member);
		return ReadUnchecked(ptr, len);
	}

	IMemoryStream& IMemoryStream::ReadString(std::string& str, const char* member)
	{
		size_t str_len;
		this->Read(&str_len, sizeof(size_t), member);

		this->_CheckValidRange(this->_off, str_len * sizeof(char), this->_buf_len, member);

		str.assign((char*)(this->_buf_ptr + this->_off), str_len);
		this->_off += str_len * sizeof(char);
		return *this;
	}

	IMemoryStream& IMemoryStream::ReadString(std::wstring& str, const char* member)
	{
		size_t str_len;
		this->Read(&str_len, sizeof(size_t), member);

		this->_CheckValidRange(this->_off, str_len * sizeof(wchar_t), this->_buf_len, member);

		str.assign((wchar_t*)(this->_buf_ptr + this->_off), str_len);
		this->_off += str_len * sizeof(wchar_t);
		return *this;
	}

	IMemoryStream& IMemoryStream::ReadString(char* str, size_t& len, size_t buf_len, const char* member)
	{
		this->Read(&len, sizeof(size_t), member);

		this->_CheckValidRange(this->_off, len * sizeof(char), this->_buf_len, member);

		size_t copy_len = YXCLib::TMin(len, buf_len - 1);
		memcpy(str, this->_buf_ptr + this->_off, sizeof(char) * copy_len);
		this->_off += len * sizeof(char);
		str[copy_len] = 0;
		return *this;
	}

	IMemoryStream& IMemoryStream::ReadString(wchar_t* wstr, size_t& len, size_t buf_len, const char* member)
	{
		this->Read(&len, sizeof(size_t), member);

		this->_CheckValidRange(this->_off, len * sizeof(wchar_t), this->_buf_len, member);

		size_t copy_len = YXCLib::TMin(len, buf_len - 1);
		memcpy(wstr, this->_buf_ptr + this->_off, sizeof(wchar_t) * copy_len);
		this->_off += len * sizeof(wchar_t);
		wstr[copy_len] = 0;
		return *this;
	}

	IMemoryStream& IMemoryStream::ReadUnchecked(void* ptr, size_t len) throw()
	{
		memcpy(ptr, this->_buf_ptr + this->_off, len);
		this->_off += len;
		return *this;
	}

	IMemoryStream& IMemoryStream::ReadUnchecked(std::string& str) throw()
	{
		size_t len;
		ReadUnchecked(&len, sizeof(size_t));

		str.assign((char*)this->_buf_ptr + this->_off, len);
		this->_off += len;
		return *this;
	}

	IMemoryStream& IMemoryStream::ReadUnchecked(std::wstring& wstr) throw()
	{
		size_t len;
		ReadUnchecked(&len, sizeof(size_t));

		wstr.assign((wchar_t*)this->_buf_ptr + this->_off, len);
		this->_off += len;
		return *this;
	}
}

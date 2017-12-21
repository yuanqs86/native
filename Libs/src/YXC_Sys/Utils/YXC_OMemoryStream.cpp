#include <YXC_Sys/YXC_OMemoryStream.hpp>

#define MIN_WRITE_MEM (1 << 12) // 4K

namespace YXCLib
{
	OMemoryStream::OMemoryStream(byte* buf_ptr, size_t buf_len)
		: MemoryStream(buf_ptr, buf_len), _manage_mem(0)
	{
	}

	OMemoryStream::OMemoryStream(size_t buf_len) : MemoryStream(NULL, 0), _manage_mem(1)
	{
		buf_len = (buf_len < MIN_WRITE_MEM) ? MIN_WRITE_MEM : buf_len;
		this->_buf_ptr = NULL;
		this->_def_buf_len = buf_len;
	}

	OMemoryStream::~OMemoryStream()
	{
		if (this->_manage_mem && this->_buf_ptr)
		{
			delete [] this->_buf_ptr;
		}
	}

	void OMemoryStream::_PromiseMemory(size_t mem_size)
	{
		try
		{
			if (this->_buf_len < mem_size)
			{
				byte* new_ptr = NULL;
				if (this->_buf_ptr == NULL)
				{
					mem_size = YXCLib::TMax(mem_size, this->_def_buf_len);
					new_ptr = new byte[mem_size];
				}
				else
				{
					mem_size = 3 * mem_size;
					new_ptr = new byte[mem_size];
					memcpy(new_ptr, this->_buf_ptr, this->_buf_len);
					delete [] this->_buf_ptr;
				}
				this->_buf_ptr = new_ptr;
				this->_buf_len = mem_size;
			}
		}
		catch (std::bad_alloc& e)
		{
			throw StreamException(e.what());
		}
	}

	void OMemoryStream::_CheckValidRange(size_t cur_off, size_t need_len, size_t max_len, const char* member)
	{
		if (this->_manage_mem)
		{
			this->_PromiseMemory(cur_off + need_len);
		}
		else
		{
			this->MemoryStream::_CheckValidRange(cur_off, need_len, max_len, member);
		}
	}

	OMemoryStream& OMemoryStream::Write(const void* buf, size_t len, const char* member)
	{
		this->_CheckValidRange(this->_off, len, this->_buf_len, member);
		return this->WriteUnchecked(buf, len);
	}

	OMemoryStream& OMemoryStream::WriteString(const std::string &str, const char* member)
	{
		return this->Write(str.c_str(), str.size(), member);
	}

	OMemoryStream& OMemoryStream::WriteString(const char* str, size_t len, const char* member)
	{
		this->Write(&len, sizeof(size_t), member);
		this->Write((const void*)str, len * sizeof(char), member);
		return *this;
	}

	OMemoryStream& OMemoryStream::WriteString(const std::wstring &wstr, const char* member)
	{
		return this->Write(wstr.c_str(), wstr.size(), member);
	}

	OMemoryStream& OMemoryStream::WriteString(const wchar_t* wstr, size_t len, const char* member)
	{
		this->Write(&len, sizeof(size_t), member);
		this->Write((const void*)wstr, len * sizeof(wchar_t), member);
		return *this;
	}

	OMemoryStream& OMemoryStream::WriteUnchecked(const void *buf, size_t len) throw()
	{
		memcpy(this->_buf_ptr + this->_off, buf, len);
		this->_off += len;
		return *this;
	}

	OMemoryStream& OMemoryStream::WriteUnchecked(const std::string& str) throw()
	{
		size_t len = str.length();
		this->WriteUnchecked(&len, sizeof(size_t));
		this->WriteUnchecked(str.c_str(), sizeof(wchar_t));
		return *this;
	}

	OMemoryStream& OMemoryStream::WriteUnchecked(const std::wstring& str) throw()
	{
		size_t len = str.length();
		this->WriteUnchecked(&len, sizeof(size_t));
		this->WriteUnchecked(str.c_str(), len * sizeof(wchar_t));
		return *this;
	}
}

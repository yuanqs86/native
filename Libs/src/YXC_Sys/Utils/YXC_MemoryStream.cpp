#include <YXC_Sys/YXC_MemoryStream.hpp>
#include <cstdio>
#include <exception>

namespace YXCLib
{
	MemoryStream::MemoryStream(byte* buf_ptr, size_t buf_len) : _buf_ptr(buf_ptr), _buf_len(buf_len), _off() {}

	MemoryStream::~MemoryStream() {}

	void MemoryStream::_EOSError(size_t cur_off, size_t need_len, size_t max_len, const char* member)
	{
		char err_msg[MAX_EXCEPTION_MESSAGE + 1];
		sprintf(err_msg, "Unexpected end of stream when streaming member(%s) : current offset %d, read length %d, total length %d",
			member, (int)cur_off, (int)need_len, (int)max_len);
		throw StreamException(err_msg);
	}

	void MemoryStream::_CheckValidRange(size_t cur_off, size_t need_len, size_t max_len, const char* member)
	{
		if (cur_off + need_len > max_len)
		{
			_EOSError(cur_off, need_len, max_len, member);
		}
	}

	void MemoryStream::_EOSError()
	{
		throw StreamException("Unexpected end of stream.");
	}
}

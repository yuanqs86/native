/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_MEMORY_STREAM_H__
#define __INC_YXC_SYS_BASE_MEMORY_STREAM_H__

#include <YXC_Sys/YXC_TypeDealer.hpp>
#include <YXC_Sys/YXC_Sys.h>
#include <exception>
#include <string>

#define MAX_EXCEPTION_MESSAGE (256 - 1)
#define STREAM_ERR_EOS (-1)
#define STREAM_ERR_INVALID (-2)

typedef unsigned char byte;

namespace YXCLib
{
	class YXC_CLASS MemoryStream
	{
	protected:
		size_t _buf_len;

		size_t _off;

		byte* _buf_ptr;

	protected:
		MemoryStream(byte* buf_ptr, size_t buf_len);

	public:
		virtual ~MemoryStream() = 0;

	public:
		inline size_t Length() const { return this->_off; }

		inline size_t BufLength() const { return this->_buf_len; }

		inline byte* BufferPtr() const { return this->_buf_ptr; }

		inline size_t GetPosition() const { return this->_off; }

		inline void SetPosition(size_t off) {
            this->_off = off;
            _CheckValidRange(off, 0, this->_buf_len, "_SetPosition");
        }

	protected:
		void _EOSError();

		virtual void _CheckValidRange(size_t cur_off, size_t need_len, size_t max_len, const char* member);

		void _EOSError(size_t cur_off, size_t need_len, size_t max_len, const char* member);
	};

	class StreamException : public std::exception
	{
	public:
		StreamException(const char* what) : std::exception(), _what(what) {}
		~StreamException() throw() {}
		StreamException(const StreamException& other) : std::exception(), _what(other.what()) {}
		StreamException& operator =(const StreamException& other)
		{
			this->std::exception::operator =(other);
			return *this;
		}

		virtual const char* what() const throw() { return this->_what.c_str(); }

    private:
        std::string _what;
	};
}

#endif /* __INC_YXC_SYS_BASE_MEMORY_STREAM_H__ */

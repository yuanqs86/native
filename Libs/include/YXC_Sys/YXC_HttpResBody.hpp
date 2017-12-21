/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_HTTP_RESPONSE_BODY_HPP__
#define __INC_YXC_SYS_BASE_HTTP_RESPONSE_BODY_HPP__

#include <time.h>
#include <YXC_Sys/YXC_SysInc.h>
#include <YXC_Sys/YXC_HttpClient.h>

namespace YXCLib
{
	typedef YXC_HttpResData HttpResponse;

	// Represents a single peer connected to the server.
	class YXC_CLASS HttpResponseBody
	{
	public:
		HttpResponseBody();

		virtual ~HttpResponseBody();

	public:
		ybool_t IsResponseReceived() const;

	public:
		void GetFullResponse(HttpResponse* request_data);

		YXC_Status AppendPacket(const void* data, yuint32_t len);

		void ClearChannel();

	private:
		YXC_Status _ParseHeader();

		YXC_Status _ParseChunked();

		YXC_Status _ParseHeaderExtenstions(const char* headers, int length);

		YXC_Status _AddHeaderDesc(const char* key_begin, const char* key_end, const char* val_begin, const char* val_end);

		YXC_Status _ParseCodeAndDesc(int len);

	public:
		static YXC_Status BuildHttpResponse(const HttpResponse* response_data, YXC_Buffer* buffer);

		static YXC_Status BuildStatusResponse(int code, const char* code_desc, const char* response_text,
			int close_connection, YXC_Buffer* buffer);

		static void FreeResponseBuffer(YXC_Buffer* buffer);

	protected:
		ybyte_t* _pTempData;
		ysize_t _stCbTempBuf;
		ysize_t _stCbTempData;
		ybool_t _chunked;
		ysize_t _stCbChunkPos;
		ysize_t _stCbDataInChunk;

		ybool_t _bHeaderParsed;
		ysize_t _stCbHttpHeader;

		char _szMethod[10];

		YXC_Buffer _hdrKeyArr;
		YXC_Buffer _hdrValArr;
		ysize_t _stNumHeaders;

		/* Header extensions. */
		ysize_t _stCbHttpBody;
		int _responseCode;
		char* _responseText;
		char* _pszContentType;
		ybool_t _bCloseConnection;

		YXCLib::CYMAlloc _emp;
	};
}

#endif  /* __INC_YXC_SYS_BASE_HTTP_RESPONSE_BODY_HPP__ */

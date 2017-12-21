/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_HTTP_REQUEST_BODY_HPP__
#define __INC_YXC_SYS_BASE_HTTP_REQUEST_BODY_HPP__

#include <time.h>
#include <YXC_Sys/YXC_SysInc.h>
#include <YXC_Sys/YXC_HttpClient.h>

namespace YXCLib
{
	typedef YXC_HttpReqData HttpRequest;

	// Represents a single peer connected to the server.
	class YXC_CLASS HttpRequestBody
	{
	public:
		HttpRequestBody();

		virtual ~HttpRequestBody();

	public:
		ybool_t IsRequestReceived() const;

	public:
		void GetFullRequest(HttpRequest* request_data);

		YXC_Status AppendPacket(const void* data, yuint32_t len);

		void ClearChannel();

	public:
		static YXC_Status BuildHttpRequest(const HttpRequest* request_data, YXC_Buffer* buffer);

		static void FreeRequestBuffer(YXC_Buffer* buffer);

	private:
		YXC_Status _ParseHeader();

		YXC_Status _ParseArguments(const char* begin, const char* end);

		YXC_Status _ParseMethodAndPath(int len);

		YXC_Status _ParseHeaderExtenstions(const char* headers, int length);

		YXC_Status _AddHeaderDesc(const char* key_begin, const char* key_end, const char* val_begin, const char* val_end);

		YXC_Status _AddArgumentDesc(const char* key_begin, const char* key_end, const char* val_begin, const char* val_end);

	protected:
		ybyte_t* _pTempData;
		ysize_t _stCbTempBuf;
		ysize_t _stCbTempData;
		ybool_t _bHeaderParsed;
		ysize_t _stCbHttpHeader;

		char _szMethod[10];

		char* _szReqPath;
		ysize_t _stCbReqPath;

		YXC_Buffer _hdrKeyArr;
		YXC_Buffer _hdrValArr;
		ysize_t _stNumHeaders;

		YXC_Buffer _argKeyArr;
		YXC_Buffer _argValArr;
		ysize_t _stNumArguments;

		/* Header extensions. */
		ysize_t _stCbHttpBody;
		char* _pszContentType;
		ybool_t _bCloseConnection;

		YXCLib::CYMAlloc _emp;
	};
}

#endif  /* __INC_YXC_SYS_BASE_HTTP_REQUEST_BODY_HPP__ */

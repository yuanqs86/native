/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_HTTP_NSM_CLIENT_HPP__
#define __INC_YXC_SYS_BASE_HTTP_NSM_CLIENT_HPP__

#include <time.h>
#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_MMInterface.h>
#include <YXC_Sys/YXC_HttpReqBody.hpp>
#include <YXC_Sys/YXC_HttpResBody.hpp>

namespace YXCLib
{
	// Represents a single peer connected to the server.
	class YXC_CLASS HttpNSMClient : public YXCLib::NSMClientBase
	{
	public:
		HttpNSMClient(const YXC_SockAddr* pAddr);

		virtual ~HttpNSMClient();

	public: /* Virtual functions, must be called in its derived class. */
		virtual void _OnReceive(const YXC_NetPackage* pPackage);

	public:
		virtual void OnHttpRequest(const HttpRequest* request_data) = 0;

	public:
		YXC_Status SendHttpResponse(const HttpResponse* response_data);

		YXC_Status SendPacketResponse(yuint32_t uCbPacket, const void* packet, int close_connection);

		YXC_Status SendStatusResponse(int code, const char* code_desc, const char* response_text, int close_connection);

	protected:
		HttpRequestBody _req;
	};
}

#endif  /* __INC_YXC_SYS_BASE_HTTP_NSM_CLIENT_HPP__ */

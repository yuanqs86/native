#define __MODULE__ "EK.Http.NSMClient"
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_HttpNSMClient.hpp>

namespace YXCLib
{
	HttpNSMClient::HttpNSMClient(const YXC_SockAddr* pAddr) : YXCLib::NSMClientBase(pAddr), _req()
	{
	}

	HttpNSMClient::~HttpNSMClient()
	{
		this->_req.ClearChannel();
	}

	YXC_Status HttpNSMClient::SendStatusResponse(int code, const char* code_desc, const char* response_text, int close_connection)
	{
		HttpResponse response = {0};
		response.connection_close = close_connection;
		response.content_data = (ybyte_t*)response_text;
		response.content_length = response_text == NULL ? 0 : strlen(response_text);
		response.content_type = "text/plain";
		response.response_code = code;
		response.response_text = code_desc;

		return this->SendHttpResponse(&response);
	}

	YXC_Status HttpNSMClient::SendHttpResponse(const HttpResponse* response_data)
	{
		YXC_Buffer buffer;
		YXC_Status rc = HttpResponseBody::BuildHttpResponse(response_data, &buffer);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<YXC_Buffer*> x_buffer_res(&buffer, HttpResponseBody::FreeResponseBuffer);
		return this->SendPacketResponse(buffer.stBufSize, buffer.pBuffer, response_data->connection_close);
	}

	YXC_Status HttpNSMClient::SendPacketResponse(yuint32_t uCbPacket, const void* packet, int close_connection)
	{
		YXC_SocketOptValue npOpt;
		YXC_Status rc = this->GetSockOpt(YXC_SOCKET_OPT_NET_PROTOCOL, &npOpt);
		_YXC_CHECK_RC_RET(rc);

		YXC_NetPackage pack;
		YXC_NPStdCreatePackage(&npOpt.sockProtocol, uCbPacket, packet, 0, TRUE, &pack);

		rc = this->SendPackage(&pack, YXC_INFINITE);
		_YXC_CHECK_RC_RET(rc);

		this->_req.ClearChannel();
		if (close_connection)
		{
			this->NotifyClose();
		}
		return YXC_ERC_SUCCESS;
	}

	void HttpNSMClient::_OnReceive(const YXC_NetPackage* pPackage)
	{
		YXC_Status rc = this->_req.AppendPacket(pPackage->pvData, pPackage->uDataLen);
		if (rc != YXC_ERC_SUCCESS)
		{
			ychar szErr[1024];
			char szMsg[2048];

			YXC_FormatError(YXC_GetLastError(), szErr, 1024 - 1);
			YXC_TEECharToUTF8(szErr, YXC_STR_NTS, (yuint8_t*)szMsg, 2048 - 1, NULL, NULL);
			this->SendStatusResponse(500, "Server Internal Error", szMsg, TRUE);
			this->NotifyClose();
			return;
		}

		if (this->_req.IsRequestReceived())
		{
			HttpRequest request;
			this->_req.GetFullRequest(&request);
			this->OnHttpRequest(&request);
		}
	}
}

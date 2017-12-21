#define __MODULE__ "EK.Http.NSMClient"
#include <EJ_Sys/EJ_ErrMacros.hpp>
#include <EJ_Sys/EJ_HttpNSMClient.hpp>

namespace EJDev
{
	HttpNSMClient::HttpNSMClient(const EJ_SockAddr* pAddr) : EJDev::NSMClientBase(pAddr), _req()
	{
	}

	HttpNSMClient::~HttpNSMClient()
	{
		this->_req.ClearChannel();
	}

	EJ_Status HttpNSMClient::SendStatusResponse(int code, const char* code_desc, const char* response_text, int close_connection)
	{
		HttpResponse response = {0};
		response.connection_close = close_connection;
		response.content_data = (ebyte_t*)response_text;
		response.content_length = response_text == NULL ? 0 : strlen(response_text);
		response.content_type = "text/plain";
		response.response_code = code;
		response.response_text = code_desc;
		
		return this->SendHttpResponse(&response);
	}

	EJ_Status HttpNSMClient::SendHttpResponse(const HttpResponse* response_data)
	{
		EJ_Buffer buffer;
		EJ_Status rc = HttpResponseBody::BuildHttpResponse(response_data, &buffer);
		_EJ_CHECK_RC_RET(rc);

		EJ::HandleRef<EJ_Buffer*> x_buffer_res(&buffer, HttpResponseBody::FreeResponseBuffer);
		return this->SendPacketResponse(buffer.stBufSize, buffer.pBuffer, response_data->connection_close);
	}

	EJ_Status HttpNSMClient::SendPacketResponse(euint32_t uCbPacket, const void* packet, int close_connection)
	{
		EJ_SocketOptValue npOpt;
		EJ_Status rc = this->GetSockOpt(EJ_SOCKET_OPT_NET_PROTOCOL, &npOpt);
		_EJ_CHECK_RC_RET(rc);

		EJ_NetPackage pack;
		EJ_NPStdCreatePackage(&npOpt.sockProtocol, uCbPacket, packet, 0, TRUE, &pack);

		rc = this->SendPackage(&pack, EJ_INFINITE);
		_EJ_CHECK_RC_RET(rc);

		this->_req.ClearChannel();
		if (close_connection)
		{
			this->NotifyClose();
		}
		return EJ_ERC_SUCCESS;
	}

	void HttpNSMClient::_OnReceive(const EJ_NetPackage* pPackage)
	{
		EJ_Status rc = this->_req.AppendPacket(pPackage->pvData, pPackage->uDataLen);
		if (rc != EJ_ERC_SUCCESS)
		{
			echar szErr[1024];
			char szMsg[2048];

			EJ_FormatError(EJ_GetLastError(), szErr, 1024 - 1);
			EJ_TEECharToUTF8(szErr, EJ_STR_NTS, (euint8_t*)szMsg, 2048 - 1, NULL, NULL);
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

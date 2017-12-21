#define __MODULE__ "EK.Sys.HttpReq"

#include <EJ_Sys/EJ_ErrMacros.hpp>
#include <EJ_Sys/EJ_HttpReqBody.hpp>  
#include <EJ_Sys/EJ_HttpResBody.hpp>

namespace
{
	static char kHeaderTerminator[] = "\r\n\r\n";
	static char kLineTerminator[] = "\r\n";
	static const char kContentLength[] = "Content-Length";
	static const char kContentType[] = "Content-Type";
	static const char kConnection[] = "Connection";

	static void _StrFindReadStartEnd(const char*& begin, const char*& end)
	{
		while (begin < end)
		{
			if (*begin != ' ')
			{
				break;
			}
			++begin;
		}
		const char* last = end - 1;
		while (begin <= last)
		{
			if (*last != ' ')
			{
				break;
			}
			--last;
		}
		end = last + 1;
	}
}

namespace EJDev
{
	HttpRequestBody::HttpRequestBody() : _pTempData(NULL), _stCbTempData(0), _stCbTempBuf(0), _stCbHttpBody(0), _stCbHttpHeader(0), _bHeaderParsed(FALSE), 
		_szReqPath(NULL), _stCbReqPath(0), _stNumHeaders(0), _stNumArguments(0), _bCloseConnection(TRUE), _pszContentType(NULL)
	{
		memset(&this->_hdrKeyArr, 0, sizeof(this->_hdrKeyArr));
		memset(&this->_hdrValArr, 0, sizeof(this->_hdrValArr));
		memset(&this->_argKeyArr, 0, sizeof(this->_argKeyArr));
		memset(&this->_argValArr, 0, sizeof(this->_argValArr));
	}

	HttpRequestBody::~HttpRequestBody()
	{
		this->ClearChannel();

		if (this->_pTempData)
		{
			EJ_MMCFreeData(this->_pTempData);
			this->_pTempData = NULL;
		}

		if (this->_szReqPath)
		{
			EJ_MMCFreeData(this->_szReqPath);
			this->_szReqPath = NULL;
		}
	}

	void HttpRequestBody::ClearChannel()
	{
		this->_stCbTempData = 0;
		this->_stCbReqPath = 0;
		this->_emp.Clear();
		this->_stNumHeaders = 0;
		this->_stNumArguments = 0;
		this->_bCloseConnection = TRUE;
		this->_pszContentType = NULL;
		this->_stCbHttpBody = 0;
		this->_bHeaderParsed = FALSE;
		this->_stCbHttpHeader = 0;
	}

	EJ_Status HttpRequestBody::_AddHeaderDesc(const char* key_begin, const char* key_end, const char* val_begin, const char* val_end)
	{
		EJ_Status rc = EJ_MMCExpandBuffer(&this->_hdrKeyArr.pBuffer, &this->_hdrKeyArr.stBufSize, (this->_stNumHeaders + 1) * sizeof(char*));
		_EJ_CHECK_RC_RET(rc);

		rc = EJ_MMCExpandBuffer(&this->_hdrValArr.pBuffer, &this->_hdrValArr.stBufSize, (this->_stNumHeaders + 1) * sizeof(char*));
		_EJ_CHECK_RC_RET(rc);

		int key_len = key_end - key_begin, val_len = val_end - val_begin;

		_ECHK_EAL_STRA_R1(new_key, this->_emp, key_len);
		_ECHK_EAL_STRA_R1(new_val, this->_emp, val_len);

		memcpy(new_key, key_begin, key_len);
		new_key[key_len] = 0;
		memcpy(new_val, val_begin, val_len);
		new_val[val_len] = 0;

		char** ppKeyArr = (char**)this->_hdrKeyArr.pBuffer, **ppValArr = (char**)this->_hdrValArr.pBuffer;
		ppKeyArr[this->_stNumHeaders] = new_key;
		ppValArr[this->_stNumHeaders] = new_val;

		++this->_stNumHeaders;
		if (key_len == EJ_STRING_ARR_LEN(kContentLength) && strncmp(key_begin, kContentLength, key_len) == 0)
		{
			this->_stCbHttpBody = atoi(new_val);
		}
		else if (key_len == EJ_STRING_ARR_LEN(kContentType) && strncmp(key_begin, kContentType, key_len) == 0)
		{
			this->_pszContentType = new_val;
		}
		else if (key_len == EJ_STRING_ARR_LEN(kConnection) && strncmp(key_begin, kConnection, key_len) == 0)
		{
			if (stricmp(new_val, "Close") == 0)
			{
				this->_bCloseConnection = TRUE;
			}
			else
			{
				this->_bCloseConnection = FALSE;
			}
		}

		return EJ_ERC_SUCCESS;
	}

	EJ_Status HttpRequestBody::_ParseHeaderExtenstions(const char* headers, int length)
	{
		const char* end = headers + length;
		while (headers < end)
		{
			const char* pos = strchr(headers, ':'); /* Found request header name. */
			if (pos > end) pos = NULL;
			const char* key_begin = headers, *key_end = NULL;
			const char* val_begin = NULL, *val_end = NULL;
			if (pos != NULL)
			{
				val_begin = pos + 1;
				key_end = pos;
			}

			const char* next_line = strstr(headers, "\r\n");
			if (!next_line)
			{
				break;
			}
			val_end = next_line;
			headers = next_line + 2;

			if (val_begin && val_begin < end)
			{
				_StrFindReadStartEnd(key_begin, key_end);
				_StrFindReadStartEnd(val_begin, val_end);
				EJ_Status rc = this->_AddHeaderDesc(key_begin, key_end, val_begin, val_end);
				_EJ_CHECK_RC_RET(rc);
			}
		}

		return EJ_ERC_SUCCESS;
	}

	EJ_Status HttpRequestBody::_AddArgumentDesc(const char* key_begin, const char* key_end, const char* val_begin, const char* val_end)
	{
		EJ_Status rc = EJ_MMCExpandBuffer(&this->_argKeyArr.pBuffer, &this->_argKeyArr.stBufSize, (this->_stNumArguments + 1) * sizeof(char*));
		_EJ_CHECK_RC_RET(rc);

		rc = EJ_MMCExpandBuffer(&this->_argValArr.pBuffer, &this->_argValArr.stBufSize, (this->_stNumArguments + 1) * sizeof(char*));
		_EJ_CHECK_RC_RET(rc);

		int key_len = key_end - key_begin, val_len = val_end - val_begin;

		_ECHK_EAL_STRA_R1(new_key, this->_emp, key_len);
		_ECHK_EAL_STRA_R1(new_val, this->_emp, val_len);

		memcpy(new_key, key_begin, key_len);
		new_key[key_len] = 0;
		memcpy(new_val, val_begin, val_len);
		new_val[val_len] = 0;

		char** ppKeyArr = (char**)this->_argKeyArr.pBuffer, **ppValArr = (char**)this->_argValArr.pBuffer;
		ppKeyArr[this->_stNumArguments] = new_key;
		ppValArr[this->_stNumArguments] = new_val;

		++this->_stNumArguments;
		return EJ_ERC_SUCCESS;
	}

	EJ_Status HttpRequestBody::_ParseArguments(const char* begin, const char* end)
	{
		while (begin < end)
		{
			const char* pos = strchr(begin, '='); /* Found request header name. */
			const char* key_begin = begin, *key_end = NULL;
			const char* val_begin = NULL, *val_end = NULL;

			if (pos == NULL || pos > end) break;

			key_end = pos;
			val_begin = pos + 1;

			const char* next_line = strchr(pos + 1, '&');
			const char* next_line2 = strchr(pos + 1, ' ');

			if (next_line == NULL && next_line2 == NULL)
			{
				val_end = end;
			}
			else
			{
				val_end = next_line == NULL ? next_line2 : next_line;
			}

			// _EJ_CHECK_REPORT_NEW_RET(val_end >= val_begin, EJ_ERC_INVALID_PARAMETER, EC("Found & before = in http request"));

			if (val_begin)
			{
				EJ_Status rc = this->_AddArgumentDesc(key_begin, key_end, val_begin, val_end);
				_EJ_CHECK_RC_RET(rc);
			}

			if (!next_line)
			{
				break;
			}
			begin = next_line + 1;
		}

		return EJ_ERC_SUCCESS;
	}

	EJ_Status HttpRequestBody::_ParseHeader()
	{
		char* header_found = (char*)strstr((char*)this->_pTempData, kHeaderTerminator);
		if (header_found)
		{
			this->_stCbHttpHeader = header_found - (char*)this->_pTempData + EJ_STRING_ARR_LEN(kHeaderTerminator);

			char* pi = (char*)strstr((char*)this->_pTempData, kLineTerminator);
			int len = pi - (char*)this->_pTempData;

			EJ_Status rc = this->_ParseMethodAndPath(len);
			_EJ_CHECK_RC_RET(rc);

			const char* headers = pi + EJ_STRING_ARR_LEN(kLineTerminator);
			int remain_len = this->_stCbHttpHeader - len - EJ_STRING_ARR_LEN(kLineTerminator);

			rc = this->_ParseHeaderExtenstions(headers, remain_len);
			_EJ_CHECK_RC_RET(rc);

			this->_bHeaderParsed = TRUE;
			return EJ_ERC_SUCCESS;
		}
		else
		{
			return EJ_ERC_SUCCESS;
		}
	}

	EJ_Status HttpRequestBody::_ParseMethodAndPath(int len)
	{
		const char* begin = (char*)this->_pTempData;
		struct {
			const char* method_name;
			int method_name_len;
		} supported_methods[] = {
			{ "GET", 3 },
			{ "POST", 4 },
			{ "OPTIONS", 7 },
		};

		const char* path = NULL;
		for (int i = 0; i < EJ_ARR_COUNT(supported_methods); ++i) {
			if (len > supported_methods[i].method_name_len &&
				begin[supported_methods[i].method_name_len] == ' ' &&
				strncmp(begin, supported_methods[i].method_name,
				supported_methods[i].method_name_len) == 0) {
					strncpy(this->_szMethod, supported_methods[i].method_name, supported_methods[i].method_name_len);
					this->_szMethod[supported_methods[i].method_name_len] = 0;
					path = begin + supported_methods[i].method_name_len;
					break;
			}
		}

		const char* end = begin + len;
		_EJ_CHECK_REPORT_NEW_RET(path && path < end, EJ_ERC_NET_INVALID_PROTOCOL, EC("Invalid http protocol"));

		ebool_t bHasArgs = FALSE;

		++path;
		begin = path;
		while (!(*path == ' ') && path < end)
		{
			if (*path == '?')
			{
				bHasArgs = TRUE;
				break;
			}
			++path;
		}

		EJ_Status rc = EJ_MMCExpandBuffer((void**)&this->_szReqPath, &this->_stCbReqPath, path - begin + 1);
		_EJ_CHECK_RC_RET(rc);

		memcpy(this->_szReqPath, begin, path - begin);
		this->_szReqPath[path - begin] = 0;

		if (bHasArgs)
		{
			++path;
			rc = this->_ParseArguments(path, end);
			_EJ_CHECK_RC_RET(rc);
		}

		return EJ_ERC_SUCCESS;
	}

	ebool_t HttpRequestBody::IsRequestReceived() const
	{
		ebool_t bFullPacket = this->_bHeaderParsed && this->_stCbTempData == this->_stCbHttpHeader + this->_stCbHttpBody;
		return bFullPacket;
	}

	void HttpRequestBody::GetFullRequest(HttpRequest* request_data)
	{
		request_data->connection_close = this->_bCloseConnection;
		request_data->content_data = this->_pTempData + this->_stCbHttpHeader;
		request_data->content_length = this->_stCbHttpBody;
		request_data->content_type = this->_pszContentType;
		request_data->header_key_arr = (const char**)this->_hdrKeyArr.pBuffer;
		request_data->header_val_arr = (const char**)this->_hdrValArr.pBuffer;
		request_data->arguments_key_arr = (const char**)this->_argKeyArr.pBuffer;
		request_data->arguments_val_arr = (const char**)this->_argValArr.pBuffer;
		request_data->num_arguments = this->_stNumArguments;
		request_data->num_headers = this->_stNumHeaders;
		request_data->req_method = this->_szMethod;
		request_data->req_path = this->_szReqPath;
	}

	EJ_Status HttpRequestBody::AppendPacket(const void* data, euint32_t len)
	{
		_EJ_CHECK_REPORT_NEW_RET(!this->IsRequestReceived(), EJ_ERC_INVALID_STATUS, EC("Received another request before response data."));

		EJ_Status rc = EJ_MMCExpandBuffer((void**)&this->_pTempData, &this->_stCbTempBuf, this->_stCbTempData + len + 1);
		_EJ_CHECK_RC_RET(rc);

		memcpy(this->_pTempData + this->_stCbTempData, data, len);
		this->_stCbTempData += len;
		this->_pTempData[this->_stCbTempData] = 0; /* \0 terminated. */

		if (!this->_bHeaderParsed)
		{
			rc = this->_ParseHeader();
			_EJ_CHECK_RC_RET(rc);
		}

		if (!this->_bHeaderParsed)
		{
			_EJ_CHECK_REPORT_NEW_RET(this->_stCbTempData < (1 << 20), EJ_ERC_BUFFER_NOT_ENOUGH, EC("Not a valid http request header(%d)."), 
				this->_stCbTempData);
		}

		return EJ_ERC_SUCCESS;
	}

	void HttpRequestBody::FreeRequestBuffer(EJ_Buffer* buffer)
	{
		free(buffer->pBuffer);
	}

	EJ_Status HttpRequestBody::BuildHttpRequest(const HttpRequest* request_data, EJ_Buffer* buffer)
	{		
		eint64_t nLenHead = ejcwrap_scprintf("%s %s HTTP/1.1\r\n", request_data->req_method, request_data->req_path);

		eint64_t nLenTotal = nLenHead;
		for (euint32_t i = 0; i < request_data->num_arguments; ++i)
		{
			nLenTotal += EJ_STRING_ARR_LEN("?"); /* ? & and = */
			nLenTotal += strlen(request_data->arguments_key_arr[i]);
			nLenTotal += EJ_STRING_ARR_LEN("="); /* ? & and = */
			nLenTotal += strlen(request_data->arguments_val_arr[i]);
		}

		ebool_t bCLength = FALSE, bCType = FALSE;
		if (strncmp(request_data->req_method, "GET", EJ_STRING_ARR_LEN("GET")) == 0)
		{
			bCLength = TRUE;
			bCType = TRUE;
		}

		for (euint32_t i = 0; i < request_data->num_headers; ++i)
		{
			if (stricmp(request_data->header_key_arr[i], "Content-Type") == 0)
			{
				bCType = TRUE;
			}

			if (stricmp(request_data->header_key_arr[i], "Content-Length") == 0)
			{
				bCLength = TRUE;
			}
			nLenTotal += strlen(request_data->header_key_arr[i]);
			nLenTotal += EJ_STRING_ARR_LEN(": ");
			nLenTotal += strlen(request_data->header_val_arr[i]);
			nLenTotal += EJ_STRING_ARR_LEN("\r\n");
		}

		if (!bCLength)
		{ 
			char szLen[20];
			sprintf(szLen, "%u", request_data->content_length, 10);
			nLenTotal += strlen("Content-Length");
			nLenTotal += EJ_STRING_ARR_LEN(": ");  
			nLenTotal += strlen(szLen);
			nLenTotal += EJ_STRING_ARR_LEN("\r\n");
		}

		if (!bCType)
		{
			nLenTotal += strlen("Content-Type");
			nLenTotal += EJ_STRING_ARR_LEN(": ");  
			nLenTotal += strlen(request_data->content_type);
			nLenTotal += EJ_STRING_ARR_LEN("\r\n");
		}

		nLenTotal += EJ_STRING_ARR_LEN("\r\n");
		eint64_t nSizeTotal = nLenTotal * sizeof(char) + request_data->content_length + sizeof(char);

		_ECHK_MAL_ARR_R1(pHttpBuf, ebyte_t, nSizeTotal);
		pHttpBuf[nSizeTotal - 1] = 0;

		char* pHead = (char*)pHttpBuf, *pEnd = pHead + nSizeTotal;

		_AppendStringChkA(pHead, pEnd, request_data->req_method, EJ_STR_NTS);
		_AppendStringChkA(pHead, pEnd, " ", EJ_STRING_ARR_LEN(" "));
		_AppendStringChkA(pHead, pEnd, request_data->req_path, EJ_STR_NTS);

		for (euint32_t i = 0; i < request_data->num_arguments; ++i)
		{
			_AppendStringChkA(pHead, pEnd, i == 0 ? "?" : "&", 1);
			_AppendStringChkA(pHead, pEnd, request_data->arguments_key_arr[i], EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, "=", 1);
			_AppendStringChkA(pHead, pEnd, request_data->arguments_val_arr[i], EJ_STR_NTS);
		}

		_AppendStringChkA(pHead, pEnd, " HTTP/1.1\r\n", EJ_STRING_ARR_LEN(" HTTP/1.1\r\n"));

		for (euint32_t i = 0; i < request_data->num_headers; ++i)
		{
			_AppendStringChkA(pHead, pEnd, request_data->header_key_arr[i], EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, ": ", EJ_STRING_ARR_LEN(": "));
			_AppendStringChkA(pHead, pEnd, request_data->header_val_arr[i], EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, "\r\n", EJ_STRING_ARR_LEN("\r\n"));
		}

		if (!bCLength)
		{
			char szLen[20];
			sprintf(szLen, "%u", request_data->content_length, 10);
			_AppendStringChkA(pHead, pEnd, "Content-Length", EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, ": ", EJ_STRING_ARR_LEN(": "));
			_AppendStringChkA(pHead, pEnd, szLen, EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, "\r\n", EJ_STRING_ARR_LEN("\r\n"));
		}

		if (!bCType)
		{
			_AppendStringChkA(pHead, pEnd, "Content-Type", EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, ": ", EJ_STRING_ARR_LEN(": "));
			_AppendStringChkA(pHead, pEnd, request_data->content_type, EJ_STR_NTS);
			_AppendStringChkA(pHead, pEnd, "\r\n", EJ_STRING_ARR_LEN("\r\n"));
		}

		_AppendStringChkA(pHead, pEnd, "\r\n", EJ_STRING_ARR_LEN("\r\n"));
		memcpy(pHead, request_data->content_data, request_data->content_length);

		buffer->stBufSize = nSizeTotal - sizeof(char);
		buffer->pBuffer = (ebyte_t*)pHttpBuf;
		return EJ_ERC_SUCCESS;
	}

}

extern "C"
{
	void EJ_HttpClientFreeRes(char* ptr)
	{
		free(ptr);
	}

	EJ_Status EJ_HttpClientRequest(const char* ip, int port, euint32_t timeoutInMS, const EJ_HttpReqData* req, char** ptr, eint64_t* sz)
	{
		EJ_Buffer reqBuf;
		EJ_Status rc = EJDev::HttpRequestBody::BuildHttpRequest(req, &reqBuf);
		_EJ_CHECK_RC_RET(rc);

		EJ::HandleRef<EJ_Buffer*> reqBuf_res(&reqBuf, EJDev::HttpRequestBody::FreeRequestBuffer);
		EJ_Socket socket;
		rc = EJ_SockConnectToServerA(EJ_SOCKET_TYPE_TCP, ip, port, 2000, &socket);
		_EJ_CHECK_RC_RET(rc);

		EJ::HandleRef<EJ_Socket> socket_res(socket, EJ_SockClose);

		EJ_SocketOptValue optVal;
		optVal.uRecvTimeout = timeoutInMS;

		rc = EJ_SockSetOption(socket, EJ_SOCKET_OPT_RECV_TIMEOUT, &optVal);
		_EJ_CHECK_RC_RET(rc);

		optVal.uSendBufSize = 1 << 16;
		rc = EJ_SockSetOption(socket, EJ_SOCKET_OPT_SEND_BUF_SIZE, &optVal);
		_EJ_CHECK_RC_RET(rc);

		optVal.uRecvBufSize = 1 << 16;
		rc = EJ_SockSetOption(socket, EJ_SOCKET_OPT_RECV_BUF_SIZE, &optVal);
		_EJ_CHECK_RC_RET(rc);

		ebyte_t* pStart = (ebyte_t*)reqBuf.pBuffer;
		esize_t sRemain = reqBuf.stBufSize, sSent, sRecved;

		while (sRemain > 0)
		{
			rc = EJ_SockSendData(socket, sRemain, pStart, &sSent);
			_EJ_CHECK_RC_RET(rc);

			sRemain -= sSent;
		}

		EJDev::HttpResponseBody resBody;

		ebyte_t recvBuffer[1024];
		while (!resBody.IsResponseReceived())
		{
			rc = EJ_SockReceiveData(socket, sizeof(recvBuffer), recvBuffer, FALSE, &sRecved);
			_EJ_CHECK_RC_RET(rc);

			rc = resBody.AppendPacket(recvBuffer, sRecved);
			_EJ_CHECK_RC_RET(rc);
		}

		EJDev::HttpResponse response;
		resBody.GetFullResponse(&response);

		_EJ_CHECK_REPORT_NEW_RET(response.response_code == 200, EJ_ERC_INVALID_OPERATION, EC("Http return code(%d), text('%@'), data('%@')"), 
			response.response_code, response.response_text, response.content_data);

		_ECHK_MAL_STRA_R1(pStr, response.content_length);
		pStr[response.content_length] = 0;
		memcpy(pStr, response.content_data, response.content_length);

		*ptr = pStr;
		*sz = response.content_length;

		return EJ_ERC_SUCCESS;
	}
};
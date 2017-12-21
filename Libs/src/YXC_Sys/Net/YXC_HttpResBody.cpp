#define __MODULE__ "EK.Sys.HttpRes"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_HttpResBody.hpp>
#include <curl/curl.h>

namespace
{
	static const char kCrossOriginAllowHeaders[] =
		"Access-Control-Allow-Origin: *\r\n"
		"Access-Control-Allow-Credentials: true\r\n"
		"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
		"Access-Control-Allow-Headers: Content-Type, "
		"Content-Length, Connection, Cache-Control\r\n"
		"Access-Control-Expose-Headers: Content-Length";

	static char kHeaderTerminator[] = "\r\n\r\n";
	static char kLineTerminator[] = "\r\n";
	static const char kContentLength[] = "Content-Length";
	static const char kContentType[] = "Content-Type";
	static const char kConnection[] = "Connection";
	static const char kTransfer[] = "Transfer-Encoding";

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

namespace YXCLib
{
	HttpResponseBody::HttpResponseBody() : _pTempData(NULL), _stCbTempData(0), _stCbTempBuf(0), _stCbHttpBody(0), _stCbHttpHeader(0),
		_bHeaderParsed(FALSE), _stNumHeaders(0), _bCloseConnection(TRUE), _pszContentType(NULL), _responseCode(0),
		_responseText(NULL), _chunked(FALSE), _stCbChunkPos(0), _stCbDataInChunk(0)
	{
		memset(&this->_hdrKeyArr, 0, sizeof(this->_hdrKeyArr));
		memset(&this->_hdrValArr, 0, sizeof(this->_hdrValArr));
	}

	HttpResponseBody::~HttpResponseBody()
	{
		this->ClearChannel();

		if (this->_pTempData)
		{
			YXC_MMCFreeData(this->_pTempData);
			this->_pTempData = NULL;
		}
	}

	YXC_Status HttpResponseBody::BuildHttpResponse(const HttpResponse* response_data, YXC_Buffer* buffer)
	{
		char szTemp1[4096];
		int nSize;

		nSize = yxcwrap_snprintf(szTemp1, 4095,
			"HTTP/1.0 %d %s\r\n"
			"Server: EJ http server base/0.1\r\n"
			"Cache-Control: no-cache\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %d\r\n"
			"Connection: %s\r\n"
			"%s\r\n"
			"\r\n",
			response_data->response_code, response_data->response_text, response_data->content_type,
			response_data->content_length,
			response_data->connection_close ? "Close" : "Keep-Alive", kCrossOriginAllowHeaders
		);

		yuint32_t nTotal = nSize + response_data->content_length;
		_YCHK_MAL_ARR_R2(buffer->pBuffer, ybyte_t, nTotal);

		memcpy(buffer->pBuffer, szTemp1, nSize);
		memcpy((ybyte_t*)buffer->pBuffer + nSize, response_data->content_data, response_data->content_length);
		buffer->stBufSize = nTotal;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status HttpResponseBody::BuildStatusResponse(int code, const char* code_desc, const char* response_text,
		int close_connection, YXC_Buffer* buffer)
	{
		HttpResponse response = {0};
		response.connection_close = close_connection;
		response.content_data = (ybyte_t*)response_text;
		response.content_length = response_text == NULL ? 0 : strlen(response_text);
		response.content_type = "text/plain;charset=utf8";
		response.response_code = code;
		response.response_text = code_desc;

		return BuildHttpResponse(&response, buffer);
	}

	void HttpResponseBody::FreeResponseBuffer(YXC_Buffer* buffer)
	{
		free(buffer->pBuffer);
	}

	YXC_Status HttpResponseBody::_AddHeaderDesc(const char* key_begin, const char* key_end, const char* val_begin, const char* val_end)
	{
		YXC_Status rc = YXC_MMCExpandBuffer(&this->_hdrKeyArr.pBuffer, &this->_hdrKeyArr.stBufSize, (this->_stNumHeaders + 1) * sizeof(char*));
		_YXC_CHECK_RC_RET(rc);

		rc = YXC_MMCExpandBuffer(&this->_hdrValArr.pBuffer, &this->_hdrValArr.stBufSize, (this->_stNumHeaders + 1) * sizeof(char*));
		_YXC_CHECK_RC_RET(rc);

		int key_len = key_end - key_begin, val_len = val_end - val_begin;

		_YCHK_EAL_STRA_R1(new_key, this->_emp, key_len);
		_YCHK_EAL_STRA_R1(new_val, this->_emp, val_len);

		memcpy(new_key, key_begin, key_len);
		new_key[key_len] = 0;
		memcpy(new_val, val_begin, val_len);
		new_val[val_len] = 0;

		char** ppKeyArr = (char**)this->_hdrKeyArr.pBuffer, **ppValArr = (char**)this->_hdrValArr.pBuffer;
		ppKeyArr[this->_stNumHeaders] = new_key;
		ppValArr[this->_stNumHeaders] = new_val;

		++this->_stNumHeaders;
		if (key_len == YXC_STRING_ARR_LEN(kContentLength) && strncmp(key_begin, kContentLength, key_len) == 0)
		{
			this->_stCbHttpBody = atoi(new_val);
		}
		else if (key_len == YXC_STRING_ARR_LEN(kContentType) && strncmp(key_begin, kContentType, key_len) == 0)
		{
			this->_pszContentType = new_val;
		}
		else if (key_len == YXC_STRING_ARR_LEN(kTransfer) && strncmp(key_begin, kTransfer, key_len) == 0)
		{
			if (strcmp(new_val, "chunked") == 0)
			{
				this->_chunked = TRUE;
				this->_stCbChunkPos = this->_stCbHttpHeader;
			}
		}
		else if (key_len == YXC_STRING_ARR_LEN(kConnection) && strncmp(key_begin, kConnection, key_len) == 0)
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

		return YXC_ERC_SUCCESS;
	}

	YXC_Status HttpResponseBody::_ParseHeaderExtenstions(const char* headers, int length)
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

			if (val_begin)
			{
				_StrFindReadStartEnd(key_begin, key_end);
				_StrFindReadStartEnd(val_begin, val_end);
				YXC_Status rc = this->_AddHeaderDesc(key_begin, key_end, val_begin, val_end);
				_YXC_CHECK_RC_RET(rc);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status HttpResponseBody::_ParseCodeAndDesc(int len)
	{
		const char* begin = (char*)this->_pTempData;
		const char* kSpace = strchr(begin, ' ');
		_YXC_CHECK_REPORT_NEW_RET(kSpace != NULL, YXC_ERC_NET_INVALID_PROTOCOL, YC("strchr for code and desc failed"));

		const char* kSpace2 = strchr(kSpace + 1, ' ');
		_YXC_CHECK_REPORT_NEW_RET(kSpace2 != NULL, YXC_ERC_NET_INVALID_PROTOCOL, YC("strchr for code and desc failed"));

		yuint32_t uCountCode = kSpace2 - kSpace - 1, uCountDesc = len - (kSpace2 - begin);
		_YCHK_EAL_STRA_R1(resCodeStr, this->_emp, uCountCode);
		strncpy(resCodeStr, kSpace + 1, uCountCode);
		resCodeStr[uCountCode] = 0;

		this->_responseCode = atoi(resCodeStr);

		_YCHK_EAL_STRA_R1(resDescStr, this->_emp, uCountDesc);
		strncpy(resDescStr, kSpace2 + 1, uCountDesc);
		resDescStr[uCountDesc] = 0;

		this->_responseText = resDescStr;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status HttpResponseBody::_ParseHeader()
	{
		char* header_found = (char*)strstr((char*)this->_pTempData, kHeaderTerminator);
		if (header_found)
		{
			this->_stCbHttpHeader = header_found - (char*)this->_pTempData + YXC_STRING_ARR_LEN(kHeaderTerminator);

			char* pi = (char*)strstr((char*)this->_pTempData, kLineTerminator);
			int len = pi - (char*)this->_pTempData;

			YXC_Status rc = this->_ParseCodeAndDesc(len);
			_YXC_CHECK_RC_RET(rc);

			const char* headers = pi + YXC_STRING_ARR_LEN(kLineTerminator);
			int remain_len = this->_stCbHttpHeader - len - YXC_STRING_ARR_LEN(kLineTerminator);

			rc = this->_ParseHeaderExtenstions(headers, remain_len);
			_YXC_CHECK_RC_RET(rc);

			this->_bHeaderParsed = TRUE;
			return YXC_ERC_SUCCESS;
		}
		else
		{
			return YXC_ERC_SUCCESS;
		}
	}

	YXC_Status HttpResponseBody::_ParseChunked()
	{
		while (this->_stCbChunkPos < this->_stCbTempData)
		{
			char szChunkSize[20] = {0};
			ysize_t index = this->_stCbChunkPos;
			char* header_found = (char*)strstr((char*)this->_pTempData + index, kLineTerminator);
			if (header_found == NULL)
			{
				return YXC_ERC_SUCCESS; /* Wait for next chunk index. */
			}

			int chunk_size_len = header_found - (char*)this->_pTempData - index;
			strncpy(szChunkSize, (char*)this->_pTempData + index, chunk_size_len);

			int chunk_size = 0;
			int scanned = sscanf(szChunkSize, "%x", &chunk_size);
			if (chunk_size == 0) /* Read completed!. */
			{
				this->_stCbHttpBody = this->_stCbDataInChunk;
				this->_pTempData[this->_stCbHttpBody + this->_stCbHttpHeader] = 0;
				return YXC_ERC_SUCCESS;
			}

			int chunk_total_size = chunk_size + chunk_size_len + 2 * YXC_STRING_ARR_LEN(kLineTerminator);
			if (chunk_total_size <= this->_stCbTempData - index) /* Read this chunk out. */
			{
				this->_stCbChunkPos += chunk_total_size;
				memmove(this->_pTempData + this->_stCbHttpHeader + this->_stCbDataInChunk,
					header_found + YXC_STRING_ARR_LEN(kLineTerminator), chunk_size);
				this->_stCbDataInChunk += chunk_size;
			}
			else/* chunk not received. */
			{
				return YXC_ERC_SUCCESS;
			}
		}

		return YXC_ERC_SUCCESS;
	}

	ybool_t HttpResponseBody::IsResponseReceived() const
	{
		if (!this->_chunked)
		{
			ybool_t bFullPacket = this->_bHeaderParsed && this->_stCbTempData == this->_stCbHttpHeader + this->_stCbHttpBody;
			return bFullPacket;
		}
		else
		{
			return this->_stCbHttpBody != 0;
		}
	}

	void HttpResponseBody::GetFullResponse(HttpResponse* request_data)
	{
		request_data->connection_close = this->_bCloseConnection;
		request_data->content_data = this->_pTempData + this->_stCbHttpHeader;
		request_data->content_length = this->_stCbHttpBody;
		request_data->content_type = this->_pszContentType;
		request_data->header_key_arr = (const char**)this->_hdrKeyArr.pBuffer;
		request_data->header_val_arr = (const char**)this->_hdrValArr.pBuffer;
		request_data->num_headers = this->_stNumHeaders;
		request_data->response_code = this->_responseCode;
		request_data->response_text = this->_responseText;
	}

	YXC_Status HttpResponseBody::AppendPacket(const void* data, yuint32_t len)
	{
		_YXC_CHECK_REPORT_NEW_RET(!this->IsResponseReceived(), YXC_ERC_INVALID_STATUS, YC("Response not clear before read"));

		YXC_Status rc = YXC_MMCExpandBuffer((void**)&this->_pTempData, &this->_stCbTempBuf, this->_stCbTempData + len + sizeof(char));
		_YXC_CHECK_RC_RET(rc);

		memcpy(this->_pTempData + this->_stCbTempData, data, len);
		this->_stCbTempData += len;
		this->_pTempData[this->_stCbTempData] = 0; /* \0 terminated. */

		if (!this->_bHeaderParsed)
		{
			rc = this->_ParseHeader();
			_YXC_CHECK_RC_RET(rc);
		}

		if (!this->_bHeaderParsed)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_stCbTempData < (1 << 20), YXC_ERC_BUFFER_NOT_ENOUGH, YC("Not a valid http response header(%d)."),
				this->_stCbTempData);
		}
		else
		{
			if (this->_chunked)
			{
				rc = this->_ParseChunked();
				_YXC_CHECK_RC_RET(rc);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	void HttpResponseBody::ClearChannel()
	{
		this->_stCbTempData = 0;
		this->_emp.Clear();
		this->_stNumHeaders = 0;
		this->_responseCode = 0;
		this->_responseText = NULL;
		this->_bCloseConnection = TRUE;
		this->_pszContentType = NULL;
		this->_stCbHttpBody = 0;
		this->_bHeaderParsed = FALSE;
		this->_stCbHttpHeader = 0;
		this->_chunked = FALSE;
		this->_stCbChunkPos = 0;
		this->_stCbDataInChunk = 0;
	}
}

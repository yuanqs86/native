/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_HTTP_CLIENT_H__
#define __INC_YXC_SYS_BASE_HTTP_CLIENT_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef struct __YXC_HTTP_REQUEST_ARGUMENTS
	{
		const char* req_method;
		const char* req_path;
		const char* content_type;
		yuint32_t content_length;
		ybyte_t* content_data;
		yuint32_t num_headers;
		const char** header_key_arr;
		const char** header_val_arr;
		yuint32_t num_arguments;
		const char** arguments_key_arr;
		const char** arguments_val_arr;
		ybool_t connection_close;
	}YXC_HttpReqData;

	typedef struct __YXC_HTTP_RESPONSE_DATA
	{
		int response_code;
		const char* response_text;
		ybool_t connection_close;

		const char* content_type;
		yuint32_t content_length;
		ybyte_t* content_data;

		yuint32_t num_headers;
		const char** header_key_arr;
		const char** header_val_arr;
	}YXC_HttpResData;

	YXC_API(YXC_Status) YXC_HttpClientRequest(const char* ip, int port, yuint32_t timeoutInMS,
		const YXC_HttpReqData* req, char** ptr, yint64_t* sz);

	YXC_API(void) YXC_HttpClientFreeRes(char* ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_HTTP_CLIENT_H__ */

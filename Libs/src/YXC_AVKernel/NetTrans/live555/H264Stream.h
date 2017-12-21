#ifndef _H_H264STREAM_
#define _H_H264STREAM_

#include <YXC_Sys/YXC_Sys.h>
#include "H264StreamWrap.h"

class CH264Stream
{
public:
	CH264Stream(void);
	~CH264Stream(void);
public:
	typedef void (*handleResult)(int resultCOde, char * resultString, void * user);

	//config/set call back function
	void configH264Stream(char *pParm, int inType, int bUseTCP);

	void setExceptionHandle(void * exceptionHandle, void * fUser);
	void setDataHandle(void * headFunc, void * audioHeader, void * streamFunc, void * user);

	int createRTSP(void);

public:
	YXC_FPathA streamURL;

public:
	CH264StreamWrap* _wrap;
	//error call back function
	handleResult	fHandleResult;
	void *			fuser;
	//get video/audio data call back function
	void *			audioHeaderfunc;
	void *			videoHeaderfunc;
	void *			dataHandlefunc;
	void *			datauser;
};

#endif

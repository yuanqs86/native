#ifndef _H_H264STREAMWRAP_
#define _H_H264STREAMWRAP_

#include <YXC_Sys/YXC_Sys.h>

class CH264StreamWrap
{
public:
	CH264StreamWrap(void);
	virtual ~CH264StreamWrap(void);
public:
	typedef void (*handleResult)(int resultCOde, char * resultString, void * user);

	//config/set call back function
	virtual void configH264Stream(char *pParm, int inType, int bUseTCP) = 0;

	void setExceptionHandle(void * exceptionHandle, void * fUser);
	void setDataHandle(void * headFunc, void * audioHeader, void * streamFunc, void * user);

	virtual int createRTSP(void) = 0;

public:
	YXC_FPathA streamURL;

public:
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

#ifndef _H_FLVFIESINK_
#define _H_FLVFIESINK_

#ifndef _FILE_SINK_HH
#include "FileSink.hh"
#endif

class FlvFileSink : public FileSink
{
public:
	static FlvFileSink * createNew(UsageEnvironment &env, char const * fileName,
		char const * sPropParamenterSetsStr = NULL,unsigned bufferSize = 10000, Boolean oneFilePerFrame = False);
	virtual void addData(unsigned char const* data, unsigned dataSize, struct timeval presentationTime);

	typedef void (*headerFunc)(intptr_t ptrMediaStream, unsigned int, unsigned char *,unsigned int, unsigned char *, void *);
	typedef void (*streamFunc)(intptr_t ptrMediaStream,int inType, unsigned char *,unsigned int,__int64,void *);

	void setStreamPtr(intptr_t ptrMediaStream);
	void setCallBackFunc(void* fheaderFunc,void* fstreamFunc, void * user);
protected:
	FlvFileSink(UsageEnvironment& env, FILE* fid,
		char const* sPropParameterSetsStr,
		unsigned bufferSize, char const* perFrameFileNamePrefix);
	virtual~FlvFileSink(void);

protected:
	virtual void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime);

private:
	char const* fsPropParameterSetsStr;
	Boolean		fHaveWrittenFristFrame;

protected:
	void *			fUser;
	headerFunc		setHeaderCallBack;
	streamFunc		setStreamBuffer;
	__int64			rtTime;
	intptr_t		ptrStream;
};
#endif

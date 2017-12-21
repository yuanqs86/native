#ifndef __H_PCMSINK_HH_
#define __H_PCMSINK_HH_

#ifndef _FILE_SINK_HH
#include "FileSink.hh"
#endif

class PCMSink : public FileSink
{
public:
	static PCMSink * createNew(UsageEnvironment &env, char const * fileName,const unsigned rtpPayloadType,
		unsigned bufferSize = 10000, Boolean oneFilePerFrame = False,unsigned fRTPFrequecy=0,unsigned fNumChannels=0);
	virtual void addData(unsigned char const* data, unsigned dataSize, struct timeval presentationTime);

	typedef void (*headerFunc)(intptr_t ptrMediaStream, unsigned int, char *, void *);
	typedef void (*streamFunc)(intptr_t ptrMediaStream,int inType, unsigned char *,unsigned int,__int64,void *);

	void setStreamPtr(intptr_t ptrMediaStream);
	void setCallBackFunc(void* fheaderFunc,void* fstreamFunc, void * user);

protected:
	PCMSink(UsageEnvironment& env, FILE* fid, const unsigned rtpPayloadType, unsigned bufferSize,char const* perFrameFileNamePrefix,unsigned fRTPFrequecy,unsigned fNumChannels);
	virtual~PCMSink(void);
protected:
	virtual void afterGettingFrame1(unsigned frameSize, struct timeval presentationTime);
	char * lookupFormat(DWORD& freq, WORD& nCh);
protected:
	Boolean		fHaveWrittenFristFrame;
	unsigned	uinRtpPayloadType;
	unsigned fFrequecy,fChannels;
protected:
	int avbyte;
	void *			fUser;
	headerFunc		setHeaderCallBack;
	streamFunc		setStreamBuffer;
	__int64			rtTime;
	intptr_t		ptrStream;
};
#endif

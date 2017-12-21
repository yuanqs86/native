#include "FlvFileSink.h"
#include "OutputFile.hh"
#include "H264VideoRTPSource.hh"

//Media define
#define VIDEOH264 0x01
#define AUDIOPCMU 0x02

FlvFileSink::FlvFileSink(UsageEnvironment& env, FILE* fid,
	char const* sPropParameterSetsStr,
	unsigned bufferSize, char const* perFrameFileNamePrefix)
	:FileSink(env,fid,bufferSize,perFrameFileNamePrefix),
	fsPropParameterSetsStr(sPropParameterSetsStr),fHaveWrittenFristFrame(False)
{
	fUser				= NULL;
	setHeaderCallBack	= NULL;
	setStreamBuffer		= NULL;
	rtTime				= 0;
}


FlvFileSink::~FlvFileSink(void)
{
	setHeaderCallBack	= NULL;
	setStreamBuffer		= NULL;
}
FlvFileSink* FlvFileSink::createNew(UsageEnvironment &env, char const * fileName, char const * sPropParamenterSetsStr /* = NULL */,
	unsigned bufferSize /* = 10000 */, Boolean oneFilePerFrame /* = False */)
{
	do
	{
		FILE * fid = NULL;
		char const* perFrameFileNamePrefix;
		if (oneFilePerFrame)
		{
			fid = NULL;
			perFrameFileNamePrefix = fileName;
		}
		else
		{
			//do nothing
#ifdef _H264

			fid =  OpenOutputFile(env, "D://234.h264");			//test 264 file.
#endif
			 perFrameFileNamePrefix = NULL;
		}
		return new FlvFileSink(env,fid,sPropParamenterSetsStr,bufferSize,perFrameFileNamePrefix);
	} while (0);
	return NULL;
}
void FlvFileSink::setStreamPtr(intptr_t ptrMediaStream)
{
	ptrStream = ptrMediaStream;
}
void FlvFileSink::setCallBackFunc(void* fheaderFunc,void*  fstreamFunc, void * user)
{
	fUser = user;
	setHeaderCallBack = (headerFunc)fheaderFunc;
	setStreamBuffer	= (streamFunc)fstreamFunc;
}

 unsigned char const start_code[4] = {0x00, 0x00, 0x00, 0x01};
void FlvFileSink::afterGettingFrame1(unsigned frameSize, struct timeval presentationTime)
{
	if (!fHaveWrittenFristFrame)
	{
		unsigned unmSPropRecords;
		SPropRecord * sPropRecords = parseSPropParameterSets(fsPropParameterSetsStr,unmSPropRecords);


		//////////////////////////////////////////////////////////////////////////
#ifdef _H264

		fwrite(start_code, 1, 4, fOutFid);				//test 264 file.
		fwrite(sPropRecords[0].sPropBytes, 1, sPropRecords[0].sPropLength, fOutFid);		//test 264 file.
		fwrite(start_code, 1, 4, fOutFid);				//test 264 file.
		fwrite(sPropRecords[1].sPropBytes, 1, sPropRecords[1].sPropLength, fOutFid);		//test 264 file.
#endif
		//////////////////////////////////////////////////////////////////////////
		//set H264 header
		if (setHeaderCallBack != NULL)
		{
			if (unmSPropRecords==1)
			{
		//		setHeaderCallBack(ptrStream, sPropRecords[0].sPropLength,sPropRecords[0].sPropBytes,0,NULL,fUser);
			}
			if (unmSPropRecords==2)
			{
				setHeaderCallBack(ptrStream, sPropRecords[0].sPropLength,sPropRecords[0].sPropBytes,sPropRecords[1].sPropLength,sPropRecords[1].sPropBytes,fUser);
				fHaveWrittenFristFrame = True;
			}
		}
		delete[] sPropRecords;

	}
	//////////////////////////////////////////////////////////////////////////
#ifdef _H264
	fwrite(start_code, 1, 4, fOutFid);			//test 264 file.
	fwrite(fBuffer, 1, frameSize, fOutFid);		//test 264 file.
#endif
	//////////////////////////////////////////////////////////////////////////
	addData(fBuffer, frameSize, presentationTime);

	continuePlaying();
}

void FlvFileSink::addData(unsigned char const* data, unsigned dataSize, struct timeval presentationTime)
{
	EnterCriticalSection(&fSection);
	rtTime = ((__int64)presentationTime.tv_sec * 1000000 + presentationTime.tv_usec) * 10;
	if (dataSize != 0 && data != NULL)
	{
		if (setStreamBuffer != NULL)
		{
			setStreamBuffer(ptrStream, VIDEOH264, (unsigned char*)data,dataSize,rtTime,fUser);
		}
	}
	LeaveCriticalSection(&fSection);
}

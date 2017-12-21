#include "PCMSink.h"

#define PCMA 0x0006
#define PCMU 0x0007
#define PAAC 2000

#define UNKN -1

//Media define
#define VIDEOH264 0x01
#define AUDIOPCMU 0x02

PCMSink::PCMSink(UsageEnvironment& env, FILE* fid, const unsigned rtpPayloadType,unsigned bufferSize,char const* perFrameFileNamePrefix,unsigned fRTPFrequecy,unsigned fNumChannels)
:FileSink(env, fid,bufferSize,perFrameFileNamePrefix),uinRtpPayloadType(rtpPayloadType),fFrequecy(fRTPFrequecy),fChannels(fNumChannels)
{
	fUser				= NULL;
	setHeaderCallBack	= NULL;
	setStreamBuffer		= NULL;
	rtTime				= 0;
	fHaveWrittenFristFrame = False;
	avbyte=0;
}


PCMSink::~PCMSink(void)
{
	setHeaderCallBack	= NULL;
	setStreamBuffer		= NULL;
}
PCMSink * PCMSink::createNew(UsageEnvironment &env, char const * fileName,
	const unsigned rtpPayloadType,unsigned bufferSize/* = 10000*/, Boolean oneFilePerFrame/* = False*/,unsigned fRTPFrequecy,unsigned fNumChannels)
{
	do {
		FILE* fid = NULL;
		char const* perFrameFileNamePrefix;
		if (oneFilePerFrame)
		{
			// Create the fid for each frame
			fid = NULL;
			perFrameFileNamePrefix = fileName;
		}
		else
		{
			//do nothing
			perFrameFileNamePrefix = NULL;
		}

		return new PCMSink(env, fid, rtpPayloadType, bufferSize, perFrameFileNamePrefix, fRTPFrequecy, fNumChannels);
	} while (0);

	return NULL;
}

void PCMSink::setStreamPtr(intptr_t ptrMediaStream)
{
	ptrStream = ptrMediaStream;
}
void PCMSink::setCallBackFunc(void* fheaderFunc,void* fstreamFunc, void * user)
{
	fUser = user;
	setHeaderCallBack = (headerFunc)fheaderFunc;
	setStreamBuffer	= (streamFunc)fstreamFunc;
}

void PCMSink::afterGettingFrame1(unsigned frameSize, struct timeval presentationTime)
{
	if (!fHaveWrittenFristFrame)
	{
		WAVEFORMATEX * wft = new WAVEFORMATEX();
		wft->wFormatTag = UNKN;
		wft->wBitsPerSample = 8;
		char * tagFormat = lookupFormat(wft->nSamplesPerSec, wft->nChannels);
		if (tagFormat)
		{
			if (strcmp(tagFormat,"PCMU") == 0)
			{
				wft->wFormatTag = PCMU;
			}
			else if (strcmp(tagFormat,"PCMA") == 0)
			{
				wft->wFormatTag = PCMA;
			}
			else if (strcmp(tagFormat,"AAC") == 0)
			{
				wft->wFormatTag = PAAC;
				wft->wBitsPerSample = 16;
			}
			else
			{
				wft->wFormatTag = UNKN;
			}
			wft->cbSize = 0;
			wft->nBlockAlign = wft->nChannels * wft->wBitsPerSample / 8;
			wft->nAvgBytesPerSec = wft->nBlockAlign * wft->nSamplesPerSec;
			if (wft->wFormatTag == PAAC)
			{
				avbyte=wft->nAvgBytesPerSec;
			}
			if (setHeaderCallBack != NULL)
			{
				setHeaderCallBack(ptrStream, sizeof(WAVEFORMATEX),(char*)wft,fUser);
			}

			delete wft;
			delete[]tagFormat;
			fHaveWrittenFristFrame = True;
		}

	}

	addData(fBuffer, frameSize, presentationTime);
	continuePlaying();
}
//BYTE * pmybuf=NULL;
//DWORD count=0;
void PCMSink::addData(unsigned char const* data, unsigned dataSize, struct timeval presentationTime)
{
/*	if (!pmybuf)
	{
		pmybuf=new BYTE[1<<15];
	}*/
	EnterCriticalSection(&fSection);
/*	if (avbyte)
	{
		rtTime= (count++) * (1000 * 1024 / 44100)*10000;
	}
	else*/
	{
		rtTime = ((__int64)presentationTime.tv_sec * 1000000 + presentationTime.tv_usec) * 10;
	}
/*	if (!rtTime || !avbyte)
	{
		rtTime = ((__int64)presentationTime.tv_sec * 1000000 + presentationTime.tv_usec) * 10;
	}
	else
	{
		rtTime+=((double)dataSize / avbyte * 1000 * 10000);
	}*/
	if (dataSize != 0 && data != NULL)
	{
		if (setStreamBuffer != NULL && data != NULL)
		{
		//	memcpy(pmybuf,data,dataSize);
			setStreamBuffer(ptrStream, AUDIOPCMU, (unsigned char*)data,dataSize,rtTime,fUser);
		}
	}
	LeaveCriticalSection(&fSection);
}

char * PCMSink::lookupFormat(DWORD& freq, WORD& nCh)
{
	 char const* temp = NULL;
	switch (uinRtpPayloadType)
	{
	case 0: {temp = "PCMU"; freq = 8000; nCh = 1; break;}
	case 2: {temp = "G726-32"; freq = 8000; nCh = 1; break;}
	case 3: {temp = "GSM"; freq = 8000; nCh = 1; break;}
	case 4: {temp = "G723"; freq = 8000; nCh = 1; break;}
	case 5: {temp = "DVI4"; freq = 8000; nCh = 1; break;}
	case 6: {temp = "DVI4"; freq = 16000; nCh = 1; break;}
	case 7: {temp = "LPC"; freq = 8000; nCh = 1; break;}
	case 8: {temp = "PCMA"; freq = 8000; nCh = 1; break;}
	case 9: {temp = "G722"; freq = 8000; nCh = 1; break;}
	case 10: {temp = "L16"; freq = 44100; nCh = 2; break;}
	case 11: {temp = "L16"; freq = 44100; nCh = 1; break;}
	case 12: {temp = "QCELP"; freq = 8000; nCh = 1; break;}
	case 14: {temp = "MPA"; freq = 90000; nCh = 1; break;}
	// 'number of channels' is actually encoded in the media stream
	case 15: {temp = "G728"; freq = 8000; nCh = 1; break;}
	case 16: {temp = "DVI4"; freq = 11025; nCh = 1; break;}
	case 17: {temp = "DVI4"; freq = 22050; nCh = 1; break;}
	case 18: {temp = "G729"; freq = 8000; nCh = 1; break;}
	case 25: {temp = "CELB"; freq = 90000; nCh = 1; break;}
	case 26: {temp = "JPEG"; freq = 90000; nCh = 1; break;}
	case 28: {temp = "NV"; freq = 90000; nCh = 1; break;}
	case 31: {temp = "H261"; freq = 90000; nCh = 1; break;}
	case 32: {temp = "MPV"; freq = 90000; nCh = 1; break;}
	case 33: {temp = "MP2T"; freq = 90000; nCh = 1; break;}
	case 34: {temp = "H263"; freq = 90000; nCh = 1; break;}
//	case 96: {temp = "AAC"; freq = 48000; nCh = 2; break;}
//	case 97: {temp = "AAC"; freq = 44100; nCh = 2; break;}
	case 96: {
				temp = "AAC";
				if (fFrequecy>0)
				{
					freq = fFrequecy;
					nCh = fChannels;
				}
				else
				{
					freq = 48000;
					nCh = 2;
				}
				break;
			 }
	case 97: {
				temp = "AAC";
				if (fFrequecy>0)
				{
					freq = fFrequecy;
					nCh = fChannels;
				}
				else
				{
					freq = 44100;
					nCh = 2;
				}
				break;
			 }
	};
	return strDup(temp);
}

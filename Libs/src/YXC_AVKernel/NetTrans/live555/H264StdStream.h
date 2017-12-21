#ifndef _H_H264STDSTREAM_
#define _H_H264STDSTREAM_

#include <liveMedia/PCMSink.h>
#include <liveMedia/liveMedia.hh>
#include "NetCommon.h"
#include <liveMedia/FlvFileSink.h>
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"
#include "H264StreamWrap.h"

typedef void (*responseHandler)(RTSPClient* rtspClient,
	int resultCode, char* resultString);

class CH264StdStream : public CH264StreamWrap
{
public:
	CH264StdStream(void);
	~CH264StdStream(void);
public:
	typedef void (*handleResult)(int resultCOde, char * resultString, void * user);

	//config/set call back function
	void configH264Stream(char *pParm, int inType, int bUseTCP);

	//operation,create environment and setup stream
	int createRTSP(void);
	void setupStream(void);
	void insertSink(void);
	void playStream(void);
	void ShutDown(void);
	void Release(void);
	void _Release();
private:
	void StartStreamThd();

public:
	TaskScheduler * scheduler;
	RTSPClient * rtspClient;
	MediaSession * session;
	int simpleRTPoffsetArg;
	unsigned short desiredPortNum;
	unsigned fileBufferSize;

	CRITICAL_SECTION crit_lock;
	CRITICAL_SECTION crit_lock2;
	Boolean streamUsingTCP;

	FlvFileSink * fileSink;
	PCMSink	*	  pcmSink;

	char  AudioMedia[10];
	char  VideoMedia[10];

	Boolean blHandleVideo;
	Boolean blHandleAudio;

	//thread
	DWORD	m_dwThdID;
	HANDLE	m_hdThdHandle;
public:
	BasicUsageEnvironment * env;
	MediaSubsessionIterator* setupIter;
	TaskToken				sessionTimerTask;
	TaskToken				arrivalCheckTimerTask;
	Boolean					blContinuePlay;
	char *					chLoopStep;
	int						errorCode;
public:
	double duration;
	double durationSlop ;
	double initialSeekTime;
	float scale;
	double endtime;
	double numReceiverByte;
	double kBytes;
	double numReceiverByteA;
	double kBytesA;
	__int64 uSecsToDelay;

	HANDLE		sigle;
	int nCheckPacket,noPacket;
};
static	DWORD WINAPI DoTaskThread(LPVOID lpParam);

static void optionHandleFunc(RTSPClient* rtspClient,	int resultCode, char* resultString);
static void discriptionHandleFunc(RTSPClient* rtspClient,	int resultCode, char* resultString);
static void continueAfterSETUP(RTSPClient * rtspClient, int resultCode, char* resultString);
static void tearDownSessionHandleFunc(RTSPClient* rtspClient,	int resultCode, char* resultString);
static void continueAfterPLAY(RTSPClient * rtspClient, int resultCode, char* resultString);
static void subsessionAfterPlaying(void * clientData);
static void checkPacketArrive(void * clientData);
static void subsessionByeHandler(void *clientData);

#endif

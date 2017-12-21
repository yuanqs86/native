#include "H264Stream.h"
#include <YXC_AVKernel/YXV_rtsp.h>

//Media define
#define VIDEOH264 0x01
#define AUDIOPCMU 0x02

#define NOMEDIA	"xxxxx"

CH264Stream::CH264Stream(void)
{
	strcpy(streamURL,"rtsp://admin:12345@192.168.1.23:554/H264");
	env					= NULL;
	scheduler			= NULL;
	setupIter			= NULL;
	rtspClient			= NULL;
	session				= NULL;
	strcpy(AudioMedia,"audio");
	strcpy(VideoMedia,"video");
	simpleRTPoffsetArg	= -1;
	fileBufferSize		= 1920*1280 * 3;
	fileSink			= NULL;
	streamUsingTCP		= false;
	fHandleResult		= NULL;

	fuser				= NULL;
	//get video/audio data call back function
	audioHeaderfunc		= NULL;
	videoHeaderfunc		= NULL;
	dataHandlefunc		= NULL;
	datauser			= NULL;
	m_hdThdHandle		= NULL;

	arrivalCheckTimerTask = NULL;

	blContinuePlay		= False;
	chLoopStep			= NULL;

	duration = 0;
	durationSlop = -1;
	initialSeekTime = 0.0f;
	scale = 1;
	numReceiverByte = 0;
	kBytes = 0;
	sigle = CreateEvent(NULL,FALSE,FALSE,NULL);
	numReceiverByteA = 0;
	kBytesA = 0;
	nCheckPacket=noPacket=0;
	InitializeCriticalSection(&crit_lock);
	InitializeCriticalSection(&crit_lock2);
}


CH264Stream::~CH264Stream(void)
{
	Release();
	DeleteCriticalSection(&crit_lock);
	DeleteCriticalSection(&crit_lock2);
	//Sleep(5000);
}

void CH264Stream::_Release()
{
	blContinuePlay = False;

	if (m_hdThdHandle != NULL)
	{
		WaitForSingleObject(m_hdThdHandle, INFINITE);
		m_hdThdHandle = NULL;
	}
	if (env != NULL)
	{
		env->taskScheduler().unscheduleDelayedTask(sessionTimerTask);
		env->taskScheduler().unscheduleDelayedTask(arrivalCheckTimerTask);
		env = NULL;
	}

	if (session != NULL)
	{
		rtspClient->sendTeardownCommand(*session,tearDownSessionHandleFunc);
	}
	else
	{
		tearDownSessionHandleFunc(NULL,0,NULL);
	}
	if (session == NULL)
	{
		//return;
	}
	else
	{
		MediaSubsessionIterator iter(*session);
		MediaSubsession  *subsession;
		while((subsession = iter.next()) != NULL)
		{
			//rtspClient->teardownMediaSubsession(*subsession);
			Medium::close(subsession->sink);
			subsession->sink = NULL;
		}
		//rtspClient->teardownMediaSession(*session);
		Medium::close(session);
		session = NULL;
	}
	if (rtspClient != NULL)
	{
		Medium::close(rtspClient);
		rtspClient = NULL;
	}
	if (scheduler != NULL)
	{
		delete scheduler;
		scheduler = NULL;
	}
}

void CH264Stream::Release(void)
{
	EnterCriticalSection(&crit_lock);
	__try
	{
		this->_Release();
		LeaveCriticalSection(&crit_lock);
	}
	__except(1)
	{
		LeaveCriticalSection(&crit_lock);
		FILE* fp = fopen("H264StreamError.txt", "a");
		if (fp != NULL)
		{
			fprintf(fp, "Error session after playing in h264 stream.");
			fclose(fp);
		}
	}
}

void CH264Stream::ShutDown(void)
{
	blContinuePlay = False;
	if (env != NULL)
	{
		env->taskScheduler().unscheduleDelayedTask(sessionTimerTask);
		env->taskScheduler().unscheduleDelayedTask(arrivalCheckTimerTask);
	}

	if (session == NULL)
	{
		return;
	}
	else
	{
		MediaSubsessionIterator iter(*session);
		MediaSubsession  *subsession;
		while((subsession = iter.next()) != NULL)
		{
			//rtspClient->teardownMediaSubsession(*subsession);
			Medium::close(subsession->sink);
			subsession->sink = NULL;
		}
		//rtspClient->teardownMediaSession(*session);
		Medium::close(session);
		session = NULL;
		Medium::close(rtspClient);
		rtspClient = NULL;
	}
}

void CH264Stream::configH264Stream(char *pParm,int inType,Boolean bUseTCP)
{
	if (pParm != NULL)
	{
		strcpy(streamURL,pParm);
	}
	if (inType != 0)
	{
		if (inType & VIDEOH264)
		{
			blHandleVideo	 = True;
		}
		else
		{
			strcpy(VideoMedia,NOMEDIA);
		}

		if (inType & AUDIOPCMU)
		{
			blHandleAudio = True;
		}
		else
		{
			strcpy(AudioMedia,NOMEDIA);
		}
	}
	streamUsingTCP=bUseTCP;
}

void CH264Stream::setExceptionHandle(void * exceptionHandle, void * fUser)
{
	fHandleResult = (handleResult)exceptionHandle;
	fuser		  = fUser;
}
void CH264Stream::setDataHandle(void * headFunc, void * audioHeader, void * streamFunc, void * user)
{
	audioHeaderfunc	 = audioHeader;
	videoHeaderfunc	 = headFunc;
	dataHandlefunc	 = streamFunc;
	datauser		 = user;
}

Boolean CH264Stream::createRTSP(void)
{
	numReceiverByte = 0;
	kBytes = 0;
	numReceiverByteA = 0;
	kBytesA = 0;
	nCheckPacket=noPacket=0;
	if (fHandleResult != NULL)
	{
		fHandleResult(RERROR_NO,"createRTSP",fuser);
	}
	Release();
	//Sleep(5000);
	blContinuePlay = True;

	scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	rtspClient = RTSPClient::createNew(*env, streamURL);
	if (rtspClient)
	{
		StartStreamThd();
		return True;
	}
	else
	{
		*env<<"create rtsp_client failure"<<env->getResultMsg()<<"\n";
		fHandleResult(ERROR_CRTEAYECLIENT,"create rtsp_client failure",fuser);
		//ShutDown();
		return False;
	}
}

void CH264Stream::setupStream(void)
{
	if (setupIter == NULL)
	{
		setupIter = new MediaSubsessionIterator(*session);
	}
	EnterCriticalSection(&crit_lock2);
	MediaSubsession *subsession;
	while ((subsession = setupIter->next()) != NULL)
	{
		// We have another subsession left to set up:
		if (subsession->clientPortNum() == 0 || !subsession->initiate())
		{
			continue; // port # was not set
		}

		if (strcmp(subsession->mediumName(), VideoMedia) == 0)
		{
			blHandleVideo = False;
		}
		if (strcmp(subsession->mediumName(), AudioMedia) == 0)
		{
			blHandleAudio = False;
		}
		rtspClient->sendSetupCommand(*subsession, continueAfterSETUP, False, streamUsingTCP, False, NULL);
		LeaveCriticalSection(&crit_lock2);
		return;
	}
	LeaveCriticalSection(&crit_lock2);
	delete setupIter;
	setupIter = NULL;

	insertSink();
	//playStream();
}
void CH264Stream::playStream(void)
{
	if (duration == 0)
	{
		if (scale > 0)
		{
			duration = session->playEndTime() - initialSeekTime;
		}
		else if (scale < 0)
		{
			duration = initialSeekTime;
		}
	}
	if (duration < 0)
	{
		duration = 0.0;
	}
	endtime = initialSeekTime;

	if (scale > 0)
	{
		if (duration <= 0)
		{
			endtime = -1.0f;
		}
		else
		{
			endtime = initialSeekTime + duration;
		}
	}
	else
	{
		endtime = initialSeekTime - duration;
		if (endtime < 0)
		{
			endtime = 0.0f;
		}
	}
	rtspClient->sendPlayCommand(*session, continueAfterPLAY, initialSeekTime,endtime,scale, NULL);
}
//H264VideoFileSink *pfilesink;
void CH264Stream::insertSink(void)
{
	MediaSubsessionIterator iter(*session);
	MediaSubsession *subsession;
	while((subsession = iter.next()) != NULL)
	{
		if (subsession->readSource() == NULL)
		{
			continue;
		}
		if (strcmp(subsession->mediumName(), "video") == 0 &&
			strcmp(subsession->codecName(), "H264") == 0)
		{
			if (blHandleVideo == True)
			{
				continue;
			}
#ifdef _H264

			pfilesink = H264VideoFileSink::createNew(*env,"d://123.h264",subsession->fmtp_spropparametersets(),fileBufferSize,False);
			subsession->sink = pfilesink;
#else
			fileSink = FlvFileSink::createNew(*env,NULL,subsession->fmtp_spropparametersets(),fileBufferSize,False);
			if (fileSink != NULL)
			{
				fileSink->setStreamPtr((intptr_t)this);
				fileSink->setCallBackFunc(videoHeaderfunc, dataHandlefunc, datauser);
				subsession->sink = fileSink;
			}
			else
			{
				if (fHandleResult != NULL)
				{
					fHandleResult(ERROR_CREATEFILSINK,"create flv_sink failure.",fuser);
				}
			}
#endif

		}
		else if (strcmp(subsession->mediumName(), "audio") == 0)// &&
		//	strcmp(subsession->codecName(), "PCMU") == 0)
		{
			if (blHandleAudio == True)
			{
				continue;
			}
			pcmSink = PCMSink::createNew(*env,NULL,subsession->rtpPayloadFormat(),fileBufferSize,False,subsession->rtpTimestampFrequency(),subsession->numChannels());
			if (pcmSink != NULL)
			{
				pcmSink->setStreamPtr((intptr_t)this);
				pcmSink->setCallBackFunc(audioHeaderfunc, dataHandlefunc, datauser);
				subsession->sink = pcmSink;
			}
			else
			{
				if (fHandleResult != NULL)
				{
					fHandleResult(ERROR_CREATEPCMSINK,"create PCM_sink failure.",fuser);
				}
			}
		}
		else
		{
			subsession->sink = NULL;
			//unknow error
			if (fHandleResult != NULL)
			{
				fHandleResult(ERROR_UNKNOW,"unknow error.",fuser);
			}
			return;
		}

		//callback
		subsession->sink->startPlaying(*(subsession->readSource()),subsessionAfterPlaying,this);

		if (subsession->rtcpInstance() != NULL)
		{
			//callback
			subsession->rtcpInstance()->setByeHandler(subsessionByeHandler,this);
		}

		if (duration == 0)
		{
			if (scale > 0)
			{
				duration = session->playEndTime() - initialSeekTime;
			}
			else if (scale < 0)
			{
				duration = initialSeekTime;
			}
		}
		if (duration < 0)
		{
			duration = 0.0;
		}
		endtime = initialSeekTime;

		if (scale > 0)
		{
			if (duration <= 0)
			{
				endtime = -1.0f;
			}
			else
			{
				endtime = initialSeekTime + duration;
			}
		}
		else
		{
			endtime = initialSeekTime - duration;
			if (endtime < 0)
			{
				endtime = 0.0f;
			}
		}
//		rtspClient->sendPlayCommand(*subsession, continueAfterPLAY, initialSeekTime,endtime,scale, NULL);

	}
	if (pcmSink || fileSink)
	{
		rtspClient->sendPlayCommand(*session,continueAfterPLAY, initialSeekTime,endtime,scale, NULL);
	}
}

void CH264Stream::StartStreamThd()
{
	rtspClient->SethandleObj(this,NULL);
	if (rtspClient->sendOptionsCommand(optionHandleFunc,NULL)==0)
	{
		rtspClient->sendDescribeCommand(discriptionHandleFunc);
		if (fHandleResult != NULL)
		{
			fHandleResult(RERROR_NO,"senddescribecmd",fuser);
		}
	}
	else
	{
	//	discriptionHandleFunc(rtspClient,0,"");
	}
	m_hdThdHandle = CreateThread(NULL,0,DoTaskThread,this,0,&m_dwThdID);
	if (nCheckPacket==0)
	{
		checkPacketArrive(this);
		nCheckPacket=1;
		if (fHandleResult != NULL)
		{
			fHandleResult(RERROR_NO,"Start checkpacket",fuser);
		}
	}
}

DWORD WINAPI DoTaskThread(LPVOID lpParam)
{
	CH264Stream * h264Stream = (CH264Stream*)lpParam;

	h264Stream->env->taskScheduler().doEventLoop((char*)&(h264Stream->blContinuePlay));
	return 0;
}

void optionHandleFunc(RTSPClient* rtspClient,	int resultCode, char* resultString)
{
	CH264Stream * h264stream = (CH264Stream *)rtspClient->fobjPtr;
	if (resultCode!= 0)
	{
		h264stream->errorCode = resultCode;
		//re_code error code
		if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(ERROR_CONNECTURL,"URL not exist in the network or device is closed.",h264stream->fuser);
		}
		//h264stream->ShutDown();
	}
	else
	{
		if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(0,"URL not exist in the network or device is closed.",h264stream->fuser);
		}
	}
	delete[]resultString;
	rtspClient->sendDescribeCommand(discriptionHandleFunc);
}

//static unsigned desiredPortNum = 1681;

void discriptionHandleFunc(RTSPClient* rtspClient,	int resultCode, char* resultString)
{
	CH264Stream * h264stream = (CH264Stream *)rtspClient->fobjPtr;
	if (resultCode!= 0)
	{
		h264stream->errorCode = resultCode;
		//re_code error code
		if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(ERROR_USERNAMEORPASSWORD,"username or password is error.",h264stream->fuser);
		}
		//h264stream->ShutDown();
	}
	else
	{
		h264stream->session = MediaSession::createNew(*(h264stream->env), resultString);
		if (h264stream->session == NULL)
		{
			*(h264stream->env) << "Failed to create a MediaSession object from the SDP description: " << h264stream->env->getResultMsg() << "\n";
			//error code
			if (h264stream->fHandleResult)
			{
				h264stream->fHandleResult(ERROR_CREATESESSION,"Failed to create a MediaSession object from the SDP description",h264stream->fuser);
			}
			//h264stream->ShutDown();
		}
		else if (!h264stream->session->hasSubsessions())
		{
			*(h264stream->env) << "This session has no media subsessions (i.e., \"m=\" lines)\n";
			//error code
			if (h264stream->fHandleResult != NULL)
			{
				h264stream->fHandleResult(ERROR_NOSUBSESSION,"This session has no media subsessions",h264stream->fuser);
			}
			//h264stream->ShutDown();
		}

		MediaSubsessionIterator iter(*(h264stream->session));
		MediaSubsession *subsession;
		while((subsession = iter.next()) != NULL)
		{
			if (strcmp(subsession->mediumName(), h264stream->VideoMedia) == 0 ||strcmp(subsession->mediumName(), h264stream->AudioMedia) == 0)//
			{

			}
			else
			{
				*(h264stream->env)<<"Ignoring \""<<subsession->mediumName()<<"/"
					<<subsession->codecName()<<"\"subsession\n";
				continue;
			}
			//set socket port
			/*if (desiredPortNum != 0)
			{
				subsession->setClientPortNum(desiredPortNum);
				desiredPortNum += 2;
			}*/
			if (!subsession->initiate(h264stream->simpleRTPoffsetArg))
			{
				if (h264stream->fHandleResult != NULL)
				{
					h264stream->fHandleResult(ERROR_CREATERECIEVER,"Unable to create receiver",h264stream->fuser);
				}
				*(h264stream->env)<<"Unable to create receiver for \""<<subsession->mediumName()
					<<"/"<<subsession->codecName()<<"\"subsession:"<<h264stream->env->getResultMsg()<<"\n";
			}
			else
			{
				*(h264stream->env)<<"Create receiver for \""<<subsession->mediumName()
					<<"/"<<subsession->codecName()
					<<"\"subsession (client ports"<<subsession->clientPortNum()
					<<"_"<<subsession->clientPortNum() + 1<<")\n";
				if (subsession->rtpSource() != NULL)
				{
					unsigned const thresh = 1000000;
					subsession->rtpSource()->setPacketReorderingThresholdTime(thresh);

					//set receiver buffer size...else use default size
					int socketNum = subsession->rtpSource()->RTPgs()->socketNum();
					unsigned curBufferSize = getReceiveBufferSize(*(h264stream->env), socketNum);
					if (h264stream->fileBufferSize > 0 || h264stream->fileBufferSize > curBufferSize)
					{
						//h264stream->fileBufferSize = h264stream->fileBufferSize > curBufferSize ? h264stream->fileBufferSize : curBufferSize;
						unsigned newBuffersSize = setReceiveBufferTo(*(h264stream->env), socketNum, h264stream->fileBufferSize);
					}
				}
				else
				{
					if (h264stream->fHandleResult != NULL)
					{
						h264stream->fHandleResult(ERROR_RTSPSOURCE,"the subsession has not rtsp_source",h264stream->fuser);
					}
				}
			}
		}
		//playsteam
	//	EnterCriticalSection(&h264stream->crit_lock2);
		h264stream->setupStream();
	//	LeaveCriticalSection(&h264stream->crit_lock2);
	}
	delete[] resultString;
}
void sessionAfterPlaying(void * clientData)
{
	CH264Stream * h264stream = (CH264Stream *)clientData;

	__try
	{
		EnterCriticalSection(&h264stream->crit_lock);
		if (h264stream->blContinuePlay == False)
		{
			h264stream->_Release();
		}
		else
		{
			if (h264stream->env != NULL)
			{
				h264stream->env->taskScheduler().unscheduleDelayedTask(h264stream->arrivalCheckTimerTask);
				h264stream->env->taskScheduler().unscheduleDelayedTask(h264stream->sessionTimerTask);
				h264stream->env = NULL;
			}
			h264stream->rtspClient->sendPlayCommand(*(h264stream->session), continueAfterPLAY, h264stream->initialSeekTime,h264stream->endtime,h264stream->scale, NULL);
		}
		LeaveCriticalSection(&h264stream->crit_lock);
	}
	__except (1)
	{
		FILE* fp = fopen("H264StreamError.txt", "a");
		if (fp != NULL)
		{
			fprintf(fp, "Error session after playing in h264 stream.");
			fclose(fp);
		}
	}
}
void sessionTimerHandler(void *clientData)
{
	CH264Stream * h264stream = (CH264Stream *)clientData;
	h264stream->sessionTimerTask = NULL;
	sessionAfterPlaying(clientData);
}
void continueAfterPLAY(RTSPClient * rtspClient, int resultCode, char* resultString)
{
	CH264Stream * h264stream = (CH264Stream *)rtspClient->fobjPtr;
	if (resultCode != 0)
	{
		*(h264stream->env) << "Failed to start playing session: " << resultString << "\n";
		if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(0,resultString,h264stream->fuser);
		}

	}
	else
	{
		*(h264stream->env) << "Started playing session\n";
		//return;
	}

	// Figure out how long to delay (if at all) before shutting down, or
	// repeating the playing
	Boolean timerIsBeingUsed = False;
	double secondsToDelay = h264stream->duration;
	if (h264stream->duration > 0)
	{
		// First, adjust "duration" based on any change to the play range (that was specified in the "PLAY" response):
		double rangeAdjustment = (h264stream->session->playEndTime() - h264stream->session->playStartTime())
			- (h264stream->endtime - h264stream->initialSeekTime);
		if (h264stream->duration + rangeAdjustment > 0.0)
		{
			h264stream->duration += rangeAdjustment;
		}

		timerIsBeingUsed = True;
		double absScale = h264stream->scale > 0 ? h264stream->scale : -h264stream->scale; // ASSERT: scale != 0
		secondsToDelay = h264stream->duration/absScale + h264stream->durationSlop;

		h264stream->uSecsToDelay = (int64_t)(secondsToDelay*1000000.0);
		h264stream->sessionTimerTask = h264stream->env->taskScheduler().
			scheduleDelayedTask(h264stream->uSecsToDelay, (TaskFunc*)sessionTimerHandler, h264stream);
	}
	else
	{
		/*if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(ERROR_PLAYSESSION,"Failed to start playing session",h264stream->fuser);
		}
		return;*/
	}

	/*h264stream->numReceiverByte = 0;
	h264stream->kBytes = 0;*/

}


void continueAfterSETUP(RTSPClient * rtspClient, int resultCode, char* resultString)
{
	CH264Stream * h264stream = (CH264Stream *)rtspClient->fobjPtr;

	if (resultCode == 0) //
	{
		/**env << "Setup \"" << subsession->mediumName()
			<< "/" << subsession->codecName()
			<< "\" subsession (client ports " << subsession->clientPortNum()
			<< "-" << subsession->clientPortNum()+1 << ")\n";*/
	//	EnterCriticalSection(&h264stream->crit_lock2);
		h264stream->setupStream();
	//	LeaveCriticalSection(&h264stream->crit_lock2);
	}
	else
	{
		h264stream->errorCode = resultCode;
		if (h264stream->fHandleResult != NULL)
		{
			//h264stream->fHandleResult(ERROR_SETUPSUBSESSION,"Failed to setup subsession.",h264stream->fuser);
		}
		delete h264stream->setupIter;
		h264stream->setupIter = NULL;
		/**env << "Failed to setup \"" << subsession->mediumName()
		<< "/" << subsession->codecName()
		<< "\" subsession: " << env->getResultMsg() << "\n";*/
	}

}

void subsessionAfterPlaying(void * clientData)
{
	CH264Stream * h264stream = (CH264Stream *)clientData;
	//MediaSubsessionIterator iter(*h264stream->session);
	//MediaSubsession* subsession;
	//while ((subsession = iter.next()) != NULL)
	//{
	//	if (subsession->sink != NULL)
	//	{
	//		Medium::close(subsession->sink);
	//		subsession->sink = NULL;
	//		return; // this subsession is still active
	//	}
	//}

	// All subsessions' streams have now been closed
	//h264stream->setupStream();
	//sessionAfterPlaying(clientData);
	//h264stream->Release();
	if (h264stream->fHandleResult != NULL/* && h264stream->blContinuePlay*/)
	{
		h264stream->fHandleResult(ERROR_PLAYSESSION,"Failed to start playing session",h264stream->fuser);
		return;
	}
}
void subsessionByeHandler(void * clientData)
{
//	subsessionAfterPlaying( clientData);
	CH264Stream * h264stream = (CH264Stream *)clientData;
	if (h264stream->fHandleResult != NULL/* && h264stream->blContinuePlay*/)
	{
		h264stream->fHandleResult(ERROR_PLAYSESSION,"Failed subsessionByeHandler",h264stream->fuser);
		return;
	}
}

void tearDownSessionHandleFunc(RTSPClient* rtspClient,	int resultCode, char* resultString)
{
	if (rtspClient == NULL)
	{
		return;
	}

	CH264Stream * h264stream = (CH264Stream *)rtspClient->fobjPtr;
	if (resultCode!= 0)
	{
		h264stream->errorCode = resultCode;
		if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(ERROR_UNKNOW,"can not make sure the error.",h264stream->fuser);
		}
	}
	else
	{
		//return ;
	}

	//if (h264stream->session == NULL)
	//{
	//	if (h264stream->scheduler)
	//	{
	//		delete h264stream->scheduler;
	//		h264stream->scheduler = NULL;
	//	}
	//	SetEvent(h264stream->sigle);
	//	return;
	//}
	//MediaSubsessionIterator iter(*(h264stream->session));
	//MediaSubsession* subsession;
	//while ((subsession = iter.next()) != NULL)
	//{
	//	Medium::close(subsession->sink);
	//	subsession->sink = NULL;
	//}
	//Medium::close(h264stream->session);
	//h264stream->session = NULL;
	//// Finally, shut down our client:
	//Medium::close(h264stream->rtspClient);
	//h264stream->rtspClient = NULL;

	//if (h264stream->scheduler)
	//{
	//	delete h264stream->scheduler;
	//	h264stream->scheduler = NULL;
	//}
	//SetEvent(h264stream->sigle);
	 //Adios...
	//exit(0);
}

//using check receiver packet number to set initial rtsp
void checkPacketArrive(void * clientData)
{
	CH264Stream * h264stream = (CH264Stream *)clientData;

	//if (h264stream->session == NULL)
	//{
	//	h264stream->createRTSP();
	//	return;
	//}
	double temp=0;
	if (h264stream->session)
	{
		MediaSubsessionIterator iter(*(h264stream->session));
		MediaSubsession*subsession;

		while((subsession = iter.next()) != NULL)
		{
			RTPSource* src = subsession->rtpSource();
			if (src && subsession->clientPortNum())//|| strcmp(subsession->mediumName(), h264stream->AudioMedia) == 0
			{
				RTPReceptionStatsDB::Iterator statsIter(src->receptionStatsDB());
				RTPReceptionStats* stats;

				while((stats = statsIter.next(True)) != NULL)
				{
					temp /*+*/= stats->totNumKBytesReceived();
				}
				if ((strcmp(subsession->mediumName(), h264stream->VideoMedia) == 0))
				{
					h264stream->kBytes=temp;
				}
				if ((strcmp(subsession->mediumName(), h264stream->AudioMedia) == 0))
				{
					h264stream->kBytesA=temp;
				}
			}
		}
	}

	if ((h264stream->kBytes>0 || h264stream->kBytesA>0)&& (h264stream->kBytes == h264stream->numReceiverByte)  &&  (h264stream->kBytesA == h264stream->numReceiverByteA) )///*< 0.001*/
	{
		//hint: printf("the net can not connect,retreat!\n");
		//sessionTimerHandler(clientData);
		if (h264stream->fHandleResult != NULL)
		{
			h264stream->fHandleResult(ERROR_PLAYSESSION,"Not received packets.",h264stream->fuser);
			return;
		}
		//h264stream->createRTSP();
	}
	else
	{
		h264stream->numReceiverByte = h264stream->kBytes;
		h264stream->numReceiverByteA = h264stream->kBytesA;
		if((h264stream->numReceiverByte==0) &&(h264stream->numReceiverByteA==0))
		{
			h264stream->noPacket++;
		}
		else
			h264stream->noPacket=0;
		if (h264stream->noPacket==10)
		{
			if (h264stream->fHandleResult != NULL)
			{
				h264stream->fHandleResult(ERROR_PLAYSESSION,"Not received packets always.",h264stream->fuser);
				return;
			}
		}
	}
	int uSecsToDelay = 5000000;
	h264stream->arrivalCheckTimerTask
		= h264stream->env->taskScheduler().scheduleDelayedTask(uSecsToDelay,
		(TaskFunc*)checkPacketArrive, clientData);
}


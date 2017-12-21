#include "H264StreamWrap.h"
#include <YXC_AVKernel/YXV_rtsp.h>

//Media define
#define VIDEOH264 0x01
#define AUDIOPCMU 0x02

#define NOMEDIA	"xxxxx"

CH264StreamWrap::CH264StreamWrap(void)
{
	strcpy(streamURL,"rtsp://admin:12345@192.168.1.23:554/H264");
	fHandleResult		= NULL;
	fuser				= NULL;
	//get video/audio data call back function
	audioHeaderfunc		= NULL;
	videoHeaderfunc		= NULL;
	dataHandlefunc		= NULL;
	datauser			= NULL;
}


CH264StreamWrap::~CH264StreamWrap(void)
{
}

void CH264StreamWrap::setExceptionHandle(void * exceptionHandle, void * fUser)
{
	fHandleResult = (handleResult)exceptionHandle;
	fuser		  = fUser;
}
void CH264StreamWrap::setDataHandle(void * headFunc, void * audioHeader, void * streamFunc, void * user)
{
	audioHeaderfunc	 = audioHeader;
	videoHeaderfunc	 = headFunc;
	dataHandlefunc	 = streamFunc;
	datauser		 = user;
}

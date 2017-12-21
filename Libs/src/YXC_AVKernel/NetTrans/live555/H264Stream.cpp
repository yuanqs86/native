#include "H264Stream.h"
#include <YXC_AVKernel/YXV_rtsp.h>
#include "H264StdStream.h"

CH264Stream::CH264Stream(void)
{
	fHandleResult		= NULL;
	fuser				= NULL;
	//get video/audio data call back function
	audioHeaderfunc		= NULL;
	videoHeaderfunc		= NULL;
	dataHandlefunc		= NULL;
	datauser			= NULL;

	_wrap = NULL;
}


CH264Stream::~CH264Stream(void)
{
	if (_wrap != NULL)
	{
		delete _wrap;
		_wrap = NULL;
	}
}

void CH264Stream::configH264Stream(char *pParm, int inType, int bUseTCP)
{
	//if (strncmp(pParm, "hsdk://", 7) == 0) /* Start. */
	//{
		//this->_wrap = new CH264HkStream();
		//this->_wrap->configH264Stream(pParm, inType, TRUE);
	//}
	//else
	{
		this->_wrap = new CH264StdStream();
		this->_wrap->configH264Stream(pParm, inType, TRUE);
	}

	if (this->fHandleResult != NULL)
	{
		this->_wrap->setExceptionHandle(this->fHandleResult, this->fuser);
	}

	if (this->videoHeaderfunc != NULL)
	{
		this->_wrap->setDataHandle(videoHeaderfunc, audioHeaderfunc, dataHandlefunc, datauser);
	}
}

void CH264Stream::setExceptionHandle(void * exceptionHandle, void * fUser)
{
	fHandleResult = (handleResult)exceptionHandle;
	fuser		  = fUser;
	if (this->_wrap != NULL)
	{
		this->_wrap->setExceptionHandle(exceptionHandle, fUser);
	}
}
void CH264Stream::setDataHandle(void * headFunc, void * audioHeader, void * streamFunc, void * user)
{
	audioHeaderfunc	 = audioHeader;
	videoHeaderfunc	 = headFunc;
	dataHandlefunc	 = streamFunc;
	datauser		 = user;

	if (this->_wrap != NULL)
	{
		this->_wrap->setDataHandle(videoHeaderfunc, audioHeaderfunc, dataHandlefunc, datauser);
	}
}

int CH264Stream::createRTSP()
{
	return this->_wrap->createRTSP();
}

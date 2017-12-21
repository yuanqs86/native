#include <YXC_AVKernel/YXV_rtsp.h>
#include "H264Stream.h"

intptr_t  CreateH264Stream(void)
{
	CH264Stream * mH264Stream = new CH264Stream();
	return intptr_t(mH264Stream);
}
void DeleteH264Stream(intptr_t inHandle)
{
	CH264Stream * mH264Stream = (CH264Stream*)inHandle;
	if (mH264Stream != NULL)
	{
		delete mH264Stream;
		mH264Stream = NULL;
	}
}

void ConfigH264Stream(intptr_t inHandle,char *pParm,int nSize,BOOL bUseTCP)
{
	CH264Stream * mH264Stream = (CH264Stream*)inHandle;
	if (mH264Stream != NULL)
	{
		mH264Stream->configH264Stream(pParm,nSize,bUseTCP);
	}
}

void SetExceptionHandle(intptr_t inHandle, void * Handle, void * fUser)
{
	CH264Stream * mH264Stream = (CH264Stream*)inHandle;
	mH264Stream->setExceptionHandle(Handle,fUser);
}
void SetDataHandle(intptr_t inHandle,void * headFunc, void * audioHeader, void * streamFunc, void * user)
{
	CH264Stream * mH264Stream = (CH264Stream*)inHandle;
	mH264Stream->setDataHandle(headFunc,audioHeader,streamFunc,user);
}

void SetupH264Stream(intptr_t inHandle)
{
	CH264Stream * mH264Stream = (CH264Stream*)inHandle;
	mH264Stream->createRTSP();
}


#ifndef __H__H264STREAM__
#define __H__H264STREAM__

#include <windows.h>
#include <YXC_Sys/YXC_Sys.h>

#ifdef _H264STREAM_
#define H264STREAMDLL __declspec(dllexport)
#else
#define H264STREAMDLL __declspec(dllimport)
#endif

//Audio Tag define
#define PCMA 0x0006
#define PCMU 0x0007

//Media define
#define VIDEOH264 0x01
#define AUDIOPCMU 0x02

#define RERROR_NO					0x0000
#define ERROR_CRTEAYECLIENT			0x0001
#define ERROR_CONNECTURL			0x0002
#define ERROR_USERNAMEORPASSWORD	0x0003
#define ERROR_CREATESESSION			0x0004
#define ERROR_NOSUBSESSION			0x0005
#define ERROR_CREATERECIEVER		0x0006
#define ERROR_RTSPSOURCE			0x0007
#define ERROR_CREATEFILSINK			0x0008
#define ERROR_CREATEPCMSINK			0x0009
#define ERROR_PLAYSESSION			0x000a
#define ERROR_SETUPSUBSESSION		0x000b
#define ERROR_CONTINUEPLAY			0x000c

#define ERROR_UNKNOW				0xffff

//callback function
typedef void (*AudioHeaderFunc)(intptr_t ptrMediaStream, unsigned int, char *, void *);
typedef void (*VideoheaderFunc)(intptr_t ptrMediaStream, unsigned int, unsigned char *,unsigned int, unsigned char *, void *);
typedef void (*streamFunc)(intptr_t ptrMediaStream,int inType, unsigned char *,unsigned int,__int64,void *);
typedef void (*ExceptionFunc)(int , char * , void * );
#ifdef __cplusplus
extern "C"
{
#endif
	YXC_API(intptr_t)  CreateH264Stream(void);
	YXC_API(void) DeleteH264Stream(intptr_t inHandle);
	YXC_API(void) ConfigH264Stream(intptr_t inHandle,const char *pParm,int inType,BOOL bUseTCP=TRUE);
	YXC_API(void) SetExceptionHandle(intptr_t inHandle, void * Handle, void * fUser);
	YXC_API(void) SetDataHandle(intptr_t inHandle,void * headFunc, void * audioHeader, void * streamFunc, void * user);
	YXC_API(void) SetupH264Stream(intptr_t inHandle);
#ifdef __cplusplus
};
#endif

#endif

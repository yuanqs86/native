#pragma once
#include <YXC_Sys/YXC_Sys.h>
#include <strmif.h>
#include <YXC_AVKernel/baseclasses/streams.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
	YXC_API long CreateWriterFile(void);
	YXC_API void DeleteWriterFile(long mHandle);

	YXC_API bool ConfigInputVideo(long mHandle, AM_MEDIA_TYPE * inpmtVideo);
	YXC_API bool ConfigInputAudio(long mHandle, AM_MEDIA_TYPE * inpmtAudio);

	YXC_API bool SetDestFileName(long mHandle, LPOLESTR pFile);
	YXC_API bool SetPushURLLink(long mHandle, LPOLESTR inPushURL,LPOLESTR inPushUser,LPOLESTR inPushPasswd);

	YXC_API bool StartMediaStreaming(long mHandle, BOOL bEncodeAudio);
	YXC_API bool StopMediaStreaming(long mHandle);

	YXC_API bool ReceiverStreamVideo(long mHandle, unsigned long long dwTime, DWORD dwDuration, char * lpBuf, int inLen, DWORD dwFlags);
	YXC_API bool ReceiverStreamAudio(long mHandle, unsigned long long dwTime, DWORD dwDuration, char * lpBuf, int inLen, DWORD dwFlags);
#ifdef __cplusplus
};
#endif /*__cplusplus*/

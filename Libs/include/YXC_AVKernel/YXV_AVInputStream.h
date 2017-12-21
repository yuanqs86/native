/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_AV_INPUT_STREAM_H__
#define __INC_YXC_AV_INPUT_STREAM_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/YXV_AVUtils.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_AVInputStream);

	typedef void (*YXV_AVISVideoCallback)(const YXV_Frame* vFrame, void* ctrl);
	typedef void (*YXV_AVISAudioCallback)(const YXV_Frame* aFrame, void* ctrl);
	typedef void (*YXV_AVISStoppedCallback)(ybool_t bManual, void* ctrl);

	typedef struct __YXV_AVIS_CONTROL_PARAM
	{
		YXV_AVISVideoCallback vCallback;
		YXV_AVISAudioCallback aCallback;
		YXV_AVISStoppedCallback stoppedCallback;
		yuint64_t maxBufferTime;
		yuint64_t minBufferTime;
		void* ptr;
	} YXV_AVISCtrlParam;

	YXC_API(YXC_Status) YXV_AVISCreate(const yuint8_t* pszFilename, const YXV_AVISCtrlParam* ctrl, YXV_AVInputStream* stream);

	YXC_API(void) YXV_AVISReadStreamInfo(YXV_AVInputStream stream, YXV_VParam* vParam, YXV_AParam* aParam);

	YXC_API(YXC_Status) YXV_AVISGetDuration(YXV_AVInputStream stream, yuint64_t* pDuration);

	YXC_API(YXC_Status) YXV_AVISGetPlayPos(YXV_AVInputStream stream, yint64_t* pPosition, yint32_t* seekStatus);

	YXC_API(YXC_Status) YXV_AVISGetBufPos(YXV_AVInputStream stream, yint64_t* pPosition);

	YXC_API(YXC_Status) YXV_AVISSeek(YXV_AVInputStream stream, yint64_t time);

	YXC_API(void) YXV_AVISGetStatus(YXV_AVInputStream stream, YXV_MediaPlayStatus* pStatus);

	YXC_API(YXC_Status) YXV_AVISPause(YXV_AVInputStream stream);

	YXC_API(YXC_Status) YXV_AVISStop(YXV_AVInputStream stream);

	YXC_API(YXC_Status) YXV_AVISPlay(YXV_AVInputStream stream);

	YXC_API(void) YXV_AVISDestroy(YXV_AVInputStream stream);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_INPUT_STREAM_H__ */

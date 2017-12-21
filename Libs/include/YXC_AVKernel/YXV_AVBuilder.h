/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_AV_MUXER_H__
#define __INC_YXC_AV_MUXER_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/YXV_AVUtils.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_AVBuilder);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_AVBuilderElement);

	typedef struct __YXV_AV_SOURCE_LIST
	{
		yuint32_t uNumSources;
		YXV_AVSource* pSources;
	}YXV_AVSourceList;

	YXC_API(YXC_Status) YXV_AVBuilderCreate(YXV_VFilterType vfType, yuint32_t uWidth, yuint32_t uHeight, YXV_PixFmt pixFmt,
		yuint32_t uNumChannels, yuint32_t uFreq, YXV_SampleFmt sampleFmt, YXV_AVBuilder* ppBuilder);

	YXC_API(void) YXV_AVBuilderDestroy(YXV_AVBuilder builder);

	/* Open sources, and mix as these sources, don't free them when processing. */
	/* You can do sort or change settings when mixing. */
	YXC_API(YXC_Status) YXV_AVBuilderOpenSources(YXV_AVBuilder builder, YXV_AVSourceList* pSourceList);

	/* Open sources, and mix as these sources. */
	YXC_API(void) YXV_AVBuilderCloseSources(YXV_AVBuilder builder, YXV_AVSourceList* pSourceList);

	/* Generate a frame at duration (duration = 100ns). */
	YXC_API(YXC_Status) YXV_AVBuilderFrame(YXV_AVBuilder builder, yint64_t i64FrameDuration);

	/* Skip this frame, delete frame data. */
	YXC_API(YXC_Status) YXV_AVBuilderSkip(YXV_AVBuilder builder, yint64_t i64SkipTime);

	YXC_API(void) YXV_AVBuilderFill(YXV_AVBuilder builder, YXV_Frame* pImgData, YXV_Frame* pAudioData);

	YXC_API(YXC_Status) YXV_AVBuilderSeek(YXV_AVBuilder builder, yint64_t i64SeekTo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_BUILDER_H__ */

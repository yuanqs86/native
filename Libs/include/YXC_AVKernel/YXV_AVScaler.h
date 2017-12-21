/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_AV_SCALER_H__
#define __INC_YXC_AV_SCALER_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_AVKernel/YXV_AVUtils.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_VScaler);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_ASampler);

	typedef struct __YXV_VIDEO_SCALER_PARAM
	{
		yuint32_t interpolation;
		ybool_t bInvert;
		ybool_t bHWAcceleration;
	}YXV_VScalerParam;

	YXC_Status YXV_VScalerCreate(const YXV_PicDesc* srcDesc, const YXV_PicDesc* dstDesc, const YXV_VScalerParam* sParam, YXV_VScaler* pScaler);

	YXC_Status YXV_VScalerAllocInSample(YXV_VScaler scaler, YXV_Frame* pSample);

	YXC_Status YXV_VScalerAllocOutSample(YXV_VScaler scaler, YXV_Frame* pSample);

	void YXV_VScalerFillInSample(YXV_VScaler scaler, ybyte_t* byBuffer, YXV_Frame* pSample);

	void YXV_VScalerFillOutSample(YXV_VScaler scaler, ybyte_t* byBuffer, YXV_Frame* pSample);

	YXC_Status YXV_VScalerScale(YXV_VScaler scaler, const YXV_Frame* sample_in, YXV_Frame* sample_out);

	void YXV_VScalerDestroy(YXV_VScaler scaler);

	YXC_Status YXV_ASamplerCreate(YXV_SampleFmt inFmt, yuint32_t sc, yuint32_t sr, YXV_SampleFmt outFmt,
		yuint32_t dc, yuint32_t dr, YXV_ASampler* pSampler);

	YXC_Status YXV_ASamplerAllocInSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample);

	YXC_Status YXV_ASamplerAllocOutSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample);

	void YXV_ASamplerFillInSample(YXV_ASampler sampler, ybyte_t* byBuffer, yuint32_t uNumSamples, YXV_Frame* pSample);

	void YXV_ASamplerFillOutSample(YXV_ASampler sampler, ybyte_t* byBuffer, yuint32_t uNumSamples, YXV_Frame* pSample);

	void YXV_ASamplerSkipInSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample);

	void YXV_ASamplerSkipOutSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample);

	YXC_Status YXV_ASamplerResample(YXV_ASampler sampler, const YXV_Frame* sample_in, YXV_Frame* sample_out);

	void YXV_ASamplerDestroy(YXV_ASampler sampler);

	/* Old Function list. */

	void __cdecl YXV_RGB24ImageResize(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs,
		int nAlign YXC_DEF_PARAM(4), int interpolation YXC_DEF_PARAM(YXV_IMAGE_INTER_LINEAR), BOOL bInvert YXC_DEF_PARAM(FALSE));

	void __cdecl YXV_YUVImageResize(char* dst, const char* src, int sw, int sh, int dw, int dh, int interpolation YXC_DEF_PARAM(YXV_IMAGE_INTER_LINEAR),BOOL bInvert YXC_DEF_PARAM(TRUE));

	void __cdecl YXV_YUVResizeNN2RGB24(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign YXC_DEF_PARAM(4),
		int interpolation YXC_DEF_PARAM(YXV_IMAGE_INTER_NN), BOOL bInvert YXC_DEF_PARAM(FALSE));

	void __cdecl YXV_RGB24ResizeNN2YUV(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign YXC_DEF_PARAM(4),
		int interpolation YXC_DEF_PARAM(YXV_IMAGE_INTER_NN), BOOL bInvert YXC_DEF_PARAM(FALSE));

	void YXV_YUVResizeNN2RGB24_Step(char* dst, const char* src[3], int stride[3], int sw, int sh, int dw, int dh, int drs,
		int nAlign YXC_DEF_PARAM(4), int interpolation YXC_DEF_PARAM(YXV_IMAGE_INTER_NN), BOOL bInvert YXC_DEF_PARAM(FALSE));

	void __cdecl YXV_NV12ResizeNN2RGB32(char* dst, const char* src, int sw, int sh, int dw, int dh, int drs, int nAlign YXC_DEF_PARAM(4),
		int interpolation YXC_DEF_PARAM(YXV_IMAGE_INTER_NN), BOOL bInvert YXC_DEF_PARAM(FALSE));

	void __cdecl YXV_YUV2RGB24(char* dst, const char* src, int sw, int sh, BOOL bInvert YXC_DEF_PARAM(TRUE));

	void __cdecl YXV_YUYV2RGB24(char* dst, const char* src, int sw, int sh, BOOL bInvert YXC_DEF_PARAM(TRUE));

	void __cdecl YXV_RGB242YUV(char* dst, const char* src, int sw, int sh, BOOL bInvert YXC_DEF_PARAM(TRUE));


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_SCALER_H__ */

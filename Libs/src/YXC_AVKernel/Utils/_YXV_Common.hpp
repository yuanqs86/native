#ifndef __INNER_INC_YXC_AV_COMMON_HPP__
#define __INNER_INC_YXC_AV_COMMON_HPP__

#include <YXC_AVKernel/YXV_AVBuilder.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>

namespace _YXV_AVUtils
{
	YXC_Status _CalculateSampleInfo(YXV_PixFmt fmt, yuint32_t w, yuint32_t h, yuint32_t* pC,
		yuint32_t* pRSArr, yuint32_t* pHArr, yuint32_t* pCSArr, yuint32_t* puS);

	void _FillVSample(YXV_Buffer* buffer, ybool_t bInvert, const YXV_PicDesc* picDesc, yuint32_t uC, yuint32_t* pRSArr,
		yuint32_t* pHArr, yuint32_t* pCSArr, YXV_Frame* pSample);

	void _FillPixInfo(YXV_PixFmt pixFmt, yuint8_t (&ratioW)[YXV_MAX_NUM_PLANARS],
		yuint8_t (&ratioH)[YXV_MAX_NUM_PLANARS], yuint8_t (&pixelSize)[YXV_MAX_NUM_PLANARS]);

	YXC_Status _AllocVSample(ybool_t bInvert, const YXV_PicDesc* picDesc, yuint32_t uC, yuint32_t* pRSArr, yuint32_t* pHArr,
		yuint32_t* pCSArr, yuint32_t uS, YXV_Frame* pSample);

	static inline yuint32_t _GetSampleBits(YXV_SampleFmt sampleFmt)
	{
		yuint32_t uBitVal = YXC_HI_16BITS(sampleFmt & YXV_SAMPLE_FMT_BITS_MASK);
		return uBitVal;
	}

	static inline ybool_t _IsPlanarSampleFmt(YXV_SampleFmt sampleFmt)
	{
		yuint32_t uVal = sampleFmt & YXV_SAMPLE_FMT_TYPE_PLANAR;
		return uVal != 0;
	}

	void _FillASample(YXV_Buffer* buffer, const YXV_SampleDesc* desc, yuint32_t uP, yuint32_t uSampleBytes, yuint32_t uNumSamples, YXV_Frame* sample);

	YXC_Status _AllocASample(const YXV_SampleDesc* desc, yuint32_t uP, yuint32_t uSampleBytes, yuint32_t uNumSamples, YXV_Frame* sample);

	extern int gs_resize_flag;
}

#endif /* __INNER_INC_YXC_AV_COMMON_HPP__ */

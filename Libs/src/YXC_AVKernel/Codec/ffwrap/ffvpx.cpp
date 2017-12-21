#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_H264.h>
#include <YXC_Sys/YXC_NetMarshal.h>

#define CONFIG_FTRAPV 0

namespace YXV_FFWrap
{
	YXC_Status FFSpecV<AV_CODEC_ID_VP8>::_FFConfigV_SpecEnc(AVCodecID codecId2, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		YXV_FFVSpecParam param2;
		YXV_FFVPX_VP8Param* vp8Param = &param2.vpx_vp8Param;
		YXV_FFVSpecParamDefault(YXV_VCODEC_ID_VP8, TRUE, &param2);

		if (pSpecd != NULL)
		{
			vp8Param = (YXV_FFVPX_VP8Param*)&pSpecd->vpx_vp8Param;
		}

		context->profile = vp8Param->profile;

		char szIntProfile[20];
		sprintf(szIntProfile, "%d", vp8Param->speed_profile);
		av_opt_set(context->priv_data, "deadline", szIntProfile, 0);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_VP8>::_FFConfigV_SpecDec(AVCodecID codecId2, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_VP8>::_FFConfigV_SpecRead(AVCodecID codecId2, AVCodecContext* context, YXV_VParam* vParam, YXV_FFVFormatParam* pSpecd)
	{
		// _YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}
}

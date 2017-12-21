#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_H264.h>
#include <YXC_Sys/YXC_NetMarshal.h>

namespace YXV_FFWrap
{
	YXC_Status FFSpecA<AV_CODEC_ID_OPUS>::_FFConfigA_SpecEnc(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpecd, AVCodecContext* context)
	{
		YXV_FFASpecParam param2;
		YXV_FFOpusParam* opusParam = &param2.opusParam;
		YXV_FFASpecParamDefault(YXV_ACODEC_ID_OPUS, TRUE, &param2);

		if (pSpecd != NULL)
		{
			opusParam = (YXV_FFOpusParam*)&pSpecd->opusParam;
		}

		int ret = av_opt_set_double(context->priv_data, "frame_duration", opusParam->dblRatio, 0);
		_YXC_CHECK_FF_RET(ret);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecA<AV_CODEC_ID_OPUS>::_FFConfigA_SpecDec(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecA<AV_CODEC_ID_OPUS>::_FFConfigA_SpecRead(AVCodecID codecId, AVCodecContext* context, YXV_AParam* param, YXV_FFAFormatParam* pSpecd)
	{
		YXV_FFASpecParamDefault(CodecToFFCodecA(AV_CODEC_ID_OPUS), TRUE, &*pSpecd[0]);
		YXV_FFExtraData* pParam = &(*pSpecd)[1].extData;

		pParam->pExt = context->extradata;
		pParam->uExt = context->extradata_size;
		pParam->uExtBuf = 0;

		return YXC_ERC_SUCCESS;
	}
}

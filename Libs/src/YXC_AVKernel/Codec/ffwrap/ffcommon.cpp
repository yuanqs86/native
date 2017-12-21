#include <YXC_AVKernel/Codec/ffwrap/ffcommon.hpp>
#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_H264.h>
#include <YXC_Sys/YXC_NetMarshal.h>

namespace YXV_FFWrap
{
	static YXX_Crit _codec_lock;
	void avcodec_close_locked(AVCodecContext* pContext)
	{
		YXX_CritLocker locker(_codec_lock);
		avcodec_close(pContext);
	}

	int avcodec_open2_locked(AVCodecContext* pContext, AVCodec* pCodec, AVDictionary** ppDict)
	{
		YXX_CritLocker locker(_codec_lock);
		return avcodec_open2(pContext, pCodec, ppDict);
	}

	void _av_free_context(AVCodecContext* pContext)
	{
		if (pContext->extradata)
		{
			av_free(pContext->extradata);
			pContext->extradata = NULL;
		}
		avcodec_close_locked(pContext);
		av_free(pContext);
	}

	YXC_Status _InitExtData(AVCodecContext* context, yuint32_t uCbData, const void* pData)
	{
		uint8_t* extradata = (uint8_t*)av_malloc(uCbData);
		_YXC_CHECK_FF_PTR_RET(extradata);

		memcpy(extradata, pData, uCbData);
		context->extradata = extradata;
		context->extradata_size = uCbData;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_NONE>::_FFConfigV_SpecEnc(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_NONE>::_FFConfigV_SpecDec(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_NONE>::_FFConfigV_SpecRead(AVCodecID codecId, AVCodecContext* context, YXV_VParam* vParam, YXV_FFVFormatParam* pSpecd)
	{
		YXV_FFExtraData* pParam = &(*pSpecd)[1].extData;
		(*pSpecd)[1].szEncDecName[0] = 0;

		pParam->pExt = context->extradata;
		pParam->uExt = context->extradata_size;
		pParam->uExtBuf = 0;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecA<AV_CODEC_ID_NONE>::_FFConfigA_SpecEnc(AVCodecID codecId, const YXV_AParam* aParam, const YXV_FFASpecParam* pSpecd, AVCodecContext* context)
	{
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecA<AV_CODEC_ID_NONE>::_FFConfigA_SpecDec(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecA<AV_CODEC_ID_NONE>::_FFConfigA_SpecRead(AVCodecID codecId, AVCodecContext* context, YXV_AParam* param, YXV_FFAFormatParam* pSpecd)
	{
		YXV_FFASpecParamDefault(CodecToFFCodecA(codecId), TRUE, &(*pSpecd)[0]);
		YXV_FFASpecParamDefault(CodecToFFCodecA(codecId), FALSE, &(*pSpecd)[1]);
		YXV_FFExtraData* pParam = &(*pSpecd)[1].extData;

		pParam->pExt = context->extradata;
		pParam->uExt = context->extradata_size;
		pParam->uExtBuf = 0;

		return YXC_ERC_SUCCESS;
	}

	static int iInit_fmt = (av_register_all(), 0);
	static int iInit_codec = (avcodec_register_all(), 0);
	static int iInit_filter = (avfilter_register_all(), 0);
	static int iInit_network = avformat_network_init();
}

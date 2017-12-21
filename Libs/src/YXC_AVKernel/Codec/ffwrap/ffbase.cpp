#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_H264.h>
#include <YXC_Sys/YXC_NetMarshal.h>

extern "C"
{
	//void *avpriv_atomic_ptr_cas(void * volatile *ptr, void *oldval, void *newval)
	//{
	//	void *ret = YXCLib::Interlocked::CompareExchangePtr(ptr, newval, oldval);
	//	return ret;
	//}
}

namespace YXV_FFWrap
{
	void _FFPacketReadCvtProcH264(YXV_Packet* pPacket);
	int _FFWriteProcH264(AVFormatContext* ctx, AVPacket* pkt);

	static AVCodec* _FindCodecByName(AVCodecID codecId, ybool_t bEnc, const char* pszName)
	{
		AVCodec* codec = NULL;
		ysize_t sLen = strlen(pszName);
		if (sLen > 0)
		{
			if (bEnc)
			{
				codec = avcodec_find_encoder_by_name(pszName);
			}
			else
			{
				codec = avcodec_find_decoder_by_name(pszName);
			}
		}
		else
		{
			if (bEnc)
			{
				codec = avcodec_find_encoder(codecId);
			}
			else
			{
				codec = avcodec_find_decoder(codecId);
			}
		}
		return codec;
	}

	static AVCodec* _FindCodecByVParam(AVCodecID codecId, ybool_t bEnc, const YXV_FFVSpecParam* pSpecd)
	{
		const char* pszName = pSpecd == NULL ? "" : pSpecd->szEncDecName;
		return _FindCodecByName(codecId, bEnc, pszName);
	}

	static AVCodec* _FindCodecByAParam(AVCodecID codecId, ybool_t bEnc, const YXV_FFASpecParam* pSpecd)
	{
		const char* pszName = pSpecd == NULL ? "" : pSpecd->szEncDecName;
		return _FindCodecByName(codecId, bEnc, pszName);
	}

	template <AVCodecID templateId>
	YXC_Status FFSpecV<templateId>::_FFConfigV_SpecEnc(AVCodecID codecId, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFSpecV<templateId>::_FFConfigV_SpecDec(AVCodecID codecId, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFSpecV<templateId>::_FFConfigV_SpecRead(AVCodecID codecId, AVCodecContext* context, YXV_VParam* param, YXV_FFVFormatParam* pSpecd)
	{
		YXV_FFVSpecParamDefault(CodecToFFCodecV(codecId), TRUE, &(*pSpecd)[0]);
		YXV_FFVSpecParamDefault(CodecToFFCodecV(codecId), TRUE, &(*pSpecd)[1]);
		YXV_FFExtraData* pParam = &(*pSpecd)[1].extData;

		pParam->pExt = context->extradata;
		pParam->uExt = context->extradata_size;
		pParam->uExtBuf = 0;

		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncV<templateId>::_FFConfigV_Enc(AVCodecID codecId, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd, AVCodecContext** ppContext)
	{
		AVCodec* codec = _FindCodecByVParam(codecId, TRUE, pSpecd);
		_YXC_CHECK_FF_PTR_RET(codec);

		AVCodecContext* context = avcodec_alloc_context3(codec);
		_YXC_CHECK_FF_PTR_RET(context);

		YXCLib::HandleRef<AVCodecContext*> hRet(context, _av_free_context);

		AVPixelFormat pixFmt = codec->pix_fmts[0];
		//if (param->desc.pixFmt != YXV_PIX_FMT_NONE)
		//{
		//	pixFmt = PixFmtFromFF(param->desc.pixFmt);
		//	context->profile = FF_PROFILE_H264_HIGH_444;
		//}

		context->bit_rate = param->uBitrate;
		AVRational rate;
		rate.num = 1;
		rate.den = param->uFPS; /* REFERENCE TIME. */
		context->time_base = rate;
		context->gop_size = param->uKeyInter;
		context->max_b_frames = 0;
		context->pix_fmt = pixFmt;
		context->refs = 0;
		context->delay = 0;

		context->width = param->desc.w;
		context->height = param->desc.h;

		context->me_range = 1;
		context->max_qdiff = 4;
		context->qmin = 1;
		context->qmax = 51;
		context->qcompress = 0.6;

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;
		context->profile = FF_PROFILE_UNKNOWN;
		context->thread_count = param->uNumThreads;

		YXC_Status rc = FFSpecV<templateId>::_FFConfigV_SpecEnc(codecId, param, pSpecd, context);
		_YXC_CHECK_RC_RET(rc);

		int ret = avcodec_open2_locked(context, codec, NULL);
		_YXC_CHECK_FF_RET(ret);

		*ppContext = hRet.Detach();
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncV<templateId>::_FFConfigV_Dec(AVCodecID codecId, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd, AVCodecContext** ppContext)
	{
		AVCodec* codec = _FindCodecByVParam(codecId, FALSE, pSpecd);
		_YXC_CHECK_FF_PTR_RET(codec);

		AVCodecContext* context = avcodec_alloc_context3(codec);
		YXCLib::HandleRef<AVCodecContext*> hRet(context, _av_free_context);

		context->gop_size = param->uKeyInter;
		context->thread_count = param->uNumThreads;
		context->delay = 0;
		context->width = param->desc.w;
		context->height = param->desc.h;

		//context->pix_fmt = codec->pix_fmts[0];
		// context->refcounted_frames = TRUE;

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;

		YXV_FFVSpecParam aSpec = {0};
		const YXV_FFVSpecParam* paSpec = &aSpec;
		YXV_FFVSpecParamDefault(CodecToFFCodecV(codecId), FALSE, &aSpec);

		if (pSpecd)
		{
			paSpec = pSpecd;
		}

		YXC_Status rc = FFSpecV<templateId>::_FFConfigV_SpecDec(codecId, param, paSpec, context);
		_YXC_CHECK_RC_RET(rc);

		int ret = avcodec_open2_locked(context, codec, NULL);
		_YXC_CHECK_FF_RET(ret);

		*ppContext = hRet.Detach();
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncV<templateId>::_FFConfigV_Read(AVCodecID codecId, AVCodecContext* context, YXV_VParam* param, YXV_FFVFormatParam* pSpecd)
	{
		param->uBitrate = context->bit_rate; /* VBR. */
		param->desc.h = context->height;
		param->desc.w = context->width;
		param->desc.pixFmt = YXV_PIX_FMT_NONE;
		if (context->codec)
		{
			param->desc.pixFmt = PixFmtToFF(context->codec->pix_fmts[0]);
		}
		param->uKeyInter = context->gop_size;
		param->desc.uPixAlign = 0;
		param->uNumThreads = 0;

		YXV_FFVSpecParamDefault(CodecToFFCodecV(codecId), TRUE, &(*pSpecd)[0]);
		YXV_FFVSpecParamDefault(CodecToFFCodecV(codecId), FALSE, &(*pSpecd)[1]);

		YXC_Status rc = FFSpecV<templateId>::_FFConfigV_SpecRead(codecId, context, param, pSpecd);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncV<templateId>::_FFConfigV_Format(AVCodecID codecId, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd, AVCodecContext** ppContext)
	{
		AVCodecContext* context = *ppContext;

		context->width = param->desc.w;
		context->height = param->desc.h;
		context->codec_id = codecId;
		context->codec_type = AVMEDIA_TYPE_VIDEO;
		context->bit_rate = param->uBitrate;

		AVRational rate;
		rate.num = 1;
		rate.den = param->uFPS;
		context->time_base = rate;

		YXV_FFVSpecParam p2;
		YXV_FFVSpecParamDefault(CodecToFFCodecV(codecId), FALSE, &p2);
		if (pSpecd != NULL)
		{
			p2 = *pSpecd;
		}

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;
		if (p2.extData.uExt > 0)
		{
			_YXC_CHECK_RC_RET( _InitExtData(context, p2.extData.uExt, p2.extData.pExt) );
		}

		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFSpecA<templateId>::_FFConfigA_SpecEnc(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext* context)
	{
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFSpecA<templateId>::_FFConfigA_SpecDec(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFSpecA<templateId>::_FFConfigA_SpecRead(AVCodecID codecId, AVCodecContext* context, YXV_AParam* param, YXV_FFAFormatParam* pSpecd)
	{
		YXV_FFASpecParamDefault(CodecToFFCodecA(codecId), TRUE, &(*pSpecd)[0]);
		YXV_FFExtraData* pParam = &(*pSpecd)[1].extData;

		YXV_FFASpecParamDefault(CodecToFFCodecA(codecId), FALSE, &(*pSpecd)[1]);
		pParam->pExt = context->extradata;
		pParam->uExt = context->extradata_size;
		(*pSpecd)[0].uFrameSize = context->frame_size;
		(*pSpecd)[1].uFrameSize = context->frame_size;

		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncA<templateId>::_FFConfigA_Enc(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext** ppContext)
	{
		AVCodec* codec = _FindCodecByAParam(codecId, TRUE, pSpecd);
		_YXC_CHECK_FF_PTR_RET(codec);

		AVCodecContext* context = avcodec_alloc_context3(codec);
		YXCLib::HandleRef<AVCodecContext*> hRet(context, _av_free_context);

		yuint32_t uNumBits = 0;

		AVSampleFormat sampleFmt = codec->sample_fmts[0];

		context->sample_fmt = sampleFmt;
		context->channels = param->desc.uNumChannels;
		context->sample_rate = param->desc.uSampleRate;
		context->bit_rate = param->uBitrate;
		context->block_align = param->uBlockAlign;
		context->profile = FF_PROFILE_UNKNOWN;
		if (context->channels == 2)
		{
			context->channel_layout = AV_CH_LAYOUT_STEREO;
		}
		else
		{
			context->channel_layout = AV_CH_LAYOUT_MONO;
		}

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;

		YXC_Status rc = FFSpecA<templateId>::_FFConfigA_SpecEnc(codecId, param, pSpecd, context);
		_YXC_CHECK_RC_RET(rc);

		int ret = avcodec_open2_locked(context, codec, NULL);
		_YXC_CHECK_FF_RET(ret);

		if (context->frame_size == 0)
		{
			context->frame_size = param->desc.uSampleRate / 10; /* 10 packets in 1 second. */
		}

		AVRational rate;
		rate.den = 10000 * 1000;
		rate.num = 1;
		context->time_base = rate;

		*ppContext = hRet.Detach();
		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncA<templateId>::_FFConfigA_Dec(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext** ppContext)
	{
		AVCodec* codec = _FindCodecByAParam(codecId, FALSE, pSpecd);
		_YXC_CHECK_FF_PTR_RET(codec);

		AVCodecContext* context = avcodec_alloc_context3(codec);
		YXCLib::HandleRef<AVCodecContext*> hRet(context, _av_free_context);

		AVSampleFormat sampleFmt = AV_SAMPLE_FMT_NONE;
		//switch (param->desc.sampleFmt)
		//{
		//case YXV_SAMPLE_FMT_16S:
		//	sampleFmt = AV_SAMPLE_FMT_S16;
		//	break;
		//}

		//*context = gs_contextData;

		context->request_sample_fmt = sampleFmt;
		context->sample_rate = param->desc.uSampleRate;
		context->frame_size = 0;
		context->bit_rate = param->uBitrate;
		context->channels = param->desc.uNumChannels;
		context->block_align = param->uBlockAlign;

		// context->refcounted_frames = TRUE;
		YXV_FFASpecParam aSpec = {0};
		const YXV_FFASpecParam* paSpec = &aSpec;
		YXV_FFASpecParamDefault(CodecToFFCodecA(codecId), FALSE, &aSpec);

		if (pSpecd)
		{
			paSpec = pSpecd;
		}

		YXC_Status rc = FFSpecA<templateId>::_FFConfigA_SpecDec(codecId, param, paSpec, context);
		_YXC_CHECK_RC_RET(rc);

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;

		int ret = avcodec_open2_locked(context, codec, NULL);
		_YXC_CHECK_FF_RET(ret);

		context->channel_layout = context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
		*ppContext = hRet.Detach();
		return YXC_ERC_SUCCESS;
	}

	static AVCodecContext gs_contextData;

	template <AVCodecID templateId>
	YXC_Status FFFuncA<templateId>::_FFConfigA_Format(AVCodecID codecId, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd, AVCodecContext** ppContext)
	{
		AVCodecContext* context = *ppContext;

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;
		context->codec_id = codecId;
		context->codec_type = AVMEDIA_TYPE_AUDIO;

		context->sample_rate = param->desc.uSampleRate;
		context->frame_size = 0;

		AVRational rate;
		rate.num = 1;
		rate.den = 10000 * 1000;
		context->time_base = rate;
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );

		return YXC_ERC_SUCCESS;
	}

	template <AVCodecID templateId>
	YXC_Status FFFuncA<templateId>::_FFConfigA_Read(AVCodecID codecId, AVCodecContext* context, YXV_AParam* param, YXV_FFAFormatParam* pSpecd)
	{
		param->uBitrate = context->bit_rate;
		param->desc.uSampleRate = context->sample_rate;
		param->desc.uNumChannels = context->channels;
		param->uBlockAlign = context->block_align;
		param->desc.sampleFmt = YXV_SAMPLE_FMT_16S;

		gs_contextData = *context;

		YXC_Status rc = FFSpecA<templateId>::_FFConfigA_SpecRead(codecId, context, param, pSpecd);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}
}

namespace YXV_FFWrap
{
	_FFObject::_FFObject() : _vCodecId(YXV_VCODEC_ID_NONE), _aCodecId(YXV_ACODEC_ID_NONE)
	{
		this->_uRef = 1;
	}

	_FFObject::~_FFObject()
	{
		this->FFUnreference();
	}

	void _FFObject::FFUnreference()
	{
		yuint32_t uRemain = YXCLib::Interlocked::Decrement(&this->_uRef);
		if (uRemain == 0)
		{
			this->~_FFObject();
			free(this);
		}
	}

	void _FFObject::FFReference()
	{
		YXCLib::Interlocked::Increment(&this->_uRef);
	}

	YXC_Status _FFObject::GetFFReadProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFReadVProc* pVProc, _FFReadAProc* pAProc,
		_FFReadCvtProc* pCvtV, _FFReadCvtProc* pCvtA)
	{
		*pVProc = NULL;
		*pAProc = NULL;
		*pCvtV = NULL;
		*pCvtA = NULL;
#define SUPPORTA(_codec) case YXV_ACODEC_ID_##_codec: *pAProc = FFFuncA<AV_CODEC_ID_##_codec>::_FFConfigA_Read; break;
#define SUPPORTV(_codec) case YXV_VCODEC_ID_##_codec: *pVProc = FFFuncV<AV_CODEC_ID_##_codec>::_FFConfigV_Read; break;

		switch (vCodecID)
		{
		case YXV_VCODEC_ID_H264:
			*pVProc = FFFuncV<AV_CODEC_ID_H264>::_FFConfigV_Read;
			*pCvtV = _FFPacketReadCvtProcH264;
			break;
			SUPPORTV(VP8);
		default:
			*pVProc = FFFuncV<AV_CODEC_ID_NONE>::_FFConfigV_Read;
			break;
		}

		switch (aCodecID)
		{
			SUPPORTA(OPUS);
		default:
			*pAProc = FFFuncA<AV_CODEC_ID_NONE>::_FFConfigA_Read;
			break;
		}

#undef SUPPORTV
#undef SUPPORTA
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFObject::GetFFDecProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFConfigVProc* pVProc, _FFConfigAProc* pAProc)
	{
		*pVProc = NULL;
		*pAProc = NULL;

#define SUPPORTA(_codec) case YXV_ACODEC_ID_##_codec: *pAProc = FFFuncA<AV_CODEC_ID_##_codec>::_FFConfigA_Dec; break;
#define SUPPORTV(_codec) case YXV_VCODEC_ID_##_codec: *pVProc = FFFuncV<AV_CODEC_ID_##_codec>::_FFConfigV_Dec; break;

		switch (vCodecID)
		{
			SUPPORTV(H264);
			SUPPORTV(VP8);
		default:
			*pVProc = FFFuncV<AV_CODEC_ID_NONE>::_FFConfigV_Dec;
			break;
		}

		switch (aCodecID)
		{
			SUPPORTA(OPUS);
		default:
			*pAProc = FFFuncA<AV_CODEC_ID_NONE>::_FFConfigA_Dec;
		}

#undef SUPPORTV
#undef SUPPORTA
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFObject::GetFFFormatProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFConfigVProc* pVProc, _FFConfigAProc* pAProc,
		_FFWriteProc* pWriteV, _FFWriteProc* pWriteA)
	{
		*pVProc = NULL;
		*pAProc = NULL;
		//*pWriteV = av_interleaved_write_frame;
		//*pWriteA = av_interleaved_write_frame;
		*pWriteV = av_write_frame;
		*pWriteA = av_write_frame;
		switch (vCodecID)
		{
		case YXV_VCODEC_ID_H264:
			*pVProc = FFFuncV<AV_CODEC_ID_H264>::_FFConfigV_Format;
			*pWriteV = _FFWriteProcH264;
			break;
		default:
			*pVProc = FFFuncV<AV_CODEC_ID_NONE>::_FFConfigV_Format;
			break;
		}

		switch (aCodecID)
		{
		case YXV_ACODEC_ID_AAC:
			*pAProc = FFFuncA<AV_CODEC_ID_AAC>::_FFConfigA_Format;
			break;
		default:
			*pAProc = FFFuncA<AV_CODEC_ID_NONE>::_FFConfigA_Format;
			break;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFObject::GetFFEncProc(YXV_VCodecID vCodecID, YXV_ACodecID aCodecID, _FFConfigVProc* pVProc, _FFConfigAProc* pAProc)
	{
		*pVProc = NULL;
		*pAProc = NULL;
		switch (vCodecID)
		{
		case YXV_VCODEC_ID_H264:
			*pVProc = FFFuncV<AV_CODEC_ID_H264>::_FFConfigV_Enc;
			break;
		default:
			*pVProc = FFFuncV<AV_CODEC_ID_NONE>::_FFConfigV_Enc;
			break;
		}

		switch (aCodecID)
		{
		case YXV_ACODEC_ID_AAC:
			*pAProc = FFFuncA<AV_CODEC_ID_AAC>::_FFConfigA_Enc;
			break;
		default:
			*pAProc = FFFuncA<AV_CODEC_ID_NONE>::_FFConfigA_Enc;
			break;
		}

		return YXC_ERC_SUCCESS;
	}
}

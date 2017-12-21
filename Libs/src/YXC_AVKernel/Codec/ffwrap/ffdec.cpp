#include <YXC_AVKernel/Codec/ffwrap/ffdec.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

namespace
{
	static void _FreeCodecFrame(AVFrame* frame, AVCodecContext* pContext)
	{
		if (frame->data[0] != NULL)
		{
			//for (yuint32_t i = 0; i < AV_NUM_DATA_POINTERS; ++i)
			//{
			//	frame->data[i] = NULL;
			//}
			//pContext->release_buffer(pContext, frame);
		}
		YXV_FFWrap::avcodec_free_frame(&frame);
	}
}

namespace _YXV_AVUtils
{
	void _FillPixInfo(YXV_PixFmt pixFmt, yuint8_t (&ratioW)[YXV_MAX_NUM_PLANARS],
		yuint8_t (&ratioH)[YXV_MAX_NUM_PLANARS], yuint8_t (&pixelSize)[YXV_MAX_NUM_PLANARS]);
}

namespace YXV_FFWrap
{
	_FFDec::_FFDec() : _vctx(NULL), _actx(NULL), _vframe(NULL), _aframe_pcm(NULL), _swr(NULL), _sampleRate(0)
	{

	}

	_FFDec::~_FFDec()
	{
		if (this->_vctx)
		{
			avcodec_close_locked(this->_vctx);
			if (this->_vctx->extradata)
			{
				av_free(this->_vctx->extradata);
			}
			av_free(this->_vctx);
		}

		if (this->_actx)
		{
			avcodec_close_locked(this->_actx);
			if (this->_actx->extradata)
			{
				av_free(this->_actx->extradata);
			}
			av_free(this->_actx);
		}

		if (this->_vframe)
		{
			avcodec_free_frame(&this->_vframe);
		}

		if (this->_aframe_pcm)
		{
			/* Frame buffer is allocated by class itself. */
			avcodec_free_frame(&this->_aframe_pcm);
		}

		if (this->_swr)
		{
			swr_free(&this->_swr);
		}
	}

	YXC_Status _FFDec::FFInit(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId)
	{
		YXC_Status rc = GetFFDecProc(vCodecId, aCodecId, &this->_confVProc, &this->_confAProc);
		_YXC_CHECK_RC_RET(rc);

		if (vCodecId != YXV_VCODEC_ID_NONE)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_confVProc != NULL, YXC_ERC_NOT_SUPPORTED, L"Not supported video codec(%d)", vCodecId);
		}

		if (aCodecId != YXV_ACODEC_ID_NONE)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_confAProc != NULL, YXC_ERC_NOT_SUPPORTED, L"Not supported audio codec(%d)", aCodecId);
		}

		this->_vCodecId = vCodecId;
		this->_aCodecId = aCodecId;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFDec::ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec)
	{
		YXC_Status rc = this->_confVProc(CodecFromFFCodecV(this->_vCodecId), param, pSpec, &this->_vctx);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFDec::ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec)
	{
		YXC_Status rc = this->_confAProc(CodecFromFFCodecA(this->_aCodecId), param, pSpec, &this->_actx);
		_YXC_CHECK_RC_RET(rc);

		this->_param_channels = param->desc.uNumChannels;
		yuint32_t uLayout = param->desc.uNumChannels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;

		this->_sampleRate = param->desc.uSampleRate;
		this->_swr = swr_alloc_set_opts(NULL, uLayout, AV_SAMPLE_FMT_S16, param->desc.uSampleRate,
			this->_actx->channel_layout, this->_actx->sample_fmt, this->_actx->sample_rate, 0, NULL);
		_YXC_CHECK_FF_PTR_RET(this->_swr);

		_YXC_CHECK_FF_RET( swr_init(this->_swr) );

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFDec::ProcessA(const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, YXV_Frame* pSample)
	{
		AVPacket packet = {0};
		packet.data = (ybyte_t*)pbyData;
		packet.size = uCbData;
		packet.pts = uRefTime;
		packet.flags = AV_PKT_FLAG_KEY;
		pSample->buffer = NULL;

		AVFrame* frame = av_frame_alloc();

		_YXC_CHECK_FF_PTR_RET(frame);
		YXCLib::HandleRefEx<AVFrame*, AVCodecContext*> frameRes(frame, this->_actx, _FreeCodecFrame);

		int got_sample_ptr;
		int ffret = avcodec_decode_audio4(this->_actx, frame, &got_sample_ptr, &packet);
		_YXC_CHECK_FF_RET(ffret);

		if (got_sample_ptr && frame->nb_samples > 0)
		{
			if (this->_aframe_pcm)
			{
				_FreeCodecFrame(this->_aframe_pcm, this->_actx);
			}

			this->_aframe_pcm = av_frame_alloc();
			_YXC_CHECK_FF_PTR_RET(this->_aframe_pcm);

			this->_aframe_pcm->channel_layout = this->_param_channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
			this->_aframe_pcm->nb_samples = this->_actx->frame_size * this->_sampleRate / this->_actx->sample_rate;
			if (this->_aframe_pcm->nb_samples == 0) this->_aframe_pcm->nb_samples = frame->nb_samples;
			this->_aframe_pcm->format = AV_SAMPLE_FMT_S16;
			ysize_t ablock_pcm = av_samples_get_buffer_size(NULL, this->_param_channels, this->_aframe_pcm->nb_samples, AV_SAMPLE_FMT_S16, 0);

			AVBufferRef* buf_ref = av_buffer_alloc(ablock_pcm);
			_YXC_CHECK_FF_PTR_RET(buf_ref);

			YXCLib::HandleRef<AVBufferRef**> buf_res(&buf_ref, av_buffer_unref);

			_YXC_CHECK_FF_RET( avcodec_fill_audio_frame(this->_aframe_pcm, this->_param_channels, AV_SAMPLE_FMT_S16, buf_ref->data,
				ablock_pcm, 0) );

			ffret = swr_convert(this->_swr, this->_aframe_pcm->data, this->_aframe_pcm->nb_samples,
				(const uint8_t**)frame->data, frame->nb_samples);
			_YXC_CHECK_FF_RET(ffret);

			for (yuint32_t i = 0; i < 4; ++i)
			{
				pSample->pData[i] = this->_aframe_pcm->data[i];
				pSample->uSize[i] = this->_aframe_pcm->linesize[i];
			}

			pSample->uSize[0] = this->_aframe_pcm->nb_samples * 2 * this->_param_channels;
			pSample->uRefTime = frame->pkt_pts;
			pSample->uNumSamples = ffret;
			pSample->uNumPlanars = 1;
			pSample->fType = YXV_FRAME_TYPE_A;

			YXC_Status rc = YXV_BufferAlloc(YXV_BUFFER_TYPE_FF_PACKET, buf_ref, 0, &pSample->buffer);
			_YXC_CHECK_RC_RET(rc);

			buf_res.Detach();

			pSample->u1.sampleDesc.uNumChannels = this->_param_channels;
			pSample->u1.sampleDesc.sampleFmt = YXV_SAMPLE_FMT_16S;
			pSample->u1.sampleDesc.uSampleRate = this->_sampleRate;

			return YXC_ERC_SUCCESS;
		}

		pSample->uNumSamples = 0;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFDec::ProcessV2(const YXV_Packet* pkt, YXV_Frame* pSample)
	{
		AVPacket packet = {0};
		packet.data = pkt->pData;
		packet.size = pkt->uDataLen;
		packet.pts = pkt->uRefTime;
		packet.dts = pkt->uDecTime;
		if (pkt->bKeyPacket)
		{
			packet.flags |= AV_PKT_FLAG_KEY;
		}

		if (this->_vframe)
		{
			_FreeCodecFrame(this->_vframe, this->_vctx);
			this->_vframe = NULL;
		}

		this->_vframe = av_frame_alloc();
		_YXC_CHECK_FF_PTR_RET(this->_vframe);

		int get_picture_ptr = 0;
		int ffret = avcodec_decode_video2(this->_vctx, this->_vframe, &get_picture_ptr, &packet);
		_YXC_CHECK_FF_RET(ffret);

		if (get_picture_ptr)
		{
			AVFrame* new_frame = av_frame_clone(this->_vframe);
			_YXC_CHECK_FF_PTR_RET(new_frame);

			yuint32_t uNumPlanars = 0;
			for (yuint32_t i = 0; i < YXV_MAX_NUM_PLANARS; ++i)
			{
				pSample->pData[i] = new_frame->data[i];
				pSample->uSize[i] = new_frame->linesize[i];
				if (pSample->uSize[i]) ++uNumPlanars;
			}

			pSample->uRefTime = this->_vframe->pkt_pts;
			pSample->uNumPlanars = uNumPlanars;
			pSample->bGotFrame = TRUE;

			YXCLib::HandleRef<AVFrame**> frame_res(&new_frame, av_frame_free);

			YXC_Status rc = YXV_BufferAlloc(YXV_BUFFER_TYPE_FF_FRAME, new_frame, 0, &pSample->buffer);
			_YXC_CHECK_RC_RET(rc);

			frame_res.Detach();

			pSample->u1.picDesc.w = this->_vctx->width;
			pSample->u1.picDesc.h = this->_vctx->height;
			pSample->u1.picDesc.pixFmt = PixFmtToFF(this->_vctx->pix_fmt);
			_YXV_AVUtils::_FillPixInfo(pSample->u1.picDesc.pixFmt, pSample->uRatioW, pSample->uRatioH, pSample->uPixSize);

			switch (this->_vframe->pict_type)
			{
			case AV_PICTURE_TYPE_P:
				pSample->fType = YXV_FRAME_TYPE_P;
				break;
			case AV_PICTURE_TYPE_B:
				pSample->fType = YXV_FRAME_TYPE_B;
				break;
			case AV_PICTURE_TYPE_I:
				pSample->fType = YXV_FRAME_TYPE_I;
				break;
			default:
				pSample->fType = YXV_FRAME_TYPE_NONE;
				break;
			}
			return YXC_ERC_SUCCESS;
		}

		pSample->bGotFrame = FALSE;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFDec::ProcessV(const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Frame* pSample)
	{
		YXV_Packet pkt = { 0 };
		pkt.pData = (ybyte_t*)pbyData;
		pkt.uDataLen = uCbData;
		pkt.bKeyPacket = fType == YXV_FRAME_TYPE_I ? TRUE : FALSE;
		pkt.uRefTime = uRefTime;
		pkt.uDecTime = uRefTime;

		return this->ProcessV2(&pkt, pSample);
	}
}

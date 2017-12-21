#include <YXC_AVKernel/Codec/ffwrap/ffenc.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/Utils/_YXV_Common.hpp>

//namespace
//{
//	static void _FillStrideRows(uint8_t (&rows)[AV_NUM_DATA_POINTERS], uint8_t* pMaxStride, AVPixelFormat pixFmt)
//	{
//		if (pixFmt == AV_PIX_FMT_YUV420P)
//		{
//			rows[0] = 1;
//			rows[1] = 2;
//			rows[2] = 2;
//			*pMaxStride = 2;
//		}
//		else if (pixFmt == AV_PIX_FMT_BGR24)
//		{
//			rows[0] = 1;
//			*pMaxStride = 1;
//		}
//		else
//		{
//			rows[0] = 1;
//			*pMaxStride = 1;
//		}
//	}
//}

namespace YXV_FFWrap
{
	YXC_Status _FFEnc::_ProcessVFrame(AVFrame* srcFrame, YXV_Packet* pPacket)
	{
		AVFrame* frameUsed = srcFrame;

		if (this->_inPixFmt != this->_vctx->pix_fmt)
		{
			int ret = sws_scale(this->_sws, srcFrame->data, srcFrame->linesize, 0, this->_vctx->height,
				this->_vframe->data, this->_vframe->linesize);

			frameUsed = this->_vframe;
			this->_vframe->pts = srcFrame->pts;
		}

		ybool_t repeat_headers = (this->_vctx->flags & CODEC_FLAG_GLOBAL_HEADER) == 0;
		ybool_t new_data = pPacket->buffer == NULL;

		AVPacket packet = {0};
		if (!new_data)
		{
			packet.data = (uint8_t*)pPacket->buffer->p;
			packet.size = pPacket->buffer->sz;

			if (repeat_headers)
			{
				packet.size -= this->_vctx->extradata_size;
			}
		}

		frameUsed->width = this->_vctx->width;
		frameUsed->height = this->_vctx->height;
		frameUsed->format = this->_vctx->pix_fmt;

		int got_packet_ptr = 0;
		int ffret = avcodec_encode_video2(this->_vctx, &packet, frameUsed, &got_packet_ptr);
		_YXC_CHECK_FF_RET(ffret);

		if (got_packet_ptr)
		{
			if (repeat_headers && (packet.flags & AV_PKT_FLAG_KEY)) /* Repeat headers on I frame. */
			{
				if (new_data)
				{
					AVBufferRef* ref = NULL;
					ffret = av_buffer_realloc(&packet.buf, packet.size + this->_vctx->extradata_size);
					if (ffret < 0)
					{
						av_buffer_unref(&packet.buf);
						_YXC_CHECK_FF_RET(ffret);
					}

					pPacket->pData = packet.buf->data;
					memcpy(pPacket->pData + this->_vctx->extradata_size, packet.data, packet.size);

					av_buffer_unref(&packet.buf);
					packet.buf = ref;
				}
				else
				{
					memmove(pPacket->pData + this->_vctx->extradata_size, pPacket->pData, packet.size);
				}

				memcpy(pPacket->pData, this->_vctx->extradata, this->_vctx->extradata_size);
				packet.size += this->_vctx->extradata_size;
			}

			if (new_data)
			{
				YXCLib::HandleRef<AVBufferRef**> hAVBuffer(&packet.buf, av_buffer_unref);

				YXC_Status rc = YXV_BufferAlloc(YXV_BUFFER_TYPE_FF_PACKET, packet.buf, 0, &pPacket->buffer);
				_YXC_CHECK_RC_RET(rc);

				hAVBuffer.Detach();
			}
		}

		pPacket->pData = packet.data;
		pPacket->bKeyPacket = (packet.flags & AV_PKT_FLAG_KEY) ? TRUE : FALSE;
		pPacket->uDataLen = packet.size;
		pPacket->uRefTime = packet.pts;
		pPacket->uDecTime = packet.dts;
		return YXC_ERC_SUCCESS;
	}

	_FFEnc::_FFEnc() : _vctx(NULL), _actx(NULL), _vframe(NULL), _aframe(NULL), _aCvtFrame(NULL), _pVBuffer(NULL), _pABuffer(NULL), _pACvtBuffer(NULL),
		_sws(NULL), _swr(NULL), _ablock(0), _acursize(0), _bFirstAEnc(TRUE)
	{
		this->_uPixAlign = 0;
		this->_inPixFmt = AV_PIX_FMT_NONE;
	}

	_FFEnc::~_FFEnc()
	{
		if (this->_sws)
		{
			sws_freeContext(this->_sws);
			this->_sws = NULL;
		}

		if (this->_swr)
		{
			swr_free(&this->_swr);
		}

		if (this->_vctx)
		{
			avcodec_close_locked(this->_vctx);
			av_free(this->_vctx);
		}

		if (this->_actx)
		{
			avcodec_close_locked(this->_actx);
			av_free(this->_actx);
		}

		if (this->_vframe)
		{
			avcodec_free_frame(&this->_vframe);
		}

		if (this->_aframe)
		{
			avcodec_free_frame(&this->_aframe);
		}

		if (this->_aCvtFrame)
		{
			avcodec_free_frame(&this->_aCvtFrame);
		}

		if (this->_pABuffer)
		{
			av_free(this->_pABuffer);
			this->_pABuffer = NULL;
		}

		if (this->_pACvtBuffer)
		{
			av_free(this->_pACvtBuffer);
			this->_pACvtBuffer = NULL;
		}

		if (this->_pVBuffer)
		{
			av_free(this->_pVBuffer);
			this->_pVBuffer = NULL;
		}
	}

	YXC_Status _FFEnc::FFInit(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId)
	{
		YXC_Status rc = GetFFEncProc(vCodecId, aCodecId, &this->_confVProc, &this->_confAProc);
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

	YXC_Status _FFEnc::ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec)
	{
		YXC_Status rc = this->_confVProc(CodecFromFFCodecV(this->_vCodecId), param, pSpec, &this->_vctx);
		_YXC_CHECK_RC_RET(rc);

		this->_vframe = av_frame_alloc();
		_YXC_CHECK_REPORT_RET(this->_vframe != NULL, YXC_ERC_OUT_OF_MEMORY, L"Failed to alloc vcodec frame.");

		int nBuf = avpicture_get_size(this->_vctx->pix_fmt, param->desc.w, param->desc.h);

		this->_pVBuffer = (ybyte_t*)av_malloc(nBuf);
		_YXC_CHECK_REPORT_RET(this->_pVBuffer != NULL, YXC_ERC_OUT_OF_MEMORY, L"Failed to alloc vraw buffer.");

		_YXC_CHECK_FF_RET( avpicture_fill((AVPicture*)this->_vframe, this->_pVBuffer, this->_vctx->pix_fmt, param->desc.w, param->desc.h) );

		this->_inPixFmt = PixFmtFromFF(param->desc.pixFmt);
		this->_sws = sws_getContext(param->desc.w, param->desc.h, this->_inPixFmt, param->desc.w, param->desc.h,
			this->_vctx->pix_fmt, SWS_POINT, NULL, NULL, NULL);
		_YXC_CHECK_FF_PTR_RET(this->_sws);

		this->_uPixAlign = param->desc.uPixAlign;
		if (param->desc.uPixAlign == 0)
		{
			this->_uPixAlign = GetImageRowAlign(this->_inPixFmt);
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFEnc::ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec)
	{
		YXC_Status rc = this->_confAProc(CodecFromFFCodecA(this->_aCodecId), param, pSpec, &this->_actx);
		_YXC_CHECK_RC_RET(rc);

		this->_aframe = av_frame_alloc();
		_YXC_CHECK_FF_PTR_RET(this->_aframe);

		yuint32_t uAlign = param->desc.uNumChannels * this->_actx->frame_bits;
		this->_bFirstAEnc = TRUE;

		AVSampleFormat sampleFmt2 = SampleFmtFromFF(param->desc.sampleFmt);

		this->_ablock = av_samples_get_buffer_size(NULL, param->desc.uNumChannels, this->_actx->frame_size, sampleFmt2, uAlign);
		this->_aframe->channel_layout = this->_actx->channel_layout;
		this->_aframe->nb_samples = this->_actx->frame_size;
		this->_aframe->format = sampleFmt2;
		this->_aframe->pts = AV_NOPTS_VALUE;

		this->_pABuffer = (ybyte_t*)av_malloc(this->_ablock);
		_YXC_CHECK_FF_PTR_RET(this->_pABuffer);

		_YXC_CHECK_FF_RET( avcodec_fill_audio_frame(this->_aframe, param->desc.uNumChannels, sampleFmt2, this->_pABuffer,
			this->_ablock, uAlign) );

		if (sampleFmt2 != this->_actx->sample_fmt)
		{
			this->_swr = swr_alloc_set_opts(NULL, this->_actx->channel_layout, this->_actx->sample_fmt, this->_actx->sample_rate,
				this->_actx->channel_layout, sampleFmt2, param->desc.uSampleRate, 0, NULL);
			_YXC_CHECK_FF_PTR_RET(this->_swr);
			_YXC_CHECK_FF_RET( swr_init(this->_swr) );

			this->_aCvtFrame = av_frame_alloc();
			_YXC_CHECK_FF_PTR_RET(this->_aCvtFrame);

			int cvt_buf = av_samples_get_buffer_size(NULL, param->desc.uNumChannels, this->_actx->frame_size, this->_actx->sample_fmt, uAlign);
			this->_aCvtFrame->channel_layout = this->_actx->channel_layout;
			this->_aCvtFrame->nb_samples = this->_actx->frame_size;
			this->_aCvtFrame->format = this->_actx->sample_fmt;
			this->_aCvtFrame->pts = AV_NOPTS_VALUE;

			this->_pACvtBuffer = (ybyte_t*)av_malloc(cvt_buf);
			_YXC_CHECK_FF_PTR_RET(this->_pABuffer);
			_YXC_CHECK_FF_RET( avcodec_fill_audio_frame(this->_aCvtFrame, param->desc.uNumChannels, this->_actx->sample_fmt, this->_pACvtBuffer,
				cvt_buf, uAlign) );
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFEnc::ReadExtraDataV(ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData)
	{
		ReadCodecExt(this->_vctx, pBuf, uCbBuf, puCbData);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFEnc::ReadExtraDataA(ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData)
	{
		ReadCodecExt(this->_actx, pBuf, uCbBuf, puCbData);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFEnc::ReadSpecA(YXV_FFASpecParam* paSpec)
	{
		switch (this->_aCodecId)
		{
		case YXV_ACODEC_ID_AAC:
		case YXV_ACODEC_ID_WMA2:
		case YXV_ACODEC_ID_MP3:
		case YXV_ACODEC_ID_G722:
		case YXV_ACODEC_ID_VORBIS:
		case YXV_ACODEC_ID_OPUS:
		default:
			strcpy(paSpec->szEncDecName, "");
			paSpec->extData.pExt = this->_actx->extradata;
			paSpec->extData.uExt = this->_actx->extradata_size;
			paSpec->extData.uExtBuf = 0;
			paSpec->uFrameSize = this->_actx->frame_size;
			break;
		//default:
		//	_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, L"Invalid codec (%d), not supported", this->_aCodecId);
		//	break;
		}

		return YXC_ERC_SUCCESS;

	}

	YXC_Status _FFEnc::ReadSpecV(YXV_FFVSpecParam* pvSpec)
	{
		switch (this->_vCodecId)
		{
		case YXV_VCODEC_ID_H264:
		case YXV_VCODEC_ID_VP8:
		case YXV_VCODEC_ID_WMV3:
		case YXV_VCODEC_ID_EQV1:
		default:
			strcpy(pvSpec->szEncDecName, "");
			pvSpec->extData.pExt = this->_vctx->extradata;
			pvSpec->extData.uExt = this->_vctx->extradata_size;
			pvSpec->extData.uExtBuf = 0;
			break;
		//default:
		//	_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, L"Invalid codec (%d), not supported", this->_vCodecId);
		//	break;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFEnc::ProcessA2(const YXV_Frame* pSample, yuint32_t* puConverted, YXV_Packet* pPacket)
	{
		yuint32_t uSampleBits = _YXV_AVUtils::_GetSampleBits(pSample->u1.sampleDesc.sampleFmt);
		yuint32_t uRealDataSize = pSample->uNumSamples * uSampleBits / 8 * pSample->u1.sampleDesc.uNumChannels;
		return ProcessA(pSample->pData[0], uRealDataSize, pSample->uRefTime, puConverted, pPacket);
	}

	YXC_Status _FFEnc::ProcessA(const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, yuint32_t* puConverted, YXV_Packet* pPacket)
	{
		ysize_t aold_cur = this->_acursize;
		if (pbyData != NULL)
		{
			yuint32_t max_copy = min(this->_ablock - this->_acursize, uCbData);

			memcpy(this->_pABuffer + this->_acursize, pbyData, max_copy);
			this->_acursize += max_copy;
			pbyData += max_copy;
			uCbData -= max_copy;

			*puConverted = max_copy;
		}

		if (this->_acursize == this->_ablock || pbyData == NULL)
		{
			int nb_samples = this->_acursize / 2 / this->_actx->channels;
			this->_acursize = 0;

			if (aold_cur != 0) /* offset reference time to older times. */
			{
				yint64_t timeDiff = (yint64_t)(aold_cur / this->_actx->channels / 2) * 10000 * 1000 / this->_actx->sample_rate;
				uRefTime -= timeDiff;
			}

			AVPacket packet = {0};

			if (pPacket->buffer)
			{
				packet.data = (uint8_t*)pPacket->buffer->p;
				packet.size = pPacket->buffer->sz;
			}

			AVFrame* pOutFrame = this->_aframe;
			if (this->_swr)
			{
				pOutFrame = this->_aCvtFrame;
				int ffret2 = swr_convert(this->_swr, this->_aCvtFrame->data, nb_samples, (const uint8_t**)this->_aframe->data, nb_samples);
				_YXC_CHECK_FF_RET(ffret2);
			}

			pOutFrame->pts = uRefTime;
			pOutFrame->nb_samples = nb_samples;
			int got_packet_ptr = 0;
			int ffret = avcodec_encode_audio2(this->_actx, &packet, pOutFrame, &got_packet_ptr);
			_YXC_CHECK_FF_RET(ffret);

			if (this->_bFirstAEnc) /* deal with aac delay. */
			{
				yuint32_t uNumLoops = 0;
				while (!got_packet_ptr && uNumLoops < 4)
				{
					ffret = avcodec_encode_audio2(this->_actx, &packet, NULL, &got_packet_ptr);
					_YXC_CHECK_FF_RET(ffret);

					++uNumLoops;
				}
				this->_bFirstAEnc = FALSE;
			}

			if (got_packet_ptr)
			{
				if (!pPacket->buffer)
				{
					YXCLib::HandleRef<AVBufferRef**> hAVBuffer(&packet.buf, av_buffer_unref);

					YXC_Status rc = YXV_BufferAlloc(YXV_BUFFER_TYPE_FF_PACKET, packet.buf, 0, &pPacket->buffer);
					_YXC_CHECK_RC_RET(rc);

					hAVBuffer.Detach();
				}
				pPacket->pData = packet.data;
				pPacket->uDataLen = packet.size;
				pPacket->uRefTime = packet.pts;
				pPacket->bKeyPacket = TRUE;
				return YXC_ERC_SUCCESS;
			}
		}

		pPacket->uDataLen = 0;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFEnc::ProcessV2(const YXV_Frame* pSample, YXV_Packet* pPacket)
	{
		AVFrame* srcFrame = av_frame_alloc();
		_YXC_CHECK_FF_PTR_RET(srcFrame);

		srcFrame->pts = pSample->uRefTime;
		srcFrame->pkt_pts = pSample->uRefTime;
		srcFrame->pkt_dts = AV_NOPTS_VALUE;
		YXCLib::HandleRef<AVFrame**> frameRes(&srcFrame, avcodec_free_frame);

		for (int i = 0; i < 4; ++i)
		{
			srcFrame->data[i] = pSample->pData[i];
			srcFrame->linesize[i] = pSample->uSize[i];
		}

		if (pSample->fType == YXV_FRAME_TYPE_I)
		{
			srcFrame->pict_type = AV_PICTURE_TYPE_I;
		}
		else if (pSample->fType == YXV_FRAME_TYPE_P)
		{
			srcFrame->pict_type = AV_PICTURE_TYPE_P;
		}

		YXC_Status rc = this->_ProcessVFrame(srcFrame, pPacket);
		return rc;
	}

	YXC_Status _FFEnc::ProcessV(const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Packet* pPacket)
	{
		AVFrame* srcFrame = av_frame_alloc();
		_YXC_CHECK_FF_PTR_RET(srcFrame);

		YXCLib::HandleRef<AVFrame**> frameRes(&srcFrame, avcodec_free_frame);

		srcFrame->pts = uRefTime;
		int ffret = av_image_fill_arrays(srcFrame->data, srcFrame->linesize, pbyData, this->_inPixFmt, this->_vctx->width, this->_vctx->height,
			this->_uPixAlign);
		_YXC_CHECK_FF_RET(ffret);

		if (this->_inPixFmt == AV_PIX_FMT_BGR24)
		{
			srcFrame->data[0] = srcFrame->data[0] + srcFrame->linesize[0] * (this->_vctx->height - 1);
			srcFrame->linesize[0] = -srcFrame->linesize[0];
		}

		if (fType == YXV_FRAME_TYPE_I)
		{
			srcFrame->pict_type = AV_PICTURE_TYPE_I;
		}
		else if (fType == YXV_FRAME_TYPE_P)
		{
			srcFrame->pict_type = AV_PICTURE_TYPE_P;
		}

		YXC_Status rc = this->_ProcessVFrame(srcFrame, pPacket);
		return rc;
	}
}

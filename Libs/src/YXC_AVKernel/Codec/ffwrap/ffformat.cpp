#include <YXC_AVKernel/Codec/ffwrap/ffformat.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_FilePath.h>
#include <YXC_Sys/YXC_NetMarshal.h>
#include <YXC_AVKernel/YXV_H264.h>

namespace YXV_FFWrap
{
	_FFFormat::_FFFormat() : _video_st(NULL), _audio_st(NULL), _ctx(NULL)
	{
		this->_base_time_v = AV_NOPTS_VALUE;
		this->_base_time_a = AV_NOPTS_VALUE;

		this->_confAProc = NULL;
		this->_confVProc = NULL;
		this->_readAProc = NULL;
		this->_readVProc = NULL;
		this->_readCvtProcA = NULL;
		this->_readCvtProcV = NULL;
		this->_writeProcA = NULL;
		this->_writeProcV = NULL;
	}

	_FFFormat::~_FFFormat()
	{
		if (this->_ctx)
		{
			if (this->_ctx->pb)
			{
				avio_close(this->_ctx->pb);
			}
			avformat_free_context(this->_ctx);
			this->_ctx = NULL;
		}

		this->_audio_st = NULL;
		this->_video_st = NULL;
	}

	YXC_Status _FFFormat::FFInit(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFormatID fmtId)
	{
		this->_ctx = avformat_alloc_context();
		_YXC_CHECK_FF_PTR_RET(this->_ctx);

		AVOutputFormat* oformat = NULL;

		switch (fmtId)
		{
		case YXV_FILE_FORMAT_ID_MP4:
			oformat = av_guess_format("mp4", NULL, NULL);
			break;
		case YXV_FILE_FORMAT_ID_FLV:
			oformat = av_guess_format("flv", NULL, NULL);
			break;
		case YXV_FILE_FORMAT_ID_MKV:
			oformat = av_guess_format("matroska", NULL, NULL);
			break;
		case YXV_FILE_FORMAT_ID_RTP:
			oformat = av_guess_format("rtp", NULL, NULL);
			break;
		case YXV_FILE_FORMAT_ID_ASF:
			oformat = av_guess_format("asf", NULL, NULL);
			break;
		case YXV_FILE_FORMAT_ID_QVGA:
			oformat = av_guess_format("qvga", NULL, NULL);
			break;
		case YXV_FILE_FORMAT_ID_H264:
			oformat = av_guess_format("h264", NULL, NULL);
			break;		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_FILE_FORMAT, L"File format (%d) not supported", fmtId);
		}

		this->_ctx->oformat = oformat;

		YXC_Status rc = GetFFFormatProc(vCodecId, aCodecId, &this->_confVProc, &this->_confAProc, &this->_writeProcV, &this->_writeProcA);
		_YXC_CHECK_RC_RET(rc);

		if (vCodecId != YXV_VCODEC_ID_NONE)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_confVProc != NULL, YXC_ERC_NOT_SUPPORTED, L"Not supported video codec(%d)", vCodecId);

			this->_video_st = avformat_new_stream(this->_ctx, NULL);
			this->_video_st->id = 0;
			_YXC_CHECK_FF_PTR_RET(this->_video_st);
		}

		if (aCodecId != YXV_ACODEC_ID_NONE)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_confAProc != NULL, YXC_ERC_NOT_SUPPORTED, L"Not supported audio codec(%d)", aCodecId);

			this->_audio_st = avformat_new_stream(this->_ctx, NULL);
			this->_audio_st->id = 1;
			_YXC_CHECK_FF_PTR_RET(this->_audio_st);
		}

		this->_aCodecId = aCodecId;
		this->_vCodecId = vCodecId;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::StartWrite(const char* filename)
	{
		wchar_t szName[YXC_MAX_CCH_PATH];

		strncpy(this->_ctx->filename, filename, YXC_MAX_CCH_PATH);
		this->_ctx->filename[YXC_MAX_CCH_PATH - 1] = 0;

#if YXC_IS_64BIT
		_YXC_CHECK_FF_RET( avio_open(&this->_ctx->pb, this->_ctx->filename, AVIO_FLAG_WRITE) );
#else
		//_YXC_CHECK_FF_RET( avio_open_eh(&this->_ctx->pb, this->_ctx->filename, AVIO_FLAG_WRITE, 6 << 20, NULL, NULL) );
		_YXC_CHECK_FF_RET( avio_open(&this->_ctx->pb, this->_ctx->filename, AVIO_FLAG_WRITE) );
#endif /* YXC_IS_64BIT */
		_YXC_CHECK_FF_RET( avformat_write_header(this->_ctx, NULL) );

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::FFInit(const char* filename, YXV_VCodecID* pvCodecId, YXV_ACodecID* paCodecId, YXV_FFormatID* pFmtId)
	{
		AVDictionary* dic = NULL;
		av_dict_set(&dic, "stimeout", "5000", 0);
		av_dict_set(&dic, "rtsp_transport", "tcp", 0);
		int ret = avformat_open_input(&this->_ctx, filename, NULL, &dic);
		av_dict_free(&dic);

		_YXC_CHECK_FF_RET(ret);

		YXV_VCodecID vCodecId = YXV_VCODEC_ID_NONE;
		YXV_ACodecID aCodecId = YXV_ACODEC_ID_NONE;
		YXV_FFormatID ffId = YXV_FILE_FORMAT_ID_NONE;

		if (strstr(this->_ctx->iformat->name, "mp4") != NULL)
		{
			ffId = YXV_FILE_FORMAT_ID_MP4;
		}
		else if (strcmp(this->_ctx->iformat->name, "flv") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_FLV;
		}
		else if (strcmp(this->_ctx->iformat->name, "asf") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_ASF;
		}
		else if (strcmp(this->_ctx->iformat->name, "qvga") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_QVGA;
		}
		else if (strcmp(this->_ctx->iformat->name, "matroska") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_MKV;
		}
		else if (strcmp(this->_ctx->iformat->name, "rtp") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_RTP;
		}
		else if (strcmp(this->_ctx->iformat->name, "rtsp") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_RTSP;
		}
		else if (strcmp(this->_ctx->iformat->name, "rm") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_RMVB;
		}
		else if (strcmp(this->_ctx->iformat->name, "image2") == 0
			|| strcmp(this->_ctx->iformat->name, "bmp_pipe") == 0 || strcmp(this->_ctx->iformat->name, "png_pipe") == 0)
		{
			ffId = YXV_FILE_FORMAT_ID_IMAGE2;
		}

		//ebyte_t codes[] = {0, 0, 0, 1};
		//ebyte_t* pSps, *pPps;
		//ULONG uSps, uPps;
		//ebool_t bOldSps = FALSE;
		//AVCodecContext* context = this->_ctx->streams[0]->codec;
		//if (memcmp(context->extradata, codes, sizeof(codes)) == 0)
		//{
		//	ParseH264Ext(context->extradata, context->extradata_size, &pSps, &pPps, &uSps, &uPps);
		//}
		//else
		//{
		//	uSps = YXC_NetUnmarshalUInt16(*(yuint16_t*)(context->extradata + 6));
		//	uPps = YXC_NetUnmarshalUInt16(*(yuint16_t*)(context->extradata + uSps + 9));
		//	pSps = context->extradata + 8;
		//	bOldSps = TRUE;
		//}

		//YXC_SPS sps;
		//DecodeH264SPS(pSps, uSps, &sps);

		//context->width = sps.mb_real_width;
		//context->height = sps.mb_real_height;

		ret = avformat_find_stream_info(this->_ctx, NULL);
		_YXC_CHECK_FF_RET(ret);

		for (yuint32_t i = 0; i < this->_ctx->nb_streams; ++i)
		{
			if (this->_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				if (this->_video_st == NULL)
				{
					this->_video_st = this->_ctx->streams[i];
					vCodecId = CodecToFFCodecV(this->_video_st->codec->codec_id);
				}
			}

			if (this->_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				if (this->_audio_st == NULL)
				{
					this->_audio_st = this->_ctx->streams[i];
					aCodecId = CodecToFFCodecA(this->_audio_st->codec->codec_id);
				}
			}
		}

		YXC_Status rc = GetFFReadProc(vCodecId, aCodecId, &this->_readVProc, &this->_readAProc, &this->_readCvtProcV, &this->_readCvtProcA);
		_YXC_CHECK_RC_RET(rc);

		if (vCodecId != YXV_VCODEC_ID_NONE)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_readVProc != NULL, YXC_ERC_NOT_SUPPORTED, L"Not supported video codec(%d)", vCodecId);
		}

		if (aCodecId != YXV_ACODEC_ID_NONE)
		{
			_YXC_CHECK_REPORT_NEW_RET(this->_readAProc != NULL, YXC_ERC_NOT_SUPPORTED, L"Not supported audio codec(%d)", vCodecId);
		}

		this->_vCodecId = vCodecId;
		this->_aCodecId = aCodecId;

		*paCodecId = aCodecId;
		*pvCodecId = vCodecId;
		*pFmtId = ffId;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::ReadStreamInfo(YXV_StreamInfo* streamInfo)
	{
		double dVal = (double)this->_ctx->duration / AV_TIME_BASE;

		streamInfo->uDuration = (yint64_t)dVal * YXV_REFTIME_PER_SEC;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::ReadParamV(YXV_VParam* param, YXV_FFVFormatParam* pSpec)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_video_st != NULL, YXC_ERC_KEY_NOT_FOUND, L"No video stream found");

		YXC_Status rc = this->_readVProc(CodecFromFFCodecV(this->_vCodecId), this->_video_st->codec, param, pSpec);
		_YXC_CHECK_RC_RET(rc);

		param->uFPS = YXC_Round((double)this->_video_st->r_frame_rate.num / this->_video_st->r_frame_rate.den);
		if (param->uFPS == 0) /* guess fps to 25. */
		{
			param->uFPS = 25;
		}

		if (param->uFPS > 60) /* Max support at 60 fps. */
		{
			param->uFPS = 60;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::ReadParamA(YXV_AParam* param, YXV_FFAFormatParam* pSpec)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_audio_st != NULL, YXC_ERC_KEY_NOT_FOUND, L"No audio stream found");

		return this->_readAProc(CodecFromFFCodecA(this->_aCodecId), this->_audio_st->codec, param, pSpec);
	}

	YXC_Status _FFFormat::Read(YXV_Packet* packet)
	{
		AVPacket pkt = {0};

		if (packet->buffer)
		{
			pkt.size = packet->buffer->sz;
			pkt.data = (uint8_t*)packet->buffer->p;
		}

		int ret = av_read_frame(this->_ctx, &pkt);
		_YXC_CHECK_REPORT_NEW_RET(ret != AVERROR_EOF && ret != -EAGAIN, YXC_ERC_EOF, YC("End of file reached"));
		_YXC_CHECK_REPORT_NEW_RET(ret != -EIO, YXC_ERC_EOF, YC("Io exception(rmvb)"));
		_YXC_CHECK_FF_RET(ret);

		packet->bKeyPacket = pkt.flags & AV_PKT_FLAG_KEY;
		packet->uDataLen = pkt.size;
		packet->pData = pkt.data;

		if (!packet->buffer)
		{
			YXC_Status rc = YXV_BufferAlloc(YXV_BUFFER_TYPE_FF_PACKET, pkt.buf, pkt.size, &packet->buffer);
			if (rc != YXC_ERC_SUCCESS)
			{
				av_free_packet(&pkt);
				_YXC_CHECK_RC_RET(rc);
			}
		}

		AVRational time_base;
		time_base.den = 10000 * 1000;
		time_base.num = 1;

		packet->uRefTime = av_rescale_q(pkt.pts, this->_ctx->streams[pkt.stream_index]->time_base, time_base);
		packet->uDecTime = av_rescale_q(pkt.dts, this->_ctx->streams[pkt.stream_index]->time_base, time_base);
		if (this->_video_st != NULL && pkt.stream_index == this->_video_st->index)
		{
			packet->uStreamIndex = 0;
			if (this->_readCvtProcV)
			{
				this->_readCvtProcV(packet);
			}
		}
		else if (this->_audio_st != NULL && pkt.stream_index == this->_audio_st->index)
		{
			packet->uStreamIndex = 1;
			if (this->_readCvtProcA)
			{
				this->_readCvtProcA(packet);
			}
		}
		else
		{
			packet->uStreamIndex = -1;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::Seek(yint64_t i64Time)
	{
		AVRational time_base;
		time_base.den = YXV_REFTIME_PER_SEC;
		time_base.num = 1;

		if (this->_video_st && this->_vCodecId != YXV_VCODEC_ID_MJPEG) /* MP3. */
		{
			int64_t pts = av_rescale_q(i64Time, time_base, this->_video_st->time_base);
			int ffret = av_seek_frame(this->_ctx, this->_video_st->index, pts, AVSEEK_FLAG_BACKWARD);
			_YXC_CHECK_FF_RET(ffret);
		}

		if (this->_audio_st)
		{
			int64_t pts = av_rescale_q(i64Time, time_base, this->_audio_st->time_base);
			int ffret = av_seek_frame(this->_ctx, this->_audio_st->index, pts, AVSEEK_FLAG_BACKWARD);
			_YXC_CHECK_FF_RET(ffret);

			return YXC_ERC_SUCCESS;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::EndRead()
	{
		_YXC_CHECK_FF_RET( avio_close(this->_ctx->pb) );

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::EndWrite()
	{
		if (this->_ctx->pb)
		{
			_YXC_CHECK_FF_RET( av_write_trailer(this->_ctx) );
			_YXC_CHECK_FF_RET( avio_close(this->_ctx->pb) );
			this->_ctx->pb = NULL;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::ConfigV(const YXV_VParam* param, const YXV_FFVSpecParam* pSpec)
	{
		YXC_Status rc = this->_confVProc(CodecFromFFCodecV(this->_vCodecId), param, pSpec, &this->_video_st->codec);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::ConfigA(const YXV_AParam* param, const YXV_FFASpecParam* pSpec)
	{
		YXC_Status rc = this->_confAProc(CodecFromFFCodecA(this->_aCodecId), param, pSpec, &this->_audio_st->codec);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::WriteV(const YXV_Packet* packet)
	{
		if (this->_base_time_v == AV_NOPTS_VALUE)
		{
			this->_base_time_v = packet->uRefTime;
		}
		yint64_t iRefTime2 = packet->uRefTime - this->_base_time_v;
		yint64_t iDecTime2 = packet->uDecTime - this->_base_time_v;

		return this->_WriteStream(iRefTime2, iDecTime2, this->_video_st, packet, this->_writeProcV);
	}

	YXC_Status _FFFormat::WriteA(const YXV_Packet* packet)
	{
		if (this->_base_time_a == AV_NOPTS_VALUE)
		{
			this->_base_time_a = packet->uRefTime;
		}
		yint64_t iRefTime2 = packet->uRefTime - this->_base_time_a;

		/* Audio has no B-Frames. */
		return this->_WriteStream(iRefTime2, iRefTime2, this->_audio_st, packet, this->_writeProcA);
	}

	YXC_Status _FFFormat::_WriteStream(yint64_t iRefTime, yint64_t iDecTime, AVStream* pStream, const YXV_Packet* packet, _FFWriteProc pfnWrite)
	{
		AVPacket av_packet = {0};
		av_packet.data = packet->pData;
		if (packet->bKeyPacket)
		{
			av_packet.flags |= AV_PKT_FLAG_KEY;
		}

		AVRational time_base;
		time_base.den = 10000 * 1000;
		time_base.num = 1;

		av_packet.stream_index = pStream->index;
		av_packet.size = packet->uDataLen;

		av_packet.pts = av_rescale_q(iRefTime, time_base, pStream->time_base);
		av_packet.dts = av_rescale_q(iDecTime, time_base, pStream->time_base);
		if (packet->bNoPts)
		{
			av_packet.pts = AV_NOPTS_VALUE;
			av_packet.dts = AV_NOPTS_VALUE;
		}

		int ffret = pfnWrite(this->_ctx, &av_packet);
		_YXC_CHECK_FF_RET(ffret);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _FFFormat::GetOpt(YXV_FFFormatOpt opt, ybyte_t* pOpt, yuint32_t uCbOpt)
	{
		int ffret;
		switch (opt)
		{
		case YXV_FORMAT_OPT_SDP:
			ffret = av_sdp_create(&this->_ctx, 1, (char*)pOpt, uCbOpt);
			_YXC_CHECK_FF_RET(ffret);
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Unsupport format option(%d)"), opt);
			break;
		}

		return YXC_ERC_SUCCESS;
	}
}

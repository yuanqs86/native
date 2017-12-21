#include <YXC_AVKernel/Codec/ffwrap/ffbase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_H264.h>
#include <YXC_Sys/YXC_NetMarshal.h>

#define CONFIG_FTRAPV 0

#include <libavcodec/get_bits.h>
#include <libavcodec/golomb.h>
#include <libavutil/mem.h>
#include <libavfilter/avfilter.h>

namespace YXV_FFWrap
{
	void _FFPacketReadCvtProcH264(YXV_Packet* pPacket)
	{
		ybyte_t* pNewPtr = pPacket->pData;
		ybyte_t* pOldPtr = pPacket->pData;

		ybyte_t* pEndPtr = pPacket->pData + pPacket->uDataLen;
		ybyte_t codes[] = {0, 0, 0, 1};

		int result = memcmp(pOldPtr, codes, sizeof(codes));
		if (result == 0) /* Have start code, just return here. */
		{
			return;
		}

		int first_flag = 1;
		while (pOldPtr < pEndPtr)
		{
			yuint32_t uDataLen = YXC_NetUnmarshalUInt32(*(yuint32_t*)(pOldPtr));
			if (uDataLen > pEndPtr - pOldPtr)
			{
				YXV_PacketUnref(pPacket);
				return;
			}
			char flag = pOldPtr[4] & 0x1f;
			if ((flag == 7 || flag == 8 || flag == 6 || first_flag) && uDataLen < 2048)
			{
				memcpy(pNewPtr, codes, 4);
				pNewPtr += 4;
				pPacket->bKeyPacket = TRUE;
			}
			else
			{
				memcpy(pNewPtr, codes + 1, 3);
				pNewPtr += 3;
			}

			if (first_flag)
			{
				first_flag = FALSE;
			}
			pOldPtr += 4;

			memmove(pNewPtr, pOldPtr, uDataLen);

			pOldPtr += uDataLen;
			pNewPtr += uDataLen;
		}

		pPacket->uDataLen = pNewPtr - pPacket->pData;
	}

	int _FFWriteProcH264(AVFormatContext* ctx, AVPacket* pkt)
	{
		ybyte_t codes[] = {0, 0, 0, 1};
		if (memcmp(codes, pkt->data, sizeof(codes)) == 0 )
		{
			ybyte_t flag = pkt->data[4] & 0x1f;
			if (flag == 7) /* sps in header, directly write it. */
			{
				return av_write_frame(ctx, pkt);
			}
		}

		ybyte_t bKeyFrame = (pkt->data[0] & 0x1f) == 0x5;
		if (bKeyFrame)
		{
			AVCodecContext* codec = ctx->streams[pkt->stream_index]->codec;
			/* add sps, pps to header and write. */
			AVPacket newPkt = *pkt;
			newPkt.size = pkt->size + codec->extradata_size;
			newPkt.data = (uint8_t*)av_malloc(newPkt.size);
			if (newPkt.data == NULL)
			{
				return -E_OUTOFMEMORY;
			}
			memcpy(newPkt.data, codec->extradata, codec->extradata_size);
			memcpy(newPkt.data + codec->extradata_size, pkt->data, pkt->size);

			int ret = av_write_frame(ctx, &newPkt);
			av_free(newPkt.data);

			return ret;
		}
		else
		{
			int ret = av_write_frame(ctx, pkt);
			return ret;
		}
	}

	YXC_Status FFSpecV<AV_CODEC_ID_H264>::_FFConfigV_SpecEnc(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		YXV_FFVSpecParam param2;
		YXV_FFX264_H264Param* x264Param = &param2.x264_h264Param;
		YXV_FFVSpecParamDefault(YXV_VCODEC_ID_H264, TRUE, &param2);

		if (pSpecd != NULL)
		{
			x264Param = (YXV_FFX264_H264Param*)&pSpecd->x264_h264Param;
  			context->profile = pSpecd->x264_h264Param.profile;
		}

		context->flags |= CODEC_FLAG_GLOBAL_HEADER;
   		context->level = x264Param->level_idc;
		context->refs = x264Param->frame_reference;
		context->max_b_frames = x264Param->bframes;

		char szX264Opts[1000];
		if (strcmp(x264Param->preset, "veryfast") == 0)
		{
			sprintf(szX264Opts, "sync-lookahead=0:sliced-threads:rc-lookahead=1:force-cfr"
				":weightp=0:subme=1:bframes=0", x264Param->weighted_pred, x264Param->bframes);
			context->max_b_frames = 0;
			int ret = av_opt_set(context->priv_data, "x264opts", szX264Opts, 0);
			_YXC_CHECK_FF_RET(ret);
		}
		else
		{
			av_opt_set(context->priv_data, "preset", "veryfast", 0);
			av_opt_set(context->priv_data, "x264opts", "force-cfr", 0);
		}
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "b-pyramid", x264Param->bframe_pyramid) );
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "intra-refresh", x264Param->intra_refresh));
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "cabac", x264Param->cabac));
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "cqm", x264Param->cqm_preset));
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "bframes", x264Param->bframes));
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "interlaced", x264Param->interlaced ? "tff" :
		//	x264Param->fakeinterlaced ? "fake" : "0" ) );
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "8x8dct", x264Param->transform_8x8) );
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "weightp", x264Param->weighted_pred) );
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "range", x264Param->mv_range) );
		//_YXC_CHECK_FF_RET( av_opt_set_val(context, "x264opts", "chromaloc", x264Param->chormaloc) );

		if (x264Param->repeat_headers) /* Manually repeat headers on frames. */
		{
			context->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_H264>::_FFConfigV_SpecDec(AVCodecID codecId, const YXV_VParam* vParam, const YXV_FFVSpecParam* pSpecd, AVCodecContext* context)
	{
		_YXC_CHECK_RC_RET( _InitExtData(context, pSpecd->extData.uExt, pSpecd->extData.pExt) );
		return YXC_ERC_SUCCESS;
	}

	YXC_Status FFSpecV<AV_CODEC_ID_H264>::_FFConfigV_SpecRead(AVCodecID codecId, AVCodecContext* context, YXV_VParam* vParam, YXV_FFVFormatParam* pSpecd)
	{
		YXV_FFX264_H264Param* pEncParam = &(*pSpecd)[0].x264_h264Param;
		YXV_FFVSpecParamDefault(YXV_VCODEC_ID_H264, TRUE, &*pSpecd[0]);

		pEncParam->repeat_headers = TRUE;

		if (context->extradata_size == 0)
		{
			return YXC_ERC_SUCCESS;
		}

		ybyte_t codes[] = {0, 0, 0, 1};
		ybyte_t* pSps, *pPps;
		ULONG uSps, uPps;
		ybool_t bOldSps = FALSE;
		if (memcmp(context->extradata, codes, sizeof(codes)) == 0)
		{
			ParseH264Ext(context->extradata, context->extradata_size, &pSps, &pPps, &uSps, &uPps);
		}
		else
		{
			uSps = YXC_NetUnmarshalUInt16(*(yuint16_t*)(context->extradata + 6));
			uPps = YXC_NetUnmarshalUInt16(*(yuint16_t*)(context->extradata + uSps + 9));
			bOldSps = TRUE;
		}

		YXC_SPS sps;
		DecodeH264SPS(context->extradata + 8, uSps, &sps);

		if (sps.profile_idc == FF_PROFILE_H264_BASELINE)
		{
			pEncParam->cabac = FALSE;
			pEncParam->bframes = 0;
			pEncParam->interlaced = FALSE;
			pEncParam->fakeinterlaced = FALSE;
			strcpy(pEncParam->weighted_pred, "none");
		}
		else if (sps.profile_idc == FF_PROFILE_H264_MAIN)
		{
			strcpy(pEncParam->weighted_pred, "weighted");
		}
		else
		{
			pEncParam->transform_8x8 = 1;
			strcpy(pEncParam->cqm_preset, "jvt");
		}
		pEncParam->profile = sps.profile_idc;

		if (sps.transform_bypass)
		{
			strcpy(pEncParam->rc_method, "cqp");
			pEncParam->qp_constant = 0;
		}
		pEncParam->level_idc = sps.level_idc;

		if (sps.vui_parameters_present_flag)
		{
			if (sps.num_reorder_frames == 2)
			{
				strcpy(pEncParam->bframe_pyramid, "normal");
			}
			else if (sps.num_reorder_frames == 1)
			{
				pEncParam->bframes = 2;
			}

			if (sps.poc_type == 2)
			{
				pEncParam->interlaced = TRUE;
			}

			if (sps.frame_mbs_only_flag)
			{
				pEncParam->interlaced = pEncParam->fakeinterlaced = FALSE;
			}
			else
			{
				pEncParam->interlaced = TRUE;
			}
		}

		if (bOldSps)
		{
			ybyte_t* pNewExt = (ybyte_t*)av_malloc(uSps + uPps + 8);
			_YXC_CHECK_FF_PTR_RET(pNewExt);

			ybyte_t byStartCode[] = { 0, 0, 0, 1 };
			memcpy(pNewExt, byStartCode, sizeof(byStartCode));
			memcpy(pNewExt + sizeof(byStartCode), context->extradata + 8, uSps);
			memcpy(pNewExt + sizeof(byStartCode) + uSps, byStartCode, sizeof(byStartCode));
			memcpy(pNewExt + 2 * sizeof(byStartCode) + uSps, context->extradata + uSps + 11, uPps);

			av_free(context->extradata);
			context->extradata = pNewExt;
			context->extradata_size = uSps + uPps + 8;
		}

		YXV_FFExtraData* pParam = &(*pSpecd)[1].extData;
		(*pSpecd)[1].szEncDecName[0] = 0;

		pParam->pExt = context->extradata;
		pParam->uExt = context->extradata_size;
		pParam->uExtBuf = 0;

		return YXC_ERC_SUCCESS;
	}
}

#define MAX_SPS_COUNT          32
#define MAX_PPS_COUNT         256
#define MAX_PICTURE_COUNT 32
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))
#define EXTENDED_SAR       255

extern "C"
{
	const uint8_t YXC_zigzag_scan[16+1] = {
		0 + 0 * 4, 1 + 0 * 4, 0 + 1 * 4, 0 + 2 * 4,
		1 + 1 * 4, 2 + 0 * 4, 3 + 0 * 4, 2 + 1 * 4,
		1 + 2 * 4, 0 + 3 * 4, 1 + 3 * 4, 2 + 2 * 4,
		3 + 1 * 4, 3 + 2 * 4, 2 + 3 * 4, 3 + 3 * 4,
	};

	const uint8_t ff_zigzag_direct[64] = {
		0,   1,  8, 16,  9,  2,  3, 10,
		17, 24, 32, 25, 18, 11,  4,  5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13,  6,  7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63
	};

	const uint8_t ff_log2_tab[256]={
		0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
	};

	const int8_t ff_se_golomb_vlc_code[512]={
		17, 17, 17, 17, 17, 17, 17, 17, 16, 17, 17, 17, 17, 17, 17, 17,  8, -8,  9, -9, 10,-10, 11,-11, 12,-12, 13,-13, 14,-14, 15,-15,
		4,  4,  4,  4, -4, -4, -4, -4,  5,  5,  5,  5, -5, -5, -5, -5,  6,  6,  6,  6, -6, -6, -6, -6,  7,  7,  7,  7, -7, -7, -7, -7,
		2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
		3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	const uint8_t ff_ue_golomb_vlc_code[512]={
		32,32,32,32,32,32,32,32,31,32,32,32,32,32,32,32,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
		7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	const uint8_t ff_golomb_vlc_len[512]={
		19,17,15,15,13,13,13,13,11,11,11,11,11,11,11,11,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	};

	const uint8_t default_scaling4[2][16]={
		{
			6,13,20,28,
			13,20,28,32,
			20,28,32,37,
			28,32,37,42
		},{
			10,14,20,24,
			14,20,24,27,
			20,24,27,30,
			24,27,30,34
		}
	};

	const uint8_t default_scaling8[2][64]={
		{
			6,10,13,16,18,23,25,27,
			10,11,16,18,23,25,27,29,
			13,16,18,23,25,27,29,31,
			16,18,23,25,27,29,31,33,
			18,23,25,27,29,31,33,36,
			23,25,27,29,31,33,36,38,
			25,27,29,31,33,36,38,40,
			27,29,31,33,36,38,40,42
		},{
			9,13,15,17,19,21,22,24,
			13,13,17,19,21,22,24,25,
			15,17,19,21,22,24,25,27,
			17,19,21,22,24,25,27,28,
			19,21,22,24,25,27,28,30,
			21,22,24,25,27,28,30,32,
			22,24,25,27,28,30,32,33,
			24,25,27,28,30,32,33,35
		}
	};

	int YXC_Decode_RBSP_trailing( LPBYTE pSrc )
	{
		int nTrailingByte = *pSrc;
		int nZeroBits;

		for (nZeroBits = 1; nZeroBits < 9; nZeroBits++)
		{
			if (nTrailingByte & 0x01)
			{
				return nZeroBits;
			}

			nTrailingByte >>= 1;
		}

		return 0;

	}

	void YXC_init_get_bits(GetBitContext *s, const uint8_t *buffer, int bit_size)
	{
		int buffer_size = (bit_size+7)>>3;
		if (buffer_size < 0 || bit_size < 0) {
			buffer_size = bit_size = 0;
			buffer = NULL;
		}

		s->buffer       = buffer;
		s->size_in_bits = bit_size;
		s->size_in_bits_plus8 = bit_size + 8;
		s->buffer_end   = buffer + buffer_size;
		s->index        = 0;
	}
	void YXC_decode_scaling_list(GetBitContext* gb, uint8_t *factors, int size, const uint8_t *jvt_list, const uint8_t *fallback_list)
	{
		int i, last = 8, next = 8;
		const uint8_t *scan = size == 16 ? YXC_zigzag_scan : ff_zigzag_direct;
		if (!get_bits1(gb))
		{
			memcpy(factors, fallback_list, size*sizeof(uint8_t));
		}
		else
		{
			for (i=0; i<size; i++)
			{
				if (next)
					next = (last + get_se_golomb(gb)) & 0xff;
				if (!i && !next)
				{
					memcpy(factors, jvt_list, size*sizeof(uint8_t));
					break;
				}
				last = factors[scan[i]] = next ? next : last;
			}
		}
	}
	void YXC_Decode_scaling_matrices(GetBitContext* gb, YXC_SPS* sps, YXC_PPS *pps, int is_sps,
		uint8_t (*scaling_matrix4)[16], uint8_t (*scaling_matrix8)[64])
	{
		int fallback_sps = !is_sps && sps->scaling_matrix_present;
		const uint8_t* fallback[4] =
		{
			fallback_sps ? sps->scaling_matrix4[0] : default_scaling4[0],
			fallback_sps ? sps->scaling_matrix4[3] : default_scaling4[1],
			fallback_sps ? sps->scaling_matrix8[0] : default_scaling8[0],
			fallback_sps ? sps->scaling_matrix8[3] : default_scaling8[1]
		};

		if (get_bits1(gb))
		{
			sps->scaling_matrix_present |= is_sps;
			YXC_decode_scaling_list(gb,scaling_matrix4[0],16,default_scaling4[0],fallback[0]); // Intra, Y
			YXC_decode_scaling_list(gb,scaling_matrix4[1],16,default_scaling4[0],scaling_matrix4[0]); // Intra, Cr
			YXC_decode_scaling_list(gb,scaling_matrix4[2],16,default_scaling4[0],scaling_matrix4[1]); // Intra, Cb
			YXC_decode_scaling_list(gb,scaling_matrix4[3],16,default_scaling4[1],fallback[1]); // Inter, Y
			YXC_decode_scaling_list(gb,scaling_matrix4[4],16,default_scaling4[1],scaling_matrix4[3]); // Inter, Cr
			YXC_decode_scaling_list(gb,scaling_matrix4[5],16,default_scaling4[1],scaling_matrix4[4]); // Inter, Cb
			if (is_sps || pps->transform_8x8_mode)
			{
				YXC_decode_scaling_list(gb,scaling_matrix8[0],64,default_scaling8[0],fallback[2]);  // Intra, Y
				YXC_decode_scaling_list(gb,scaling_matrix8[3],64,default_scaling8[1],fallback[3]);  // Inter, Y
				if (sps->chroma_format_idc == 3)
				{
					YXC_decode_scaling_list(gb,scaling_matrix8[1],64,default_scaling8[0],scaling_matrix8[0]);  // Intra, Cr
					YXC_decode_scaling_list(gb,scaling_matrix8[4],64,default_scaling8[1],scaling_matrix8[3]);  // Inter, Cr
					YXC_decode_scaling_list(gb,scaling_matrix8[2],64,default_scaling8[0],scaling_matrix8[1]);  // Intra, Cb
					YXC_decode_scaling_list(gb,scaling_matrix8[5],64,default_scaling8[1],scaling_matrix8[4]);  // Inter, Cb
				}
			}
		}
	}

	static const YXC_AVRational pixel_aspect[17] = {
		{0, 1},
		{1, 1},
		{12, 11},
		{10, 11},
		{16, 11},
		{40, 33},
		{24, 11},
		{20, 11},
		{32, 11},
		{80, 33},
		{18, 11},
		{15, 11},
		{64, 33},
		{160,99},
		{4, 3},
		{3, 2},
		{2, 1},
	};

	static int decode_vui_parameters(GetBitContext *gb, YXC_SPS* sps)
	{
		int aspect_ratio_info_present_flag;
		unsigned int aspect_ratio_idc;

		aspect_ratio_info_present_flag = get_bits1(gb);

		if( aspect_ratio_info_present_flag ) {
			aspect_ratio_idc= get_bits(gb, 8);
			if( aspect_ratio_idc == EXTENDED_SAR ) {
				sps->sar.num= get_bits(gb, 16);
				sps->sar.den= get_bits(gb, 16);
			}else if(aspect_ratio_idc < FF_ARRAY_ELEMS(pixel_aspect)){
				sps->sar =  pixel_aspect[aspect_ratio_idc];
			}else{
				return -1;
			}
		}else{
			sps->sar.num = sps->sar.den = 0;
		}
		//            s->avctx->aspect_ratio= sar_width*s->width / (float)(s->height*sar_height);

		if(get_bits1(gb)){      /* overscan_info_present_flag */
			get_bits1(gb);      /* overscan_appropriate_flag */
		}

		sps->video_signal_type_present_flag = get_bits1(gb);
		if(sps->video_signal_type_present_flag){
			get_bits(gb, 3);    /* video_format */
			sps->full_range = get_bits1(gb); /* video_full_range_flag */

			sps->colour_description_present_flag = get_bits1(gb);
			if(sps->colour_description_present_flag){
				sps->color_primaries = (YXC_AVColorPrimaries)get_bits(gb, 8); /* colour_primaries */
				sps->color_trc       = (YXC_AVColorTransferCharacteristic)get_bits(gb, 8); /* transfer_characteristics */
				sps->colorspace      = (YXC_AVColorSpace)get_bits(gb, 8); /* matrix_coefficients */
				if (sps->color_primaries >= AVCOL_PRI_NB)
					sps->color_primaries  = (YXC_AVColorPrimaries)AVCOL_PRI_UNSPECIFIED;
				if (sps->color_trc >= AVCOL_TRC_NB)
					sps->color_trc  = (YXC_AVColorTransferCharacteristic)AVCOL_TRC_UNSPECIFIED;
				if (sps->colorspace >= AVCOL_SPC_NB)
					sps->colorspace  = (YXC_AVColorSpace)AVCOL_SPC_UNSPECIFIED;
			}
		}

		if(get_bits1(gb)){      /* chroma_location_info_present_flag */
			// h->avctx->chroma_sample_location = get_ue_golomb(&h->gb)+1;  /* chroma_sample_location_type_top_field */
			get_ue_golomb(gb);  /* chroma_sample_location_type_bottom_field */
		}

		sps->timing_info_present_flag = get_bits1(gb);
		if(sps->timing_info_present_flag){
			sps->num_units_in_tick = get_bits_long(gb, 32);
			sps->time_scale = get_bits_long(gb, 32);
			if(!sps->num_units_in_tick || !sps->time_scale){
				//av_log(h->avctx, AV_LOG_ERROR, "time_scale/num_units_in_tick invalid or unsupported (%d/%d)\n", sps->time_scale, sps->num_units_in_tick);
				return -1;
			}
			sps->fixed_frame_rate_flag = get_bits1(gb);
		}

		sps->nal_hrd_parameters_present_flag = get_bits1(gb);
		//if(sps->nal_hrd_parameters_present_flag)
		//	if(decode_hrd_parameters(h, sps) < 0)
		//		return -1;
		sps->vcl_hrd_parameters_present_flag = get_bits1(gb);
		//if(sps->vcl_hrd_parameters_present_flag)
		//	if(decode_hrd_parameters(h, sps) < 0)
		//		return -1;
		if(sps->nal_hrd_parameters_present_flag || sps->vcl_hrd_parameters_present_flag)
			get_bits1(gb);     /* low_delay_hrd_flag */
		sps->pic_struct_present_flag = get_bits1(gb);
		if(!get_bits_left(gb))
			return 0;
		sps->bitstream_restriction_flag = get_bits1(gb);
		if(sps->bitstream_restriction_flag){
			get_bits1(gb);     /* motion_vectors_over_pic_boundaries_flag */
			get_ue_golomb(gb); /* max_bytes_per_pic_denom */
			get_ue_golomb(gb); /* max_bits_per_mb_denom */
			get_ue_golomb(gb); /* log2_max_mv_length_horizontal */
			get_ue_golomb(gb); /* log2_max_mv_length_vertical */
			sps->num_reorder_frames= get_ue_golomb(gb);
			get_ue_golomb(gb); /*max_dec_frame_buffering*/

			if (get_bits_left(gb) < 0) {
				sps->num_reorder_frames=0;
				sps->bitstream_restriction_flag= 0;
			}

			if(sps->num_reorder_frames > 16U /*max_dec_frame_buffering || max_dec_frame_buffering > 16*/){
				//av_log(h->avctx, AV_LOG_ERROR, "illegal num_reorder_frames %d\n", sps->num_reorder_frames);
				return -1;
			}
		}

		if (get_bits_left(gb) < 0) {
			//av_log(h->avctx, AV_LOG_ERROR, "Overread VUI by %d bits\n", -get_bits_left(&h->gb));
			return AVERROR_INVALIDDATA;
		}

		return 0;
	}

	static LPBYTE _FindNextCode(LPBYTE start, LPBYTE end) /* Not precise enough. */
	{
		LONG uSizeTotal = end - start;

		BYTE startcode[4] = {0, 0, 0, 1};
		BYTE midcode[3] = {0, 0, 1};

		for (LONG i = 0; i < uSizeTotal - 5 ; ++i)
		{
			if (start[i] == 0 && start[i + 1] == 0)
			{
				if (start[i + 2] == 0 && start[i + 3] == 1) return start + i;

				if (start[i + 2] == 1) return start + i;
			}
		}

		return end;
	}

	LONG ParseH264Ext(LPBYTE pData, ULONG32 uDataSize, LPBYTE* ppSps, LPBYTE* ppPps, ULONG* pSpsSize, ULONG* pPpsSize)
	{
		BYTE* pSps = NULL, *pPps = NULL, *pEnd = pData + uDataSize;
		ULONG uSps = 0, uPps = 0;
		BYTE startcode[4] = {0, 0, 0, 1};

		LPBYTE pData2 = pData;
		while (pData2 < pEnd)
		{
			int cmp = memcmp(pData2, startcode, 4);
			if (cmp == 0)
			{
				int flag = pData2[4] & 0x1f;
				if (flag == 0x8) /* PPS. */
				{
					pPps = pData2 + 4;
					pData2 = _FindNextCode(pData2 + 4, pEnd);
					uPps = pData2 - pPps;
				}
				else if (flag == 0x7) /* SPS. */
				{
					pSps = pData2 + 4;
					pData2 = _FindNextCode(pData2 + 4, pEnd);
					uSps = pData2 - pSps;
				}
				else
				{
					pData2 = _FindNextCode(pData2 + 4, pEnd);
				}
			}
			else
			{
				++pData2;
			}
		}

		*ppSps = pSps;
		*ppPps = pPps;
		*pPpsSize = uPps;
		*pSpsSize = uSps;
		return 0;
	}
	//
	LONG DecodeH264SPS( LPBYTE pData, ULONG32 uDataSize, YXC_SPS* sps )
	{
		if (pData == NULL  || uDataSize == 0 || sps == NULL)
		{
			return -1;
		}


		ZeroMemory(sps, sizeof(YXC_SPS));

		int profile_idc, level_idc, constraint_set_flags = 0;
		unsigned int sps_id;
		int i;
		GetBitContext gb;
		int nBits = 8 * (uDataSize - 1) - YXC_Decode_RBSP_trailing( pData + uDataSize - 1);

		YXC_init_get_bits(&gb, pData + 1, nBits);

		profile_idc= get_bits(&gb, 8);
		constraint_set_flags |= get_bits1(&gb) << 0;   //取H264级别，下同
		constraint_set_flags |= get_bits1(&gb) << 1;
		constraint_set_flags |= get_bits1(&gb) << 2;
		constraint_set_flags |= get_bits1(&gb) << 3;
		constraint_set_flags |= get_bits1(&gb) << 4;
		constraint_set_flags |= get_bits1(&gb) << 5;
		get_bits(&gb, 2); //保留位为0，跳过
		level_idc= get_bits(&gb, 8);
		sps_id= get_ue_golomb_31(&gb);

		if (sps_id >= MAX_SPS_COUNT)
		{
			return -1;
		}

		sps->time_offset_length = 24;
		sps->profile_idc= profile_idc;
		sps->constraint_set_flags = constraint_set_flags;
		sps->level_idc= level_idc;
		sps->full_range = -1;

		memset(sps->scaling_matrix4, 16, sizeof(sps->scaling_matrix4));
		memset(sps->scaling_matrix8, 16, sizeof(sps->scaling_matrix8));
		sps->scaling_matrix_present = 0;
		sps->colorspace = YXC_AVCOL_SPC_UNSPECIFIED;

		if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
			sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc ==  44 ||
			sps->profile_idc ==  83 || sps->profile_idc ==  86 || sps->profile_idc == 118 ||
			sps->profile_idc == 128 || sps->profile_idc == 144)
		{
			sps->chroma_format_idc= get_ue_golomb_31(&gb);

			if (sps->chroma_format_idc > 3U)
			{
				return -1;
			}
			else if (sps->chroma_format_idc == 3)
			{
				sps->residual_color_transform_flag = get_bits1(&gb);
				if (sps->residual_color_transform_flag)
				{
					return -1;
				}
			}

			sps->bit_depth_luma = get_ue_golomb(&gb) + 8;
			sps->bit_depth_chroma = get_ue_golomb(&gb) + 8;

			if (sps->bit_depth_luma > 14U || sps->bit_depth_chroma > 14U)
			{
				return -1;
			}

			sps->transform_bypass = get_bits1(&gb);
			YXC_Decode_scaling_matrices(&gb, sps, NULL, 1, sps->scaling_matrix4, sps->scaling_matrix8);
		}
		else
		{
			sps->chroma_format_idc = 1;
			sps->bit_depth_luma = 8;
			sps->bit_depth_chroma = 8;
		}

		sps->log2_max_frame_num = get_ue_golomb(&gb) + 4;
		if (sps->log2_max_frame_num < 4 || sps->log2_max_frame_num > 16)
		{
			return -1;
		}

		sps->poc_type = get_ue_golomb_31(&gb);

		if (sps->poc_type == 0)
		{
			unsigned t = get_ue_golomb(&gb);
			if (t > 12)
			{
				return -1;
			}

			sps->log2_max_poc_lsb = t + 4;
		}
		else if (sps->poc_type == 1)
		{
			sps->delta_pic_order_always_zero_flag = get_bits1(&gb);
			sps->offset_for_non_ref_pic = get_se_golomb(&gb);
			sps->offset_for_top_to_bottom_field = get_se_golomb(&gb);
			sps->poc_cycle_length = get_ue_golomb(&gb);

			if ((unsigned)sps->poc_cycle_length >= FF_ARRAY_ELEMS(sps->offset_for_ref_frame))
			{
				return -1;
			}

			for (i=0; i<sps->poc_cycle_length; i++)
				sps->offset_for_ref_frame[i] = get_se_golomb(&gb);
		}
		else if (sps->poc_type != 2)
		{
			return -1;
		}

		sps->ref_frame_count = get_ue_golomb_31(&gb);
		if (sps->ref_frame_count > MAX_PICTURE_COUNT-2 || sps->ref_frame_count > 16U)
		{
			return -1;
		}

		sps->gaps_in_frame_num_allowed_flag = get_bits1(&gb);
		sps->mb_width = get_ue_golomb(&gb) + 1;
		sps->mb_height = get_ue_golomb(&gb) + 1;
		sps->mb_real_width = sps->mb_width * 16;
		sps->mb_real_height = sps->mb_height * 16;

		if ( (unsigned)sps->mb_width >= INT_MAX/16 || (unsigned)sps->mb_height >= INT_MAX/16 )
		{
			return -1;
		}

		sps->frame_mbs_only_flag = get_bits1(&gb);
		if (!sps->frame_mbs_only_flag)
			sps->mb_aff= get_bits1(&gb);
		else
			sps->mb_aff= 0;

		sps->direct_8x8_inference_flag = get_bits1(&gb);

		sps->crop = get_bits1(&gb);
		if (sps->crop)
		{
			int crop_vertical_limit = sps->chroma_format_idc  & 2 ? 16 : 8;
			int crop_horizontal_limit = sps->chroma_format_idc == 3 ? 16 : 8;
			sps->crop_left = get_ue_golomb(&gb);
			sps->crop_right = get_ue_golomb(&gb);
			sps->crop_top = get_ue_golomb(&gb);
			sps->crop_bottom = get_ue_golomb(&gb);
			//         if(sps->crop_left || sps->crop_top)
			//         {
			//             av_log(h->s.avctx, AV_LOG_ERROR, "insane cropping not completely supported, this could look slightly wrong ... (left: %d, top: %d)\n", sps->crop_left, sps->crop_top);
			//         }
			if (sps->crop_right >= crop_horizontal_limit || sps->crop_bottom >= crop_vertical_limit)
			{
				sps->crop_left = sps->crop_right = sps->crop_top = sps->crop_bottom = 0;
			}

			if (crop_vertical_limit == 8)
			{
				sps->mb_real_height -= sps->crop_bottom * 2;
			}
			else
			{
				sps->mb_real_height -= sps->crop_bottom;
			}

			if (crop_horizontal_limit == 8)
			{
				sps->mb_real_width -= sps->crop_right * 2;
			}
			else
			{
				sps->mb_real_width -= sps->crop_right;
			}
		}
		else
		{
			sps->crop_left = sps->crop_right = sps->crop_top = sps->crop_bottom = 0;
		}

		sps->vui_parameters_present_flag= get_bits1(&gb);
		if( sps->vui_parameters_present_flag ) {
			if (decode_vui_parameters(&gb, sps) < 0)
			{
				return -1;
			}
		}

		if (sps->sar.den == 0) sps->sar.den = 1;

		return 0;
	}
};

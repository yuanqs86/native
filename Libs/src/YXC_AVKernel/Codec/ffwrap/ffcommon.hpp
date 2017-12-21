#ifndef __INNER_INC_YXC_AV_FFWRAP_FF_COMMON_HPP__
#define __INNER_INC_YXC_AV_FFWRAP_FF_COMMON_HPP__

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include "stdint.h"
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	#include <libavfilter/avfilter.h>
	#include <pthread.h>
};

#include <YXC_AVKernel/YXV_ffmpeg.h>

#define _YXC_CHECK_FF_RET(ffrc)																\
	do {																					\
		int ffx = (ffrc);																	\
		_YXC_CHECK_REPORT_RET_EX(ffx >= 0, YXC_ERROR_SRC_3RD_PARTY, YXC_ERR_CAT_3RD_FFMPEG,	\
			ffx, YXC_ERC_3RD, L"%s", L"Inv:" L#ffrc);										\
	} while (0)

#define _YXC_CHECK_FF_PTR_RET(ffptr)															\
	do {																					\
		const void* ffx = (ffptr);															\
		_YXC_CHECK_REPORT_RET_EX(ffx != NULL, YXC_ERROR_SRC_3RD_PARTY, YXC_ERR_CAT_3RD_FFMPEG,	\
			-3, YXC_ERC_3RD, L"%s", L"Inp:" L#ffptr);										\
	} while (0)

namespace YXV_FFWrap
{
	int avcodec_open2_locked(AVCodecContext* pContext, AVCodec* pCodec, AVDictionary** ppDict);
	void avcodec_close_locked(AVCodecContext* pContext);
	void _av_free_context(AVCodecContext* pContext);
	YXC_Status _InitExtData(AVCodecContext* context, yuint32_t uCbData, const void* pData);

	static inline yint64_t RefTimeToPts(yint64_t uRefTime, AVRational time_base)
	{
		yint64_t uCSTimePerPts = (yint64_t)10000 * 10 * time_base.num;
		yint64_t ucsTime = uRefTime / 100;

		if (uRefTime > 0)
		{
			yint64_t pts = (ucsTime * time_base.den + uCSTimePerPts / 10) / uCSTimePerPts; // floor 0.1.
			return pts;
		}
		else
		{
			yint64_t pts = (ucsTime * time_base.den - uCSTimePerPts / 10) / uCSTimePerPts; // floor 0.1.
			return pts;
		}
	}

	static inline yint64_t PtsToRefTime(yint64_t pts, AVRational time_base)
	{
		yint64_t uCSTimePerPts = (yint64_t)10000 * 1000 * time_base.num;

		yint64_t uRefTime = pts * uCSTimePerPts / time_base.den;

		printf("RefTime = %lld, APts = %lld\n", uRefTime, pts);
		return uRefTime;
	}

	static inline YXV_ACodecID CodecToFFCodecA(AVCodecID codecId)
	{
		switch (codecId)
		{
		case AV_CODEC_ID_AAC:
			return YXV_ACODEC_ID_AAC;
		case AV_CODEC_ID_MP3:
			return YXV_ACODEC_ID_MP3;
		case AV_CODEC_ID_WMAV2:
			return YXV_ACODEC_ID_WMA2;
		case AV_CODEC_ID_WMAPRO:
			return YXV_ACODEC_ID_WMAPRO;
		case AV_CODEC_ID_ADPCM_G722:
			return YXV_ACODEC_ID_G722;
		case AV_CODEC_ID_OPUS:
			return YXV_ACODEC_ID_OPUS;
		case AV_CODEC_ID_VORBIS:
			return YXV_ACODEC_ID_VORBIS;
		case AV_CODEC_ID_COOK:
			return YXV_ACODEC_ID_COOK;
		case AV_CODEC_ID_NONE:
			return YXV_ACODEC_ID_NONE;
		}

		return (YXV_ACodecID)(codecId | 0x80000000);
	}

	static inline AVCodecID CodecFromFFCodecA(YXV_ACodecID codecId)
	{
		switch (codecId)
		{
		case YXV_ACODEC_ID_AAC:
			return AV_CODEC_ID_AAC;
		case YXV_ACODEC_ID_MP3:
			return AV_CODEC_ID_MP3;
		case YXV_ACODEC_ID_WMA2:
			return AV_CODEC_ID_WMAV2;
		case YXV_ACODEC_ID_WMAPRO:
			return AV_CODEC_ID_WMAPRO;
		case YXV_ACODEC_ID_G722:
			return AV_CODEC_ID_ADPCM_G722;
		case YXV_ACODEC_ID_OPUS:
			return AV_CODEC_ID_OPUS;
		case YXV_ACODEC_ID_VORBIS:
			return AV_CODEC_ID_VORBIS;
		case YXV_ACODEC_ID_COOK:
			return AV_CODEC_ID_COOK;
		case YXV_ACODEC_ID_NONE:
			return AV_CODEC_ID_NONE;
		}

		return (AVCodecID)(codecId & 0x7fffffff);
	}

	static inline YXV_VCodecID CodecToFFCodecV(AVCodecID codecId)
	{
		switch (codecId)
		{
		case AV_CODEC_ID_H264:
			return YXV_VCODEC_ID_H264;
		case AV_CODEC_ID_WMV3:
			return YXV_VCODEC_ID_WMV3;
		case AV_CODEC_ID_QVGA:
			return YXV_VCODEC_ID_EQV1;
		case AV_CODEC_ID_VP8:
			return YXV_VCODEC_ID_VP8;
		case AV_CODEC_ID_RV10:
			return YXV_VCODEC_ID_RV10;
		case AV_CODEC_ID_RV20:
			return YXV_VCODEC_ID_RV20;
		case AV_CODEC_ID_RV30:
			return YXV_VCODEC_ID_RV30;
		case AV_CODEC_ID_RV40:
			return YXV_VCODEC_ID_RV40;
		case AV_CODEC_ID_JPEG2000:
			return YXV_VCODEC_ID_JPEG2000;
		case AV_CODEC_ID_JPEGLS:
			return YXV_VCODEC_ID_JPEGLS;
		case AV_CODEC_ID_MJPEG:
			return YXV_VCODEC_ID_MJPEG;
		case AV_CODEC_ID_PNG:
			return YXV_VCODEC_ID_PNG;
		case AV_CODEC_ID_GIF:
			return YXV_VCODEC_ID_GIF;
		case AV_CODEC_ID_BMP:
			return YXV_VCODEC_ID_BMP;
		case AV_CODEC_ID_NONE:
			return YXV_VCODEC_ID_NONE;
		}

		return (YXV_VCodecID)(codecId | 0x80000000);
	}

	static inline AVCodecID CodecFromFFCodecV(YXV_VCodecID codecId)
	{
		switch (codecId)
		{
		case YXV_VCODEC_ID_H264:
			return AV_CODEC_ID_H264;
		case YXV_VCODEC_ID_WMV3:
			return AV_CODEC_ID_WMV3;
		case YXV_VCODEC_ID_EQV1:
			return AV_CODEC_ID_QVGA;
		case YXV_VCODEC_ID_VP8:
			return AV_CODEC_ID_VP8;
		case YXV_VCODEC_ID_RV10:
			return AV_CODEC_ID_RV10;
		case YXV_VCODEC_ID_RV20:
			return AV_CODEC_ID_RV20;
		case YXV_VCODEC_ID_RV30:
			return AV_CODEC_ID_RV30;
		case YXV_VCODEC_ID_RV40:
			return AV_CODEC_ID_RV40;
		case YXV_VCODEC_ID_JPEG2000:
			return AV_CODEC_ID_JPEG2000;
		case YXV_VCODEC_ID_JPEGLS:
			return AV_CODEC_ID_JPEGLS;
		case YXV_VCODEC_ID_MJPEG:
			return AV_CODEC_ID_MJPEG;
		case YXV_VCODEC_ID_PNG:
			return AV_CODEC_ID_PNG;
		case YXV_VCODEC_ID_GIF:
			return AV_CODEC_ID_GIF;
		case YXV_VCODEC_ID_BMP:
			return AV_CODEC_ID_BMP;
		case YXV_VCODEC_ID_NONE:
			return AV_CODEC_ID_NONE;
		}

		return (AVCodecID)(codecId & 0x7fffffff);
	}

	static inline AVPixelFormat PixFmtFromFF(YXV_PixFmt pixFmt)
	{
		switch (pixFmt)
		{
		case YXV_PIX_FMT_BGR24:
			return AV_PIX_FMT_BGR24;
		case YXV_PIX_FMT_RGB24:
			return AV_PIX_FMT_RGB24;
		case YXV_PIX_FMT_RGB0:
			return AV_PIX_FMT_RGB0;
		case YXV_PIX_FMT_RGBA:
			return AV_PIX_FMT_RGBA;
		case YXV_PIX_FMT_0RGB:
			return AV_PIX_FMT_0RGB;
		case YXV_PIX_FMT_ARGB:
			return AV_PIX_FMT_ARGB;
		case YXV_PIX_FMT_BGR0:
			return AV_PIX_FMT_BGR0;
		case YXV_PIX_FMT_BGRA:
			return AV_PIX_FMT_BGRA;
		case YXV_PIX_FMT_0BGR:
			return AV_PIX_FMT_0BGR;
		case YXV_PIX_FMT_ABGR:
			return AV_PIX_FMT_ABGR;
		case YXV_PIX_FMT_YUV420P:
			return AV_PIX_FMT_YUV420P;
		case YXV_PIX_FMT_YUVA420P:
			return AV_PIX_FMT_YUVA420P;
		case YXV_PIX_FMT_YUVJ420P:
			return AV_PIX_FMT_YUVJ420P;
		case YXV_PIX_FMT_YUVJ444P:
			return AV_PIX_FMT_YUVJ444P;
		case YXV_PIX_FMT_YUVA444P:
			return AV_PIX_FMT_YUVA444P;
		case YXV_PIX_FMT_YUYV:
			return AV_PIX_FMT_YUYV422;
		case YXV_PIX_FMT_UYVY:
			return AV_PIX_FMT_UYVY422;
		case YXV_PIX_FMT_NV12:
			return AV_PIX_FMT_NV12;
		case YXV_PIX_FMT_YUV444P:
			return AV_PIX_FMT_YUV444P;
		default:
			return AV_PIX_FMT_NONE;
		}
	}

	static inline YXV_PixFmt PixFmtToFF(AVPixelFormat pixFmt)
	{
		switch (pixFmt)
		{
		case AV_PIX_FMT_BGR24:
			return YXV_PIX_FMT_BGR24;
		case AV_PIX_FMT_RGB24:
			return YXV_PIX_FMT_RGB24;
		case AV_PIX_FMT_RGB0:
			return YXV_PIX_FMT_RGB0;
		case AV_PIX_FMT_RGBA:
			return YXV_PIX_FMT_RGBA;
		case AV_PIX_FMT_0RGB:
			return YXV_PIX_FMT_0RGB;
		case AV_PIX_FMT_ARGB:
			return YXV_PIX_FMT_ARGB;
		case AV_PIX_FMT_BGR0:
			return YXV_PIX_FMT_BGR0;
		case AV_PIX_FMT_BGRA:
			return YXV_PIX_FMT_BGRA;
		case AV_PIX_FMT_0BGR:
			return YXV_PIX_FMT_0BGR;
		case AV_PIX_FMT_ABGR:
			return YXV_PIX_FMT_ABGR;
		case AV_PIX_FMT_YUV420P:
			return YXV_PIX_FMT_YUV420P;
		case AV_PIX_FMT_YUVA420P:
			return YXV_PIX_FMT_YUVA420P;
		case AV_PIX_FMT_YUVJ420P:
			return YXV_PIX_FMT_YUVJ420P;
		case AV_PIX_FMT_YUVJ444P:
			return YXV_PIX_FMT_YUVJ444P;
		case AV_PIX_FMT_YUVA444P:
			return YXV_PIX_FMT_YUVA444P;
		case AV_PIX_FMT_UYVY422:
			return YXV_PIX_FMT_UYVY;
		case AV_PIX_FMT_YUYV422:
			return YXV_PIX_FMT_YUYV;
		default:
			return YXV_PIX_FMT_NONE;
		}
	}

	static inline AVSampleFormat SampleFmtFromFF(YXV_SampleFmt sampleFmt)
	{
		switch (sampleFmt)
		{
		case YXV_SAMPLE_FMT_FLTP:
			return AV_SAMPLE_FMT_FLTP;
		case YXV_SAMPLE_FMT_16S:
			return AV_SAMPLE_FMT_S16;
		case YXV_SAMPLE_FMT_FLT:
			return AV_SAMPLE_FMT_FLT;
		default:
			return AV_SAMPLE_FMT_NONE;
		}
	}

	static inline YXV_SampleFmt SampleFmtToFF(AVSampleFormat sampleFmt)
	{
		switch (sampleFmt)
		{
		case AV_SAMPLE_FMT_FLTP:
			return YXV_SAMPLE_FMT_FLTP;
		case AV_SAMPLE_FMT_S16:
			return YXV_SAMPLE_FMT_16S;
		case AV_SAMPLE_FMT_FLT:
			return YXV_SAMPLE_FMT_FLT;
		default:
			return YXV_SAMPLE_FMT_NONE;
		}
	}

	static inline void ReadCodecExt(AVCodecContext* ctx, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData)
	{
		yuint32_t nMax = max(uCbBuf, ctx->extradata_size);
		memcpy(pBuf, ctx->extradata, ctx->extradata_size);
		*puCbData = ctx->extradata_size;
	}

	static inline int GetImageRowAlign(AVPixelFormat pixFmt)
	{
		switch (pixFmt)
		{
		case AV_PIX_FMT_BGR24:
		case AV_PIX_FMT_RGB24:
			return 4;
		default:
			return 1;
		}
	}

	static int av_opt_set_val(AVCodecContext* context, const char* name, const char* desc, int val)
	{
		char szDesc[500];
		sprintf(szDesc, "%s=%d", desc, val);
		int ret = av_opt_set(context->priv_data, name, szDesc, 0);
		return ret;
	}

	static int av_opt_set_val(AVCodecContext* context, const char* name, const char* desc, const char* val)
	{
		char szDesc[500];
		sprintf(szDesc, "%s=%s", desc, val);
		int ret = av_opt_set(context->priv_data, name, szDesc, 0);
		return ret;
	}

	static int av_opt_set_format(void* priv_data, const char* name, const char* fmt, ...)
	{
		va_list vaList;
		va_start(vaList, fmt);

		char szDesc[500];
		vsprintf(szDesc, fmt, vaList);

		int ret = av_opt_set(priv_data, name, szDesc, 0);
		return ret;
	}

	static inline void avcodec_free_frame(AVFrame** frame)
	{
		av_frame_free(frame);
	}
}

#endif /* __INNER_INC_YXC_AV_FFWRAP_FF_COMMON_HPP__ */

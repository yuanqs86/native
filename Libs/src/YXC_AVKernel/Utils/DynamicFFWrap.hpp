#ifndef __INNER_INC_YXC_AV_DYNAMIC_FFWRAP_HPP__
#define __INNER_INC_YXC_AV_DYNAMIC_FFWRAP_HPP__

#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/YXV_H264.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
};

namespace _YXV_FFWrap
{
	typedef YXC_Status (*func_YXV_FFObjConfigV)(YXV_FFObject obj, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd);
	typedef YXC_Status (*func_YXV_FFObjConfigA)(YXV_FFObject obj, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd);
	typedef void (*func_YXV_FFObjClose)(YXV_FFObject obj);
	typedef YXC_Status (*func_YXV_FFDecCreate)(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFDec* pDec);
	typedef YXC_Status (*func_YXV_FFDecProcessV)(YXV_FFDec dec, const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Frame* pSample);
	typedef YXC_Status (*func_YXV_FFDecProcessA)(YXV_FFDec dec, const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, YXV_Frame* pSample);
	typedef YXC_Status (*func_YXV_FFEncCreate)(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFEnc* pEnc);
	typedef YXC_Status (*func_YXV_FFEncProcessV)(YXV_FFEnc enc, const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Packet* pPacket);
	typedef YXC_Status (*func_YXV_FFEncProcessA)(YXV_FFEnc enc, const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, yuint32_t* puConverted, YXV_Packet* pPacket);
	typedef YXC_Status (*func_YXV_FFEncReadExtV)(YXV_FFEnc enc, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData);
	typedef YXC_Status (*func_YXV_FFEncReadExtA)(YXV_FFEnc enc, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData);
	typedef YXC_Status (*func_YXV_FFFormatCreate)(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFormatID fmtId, YXV_FFFormat* pFormat);
	typedef YXC_Status (*func_YXV_FFFormatStartWrite)(YXV_FFFormat ffFormat, const char* filename);
	typedef YXC_Status (*func_YXV_FFFormatWriteV)(YXV_FFFormat ffFormat, const YXV_Packet* packet);
	typedef YXC_Status (*func_YXV_FFFormatWriteA)(YXV_FFFormat ffFormat, const YXV_Packet* packet);
	typedef YXC_Status (*func_YXV_FFFormatEndWrite)(YXV_FFFormat ffFormat);
	typedef void (*func_YXV_FFVSpecParamDefault)(YXV_VCodecID vCodecId, ybool_t bIsEnc, YXV_FFVSpecParam* pPar);
	typedef LONG (*func_DecodeH264SPS)(LPBYTE pData/*in*/, ULONG32 uDataSize/*in*/, YXC_SPS* pEJ_SPS/*in, out*/);

	typedef void (*func_av_buffer_unref)(AVBufferRef** buffer);
	typedef void (*func_av_frame_unref)(AVFrame* frame);
	typedef void (*func_av_frame_free)(AVFrame** frame);

	typedef struct SwsContext* (*func_sws_getCachedContext)(struct SwsContext *context,
		int srcW, int srcH, enum AVPixelFormat srcFormat,
		int dstW, int dstH, enum AVPixelFormat dstFormat,
		int flags, SwsFilter *srcFilter,
		SwsFilter *dstFilter, const double *param);

	typedef int (*func_sws_scale)(struct SwsContext *c, const uint8_t *const srcSlice[],
		const int srcStride[], int srcSliceY, int srcSliceH,
		uint8_t *const dst[], const int dstStride[]);

	typedef void (*func_sws_freeContext)(struct SwsContext* context);

	typedef struct SwrContext* (*func_swr_alloc_set_opts)(struct SwrContext *s,
		int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate,
		int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate,
		int log_offset, void *log_ctx);

	typedef void (*func_swr_free)(struct SwrContext **s);
	typedef int (*func_swr_init)(struct SwrContext *s);

	typedef int (*func_swr_convert)(struct SwrContext *s, uint8_t **out, int out_count,
		const uint8_t **in , int in_count);

	extern func_YXV_FFObjConfigV g_fn_YXV_FFObjConfigV;
	extern func_YXV_FFObjConfigA g_fn_YXV_FFObjConfigA;
	extern func_YXV_FFObjClose g_fn_YXV_FFObjClose;
	extern func_YXV_FFDecCreate g_fn_YXV_FFDecCreate;
	extern func_YXV_FFDecProcessV g_fn_YXV_FFDecProcessV;
	extern func_YXV_FFDecProcessA g_fn_YXV_FFDecProcessA;
	extern func_YXV_FFEncCreate g_fn_YXV_FFEncCreate;
	extern func_YXV_FFEncProcessV g_fn_YXV_FFEncProcessV;
	extern func_YXV_FFEncProcessA g_fn_YXV_FFEncProcessA;
	extern func_YXV_FFEncReadExtV g_fn_YXV_FFEncReadExtV;
	extern func_YXV_FFEncReadExtA g_fn_YXV_FFEncReadExtA;
	extern func_YXV_FFFormatCreate g_fn_YXV_FFFormatCreate;
	extern func_YXV_FFFormatStartWrite g_fn_YXV_FFFormatStartWrite;
	extern func_YXV_FFFormatWriteV g_fn_YXV_FFFormatWriteV;
	extern func_YXV_FFFormatWriteA g_fn_YXV_FFFormatWriteA;
	extern func_YXV_FFFormatEndWrite g_fn_YXV_FFFormatEndWrite;
	extern func_YXV_FFVSpecParamDefault g_fn_YXV_FFVSpecParamDefault;
	extern func_DecodeH264SPS g_fn_DecodeH264SPS;
	extern func_av_buffer_unref g_fn_av_buffer_unref;
	extern func_av_frame_unref g_fn_av_frame_unref;
	extern func_av_frame_free g_fn_av_frame_free;

	extern func_sws_getCachedContext g_fn_sws_getCachedContext;
	extern func_sws_scale g_fn_sws_scale;
	extern func_sws_freeContext g_fn_sws_freeContext;
	extern func_swr_alloc_set_opts g_fn_swr_alloc_set_opts;
	extern func_swr_init g_fn_swr_init;
	extern func_swr_convert g_fn_swr_convert;
	extern func_swr_free g_fn_swr_free;
}

#endif /* __INNER_INC_YXC_AV_DYNAMIC_FFWRAP_H__ */

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_AV_UTILS_H__
#define __INC_YXC_AV_UTILS_H__

#define YXV_IMAGE_INTER_DEFAULT 0
#define YXV_IMAGE_INTER_LINEAR 1
#define YXV_IMAGE_INTER_NN 2

#define YXV_MAX_NUM_PLANARS 6

#define YXV_REFTIME_PER_SEC (yint64_t)(1000 * 10000)

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	typedef enum __YXV_VIDEO_CODEC_ID
	{
		YXV_VCODEC_ID_NONE = 0,
		YXV_VCODEC_ID_H264,
		YXV_VCODEC_ID_EQV1, /* Fast ej-qvga 1.0. */
		YXV_VCODEC_ID_WMV3, /* WMV3 encoding, supported on windows. */
		YXV_VCODEC_ID_COPY, /* encoding copy from source. */
		YXV_VCODEC_ID_VP8, /* vp8, supported on windows. */
		YXV_VCODEC_ID_MPEG2,
		YXV_VCODEC_ID_MJPEG,
		YXV_VCODEC_ID_JPEG2000,
		YXV_VCODEC_ID_JPEGLS,
		YXV_VCODEC_ID_GIF,
		YXV_VCODEC_ID_RV10,
		YXV_VCODEC_ID_RV20,
		YXV_VCODEC_ID_RV30,
		YXV_VCODEC_ID_RV40,
		YXV_VCODEC_ID_BMP,
		YXV_VCODEC_ID_PNG,
		YXV_VCODEC_ID_UNKNOWN, /* Unknown codec. */
	}YXV_VCodecID;

	typedef enum __YXV_FILE_FORMAT_ID
	{
		YXV_FILE_FORMAT_ID_NONE = 0,
		YXV_FILE_FORMAT_ID_MP4,
		YXV_FILE_FORMAT_ID_H264,
		YXV_FILE_FORMAT_ID_FLV,
		YXV_FILE_FORMAT_ID_M3U8,
		YXV_FILE_FORMAT_ID_ASF,
		YXV_FILE_FORMAT_ID_AVI,
		YXV_FILE_FORMAT_ID_RMVB,
		YXV_FILE_FORMAT_ID_QVGA,
		YXV_FILE_FORMAT_ID_MKV,
		YXV_FILE_FORMAT_ID_RTP,
		YXV_FILE_FORMAT_ID_RTSP,
		YXV_FILE_FORMAT_ID_RTMP,
		YXV_FILE_FORMAT_ID_IMAGE2,
		YXV_FILE_FORMAT_ID_UNKNOWN,
	}YXV_FFormatID;

	typedef enum __YXV_PIX_FMT
	{
		YXV_PIX_FMT_NONE = 0,
		YXV_PIX_FMT_TYPE_BGR = 0x1000000,
		YXV_PIX_FMT_TYPE_YUV_FULL = 0x2000000,
		YXV_PIX_FMT_TYPE_HSL = 0x4000000,
		YXV_PIX_FMT_TYPE_YUV_HALF = 0x8000000,
		YXV_PIX_FMT_TYPE_YUV = 0x10000000,
		YXV_PIX_FMT_TYPE_MASK = 0xff000000,
		YXV_PIX_FMT_BGR24 = YXV_PIX_FMT_TYPE_BGR,
		YXV_PIX_FMT_RGB24,
		YXV_PIX_FMT_RGB0,
		YXV_PIX_FMT_BGR0,
		YXV_PIX_FMT_0RGB,
		YXV_PIX_FMT_0BGR,
		YXV_PIX_FMT_RGBA,
		YXV_PIX_FMT_ARGB,
		YXV_PIX_FMT_BGRA,
		YXV_PIX_FMT_ABGR,
		YXV_PIX_FMT_YUV420P = YXV_PIX_FMT_TYPE_YUV_HALF,
		YXV_PIX_FMT_NV12,
		YXV_PIX_FMT_YUVA420P,
		YXV_PIX_FMT_YUVJ420P,
		YXV_PIX_FMT_YUVJ444P = YXV_PIX_FMT_TYPE_YUV_FULL,
		YXV_PIX_FMT_YUVA444P,
		YXV_PIX_FMT_YUV444P,
		YXV_PIX_FMT_YUYV = YXV_PIX_FMT_TYPE_YUV,
		YXV_PIX_FMT_UYVY,
		YXV_PIX_FMT_YVYU,
	}YXV_PixFmt;

	typedef enum __YXV_BUFFER_TYPE
	{
		YXV_BUFFER_TYPE_NONE = 0,
		YXV_BUFFER_TYPE_FF_FRAME,
		YXV_BUFFER_TYPE_FF_PACKET,
		YXV_BUFFER_TYPE_C_RUNTIME,
		YXV_BUFFER_TYPE_PNCMP,
	}YXV_BufType;

	typedef void (*YXV_BufferFreeFunc)(void* ptr);

	typedef struct __YXV_BUFFER
	{
		YXV_BufType type;
		void* p;
		yuint32_t sz;
		volatile yuint32_t uRefCount;
		YXV_BufferFreeFunc free_func;
		void* c;
	}YXV_Buffer;

#if YXC_PLATFORM_WIN
	typedef RECT YXV_Rect;
	typedef POINT YXV_Point;
#else
	typedef struct __YXV_RECT
	{
		int left;
		int top;
		int right;
		int bottom;
	}YXV_Rect;

	typedef struct __YXV_POINT
	{
		int x;
		int y;
	}YXV_Point;
#endif /* YXC_PLATFORM_WIN */

	typedef enum __YXV_AUDIO_CODEC_ID
	{
		YXV_ACODEC_ID_NONE = 0,
		YXV_ACODEC_ID_AAC,
		YXV_ACODEC_ID_MP3,
		YXV_ACODEC_ID_OPUS,
		YXV_ACODEC_ID_VORBIS,
		YXV_ACODEC_ID_WMA2,
		YXV_ACODEC_ID_WMAPRO,
		YXV_ACODEC_ID_COPY, /* encoding copy from source. */
		YXV_ACODEC_ID_G722,
		YXV_ACODEC_ID_PCM_ALAW,
		YXV_ACODEC_ID_COOK, /* RMVB audios. */
		YXV_ACODEC_ID_UNKNOWN,
	}YXV_ACodecID;

	typedef enum __YXV_SAMPLE_FMT
	{
		YXV_SAMPLE_FMT_NONE = 0,
		YXV_SAMPLE_FMT_TYPE_PLANAR = 0x01000000,
		YXV_SAMPLE_FMT_TYPE_NORMAL = 0x02000000,
		YXV_SAMPLE_FMT_TYPE_MASK = 0xff000000,
		YXV_SAMPLE_FMT_BITS_8 = 0x00080000,
		YXV_SAMPLE_FMT_BITS_16 = 0x00100000,
		YXV_SAMPLE_FMT_BITS_32 = 0x00200000,
		YXV_SAMPLE_FMT_BITS_MASK = 0x00ff0000,
		YXV_SAMPLE_FMT_16S = YXV_SAMPLE_FMT_TYPE_NORMAL | YXV_SAMPLE_FMT_BITS_16,
		YXV_SAMPLE_FMT_32S = YXV_SAMPLE_FMT_TYPE_NORMAL | YXV_SAMPLE_FMT_BITS_32,
		YXV_SAMPLE_FMT_FLT,
		YXV_SAMPLE_FMT_FLTP = YXV_SAMPLE_FMT_TYPE_PLANAR | YXV_SAMPLE_FMT_BITS_32,
	}YXV_SampleFmt;

	typedef enum __YXV_FRAME_TYPE
	{
		YXV_FRAME_TYPE_NONE = 0,
		YXV_FRAME_TYPE_I,
		YXV_FRAME_TYPE_P,
		YXV_FRAME_TYPE_B,
		YXV_FRAME_TYPE_A, /* Audio data. */
	}YXV_FrameType;

	typedef struct __YXV_PACKET
	{
		ybyte_t* pData;
		yuint32_t uStreamIndex;
		yuint32_t uDataLen;
		ybool_t bKeyPacket;
		ybool_t bNoPts;
		yint64_t uRefTime;
		yint64_t uDecTime;

		YXV_Buffer* buffer;
	}YXV_Packet;

	typedef struct __YXV_PICTURE_DESC
	{
		YXV_PixFmt pixFmt;

		yuint32_t w;
		yuint32_t h;
		yuint32_t uPixAlign;
	}YXV_PicDesc;

	typedef struct __YXV_SAMPLE_DESC
	{
		YXV_SampleFmt sampleFmt;

		yuint32_t uNumChannels;
		yuint32_t uSampleRate;
	}YXV_SampleDesc;

	typedef struct __YXV_VIDEO_PARAM
	{
		YXV_PicDesc desc;
		yuint32_t uBitrate;
		yuint32_t uKeyInter;
		yuint32_t uFPS;
		yuint32_t uNumThreads;
	}YXV_VParam;

	/* raw pcm only. */
	typedef struct __YXV_AUDIO_PARAM
	{
		YXV_SampleDesc desc;
		yuint32_t uBitrate;
		yuint32_t uBlockAlign;
	}YXV_AParam;

	typedef struct __YXV_AV_PIC_DRAW_PARAM
	{
		ybool_t bZoom;
		YXV_Rect partRect;
		double opacity;
		yuint32_t interpolation;
	}YXV_PicDrawParam;

	typedef struct __YXV_FRAME
	{
		ybyte_t* pData[YXV_MAX_NUM_PLANARS]; // at most 6 planars data.
		yint32_t uSize[YXV_MAX_NUM_PLANARS];
		yint64_t uRefTime;

		yuint8_t uRatioW[YXV_MAX_NUM_PLANARS];
		yuint8_t uRatioH[YXV_MAX_NUM_PLANARS];
		yuint8_t uPixSize[YXV_MAX_NUM_PLANARS];

		YXV_FrameType fType;
		YXV_Buffer* buffer;
		yuint32_t uNumPlanars;

		union
		{
			yuint32_t uNumSamples;
			yuint32_t bGotFrame;
		};

		union
		{
			YXV_PicDesc picDesc;
			YXV_SampleDesc sampleDesc;
		}u1;
	}YXV_Frame;

	typedef enum __YXV_FILTER_IMPL_TYPE
	{
		YXV_VFILTER_TYPE_UNKNOWN = 0,
		YXV_VFILTER_TYPE_HDC,
		YXV_VFILTER_TYPE_FFMPEG
	}YXV_VFilterType;

	typedef struct __YXV_TEXT_FONT_DESC
	{
		char szFace[64];
		yuint32_t uFontW;
		yuint32_t uFontH;
	}YXV_TFontDesc;

	typedef struct __YXV_STREAM_INFO
	{
		yuint64_t uDuration;
	}YXV_StreamInfo;

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_TFont);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_SourceContent);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_Window);

	typedef enum __YXV_AV_SOURCE_TYPE
	{
		YXV_AV_SOURCE_UNKNOWN = 0,
		YXV_AV_SOURCE_PICTURE = 3,
		YXV_AV_SOURCE_AV_FILE = 4,
		YXV_AV_SOURCE_TEXT = 5,
		YXV_AV_SOURCE_VIDEO_CAPTURE = 6,
		YXV_AV_SOURCE_AUDIO_CAPTURE = 7,
	}YXV_AVSourceType;

	typedef struct __YXV_AV_TEXT_SOURCE_DESC
	{
		YXV_TFontDesc fontDesc;

		char* pszText;

		/* 1 ~ 9 align value(little keyboard align). */
		yuint32_t uTextAlign;
		yuint32_t uTextColor;
	}YXV_AVTextSourceDesc;

	typedef struct __YXV_AV_IMAGE_SOURCE_DESC
	{
		YXC_FPathA imgPath;

		YXV_PicDrawParam picInfo;
	}YXV_AVImageSourceDesc;

	typedef struct __YXV_AV_IMAGE_SCALAR
	{
		ybyte_t byScalar[YXV_MAX_NUM_PLANARS];
	}YXV_Scalar;

	typedef struct __YXV_AV_MEDIA_FILE_DESC
	{
		YXC_FPathA mfPath;

		YXV_PicDrawParam picInfo;
		double dblAudioRatio;
		ybool_t bNoVideo;
	}YXV_AVMediaFileDesc;

	typedef enum __YXV_AV_EFFECT_TYPE
	{
		YXV_AV_EFFECT_TYPE_UNKNOWN = 0,
		YXV_AV_EFFECT_TYPE_FADE_IN,
		YXV_AV_EFFECT_TYPE_FADE_OUT,
	}YXV_AVEffectType;

	typedef struct __YXV_AV_SOURCE
	{
		YXV_AVSourceType sourceType;

		yint64_t i64Start;
		yint64_t i64End;
		yint64_t i64StartOff;

		double fltX;
		double fltY;
		double fltW;
		double fltH;

		ybool_t bXInRatio;
		ybool_t bYInRatio;
		ybool_t bWInRatio;
		ybool_t bHInRatio;
		ybool_t bHReverse;
		ybool_t bWReverse;

		union
		{
			YXV_AVTextSourceDesc textDesc;
			YXV_AVImageSourceDesc imageDesc;
			YXV_AVMediaFileDesc mfDesc;
		};

		YXV_AVEffectType inEffectType;

		/* End time of in effect. */
		yint64_t i64EffectInEnd;

		YXV_AVEffectType outEffectType;

		/* Start time of out effect. */
		yint64_t i64EffectOutStart;

		YXV_SourceContent element; /* Reserved for inner use and should be initialized to NULL. */
	}YXV_AVSource;


	YXC_API(YXC_Status) YXV_BufferAlloc(YXV_BufType type, void* ptr, yuint32_t sz, YXV_Buffer** ppBuffer);

	YXC_API(YXC_Status) YXV_BufferAlloc_C(YXV_BufType type, void* ptr, void* c, yuint32_t sz, YXV_Buffer** ppBuffer);

	YXC_API(void) YXV_BufferUnref(YXV_Buffer* pBuffer);

	YXC_API(YXC_Status) YXV_VFrameAlloc(const YXV_PicDesc* pDesc, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_VFrameFill(const YXV_PicDesc* pDesc, ybyte_t* byBuffer, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_VFrameCopy(const YXV_Frame* pSrc, YXV_Frame* pDst);

	YXC_API(YXC_Status) YXV_VFrameCalcSize(YXV_Frame* f, yuint32_t* pSize);

	YXC_API(YXC_Status) YXV_WindowCreate(const ychar* name, YXV_Window* pOut);

	YXC_API(YXC_Status) YXV_WindowRefVFrame(YXV_Window window, YXV_Frame* frame);

	YXC_API(YXC_Window) YXV_WindowGetHandle(YXV_Window window);

	YXC_API(void) YXV_WindowDestroy(YXV_Window window);

	YXC_API(void) YXV_VFrameROI(const YXV_Frame* f1, yuint32_t offX, yuint32_t offY, yuint32_t w, yuint32_t h, YXV_Frame* f2);

	YXC_API(YXC_Status) YXV_VFrameFloodReplace(YXV_Frame* vf, const YXV_Scalar* c1, const YXV_Scalar* cOut, yint32_t iDiff);

	YXC_API(YXC_Status) YXV_VFrameFloodReplace2(YXV_Frame* vf, const YXV_Scalar* c1, const YXV_Frame* background, yint32_t iDiff);

	YXC_API(YXC_Status) YXV_AFrameAlloc(const YXV_SampleDesc* pDesc, yuint32_t uMaxSamples, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_AFrameFill(const YXV_SampleDesc* pDesc, ybyte_t* byBuffer, yuint32_t uNumSamples, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_AFrameCopy(YXV_Frame* aSampleDst, yuint32_t uDstIndex, const YXV_Frame* aSampleSrc,
		yuint32_t uSrcIndex, yuint32_t uNumSamplesToCopy);

	YXC_API(void) YXV_FrameRef(YXV_Frame* sample);

	YXC_API(void) YXV_FrameUnref(YXV_Frame* sample);

	YXC_API(void) YXV_FrameRef(YXV_Frame* sample);

	YXC_API(void) YXV_FrameZero(YXV_Frame* sample,ybool_t blend=TRUE);

	YXC_API(void) YXV_PacketRef(YXV_Packet* packet);

	YXC_API(void) YXV_PacketUnref(YXV_Packet* packet);

	YXC_API(void) YXV_VFrameBK(YXV_Frame* sample, YXV_Scalar* pscalar);

	typedef struct __YXV_CAPTURE_FRAME_EX
	{
		yint64_t refTime;
		int nWidth;
		int nHeight;
		YXV_PixFmt pixFmt;
	}YXV_CaptureFrameEx;

	typedef struct __YXV_CAPTURE_AUDIO_EX
	{
		yint64_t refTime;
		int nChannels;
		int nFreq;
		int nBitsPerSample;
		yint64_t captureTime;
	}YXV_CaptureAudioEx;

	typedef enum __YXV_MEDIA_PLAY_STATUS
	{
		YXV_MEDIA_PLAY_STATUS_UNKNOWN = 0,
		YXV_MEDIA_PLAY_STATUS_PLAYING = 1,
		YXV_MEDIA_PLAY_STATUS_PAUSED = 2,
		YXV_MEDIA_PLAY_STATUS_STOPPED = 3,
		YXV_MEDIA_PLAY_STATUS_COMPLETED = 4,
		YXV_MEDIA_PLAY_STATUS_BUFFERING = 5,
	}YXV_MediaPlayStatus;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
static inline int YXV_ImageAlignRow(int nWidthBytes, int nAlign YXC_DEF_PARAM(4))
{
	return ((nWidthBytes - 1) / nAlign + 1) * nAlign;
}
#endif /* __cplusplus */

#endif /* __INC_YXC_AV_UTILS_H__ */

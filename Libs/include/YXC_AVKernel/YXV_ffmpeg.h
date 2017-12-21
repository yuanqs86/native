/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXV_FFMPEG_H__
#define __INC_YXV_FFMPEG_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_AVKernel/YXV_AVUtils.h>

#define YXV_ENCDEC_NAME 32

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_FFObject);
	YXC_DECLARE_INHERIT_HANDLE(YXV_FFEnc, YXV_FFObject);
	YXC_DECLARE_INHERIT_HANDLE(YXV_FFDec, YXV_FFObject);
	YXC_DECLARE_INHERIT_HANDLE(YXV_FFFormat, YXV_FFObject);

	typedef struct __YXV_EXTRA_DATA
	{
		ybyte_t* pExt;
		yuint32_t uExt;
		yuint32_t uExtBuf;
	}YXV_FFExtraData;

	typedef struct __YXV_X264_H264_PARAM
	{
		int repeat_headers;
		int profile;
		int level_idc;
		int frame_reference;
		char bframe_pyramid[20];
		int intra_refresh;
		int csp;
		char cqm_preset[20];
		int cabac;
		int bframes;
		int interlaced;
		int fakeinterlaced;
		int vfr_input;
		int nal_hrd;
		int pic_struct;

		char rc_method[20];
		int qp_constant;

		int transform_8x8;
		char weighted_pred[20];
		int mv_range;

		int sar_width;
		int sar_height;
		char colorprim[20];
		char transfer[2];
		char colmatrix[20];
		char overscan[20];
		char vidformat[20];
		char fullrange[20];
		char preset[20];
		int chormaloc;
	}YXV_FFX264_H264Param;

	typedef struct __YXV_OPUS_PARAM
	{
		double dblRatio;
	}YXV_FFOpusParam;

	typedef struct __YXV_VPX_VP8_PARAM
	{
		int profile;
		int speed_profile;
	}YXV_FFVPX_VP8Param;

	typedef struct __YXV_VIDEO_SPEC_PARAM
	{
		char szEncDecName[YXV_ENCDEC_NAME];
		union
		{
			YXV_FFX264_H264Param x264_h264Param;
			YXV_FFVPX_VP8Param vpx_vp8Param;
			YXV_FFExtraData extData;
		};
	}YXV_FFVSpecParam;

	typedef struct __YXV_FAAC_AAC_PARAM
	{
		yuint32_t uProfile;
	}YXV_FFFaac_AacParam;

	typedef struct __YXV_AUDIO_SPEC_PARAM
	{
		char szEncDecName[YXV_ENCDEC_NAME];
		union
		{
			YXV_FFFaac_AacParam faac_aacParam;
			YXV_FFOpusParam opusParam;
			YXV_FFExtraData extData;
		};

		yuint32_t uFrameSize;
	}YXV_FFASpecParam;

	typedef enum __YXV_FORMAT_OPTION
	{
		YXV_FORMAT_OPT_UNKNOWN = 0,
		YXV_FORMAT_OPT_SDP = 1,
	}YXV_FFFormatOpt;

	typedef YXV_FFASpecParam YXV_FFAFormatParam[2];
	typedef YXV_FFVSpecParam YXV_FFVFormatParam[2];

	YXC_API(YXC_Status) YXV_FFObjConfigV(YXV_FFObject obj, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd);

	YXC_API(YXC_Status) YXV_FFObjConfigA(YXV_FFObject obj, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd);

	YXC_API(void) YXV_FFObjClose(YXV_FFObject obj);

	YXC_API(YXC_Status) YXV_FFDecCreate(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFDec* pDec);

	YXC_API(YXC_Status) YXV_FFDecProcessV(YXV_FFDec dec, const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_FFDecProcessV2(YXV_FFDec dec, const YXV_Packet* pPacket, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_FFDecProcessA(YXV_FFDec dec, const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, YXV_Frame* pSample);

	YXC_API(YXC_Status) YXV_FFEncCreate(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFEnc* pEnc);

	YXC_API(YXC_Status) YXV_FFEncProcessV(YXV_FFEnc enc, const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Packet* pPacket);

	YXC_API(YXC_Status) YXV_FFEncProcessV2(YXV_FFEnc enc, const YXV_Frame* pSample, YXV_Packet* pPacket);

	YXC_API(YXC_Status) YXV_FFEncProcessA(YXV_FFEnc enc, const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, yuint32_t* puConverted, YXV_Packet* pPacket);

	YXC_API(YXC_Status) YXV_FFEncProcessA2(YXV_FFEnc enc, const YXV_Frame* pSample, yuint32_t* puConverted, YXV_Packet* pPacket);

	YXC_API(YXC_Status) YXV_FFEncReadExtV(YXV_FFEnc enc, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData);

	YXC_API(YXC_Status) YXV_FFEncReadExtA(YXV_FFEnc enc, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData);

	YXC_API(YXC_Status) YXV_FFEncReadSpecV(YXV_FFEnc enc, YXV_FFVSpecParam* pvSpec);

	YXC_API(YXC_Status) YXV_FFEncReadSpecA(YXV_FFEnc enc, YXV_FFASpecParam* paSpec);

	YXC_API(YXC_Status) YXV_FFFormatCreateRead(const char* filename, YXV_VCodecID* pvCodecId, YXV_ACodecID* paCodecId,
		YXV_FFormatID* pFmtId, YXV_FFFormat* pFormat);

	YXC_API(YXC_Status) YXV_FFFormatRead(YXV_FFFormat ffFormat, YXV_Packet* packet);

	YXC_API(YXC_Status) YXV_FFFormatSeek(YXV_FFFormat ffFormat, yint64_t timestamp);

	YXC_API(YXC_Status) YXV_FFFormatEndRead(YXV_FFFormat ffFormat);

	YXC_API(YXC_Status) YXV_FFFormatReadParamA(YXV_FFFormat ffFormat, YXV_AParam* pAParam, YXV_FFAFormatParam* pASpecParam);

	YXC_API(YXC_Status) YXV_FFFormatReadParamV(YXV_FFFormat ffFormat, YXV_VParam* pVParam, YXV_FFVFormatParam* pVSpecParam);

	YXC_API(YXC_Status) YXV_FFFormatReadStreamInfo(YXV_FFFormat ffFormat, YXV_StreamInfo* streamInfo);

	YXC_API(YXC_Status) YXV_FFFormatCreate(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFormatID fmtId, YXV_FFFormat* pFormat);

	YXC_API(YXC_Status) YXV_FFFormatStartWrite(YXV_FFFormat ffFormat, const char* filename);

	YXC_API(YXC_Status) YXV_FFFormatWriteV(YXV_FFFormat ffFormat, const YXV_Packet* packet);

	YXC_API(YXC_Status) YXV_FFFormatWriteA(YXV_FFFormat ffFormat, const YXV_Packet* packet);

	YXC_API(YXC_Status) YXV_FFFormatEndWrite(YXV_FFFormat ffFormat);

	YXC_API(YXC_Status) YXV_FFFormatGetOpt(YXV_FFFormat ffFormat, YXV_FFFormatOpt opt, ybyte_t* pOpt, yuint32_t uCbOpt);

	YXC_API(void) YXV_FFVSpecParamDefault(YXV_VCodecID vCodecId, ybool_t bEnc, YXV_FFVSpecParam* pSpecParam);

	YXC_API(void) YXV_FFASpecParamDefault(YXV_ACodecID aCodecId, ybool_t bEnc, YXV_FFASpecParam* pSepcParam);

#ifdef __cplusplus
};
#endif /*__cplusplus*/

#endif /* __INC_YXV_FFMPEG_H__ */

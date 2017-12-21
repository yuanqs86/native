/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_NP_UTILITIES_AV_H__
#define __INC_YXC_BASE_NP_UTILITIES_AV_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_NetSocket.h>
#include <YXC_Sys/YXC_NPBase.h>
#include <YXC_Sys/YXC_NetSModelServer.h>
#include <YXC_Sys/YXC_NPUtilitiesAVCtrl.h>


#if YXC_PLATFORM_WIN
#include <DShow.h> // Include VIDEOINFOHEADER
#endif /* YXC_PLATFORM_WIN */

#define YXC_NP_AV_MAX_PACKAGE_SIZE (6 << 20) // 6M

#define YXC_AVDP_AUDIO_FORMAT 0
#define YXC_AVDP_VIDEO_VI_FORMAT 1
#define YXC_AVDP_VIDEO_BI_FORMAT 2
#define YXC_AVDP_VIDEO_KEY_FRAME 3
#define YXC_AVDP_VIDEO_FRAME 4
#define YXC_AVDP_AUDIO 5
#define YXC_AVDP_VIDEO_H264_FORMAT 6
#define YXC_AVDP_VIDEO_VP8_FORMAT  7
#define YXC_AVDP_VIDEO_HW_FORMAT	  8


// Live server cast information
#define YXC_AVDP_LIVE_CAST_INFO 6

// Teacher machine index info
#define YXC_AVDP_TM_INDEX 7

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#pragma pack(push, 1)
	typedef struct __YXC_AV_DP_COMMON_HEADER
	{
		ybyte_t byDataType;
	}YXC_AVDPCmnHeader;

	typedef struct __YXC_AV_DP_VIDEO_FRAME_HEADER
	{
		yuint16_t uVideoId;
		yuint64_t u64Time;
		yuint16_t uPackageId;
		yuint16_t uNumSubPackages;
		yuint16_t uSubPackageId;
	}YXC_AVDPVideoFrameHeader;

	typedef struct __YXC_AV_DP_H264_HEADER
	{
		yuint16_t uVideoId;
		yuint32_t uCbSps;
		yuint32_t uCbPps;
	}YXC_AVDPH264Header;

	typedef struct __YXC_AV_DP_AUDIO_HEADER
	{
		yuint16_t uAudioId;
		yuint64_t u64Time;
	}YXC_AVDPAudioHeader;

	typedef struct __YXC_AV_DP_TM_INDEX_HEADER
	{
		yuint64_t u64Time;
	}YXC_AVDPTMIndexHeader;
#pragma pack(pop)

#define YXC_AVDP_H264_HEADER_SIZE 200
#if YXC_PLATFORM_WIN

#define YXC_AVDP_VI_EXTRA_SIZE 64 /* Contain sps, pps. */
#define YXC_AVDP_WF_EXTRA_SIZE 32 /* Contain mp3, aac extradata. */
	typedef struct __YXC_AV_VIDEO_VI_FORMAT
	{
		yuint16_t uVideoId;
		VIDEOINFOHEADER viHeader;
		ybyte_t byExtra[YXC_AVDP_VI_EXTRA_SIZE]; // for extra header info
	}YXC_AVVideoViFormat;

	typedef struct __YXC_AV_VIDEO_BI_FORMAT
	{
		yuint16_t uVideoId;
		union
		{
			BITMAPINFOHEADER biHeader;
			BITMAPV4HEADER biV4Header;
			BITMAPV5HEADER biV5Header;
		}u1;
	}YXC_AVVideoBiFormat;

	typedef enum __YXC_AV_PACKAGE_SRC
	{
		YXC_AV_PACKAGE_SRC_UNKNOWN = 0,
		YXC_AV_PACKAGE_SRC_RECORD = 1,
		YXC_AV_PACKAGE_SRC_TM = 2,
		YXC_AV_PACKAGE_SRC_DIRECTOR = 3,
		YXC_AV_PACKAGE_SRC_LIVE = 4,
		YXC_AV_PACKAGE_SRC_LIVE_CLIENT = 5,
		YXC_AV_PACKAGE_SRC_CENTRAL_CTRL = 6,
		YXC_AV_PACKAGE_SRC_ENCODER = 7,
		YXC_AV_PACKAGE_SRC_PREVIEWER = 8,
		YXC_AV_PACKAGE_SRC_TRANSFORM = 9,
	}YXC_AVPackageSrc;

	typedef struct __YXC_AV_AUDIO_FORMAT
	{
		yuint16_t uAudioId;
		WAVEFORMATEX waveFormat;
		ybyte_t byExtra[YXC_AVDP_WF_EXTRA_SIZE];
	}YXC_AVAudioFormat;

#endif /* YXC_PLATFORM_WIN */

	typedef struct __YXC_AV_VIDEO_H264_FORMAT
	{
		yuint16_t uVideoId;
		yuint32_t uCbSps;
		yuint32_t uCbPps;

		ybyte_t byHdrData[YXC_AVDP_H264_HEADER_SIZE];
	}YXC_AVVideoH264Format;

	typedef struct __YXC_AV_VIDEO_FRAME_HEADER
	{
		yuint16_t uVideoId;
		yuint64_t u64Time;
		yuint16_t uPackageId;
		yuint16_t uNumSubPackages;
		yuint16_t uSubPackageId;
		ybool_t bIsKeyFrame;
		yuint32_t uDataLen;
		void* pvData;
	}YXC_AVVideoFramePack;

	typedef struct __YXC_AV_AUDIO_HEADER_PACKAGE
	{
		yuint16_t uAudioId;
		yuint64_t u64Time;
		yuint32_t uDataLen;
		void* pvData;
	}YXC_AVAudio;

	typedef struct __YXC_AV_TM_INDEX
	{
		yuint64_t u64Time;
		yuint32_t uDataLen;
		void* pvData;
	}YXC_AVTMIndex;

	typedef struct __YXC_AV_HEADER
	{
		ybyte_t byDataType;
		yuint16_t uId;
		yuint64_t u64Time;
	}YXC_AVHeader;

	typedef struct __YXC_AV_DATA_PACKAGE
	{
		ybyte_t byDataType;
		yuint32_t uDataLen;

		union
		{
			YXC_AVVideoViFormat viFormat;
			YXC_AVVideoH264Format h264Format;
			YXC_AVVideoBiFormat biFormat;
			YXC_AVAudioFormat auFormat;
			YXC_AVTMIndex tmIndex;
			YXC_AVVideoFramePack vfPack;
			YXC_AVAudio audio;
		}u1;
	}YXC_AVDataPackage;

	YXC_API(void) YXC_AVDPSealVideoFrame(ybool_t bFromServer, const YXC_AVVideoFramePack* pVfPack, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(void) YXC_AVDPSealAudio(ybool_t bFromServer, const YXC_AVAudio* pAudio, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(void) YXC_AVDPSealVideoViFormat(ybool_t bFromServer, const YXC_AVVideoViFormat* pViFormat, 	YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(void) YXC_AVDPSealVideoBiFormat(ybool_t bFromServer, const YXC_AVVideoBiFormat* pBiFormat, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(void) YXC_AVDPSealAudioFormat(ybool_t bFromServer, const YXC_AVAudioFormat* pAuFormat, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(void) YXC_AVDPSealTMIndex(ybool_t bFromServer, const YXC_AVTMIndex* pIndex, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(void) YXC_AVDPSealVideoH264Format(ybool_t bFromServer, const YXC_AVVideoH264Format* pH264Format, YXC_NetPackage* pPackage, YXC_NetTransferInfo* pTransInfo);

	YXC_API(YXC_Status) YXC_AVDPParseVideoFrame(const YXC_NetPackage* pPackage, YXC_AVVideoFramePack* pVfPack);

	YXC_API(YXC_Status) YXC_AVDPParseAudio(const YXC_NetPackage* pPackage, YXC_AVAudio* pAudio);

	YXC_API(YXC_Status) YXC_AVDPParseVideoViFormat(const YXC_NetPackage* pPackage, YXC_AVVideoViFormat* pViFormat);

	YXC_API(YXC_Status) YXC_AVDPParseVideoBiFormat(const YXC_NetPackage* pPackage, YXC_AVVideoBiFormat* pBiFormat);

	YXC_API(YXC_Status) YXC_AVDPParseAudioFormat(const YXC_NetPackage* pPackage, YXC_AVAudioFormat* pAuFormat);

	YXC_API(YXC_Status) YXC_AVDPParseTMIndex(const YXC_NetPackage* pPackage, YXC_AVTMIndex* pTiHeader);

	YXC_API(YXC_Status) YXC_AVDPParsePackage(const YXC_NetPackage* pPackage, YXC_AVDataPackage* pDataPackage);

	YXC_API(YXC_Status) YXC_NetSModelServerBroadcastVideoFrame(YXC_NetSModelServer sServer, yuint32_t uGroupId, const YXC_AVVideoFramePack* pVfPack);

	YXC_API(YXC_Status) YXC_NetSModelServerBroadcastAudio(YXC_NetSModelServer sServer, yuint32_t uGroupId, const YXC_AVAudio* pAudio);

	YXC_API(YXC_Status) YXC_NetSModelServerBroadcastTMIndex(YXC_NetSModelServer sServer, yuint32_t uGroupId, const YXC_AVTMIndex* pTiHeader);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_BASE_NP_UTILITIES_AV_H__ */

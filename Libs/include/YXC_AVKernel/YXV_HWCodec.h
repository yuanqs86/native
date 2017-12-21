#pragma once

/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_AV_HARDWARE_CODEC_H__
#define __INC_YXC_AV_HARDWARE_CODEC_H__

#include <YXC_Sys/YXC_Sys.h>
#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/YXV_ffmpeg.h>

#define _YXV_HW_DEV_NAME_LEN (128 - 1)

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/
	typedef enum __YXV_HARDWARE_DEVICE_TYPE
	{
		YXV_HW_DEV_UNKNOWN = 0,
		YXV_HW_DEV_INTEL_GPU = 1,
		YXV_HW_DEV_NVIDIA_GPU = 2,
		YXV_HW_DEV_AMD_GPU = 3,
	}YXV_HWDevType;

	typedef enum __YXV_HARDWARE_INTERFACE_TYPE
	{
		YXV_HW_IFACE_TYPE_DEFAULT = 0,
		YXV_HW_IFACE_TYPE_DEVICE = 1,
		YXV_HW_IFACE_TYPE_SYSTEM = 2,

#if YXC_SUPPORT_DX
		YXV_HW_IFACE_TYPE_D3D9 = 3,
		YXV_HW_IFACE_TYPE_D3D10 = 4,
		YXV_HW_IFACE_TYPE_D3D11 = 5,
#endif /* YXC_SUPPORT_DX */
	}YXV_HWIFaceType;

	typedef struct __YXV_HARDWARE_ENCODER_INFO
	{
		YXV_PixFmt inputType;
		char szDevName[_YXV_HW_DEV_NAME_LEN + 1];
	}YXV_HWEncoderInfo, YXV_HWDecoderInfo;

	typedef enum __YXV_HARDWARE_ENCODER_PRESET
	{
		YXV_HW_ENCODER_PRESET_NONE = 0,
		YXV_HW_ENCODER_PRESET_FAST = 1,
		YXV_HW_ENCODER_PRESET_BALANCED = 2,
		YXV_HW_ENCODER_PRESET_HIGH_QUALITY = 3,
	}YXV_HWEncoderPreset;

	typedef struct __YXV_HW_PARAM_EXT_INTEL_GPU
	{

	}YXV_HWParIntelGPU;

	typedef struct __YXV_HW_PARAM_EXT_NVIDIA_GPU
	{

	}YXV_HWParNvidiaGPU;

	typedef struct __YXV_HW_ENCODER_CODEC_PARAM
	{
		YXV_VParam vParam;
		yuint32_t uNumBFrames;
		yuint32_t uProfile;
		yuint32_t uCodecDelay;
		YXV_HWEncoderPreset encoderPreset;
		YXV_VCodecID vCodecId;

		union
		{
			YXV_HWParIntelGPU extIntelGPU;
			YXV_HWParNvidiaGPU extNvidiaGPU;
			YXV_FFExtraData extData;
		};
	}YXV_HWCodecParam;

	YXC_DECLARE_STRUCTURE_HANDLE(YXV_HWDev);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_HWEncoder);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_HWDecoder);
	YXC_DECLARE_STRUCTURE_HANDLE(YXV_HWVPP);

	YXC_API YXC_Status YXV_HWDeviceGetNumber(YXV_HWDevType devType, yuint32_t* puNumDevices);

	YXC_API YXC_Status YXV_HWDeviceOpen(YXV_HWDevType devType, yuint32_t uDevIndex, YXV_HWIFaceType ifaceType, YXV_HWDev* dev);

	YXC_API void YXV_HWDeviceClose(YXV_HWDev dev);

	YXC_API YXC_Status YXV_HWEncoderOpen(YXV_HWDev dev, YXV_HWEncoder* encoder);

	YXC_API void YXV_HWEncoderGetInfo(YXV_HWEncoder encoder, YXV_HWEncoderInfo* pInfo);

	YXC_API YXC_Status YXV_HWEncoderConfig(YXV_HWEncoder encoder, const YXV_HWCodecParam* param);

	YXC_API YXC_Status YXV_HWEncoderQueuedInput(YXV_HWEncoder encoder, const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp);

	YXC_API YXC_Status YXV_HWEncoderGetOutput(YXV_HWEncoder encoder, ybyte_t** ppOutputBuf, yuint32_t* puOutputSize,
		ybool_t* pbKey, yint64_t* pTimestamp);

	YXC_API void YXV_HWEncoderClose(YXV_HWEncoder encoder);

	YXC_API void YXV_HWEncoderGetSpsPps(YXV_HWEncoder encoder,ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize);

	YXC_API YXC_Status YXV_HWDecoderOpen(YXV_HWDev dev, YXV_HWDecoder* decoder);

	YXC_API void YXV_HWDecoderGetInfo(YXV_HWDecoder decoder, YXV_HWDecoderInfo* pInfo);

	YXC_API YXC_Status YXV_HWDecoderConfig(YXV_HWDecoder decoder, const YXV_HWCodecParam* param);

	YXC_API YXC_Status YXV_HWDecoderQueuedInput(YXV_HWDecoder decoder, const YXV_Packet* packet);

	YXC_API YXC_Status YXV_HWDecoderGetOutput(YXV_HWDecoder decoder, YXV_Frame* sample);

	YXC_API void YXV_HWDecoderClose(YXV_HWDecoder decoder);

	YXC_API YXC_Status YXV_HWVPPOpen(YXV_HWDev dev, YXV_HWVPP* pVPP);

	YXC_API void YXV_HWVPPClose(YXV_HWVPP vpp);

	YXC_API YXC_Status YXV_HWVPPConfig(YXV_HWVPP vpp, const YXV_HWCodecParam* param_in,const YXV_HWCodecParam* param_out);

	YXC_API YXC_Status YXV_HWRunFrameVPP(YXV_HWVPP vpp, ybyte_t** pbuffer_in, ybyte_t* pbuffer_out);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __INC_YXC_AV_HARDWARE_CODEC_H__ */

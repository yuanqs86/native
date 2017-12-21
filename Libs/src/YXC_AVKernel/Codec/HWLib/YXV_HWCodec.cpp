#define __MODULE__ "EV.HW.Codec"

#include <YXC_AVKernel/YXV_HWCodec.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>

#include <YXC_AVKernel/Codec/HWLib/_YXV_NvidiaGPU.hpp>
#include <YXC_AVKernel/Codec/HWLib/_YXV_HWCodecBase.hpp>
#include <new>
#include <YXC_AVKernel/Codec/HWLib/_YXV_IntelGPU.hpp>

using namespace _YXV_HWLib;

extern "C"
{

	YXC_Status YXV_HWDeviceGetNumber(YXV_HWDevType devType, yuint32_t* puNumDevices)
	{
		switch (devType)
		{
		case YXV_HW_DEV_INTEL_GPU:
			return _YXV_IntelGetDeviceNum(puNumDevices);
		case YXV_HW_DEV_NVIDIA_GPU:
			return _YXV_NvidiaGetDeviceNum(puNumDevices);
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_TYPE, YC("Invliad hw device type %d"), devType);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_HWDeviceOpen(YXV_HWDevType devType, yuint32_t uDevIndex, YXV_HWIFaceType ifaceType, YXV_HWDev* dev)
	{
		_HWDevBase* pDev = NULL;
		switch (devType)
		{
		case YXV_HW_DEV_INTEL_GPU:
			_YCHK_MAL_R2(pDev, _IntelDev);
			new (pDev) _IntelDev();
			break;
		case YXV_HW_DEV_NVIDIA_GPU:
			_YCHK_MAL_R2(pDev, _NvidiaDev);
			new (pDev) _NvidiaDev();
			break;

		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_TYPE, YC("Invliad hw device type %d"), devType);
		}

		YXC_Status rcRet;
		YXC_Status rc = pDev->Init(uDevIndex, ifaceType);
		_YXC_CHECK_RC_GOTO(rc);

		*dev = _HWHdl_D(pDev);

		return YXC_ERC_SUCCESS;
err_ret:
		if (pDev)
		{
			pDev->~_HWDevBase();
			free(pDev);
		}

		return rcRet;
	}

	void YXV_HWDeviceClose(YXV_HWDev dev)
	{
		_HWDevBase* pDev = _HWPtr_D(dev);

		pDev->Close();
		pDev->~_HWDevBase();
		free(pDev);
	}

	YXC_Status YXV_HWEncoderOpen(YXV_HWDev dev, YXV_HWEncoder* encoder)
	{
		_HWDevBase* pDev = _HWPtr_D(dev);
		_HWEncoderBase* pEnc = NULL;

		YXC_Status rc = pDev->CreateEncoder(&pEnc);
		_YXC_CHECK_RC_RET(rc);

		*encoder = _HWHdl_E(pEnc);
		return YXC_ERC_SUCCESS;
	}

	void YXV_HWEncoderClose(YXV_HWEncoder encoder)
	{
		_HWEncoderBase* pEnc = _HWPtr_E(encoder);

		pEnc->Close();
		pEnc->~_HWEncoderBase();
		free(pEnc);
	}

	YXC_Status YXV_HWEncoderConfig(YXV_HWEncoder encoder, const YXV_HWCodecParam* param)
	{
		_HWEncoderBase* pEnc = _HWPtr_E(encoder);

		YXC_Status rc = pEnc->Config(param);
		return rc;
	}

	YXC_Status YXV_HWEncoderQueuedInput(YXV_HWEncoder encoder, const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp)
	{
		_HWEncoderBase* pEnc = _HWPtr_E(encoder);

		YXC_Status rc = pEnc->QueuedInput(pic_buffer, fType, timestamp);
		return rc;
	}

	void YXV_HWEncoderGetInfo(YXV_HWEncoder encoder, YXV_HWEncoderInfo* pInfo)
	{
		_HWEncoderBase* pEnc = _HWPtr_E(encoder);

		pEnc->GetInfo(pInfo);
	}

	YXC_Status YXV_HWEncoderGetOutput(YXV_HWEncoder encoder, ybyte_t** ppOutputBuf, yuint32_t* puOutputSize, ybool_t* pbKey, yint64_t* pTimestamp)
	{
		_HWEncoderBase* pEnc = _HWPtr_E(encoder);

		YXC_Status rc = pEnc->GetOutput(ppOutputBuf, puOutputSize, pbKey, pTimestamp);
		return rc;
	}

	void YXV_HWEncoderGetSpsPps(YXV_HWEncoder encoder,ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize)
	{
		_HWEncoderBase* pEnc = _HWPtr_E(encoder);

		pEnc->GetSpsPps(ppSps, ppPps, pSpsSize, pPpsSize);
	}

	YXC_Status YXV_HWDecoderOpen(YXV_HWDev dev, YXV_HWDecoder* decoder)
	{
		_HWDevBase* pDev = _HWPtr_D(dev);
		_HWDecoderBase* pDec = NULL;

		YXC_Status rc = pDev->CreateDecoder(&pDec);
		_YXC_CHECK_RC_RET(rc);

		*decoder = _HWHdl_X(pDec);
		return YXC_ERC_SUCCESS;
	}

	void YXV_HWDecoderClose(YXV_HWDecoder decoder)
	{
		_HWDecoderBase* pDec = _HWPtr_X(decoder);

		pDec->Close();
		pDec->~_HWDecoderBase();
		free(pDec);
	}

	YXC_Status YXV_HWDecoderConfig(YXV_HWDecoder decoder, const YXV_HWCodecParam* param)
	{
		_HWDecoderBase* pDec = _HWPtr_X(decoder);

		YXC_Status rc = pDec->Config(param);
		return rc;
	}

	YXC_Status YXV_HWDecoderQueuedInput(YXV_HWDecoder decoder, const YXV_Packet* packet)
	{
		_HWDecoderBase* pDec = _HWPtr_X(decoder);

		YXC_Status rc = pDec->QueuedInput(packet);
		return rc;
	}

	void YXV_HWDecoderGetInfo(YXV_HWDecoder decoder, YXV_HWDecoderInfo* pInfo)
	{
		_HWDecoderBase* pDec = _HWPtr_X(decoder);

		pDec->GetInfo(pInfo);
	}

	YXC_Status YXV_HWDecoderGetOutput(YXV_HWDecoder decoder, YXV_Frame* pSample)
	{
		_HWDecoderBase* pDec = _HWPtr_X(decoder);

		YXC_Status rc = pDec->GetOutput(pSample);
		return rc;
	}
	YXC_Status YXV_HWVPPOpen(YXV_HWDev dev, YXV_HWVPP* pVPP)
	{
		_HWDevBase* pDev = _HWPtr_D(dev);
		_HWVPPBase* pvpp = NULL;

		YXC_Status rc = pDev->CreateVPP(&pvpp);
		_YXC_CHECK_RC_RET(rc);

		*pVPP = _HWHdl_V(pvpp);
		return YXC_ERC_SUCCESS;
	}

	void YXV_HWVPPClose(YXV_HWVPP vpp)
	{
		_HWVPPBase* pVPP = _HWPtr_V(vpp);

		pVPP->Close();
		pVPP->~_HWVPPBase();
		free(pVPP);
	}

	YXC_Status YXV_HWVPPConfig(YXV_HWVPP vpp, const YXV_HWCodecParam* param_in,const YXV_HWCodecParam* param_out)
	{
		_HWVPPBase* pVPP = _HWPtr_V(vpp);

		YXC_Status rc = pVPP->Config(param_in,param_out);
		return rc;
	}

	YXC_Status YXV_HWRunFrameVPP(YXV_HWVPP vpp, ybyte_t** pbuffer_in,ybyte_t* pbuffer_out)
	{
		_HWVPPBase* pVPP = _HWPtr_V(vpp);

		YXC_Status rc = pVPP->RunFrameVPP(pbuffer_in,pbuffer_out);
		return rc;
	}
};

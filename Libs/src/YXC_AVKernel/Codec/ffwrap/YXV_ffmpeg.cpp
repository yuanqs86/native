#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/Codec/ffwrap/ffenc.hpp>
#include <YXC_AVKernel/Codec/ffwrap/ffdec.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/Codec/ffwrap/ffformat.hpp>
#include <new>
#include <vpx/vpx_encoder.h>

using namespace YXV_FFWrap;

extern "C"
{
	YXC_Status YXV_FFEncCreate(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFEnc* pEnc)
	{
		_YCHK_MAL_R1(pFFEnc, _FFEnc);
		new (pFFEnc) _FFEnc();

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = pFFEnc->FFInit(vCodecId, aCodecId);
		_YXC_CHECK_RC_GOTO(rc);

		*pEnc = _FFEncHdl(pFFEnc);
		return YXC_ERC_SUCCESS;
err_ret:
		pFFEnc->~_FFEnc();
		return rcRet;
	}

	YXC_Status YXV_FFEncProcessA(YXV_FFEnc enc, const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, yuint32_t* puConverted, YXV_Packet* pPacket)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ProcessA(pbyData, uCbData, uRefTime, puConverted, pPacket);
	}

	YXC_Status YXV_FFEncProcessV(YXV_FFEnc enc, const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Packet* pPacket)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ProcessV(pbyData, uCbData, fType, uRefTime, pPacket);
	}

	YXC_Status YXV_FFEncProcessV2(YXV_FFEnc enc, const YXV_Frame* pSample, YXV_Packet* pPacket)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ProcessV2(pSample, pPacket);
	}

	YXC_Status YXV_FFEncProcessA2(YXV_FFEnc enc, const YXV_Frame* pSample, yuint32_t* puConverted, YXV_Packet* pPacket)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ProcessA2(pSample, puConverted, pPacket);
	}

	YXC_Status YXV_FFEncReadExtV(YXV_FFEnc enc, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ReadExtraDataV(pBuf, uCbBuf, puCbData);
	}

	YXC_Status YXV_FFEncReadExtA(YXV_FFEnc enc, ybyte_t* pBuf, yuint32_t uCbBuf, yuint32_t* puCbData)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ReadExtraDataA(pBuf, uCbBuf, puCbData);
	}

	YXC_Status YXV_FFEncReadSpecV(YXV_FFEnc enc, YXV_FFVSpecParam* pvSpec)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ReadSpecV(pvSpec);
	}

	YXC_Status YXV_FFEncReadSpecA(YXV_FFEnc enc, YXV_FFASpecParam* paSpec)
	{
		_FFEnc* pFFEnc = _FFEncPtr(enc);

		return pFFEnc->ReadSpecA(paSpec);
	}

	YXC_Status YXV_FFDecCreate(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFDec* pDec)
	{
		_YCHK_MAL_R1(pFFDec, _FFDec);
		new (pFFDec) _FFDec();

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = pFFDec->FFInit(vCodecId, aCodecId);
		_YXC_CHECK_RC_GOTO(rc);

		*pDec = _FFDecHdl(pFFDec);
		return YXC_ERC_SUCCESS;
err_ret:
		pFFDec->~_FFDec();
		return rcRet;
	}

	YXC_Status YXV_FFDecProcessA(YXV_FFDec dec, const ybyte_t* pbyData, yuint32_t uCbData, yint64_t uRefTime, YXV_Frame* pSample)
	{
		_FFDec* pFFDec = _FFDecPtr(dec);

		return pFFDec->ProcessA(pbyData, uCbData, uRefTime, pSample);
	}

	YXC_Status YXV_FFDecProcessV(YXV_FFDec dec, const ybyte_t* pbyData, yuint32_t uCbData, YXV_FrameType fType, yint64_t uRefTime, YXV_Frame* pSample)
	{
		_FFDec* pFFDec = _FFDecPtr(dec);

		return pFFDec->ProcessV(pbyData, uCbData, fType, uRefTime, pSample);
	}

	YXC_Status YXV_FFDecProcessV2(YXV_FFDec dec, const YXV_Packet* pPacket, YXV_Frame* pSample)
	{
		_FFDec* pFFDec = _FFDecPtr(dec);

		return pFFDec->ProcessV2(pPacket, pSample);
	}

	YXC_Status YXV_FFObjConfigA(YXV_FFObject obj, const YXV_AParam* param, const YXV_FFASpecParam* pSpecd)
	{
		_FFObject* pFFObject = _FFObjPtr(obj);

		return pFFObject->ConfigA(param, pSpecd);
	}

	YXC_Status YXV_FFObjConfigV(YXV_FFObject obj, const YXV_VParam* param, const YXV_FFVSpecParam* pSpecd)
	{
		_FFObject* pFFObject = _FFObjPtr(obj);

		return pFFObject->ConfigV(param, pSpecd);
	}

	void YXV_FFObjClose(YXV_FFObject obj)
	{
		_FFObject* pFFObject = _FFObjPtr(obj);

		pFFObject->FFUnreference();
	}

	YXC_Status YXV_FFFormatCreate(YXV_VCodecID vCodecId, YXV_ACodecID aCodecId, YXV_FFormatID fmtId, YXV_FFFormat* pFormat)
	{
		_YCHK_MAL_R1(pFFFormat, _FFFormat);
		new (pFFFormat) _FFFormat();

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = pFFFormat->FFInit(vCodecId, aCodecId, fmtId);
		_YXC_CHECK_RC_GOTO(rc);

		*pFormat = _FFFmtHdl(pFFFormat);
		return YXC_ERC_SUCCESS;
err_ret:
		pFFFormat->~_FFFormat();
		return rcRet;
	}

	YXC_Status YXV_FFFormatCreateRead(const char* filename, YXV_VCodecID* pvCodecId, YXV_ACodecID* paCodecId, YXV_FFormatID* pFmtId, YXV_FFFormat* pFormat)
	{
		_YCHK_MAL_R1(pFFFormat, _FFFormat);
		new (pFFFormat) _FFFormat();

		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		YXC_Status rc = pFFFormat->FFInit(filename, pvCodecId, paCodecId, pFmtId);
		_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, YC("Failed to read file %@"), filename);

		*pFormat = _FFFmtHdl(pFFFormat);
		return YXC_ERC_SUCCESS;
err_ret:
		pFFFormat->~_FFFormat();
		return rcRet;
	}

	YXC_Status YXV_FFFormatRead(YXV_FFFormat ffFormat, YXV_Packet* packet)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);

		return pFFFormat->Read(packet);
	}

	YXC_Status YXV_FFFormatSeek(YXV_FFFormat ffFormat, yint64_t timestamp)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);

		return pFFFormat->Seek(timestamp);
	}

	YXC_Status YXV_FFFormatReadStreamInfo(YXV_FFFormat ffFormat, YXV_StreamInfo* streamInfo)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);

		return pFFFormat->ReadStreamInfo(streamInfo);
	}

	YXC_Status YXV_FFFormatReadParamV(YXV_FFFormat ffFormat, YXV_VParam* pVParam, YXV_FFVFormatParam* pVSpecParam)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);

		return pFFFormat->ReadParamV(pVParam, pVSpecParam);
	}

	YXC_Status YXV_FFFormatReadParamA(YXV_FFFormat ffFormat, YXV_AParam* pAParam, YXV_FFAFormatParam* pASpecParam)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);

		return pFFFormat->ReadParamA(pAParam, pASpecParam);
	}

	YXC_Status YXV_FFFormatStartWrite(YXV_FFFormat ffFormat, const char* filename)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);
		return pFFFormat->StartWrite(filename);
	}

	YXC_Status YXV_FFFormatEndWrite(YXV_FFFormat ffFormat)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);
		return pFFFormat->EndWrite();
	}

	YXC_Status YXV_FFFormatWriteA(YXV_FFFormat ffFormat, const YXV_Packet* packet)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);
		return pFFFormat->WriteA(packet);
	}

	YXC_Status YXV_FFFormatWriteV(YXV_FFFormat ffFormat, const YXV_Packet* packet)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);
		return pFFFormat->WriteV(packet);
	}

	static void YXV_FFX264ParamDefault(YXV_FFX264_H264Param* pPar)
	{
		memset(pPar, 0, sizeof(YXV_FFX264_H264Param));

		pPar->bframes = 3;
		pPar->profile = FF_PROFILE_H264_MAIN;
	}

	void YXV_FFVSpecParamDefault(YXV_VCodecID vCodecId, ybool_t bEnc, YXV_FFVSpecParam* pSpecParam)
	{
		strcpy(pSpecParam->szEncDecName, "");
		switch (vCodecId)
		{
		case YXV_VCODEC_ID_H264:
			if (bEnc)
			{
				YXV_FFX264ParamDefault(&pSpecParam->x264_h264Param);
			}
			else
			{
				pSpecParam->extData.pExt = NULL;
				pSpecParam->extData.uExt = pSpecParam->extData.uExtBuf = 0;
			}
			break;
		case YXV_VCODEC_ID_VP8:
			if (bEnc)
			{
				pSpecParam->vpx_vp8Param.profile = 3;
				pSpecParam->vpx_vp8Param.speed_profile = VPX_DL_REALTIME;
			}
			else
			{
				pSpecParam->extData.pExt = NULL;
				pSpecParam->extData.uExt = pSpecParam->extData.uExtBuf = 0;
			}
			break;
		default:
			pSpecParam->extData.pExt = NULL;
			pSpecParam->extData.uExt = pSpecParam->extData.uExtBuf = 0;
			break;
		}
	}

	void YXV_FFASpecParamDefault(YXV_ACodecID aCodecId, ybool_t bEnc, YXV_FFASpecParam* pSpecParam)
	{
		strcpy(pSpecParam->szEncDecName, "");
		pSpecParam->uFrameSize = 0;
		switch (aCodecId)
		{
		case YXV_ACODEC_ID_AAC:
			if (bEnc)
			{
				pSpecParam->faac_aacParam.uProfile = 0;
			}
			else
			{
				pSpecParam->extData.pExt = NULL;
				pSpecParam->extData.uExt = pSpecParam->extData.uExtBuf = 0;
			}
			break;
		case YXV_ACODEC_ID_OPUS:
			if (bEnc)
			{
				pSpecParam->opusParam.dblRatio = 10.0;
			}
			break;
		case YXV_ACODEC_ID_MP3:
			if (bEnc)
			{
			}
			else
			{
				pSpecParam->extData.pExt = NULL;
				pSpecParam->extData.uExt = pSpecParam->extData.uExtBuf = 0;
			}
			break;
		default:
			pSpecParam->extData.pExt = NULL;
			pSpecParam->extData.uExt = pSpecParam->extData.uExtBuf = 0;
			break;
		}
	}

	YXC_Status YXV_FFFormatGetOpt(YXV_FFFormat ffFormat, YXV_FFFormatOpt opt, ybyte_t* pOpt, yuint32_t uCbOpt)
	{
		_FFFormat* pFFFormat = _FFFmtPtr(ffFormat);
		return pFFFormat->GetOpt(opt, pOpt, uCbOpt);
	}

#if YXC_PLATFORM_WIN && YXC_IS_32BIT && YXC_EXPORTS_FLAG == YXC_EXPORTS_DISPATCH_DLL
	int WINAPI DllMain(_In_ HANDLE _HDllHandle, _In_ DWORD _Reason, _In_opt_ LPVOID _Reserved)
	{
		switch (_Reason)
		{
		case DLL_PROCESS_ATTACH:
			pthread_win32_process_attach_np();
			break;
		case DLL_PROCESS_DETACH:
			pthread_win32_process_detach_np();
			break;
		case DLL_THREAD_ATTACH:
			pthread_win32_thread_attach_np();
			break;
		case DLL_THREAD_DETACH:
			pthread_win32_thread_detach_np();
			break;
		}

		return TRUE;

	}
#endif /* YXC_PLATFORM_WIN */
};

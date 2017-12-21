#include <YXC_AVKernel/YXV_AVScaler.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <YXC_AVKernel/Codec/ffwrap/ffcommon.hpp>

#define __MODULE__ "EV.AVScaler"
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_AVKernel/Utils/_YXV_Common.hpp>
#include <YXC_AVKernel/Utils/DynamicFFWrap.hpp>

using namespace YXV_FFWrap;

namespace
{
	struct VScaler
	{
		SwsContext* context;
		yuint32_t sw;
		yuint32_t sh;
		AVPixelFormat avSrcPixFmt;
		YXV_PixFmt srcPixFmt;
		yuint32_t uSrcRSArr[YXV_MAX_NUM_PLANARS]; // src row stride array
		yuint32_t uSrcHArr[YXV_MAX_NUM_PLANARS]; // src height array
		yuint32_t uSrcCSArr[YXV_MAX_NUM_PLANARS]; // src channel size array
		yuint32_t uSrcC; // src num channels
		yuint32_t uSrcS; // src total buffer size

		yuint32_t dw;
		yuint32_t dh;
		AVPixelFormat avDstPixFmt;
		YXV_PixFmt dstPixFmt;
		yuint32_t uDstRSArr[YXV_MAX_NUM_PLANARS]; // dst row stride array
		yuint32_t uDstHArr[YXV_MAX_NUM_PLANARS]; // dst height array
		yuint32_t uDstCSArr[YXV_MAX_NUM_PLANARS]; // dst channel size array
		yuint32_t uDstC; // dst num channels
		yuint32_t uDstS; // dst total buffer size

		YXV_VScalerParam scalerParam;
	};

	struct ASampler
	{
		SwrContext* context;

		yuint32_t uSrcC;
		yuint32_t uSrcR;
		yuint32_t uSrcBytes;
		yuint32_t uSrcP;
		AVSampleFormat avSrcSampleFmt;
		YXV_SampleFmt srcSampleFmt;

		yuint32_t uDstC;
		yuint32_t uDstR;
		yuint32_t uDstBytes;
		yuint32_t uDstP;
		AVSampleFormat avDstSampleFmt;
		YXV_SampleFmt dstSampleFmt;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_VScaler, VScaler, _VSPtr, _VSHdl);
	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_ASampler, ASampler, _ASPtr, _ASHdl);
}

using namespace _YXV_AVUtils;

extern "C"
{
	YXC_Status YXV_VScalerCreate(const YXV_PicDesc* srcDesc, const YXV_PicDesc* dstDesc, const YXV_VScalerParam* sParam, YXV_VScaler* pScaler)
	{
		_YCHK_MAL_R1(pScalerImpl, VScaler);
		YXCLib::HandleRef<void*> pScalerImpl_res(pScalerImpl, free);

		YXV_PixFmt inFmt = srcDesc->pixFmt, outFmt = dstDesc->pixFmt;
		yuint32_t sw = srcDesc->w, sh = srcDesc->h;
		yuint32_t dw = dstDesc->w, dh = dstDesc->h;

		pScalerImpl->avDstPixFmt = PixFmtFromFF(outFmt);
		pScalerImpl->avSrcPixFmt = PixFmtFromFF(inFmt);

		//if (inFmt & YXV_PIX_FMT_TYPE_YUV_HALF)
		//{
		//	_YXC_CHECK_REPORT_NEW_RET(sw % 2 == 0, YXC_ERC_INVALID_PARAMETER, YC("Invalid odd width number(%d)"), sw);
		//	_YXC_CHECK_REPORT_NEW_RET(sh % 2 == 0, YXC_ERC_INVALID_PARAMETER, YC("Invalid odd height number(%d)"), sh);
		//}

		//if (outFmt & YXV_PIX_FMT_TYPE_YUV_HALF)
		//{
		//	_YXC_CHECK_REPORT_NEW_RET(dw % 2 == 0, YXC_ERC_INVALID_PARAMETER, YC("Invalid odd width number(%d)"), dw);
		//	_YXC_CHECK_REPORT_NEW_RET(dh % 2 == 0, YXC_ERC_INVALID_PARAMETER, YC("Invalid odd height number(%d)"), dh);
		//}

		pScalerImpl->scalerParam.bHWAcceleration = FALSE;
		pScalerImpl->scalerParam.bInvert = FALSE;
		pScalerImpl->scalerParam.interpolation = YXV_IMAGE_INTER_DEFAULT;
		if (sParam != NULL)
		{
			pScalerImpl->scalerParam = *sParam;
		}

		if (pScalerImpl->scalerParam.bHWAcceleration)
		{
			_YXC_CHECK_REPORT_NEW_RET(!pScalerImpl->scalerParam.bInvert, YXC_ERC_INVALID_PARAMETER,
				YC("HW Accelerator can't invert images."));
		}

		YXC_Status rc = _CalculateSampleInfo(inFmt, sw, sh, &pScalerImpl->uSrcC, pScalerImpl->uSrcRSArr,
			pScalerImpl->uSrcHArr, pScalerImpl->uSrcCSArr, &pScalerImpl->uSrcS);
		_YXC_CHECK_RC_RET(rc);

		rc = _CalculateSampleInfo(outFmt, dw, dh, &pScalerImpl->uDstC, pScalerImpl->uDstRSArr,
			pScalerImpl->uDstHArr, pScalerImpl->uDstCSArr, &pScalerImpl->uDstS);
		_YXC_CHECK_RC_RET(rc);

		pScalerImpl->context = NULL;
		pScalerImpl->dh = dh;
		pScalerImpl->dw = dw;
		pScalerImpl->dstPixFmt = outFmt;
		pScalerImpl->srcPixFmt = inFmt;
		pScalerImpl->sw = sw;
		pScalerImpl->sh = sh;

		pScalerImpl_res.Detach();
		*pScaler = _VSHdl(pScalerImpl);
		return YXC_ERC_SUCCESS;
	}

	void YXV_VScalerDestroy(YXV_VScaler scaler)
	{
		VScaler* pScaler = _VSPtr(scaler);

		if (pScaler->context)
		{
			_YXV_FFWrap::g_fn_sws_freeContext(pScaler->context);
		}
		free(pScaler);
	}

	YXC_Status YXV_VScalerAllocInSample(YXV_VScaler scaler, YXV_Frame* pSample)
	{
		VScaler* pScaler = _VSPtr(scaler);

		YXV_PicDesc desc = { pScaler->srcPixFmt, pScaler->sw, pScaler->sh };
		YXC_Status rc = _AllocVSample(pScaler->scalerParam.bInvert, &desc, pScaler->uSrcC, pScaler->uSrcRSArr, pScaler->uSrcHArr,
			pScaler->uSrcCSArr, pScaler->uSrcS, pSample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_VScalerAllocOutSample(YXV_VScaler scaler, YXV_Frame* pSample)
	{
		VScaler* pScaler = _VSPtr(scaler);

		YXV_PicDesc desc = { pScaler->dstPixFmt, pScaler->dw, pScaler->dh };
		YXC_Status rc = _AllocVSample(FALSE, &desc, pScaler->uDstC, pScaler->uDstRSArr, pScaler->uDstHArr, pScaler->uDstCSArr,
			pScaler->uDstS, pSample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void YXV_VScalerFillInSample(YXV_VScaler scaler, ybyte_t* byBuffer, YXV_Frame* pSample)
	{
		VScaler* pScaler = _VSPtr(scaler);

		YXV_Buffer buffer = { YXV_BUFFER_TYPE_NONE, byBuffer, 0, 1 };
		YXV_PicDesc desc = { pScaler->srcPixFmt, pScaler->sw, pScaler->sh };
		_FillVSample(&buffer, pScaler->scalerParam.bInvert, &desc, pScaler->uSrcC, pScaler->uSrcRSArr, pScaler->uSrcHArr,
			pScaler->uSrcCSArr, pSample);
		_FillPixInfo(pScaler->srcPixFmt, pSample->uRatioW, pSample->uRatioH, pSample->uPixSize);

		pSample->buffer = NULL;
	}

	void YXV_VScalerFillOutSample(YXV_VScaler scaler, ybyte_t* byBuffer, YXV_Frame* pSample)
	{
		VScaler* pScaler = _VSPtr(scaler);

		YXV_Buffer buffer = { YXV_BUFFER_TYPE_NONE, byBuffer, 0, 1 };
		YXV_PicDesc desc = { pScaler->dstPixFmt, pScaler->dw, pScaler->dh };
		_FillVSample(&buffer, FALSE, &desc, pScaler->uDstC, pScaler->uDstRSArr, pScaler->uDstHArr, pScaler->uDstCSArr, pSample);
		_FillPixInfo(pScaler->dstPixFmt, pSample->uRatioW, pSample->uRatioH, pSample->uPixSize);

		pSample->buffer = NULL;
	}

	YXC_Status YXV_VScalerScale(YXV_VScaler scaler, const YXV_Frame* sample_in, YXV_Frame* sample_out)
	{
		VScaler* pScaler = _VSPtr(scaler);

		if (pScaler->context == NULL)
		{
			int flag = gs_resize_flag;
			if (pScaler->scalerParam.interpolation == YXV_IMAGE_INTER_NN) flag = SWS_POINT;
			else if (pScaler->scalerParam.interpolation == YXV_IMAGE_INTER_LINEAR) flag = SWS_BILINEAR;

			SwsContext* context = _YXV_FFWrap::g_fn_sws_getCachedContext(NULL, pScaler->sw, pScaler->sh, pScaler->avSrcPixFmt,
				pScaler->dw, pScaler->dh, pScaler->avDstPixFmt, flag, NULL, NULL, NULL);
			_YXC_CHECK_FF_PTR_RET(context);

			pScaler->context = context;
		}

		int ffret = _YXV_FFWrap::g_fn_sws_scale(pScaler->context, sample_in->pData, (int*)sample_in->uSize, 0, pScaler->sh,
			sample_out->pData, (int*)sample_out->uSize);
		_YXC_CHECK_FF_RET(ffret);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_ASamplerCreate(YXV_SampleFmt inFmt, yuint32_t sc, yuint32_t sr, YXV_SampleFmt outFmt,
		yuint32_t dc, yuint32_t dr, YXV_ASampler* pSampler)
	{
		_YCHK_MAL_R1(pSamplerImpl, ASampler);
		YXCLib::HandleRef<void*> pSamplerImpl_res(pSamplerImpl, free);

		pSamplerImpl->avDstSampleFmt = SampleFmtFromFF(outFmt);
		pSamplerImpl->avSrcSampleFmt = SampleFmtFromFF(inFmt);

		pSamplerImpl->context = NULL;

		pSamplerImpl->uSrcBytes = _GetSampleBits(inFmt) / 8;
		pSamplerImpl->srcSampleFmt = inFmt;
		pSamplerImpl->uSrcC = sc;
		pSamplerImpl->uSrcR = sr;
		pSamplerImpl->uSrcP = sc;

		if (!_IsPlanarSampleFmt(inFmt))
		{
			pSamplerImpl->uSrcP = 1;
			pSamplerImpl->uSrcBytes *= sc;
		}
		pSamplerImpl->uDstBytes = _GetSampleBits(outFmt) / 8;
		pSamplerImpl->dstSampleFmt = outFmt;
		pSamplerImpl->uDstC = dc;
		pSamplerImpl->uDstR = dr;
		pSamplerImpl->uDstP = dc;

		if (!_IsPlanarSampleFmt(outFmt))
		{
			pSamplerImpl->uDstP = 1;
			pSamplerImpl->uDstBytes *= dc;
		}

		*pSampler = _ASHdl(pSamplerImpl);
		pSamplerImpl_res.Detach();
		return YXC_ERC_SUCCESS;
	}

	void YXV_ASamplerDestroy(YXV_ASampler sampler)
	{
		ASampler* pSampler = _ASPtr(sampler);

		if (pSampler->context)
		{
			_YXV_FFWrap::g_fn_swr_free(&pSampler->context);
		}

		free(pSampler);
	}

	YXC_Status YXV_ASamplerAllocInSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		ASampler* pSampler = _ASPtr(sampler);

		YXV_SampleDesc desc = { pSampler->srcSampleFmt, pSampler->uSrcC, pSampler->uSrcR };
		YXC_Status rc = _AllocASample(&desc, pSampler->uSrcP, pSampler->uSrcBytes, uNumSamples, pSample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_ASamplerAllocOutSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		ASampler* pSampler = _ASPtr(sampler);

		YXV_SampleDesc desc = { pSampler->dstSampleFmt, pSampler->uDstC, pSampler->uDstR };
		YXC_Status rc = _AllocASample(&desc, pSampler->uDstP, pSampler->uDstBytes, uNumSamples, pSample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void YXV_ASamplerFillInSample(YXV_ASampler sampler, ybyte_t* byBuffer, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		ASampler* pSampler = _ASPtr(sampler);

		YXV_Buffer buffer = { YXV_BUFFER_TYPE_NONE, byBuffer, 0, 1 };
		YXV_SampleDesc desc = { pSampler->srcSampleFmt, pSampler->uSrcC, pSampler->uSrcR };
		_FillASample(&buffer, &desc, pSampler->uSrcP, pSampler->uSrcBytes, uNumSamples, pSample);

		pSample->buffer = NULL;
	}

	void YXV_ASamplerFillOutSample(YXV_ASampler sampler, ybyte_t* byBuffer, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		ASampler* pSampler = _ASPtr(sampler);

		YXV_Buffer buffer = { YXV_BUFFER_TYPE_NONE, byBuffer, 0, 1 };
		YXV_SampleDesc desc = { pSampler->dstSampleFmt, pSampler->uDstC, pSampler->uDstR };
		_FillASample(&buffer, &desc, pSampler->uDstC, pSampler->uDstBytes, uNumSamples, pSample);

		pSample->buffer = NULL;
	}

	void YXV_ASamplerSkipInSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		ASampler* pSampler = _ASPtr(sampler);

		for (yuint32_t i = 0; i < pSampler->uSrcP; ++i)
		{
			pSample->pData[i] += uNumSamples * pSampler->uSrcBytes;
			pSample->uSize[i] -= uNumSamples * pSampler->uSrcBytes;
		}
	}

	void YXV_ASamplerSkipOutSample(YXV_ASampler sampler, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		ASampler* pSampler = _ASPtr(sampler);

		for (yuint32_t i = 0; i < pSampler->uDstP; ++i)
		{
			pSample->pData[i] += uNumSamples * pSampler->uDstBytes;
			pSample->uSize[i] -= uNumSamples * pSampler->uDstBytes;
		}
	}

	YXC_Status YXV_ASamplerResample(YXV_ASampler sampler, const YXV_Frame* sample_in, YXV_Frame* sample_out)
	{
		ASampler* pSampler = _ASPtr(sampler);

		if (pSampler->context == NULL)
		{
			int64_t layoutIn = pSampler->uSrcC == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
			int64_t layoutOut = pSampler->uDstC == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
			SwrContext* context = _YXV_FFWrap::g_fn_swr_alloc_set_opts(NULL, layoutOut, pSampler->avDstSampleFmt, pSampler->uDstR, layoutIn,
				pSampler->avSrcSampleFmt, pSampler->uSrcR, 0, NULL);
			_YXC_CHECK_FF_PTR_RET(context);

			int iRet = _YXV_FFWrap::g_fn_swr_init(context);
			if (iRet != 0)
			{
				_YXV_FFWrap::g_fn_swr_free(&context);
				_YXC_CHECK_FF_RET(iRet);
			}

			pSampler->context = context;
		}

		yuint32_t uInSamples = sample_in->uNumSamples;
		yuint32_t uOutSamples = sample_out->uSize[0] / pSampler->uDstBytes;

		int ffret = _YXV_FFWrap::g_fn_swr_convert(pSampler->context, (uint8_t**)sample_out->pData, uOutSamples, (const uint8_t**)sample_in->pData, uInSamples);
		_YXC_CHECK_FF_RET(ffret);

		sample_out->uNumSamples = ffret;
		return YXC_ERC_SUCCESS;
	}
};

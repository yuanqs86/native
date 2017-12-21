#define __MODULE__ "EJ.AV.Utils"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_AVKernel/Utils/_YXV_Common.hpp>
#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/Utils/DynamicFFWrap.hpp>
#include <YXC_Sys/YXC_PNCMP.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
};

namespace _YXV_AVUtils
{
	inline byte ADJUST(int tmp)
	{
		return (byte)((tmp >= 0 && tmp <= 255)?tmp:(tmp < 0 ? 0 : 255));
	}

	static void _rgba_to_yuv_scalar(const YXV_Scalar* rgba, YXV_Scalar* yuva)
	{
		ybyte_t r = rgba->byScalar[0];
		ybyte_t g = rgba->byScalar[1];
		ybyte_t b = rgba->byScalar[2];
		ybyte_t a = rgba->byScalar[3];

		int y = ADJUST((int)(0.257*r + 0.504*g + 0.098*b + 16.5));
		int u = ADJUST((int)(-0.148*r - 0.291*g + 0.439*b + 128.5));
		int v = ADJUST((int)(0.439*r - 0.368*g - 0.071*b + 128.5));

		yuva->byScalar[0] = y;
		yuva->byScalar[1] = u;
		yuva->byScalar[2] = v;
		yuva->byScalar[3] = a;
	}

	static void _ConvertBlackScalar(YXV_Scalar* scalar, YXV_PixFmt pixFmt,ybool_t blend)
	{
		ybyte_t* byScalar = scalar->byScalar;
		if ((pixFmt & YXV_PIX_FMT_TYPE_YUV_HALF) || (pixFmt & YXV_PIX_FMT_TYPE_YUV_FULL))
		{
			byScalar[0] = 16;
			byScalar[1] = 128;
			byScalar[2] = 128;
			byScalar[3] = blend ? 0:255;
		}
		else
		{
			byScalar[0] = 0;
			byScalar[1] = 0;
			byScalar[2] = 0;
			byScalar[3] = 255;
		}
	}

	void _FillPixInfo(YXV_PixFmt pixFmt, yuint8_t (&ratioW)[YXV_MAX_NUM_PLANARS],
		yuint8_t (&ratioH)[YXV_MAX_NUM_PLANARS], yuint8_t (&pixelSize)[YXV_MAX_NUM_PLANARS])
	{
		yuint32_t pixSize = 0;
		switch (pixFmt)
		{
		case YXV_PIX_FMT_BGR24:
		case YXV_PIX_FMT_RGB24:
			ratioW[0] = 1;
			ratioH[0] = 1;
			pixelSize[0] = 3;
			break;
		case YXV_PIX_FMT_RGBA:
		case YXV_PIX_FMT_BGRA:
		case YXV_PIX_FMT_RGB0:
		case YXV_PIX_FMT_BGR0:
		case YXV_PIX_FMT_ARGB:
		case YXV_PIX_FMT_ABGR:
		case YXV_PIX_FMT_0RGB:
		case YXV_PIX_FMT_0BGR:
			ratioW[0] = 1;
			ratioH[0] = 1;
			pixelSize[0] = 4;
			break;
		case YXV_PIX_FMT_NV12:
			ratioW[0] = 1;
			ratioH[0] = 1;
			pixelSize[0] = 1;
			ratioW[1] = 1;
			ratioH[1] = 1;
			pixelSize[0] = 2;
			break;
		case YXV_PIX_FMT_YUYV:
		case YXV_PIX_FMT_UYVY:
			ratioW[0] = 2;
			ratioH[0] = 1;
			pixelSize[0] = 4;
			break;
		case YXV_PIX_FMT_YUV420P:
		case YXV_PIX_FMT_YUVJ420P:
		case YXV_PIX_FMT_YUVA420P:
			ratioW[0] = ratioW[3] = 1;
			ratioW[1] = ratioW[2] = 2;
			ratioH[0] = ratioH[3] = 1;
			ratioH[1] = ratioH[2] = 2;
			pixelSize[0] = pixelSize[1] = pixelSize[2] = pixelSize[3] = 1;
			break;
		case YXV_PIX_FMT_YUVJ444P:
		default:
			ratioW[0] = ratioW[1] = ratioW[2] = ratioW[3] = 1;
			ratioH[0] = ratioH[1] = ratioH[2] = ratioH[3] = 1;
			pixelSize[0] = pixelSize[1] = pixelSize[2] = pixelSize[3] = 1;
			break;
		}
	}

	YXC_Status _CalculateSampleInfo(YXV_PixFmt fmt, yuint32_t w, yuint32_t h, yuint32_t* puC,
		yuint32_t* pRSArr, yuint32_t* pHArr, yuint32_t* pCSArr, yuint32_t* puS)
	{
		yuint32_t uC = 0, uS = 0, uP = 0;
		switch (fmt)
		{
		case YXV_PIX_FMT_BGR24:
		case YXV_PIX_FMT_RGB24:
			uC = 1;
			pRSArr[0] = YXV_ImageAlignRow(w * 3);
			pHArr[0] = h;
			break;
		case YXV_PIX_FMT_RGBA:
		case YXV_PIX_FMT_BGRA:
		case YXV_PIX_FMT_RGB0:
		case YXV_PIX_FMT_BGR0:
		case YXV_PIX_FMT_ARGB:
		case YXV_PIX_FMT_ABGR:
		case YXV_PIX_FMT_0RGB:
		case YXV_PIX_FMT_0BGR:
			uC = 1;
			pRSArr[0] = YXV_ImageAlignRow(w * 4);
			pHArr[0] = h;
			break;
		case YXV_PIX_FMT_NV12:
			uC = 2;
			pRSArr[0] = pRSArr[1] = w;
			pHArr[0] = h;
			pHArr[1] = h / 2;
			break;
		case YXV_PIX_FMT_YUYV:
		case YXV_PIX_FMT_UYVY:
			uC = 1;
			pRSArr[0] = w * 2;
			pHArr[0] = h;
			break;
		case YXV_PIX_FMT_YUV420P:
		case YXV_PIX_FMT_YUVJ420P:
			uC = 3;
			pRSArr[0] = w;
			pHArr[0] = h;
			pRSArr[1] = pRSArr[2] = w / 2;
			pHArr[1] = pHArr[2] = h / 2;
			break;
		case YXV_PIX_FMT_YUVA420P:
			uC = 4;
			pRSArr[0] = w;
			pHArr[0] = h;
			pRSArr[1] = pRSArr[2] = w / 2;
			pHArr[1] = pHArr[2] = h / 2;
			pRSArr[3] = w;
			pHArr[3] = h;
			break;
		case YXV_PIX_FMT_YUVJ444P:
			uC = 3;
			pRSArr[0] = pRSArr[1] = pRSArr[2] = w;
			pHArr[0] = pHArr[1] = pHArr[2] = h;
			break;
		case YXV_PIX_FMT_YUVA444P:
			uC = 4;
			pRSArr[0] = pRSArr[1] = pRSArr[2] = pRSArr[3] = w;
			pHArr[0] = pHArr[1] = pHArr[2] = pHArr[3] = h;
			break;
		case YXV_PIX_FMT_YUV444P:
			uC = 3;
			pRSArr[0] = pRSArr[1] = pRSArr[2]  = w;
			pHArr[0] = pHArr[1] = pHArr[2]  = h;
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("Invalid video sample format (0x%x)"), fmt);
			break;
		}

		for (yuint32_t i = 0; i < uC; ++i)
		{
			pCSArr[i] = pRSArr[i] * pHArr[i];
			uS += pCSArr[i];
		}

		*puS = uS;
		*puC = uC;
		return YXC_ERC_SUCCESS;
	}

	void _FillVSample(YXV_Buffer* buffer, ybool_t bInvert, const YXV_PicDesc* picDesc, yuint32_t uC, yuint32_t* pRSArr,
		yuint32_t* pHArr, yuint32_t* pCSArr, YXV_Frame* pSample)
	{
		memset(pSample, 0, sizeof(YXV_Frame));

		ybyte_t* curBuf = (ybyte_t*)buffer->p;
		if (!bInvert)
		{
			for (yuint32_t i = 0; i < uC; ++i)
			{
				pSample->pData[i] = curBuf;
				pSample->uSize[i] = pRSArr[i];

				curBuf += pCSArr[i];
			}
		}
		else
		{
			for (yuint32_t i = 0; i < uC; ++i)
			{
				ybyte_t* lastRow = curBuf + pRSArr[i] * (pHArr[i] - 1);
				pSample->pData[i] = lastRow;
				pSample->uSize[i] = -(yint32_t)pRSArr[i];

				curBuf += pCSArr[i];
			}
		}

		pSample->uNumPlanars = uC;
		pSample->u1.picDesc = *picDesc;
		pSample->buffer = buffer;
		pSample->bGotFrame = TRUE;
	}

	YXC_Status _AllocVSample(ybool_t bInvert, const YXV_PicDesc* picDesc, yuint32_t uC, yuint32_t* pRSArr, yuint32_t* pHArr, yuint32_t* pCSArr,
		yuint32_t uS, YXV_Frame* pSample)
	{
		_YCHK_MAL_ARR_R1(pByteBuffer, ybyte_t, uS + sizeof(YXV_Buffer));
		YXV_Buffer* buffer = (YXV_Buffer*)pByteBuffer;
		buffer->free_func = free;
		buffer->sz = uS;
		buffer->p = pByteBuffer + sizeof(YXV_Buffer);
		buffer->type = YXV_BUFFER_TYPE_NONE;
		buffer->uRefCount = 1;

		_FillVSample(buffer, bInvert, picDesc, uC, pRSArr, pHArr, pCSArr, pSample);
		_FillPixInfo(picDesc->pixFmt, pSample->uRatioW, pSample->uRatioH, pSample->uPixSize);
		return YXC_ERC_SUCCESS;
	}

	void _FillASample(YXV_Buffer* buffer, const YXV_SampleDesc* desc, yuint32_t uP, yuint32_t uSampleBytes,
		yuint32_t uNumSamples, YXV_Frame* sample)
	{
		memset(sample, 0, sizeof(YXV_Frame));

		ybyte_t* begin = (ybyte_t*)buffer->p;
		for (yuint32_t i = 0; i < uP; ++i)
		{
			sample->pData[i] = begin;
			sample->uSize[i] = uNumSamples * uSampleBytes;

			begin += sample->uSize[i];
		}

		sample->uNumPlanars = uP;
		sample->uNumSamples = uNumSamples;
		sample->u1.sampleDesc = *desc;
		sample->fType = YXV_FRAME_TYPE_A;
		sample->buffer = buffer;
	}

	YXC_Status _AllocASample(const YXV_SampleDesc* desc, yuint32_t uP, yuint32_t uSampleBytes, yuint32_t uNumSamples, YXV_Frame* sample)
	{
		// yuint32_t uAlign = uC * uSampleBytes;
		yuint32_t uTotalSize = uP * uSampleBytes * uNumSamples;
		_YCHK_MAL_ARR_R1(pByteBuffer, ybyte_t, uTotalSize + sizeof(YXV_Buffer));

		YXV_Buffer* buffer = (YXV_Buffer*)pByteBuffer;
		buffer->free_func = free;
		buffer->sz = uTotalSize;
		buffer->p = pByteBuffer + sizeof(YXV_Buffer);
		buffer->type = YXV_BUFFER_TYPE_NONE;
		buffer->uRefCount = 1;

		_FillASample(buffer, desc, uP, uSampleBytes, uNumSamples, sample);
		return YXC_ERC_SUCCESS;
	}

	void _YXV_BufferRelease(YXV_Buffer* buffer)
	{
		if (buffer != NULL)
		{
			yuint32_t iRef = YXCLib::Interlocked::Decrement(&buffer->uRefCount);
			if (iRef == 0)
			{
				AVFrame* frame;
				AVBufferRef* buf;
				YXC_PNCMPConsumer c;
				switch (buffer->type)
				{
				case YXV_BUFFER_TYPE_FF_FRAME:
					frame = (AVFrame*)buffer->p;
					//_YXV_FFWrap::g_fn_av_frame_unref(frame);
					_YXV_FFWrap::g_fn_av_frame_free(&frame);
					break;
				case YXV_BUFFER_TYPE_FF_PACKET:
					buf = (AVBufferRef*)buffer->p;
					_YXV_FFWrap::g_fn_av_buffer_unref(&buf);
					break;
				case YXV_BUFFER_TYPE_C_RUNTIME:
					free(buffer->p);
					break;
				case YXV_BUFFER_TYPE_PNCMP:
					c = (YXC_PNCMPConsumer)buffer->c;
					YXC_PNCMPConsumerUnreferenceBlock(c, buffer->p);
				default:
					break;
				}

				YXV_BufferFreeFunc free_func = buffer->free_func;
				free_func(buffer);
			}
		}
	}
}

using namespace _YXV_AVUtils;

extern "C"
{
	void YXV_BufferUnref(YXV_Buffer* pBuffer)
	{
		_YXV_BufferRelease(pBuffer);
	}

	YXC_Status YXV_BufferAlloc_C(YXV_BufType type, void* ptr, void* c, yuint32_t sz, YXV_Buffer** ppBuffer)
	{
		_YCHK_MAL_R1(pBuffer, YXV_Buffer);

		pBuffer->free_func = free;
		pBuffer->p = ptr;
		pBuffer->c = c;
		pBuffer->sz = sz;
		pBuffer->type = type;
		pBuffer->uRefCount = 1;

		*ppBuffer = pBuffer;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_BufferAlloc(YXV_BufType type, void* ptr, yuint32_t sz, YXV_Buffer** ppBuffer)
	{
		YXC_Status rc = YXV_BufferAlloc_C(type, ptr, NULL, sz, ppBuffer);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void YXV_FrameZero(YXV_Frame* sample,ybool_t blend)
	{
		if (sample->fType == YXV_FRAME_TYPE_A)
		{
			yuint32_t sample_bytes = _YXV_AVUtils::_GetSampleBits(sample->u1.sampleDesc.sampleFmt) / 8;
			yuint32_t num_planars = sample->u1.sampleDesc.uNumChannels;

			if (!_YXV_AVUtils::_IsPlanarSampleFmt(sample->u1.sampleDesc.sampleFmt))
			{
				sample_bytes *= sample->u1.sampleDesc.uNumChannels;
				num_planars = 1;
			}

			for (yuint32_t i = 0; i < num_planars; ++i)
			{
				memset(sample->pData[i], 0, sample->uNumSamples * sample_bytes);
			}
		}
		else
		{
			YXV_Scalar scalar;
			_ConvertBlackScalar(&scalar, sample->u1.picDesc.pixFmt,blend);

			for (yuint32_t i = 0; i < sample->uNumPlanars; ++i)
			{
				yuint32_t rh = sample->u1.picDesc.h / sample->uRatioH[i], rw = sample->u1.picDesc.w / sample->uRatioW[i];
				if (sample->pData[i])
				{
					for (yuint32_t j = 0; j < rh; ++j)
					{
						ybool_t bInvert = sample->uSize[i] < 0;

						memset(sample->pData[i] + sample->uSize[i] * j, scalar.byScalar[i], rw * sample->uPixSize[i]);
					}
				}
			}
		}
	}

	void YXV_FrameRef(YXV_Frame* pSample)
	{
		if (pSample->buffer)
		{
			YXCLib::Interlocked::Increment(&pSample->buffer->uRefCount);
		}
	}

	void YXV_FrameUnref(YXV_Frame* pSample)
	{
		YXV_Buffer* buffer = pSample->buffer;
		pSample->buffer = NULL;
		for (yuint32_t i = 0; i < YXV_MAX_NUM_PLANARS; ++i)
		{
			pSample->pData[i] = 0;
			pSample->uSize[i] = 0;
		}
		pSample->uNumSamples = 0;
		pSample->uNumPlanars = 0;

		YXV_BufferUnref(buffer);
	}

	YXC_Status YXV_VFrameAlloc(const YXV_PicDesc* picDesc, YXV_Frame* pSample)
	{
		yuint32_t uSrcRSArr[YXV_MAX_NUM_PLANARS]; // src row stride array
		yuint32_t uSrcHArr[YXV_MAX_NUM_PLANARS]; // src height array
		yuint32_t uSrcCSArr[YXV_MAX_NUM_PLANARS]; // src channel size array
		yuint32_t uSrcC; // src num channels
		yuint32_t uSrcS; // src total buffer size

		YXC_Status rc = _CalculateSampleInfo(picDesc->pixFmt, picDesc->w, picDesc->h, &uSrcC, uSrcRSArr, uSrcHArr, uSrcCSArr, &uSrcS);
		_YXC_CHECK_RC_RET(rc);

		rc = _AllocVSample(FALSE, picDesc, uSrcC, uSrcRSArr, uSrcHArr, uSrcCSArr, uSrcS, pSample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_VFrameFill(const YXV_PicDesc* picDesc, ybyte_t* byBuffer, YXV_Frame* pSample)
	{
		yuint32_t uSrcRSArr[YXV_MAX_NUM_PLANARS]; // src row stride array
		yuint32_t uSrcHArr[YXV_MAX_NUM_PLANARS]; // src height array
		yuint32_t uSrcCSArr[YXV_MAX_NUM_PLANARS]; // src channel size array
		yuint32_t uSrcC; // src num channels
		yuint32_t uSrcS; // src total buffer size

		YXC_Status rc = _CalculateSampleInfo(picDesc->pixFmt, picDesc->w, picDesc->h, &uSrcC, uSrcRSArr, uSrcHArr, uSrcCSArr, &uSrcS);
		_YXC_CHECK_RC_RET(rc);

		YXV_Buffer buffer = { YXV_BUFFER_TYPE_NONE, byBuffer, 0, 1 };
		_FillVSample(&buffer, FALSE, picDesc, uSrcC, uSrcRSArr, uSrcHArr, uSrcCSArr, pSample);
		_FillPixInfo(picDesc->pixFmt, pSample->uRatioW, pSample->uRatioH, pSample->uPixSize);

		pSample->buffer = NULL;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_VFrameCopy(const YXV_Frame* pSrc, YXV_Frame* pDst)
	{
		yuint32_t uSrcRSArr[YXV_MAX_NUM_PLANARS]; // src row stride array
		yuint32_t uSrcHArr[YXV_MAX_NUM_PLANARS]; // src height array
		yuint32_t uSrcCSArr[YXV_MAX_NUM_PLANARS]; // src channel size array
		yuint32_t uSrcC; // src num channels
		yuint32_t uSrcS; // src total buffer size

		YXV_PicDesc descSrc = pSrc->u1.picDesc;
		YXV_PicDesc descDst = pDst->u1.picDesc;

		_YXC_CHECK_REPORT_NEW_RET(descSrc.pixFmt == descDst.pixFmt, YXC_ERC_INVALID_PARAMETER, YC("Pixel format not match."));
		_YXC_CHECK_REPORT_NEW_RET(descSrc.w == descDst.w && descSrc.h == descDst.h, YXC_ERC_INVALID_PARAMETER, YC("dimension not match."));

		YXC_Status rc = _CalculateSampleInfo(descSrc.pixFmt, descSrc.w, descSrc.h, &uSrcC, uSrcRSArr, uSrcHArr, uSrcCSArr, &uSrcS);
		_YXC_CHECK_RC_RET(rc);

		for (yuint32_t i = 0; i < pSrc->uNumPlanars; ++i)
		{
			ybyte_t* srcData = pSrc->pData[i];
			ybyte_t* dstData = pDst->pData[i];

			for (yuint32_t j = 0; j < uSrcHArr[i]; ++j)
			{
				memcpy(dstData, srcData, descSrc.w * pSrc->uPixSize[i] / pSrc->uRatioW[i]);
				srcData += pSrc->uSize[i];
				dstData += pDst->uSize[i];
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AFrameFill(const YXV_SampleDesc* pDesc, ybyte_t* byBuffer, yuint32_t uNumSamples, YXV_Frame* pSample)
	{
		yuint32_t sample_bytes = _YXV_AVUtils::_GetSampleBits(pDesc->sampleFmt) / 8;
		yuint32_t num_planars = pDesc->uNumChannels;

		if (!_YXV_AVUtils::_IsPlanarSampleFmt(pDesc->sampleFmt))
		{
			sample_bytes *= pDesc->uNumChannels;
			num_planars = 1;
		}

		YXV_Buffer buffer = { YXV_BUFFER_TYPE_NONE, byBuffer, 0, 1 };
		_FillASample(&buffer, pDesc, num_planars, sample_bytes, uNumSamples, pSample);

		pSample->buffer = NULL;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AFrameAlloc(const YXV_SampleDesc* pDesc, yuint32_t uMaxSamples, YXV_Frame* pSample)
	{
		yuint32_t sample_bytes = _YXV_AVUtils::_GetSampleBits(pDesc->sampleFmt) / 8;
		yuint32_t num_planars = pDesc->uNumChannels;

		if (!_YXV_AVUtils::_IsPlanarSampleFmt(pDesc->sampleFmt))
		{
			sample_bytes *= pDesc->uNumChannels;
			num_planars = 1;
		}

		YXC_Status rc = _AllocASample(pDesc, num_planars, sample_bytes, uMaxSamples, pSample);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXV_AFrameCopy(YXV_Frame* aSampleDst, yuint32_t uDstIndex, const YXV_Frame* aSampleSrc, yuint32_t uSrcIndex, yuint32_t uNumSamplesToCopy)
	{
		_YXC_CHECK_REPORT_NEW_RET(aSampleDst->u1.sampleDesc.sampleFmt == aSampleSrc->u1.sampleDesc.sampleFmt, YXC_ERC_INVALID_PARAMETER, YC("Can't copy sample from different formats"));
		_YXC_CHECK_REPORT_NEW_RET(aSampleDst->uNumSamples >= uDstIndex, YXC_ERC_INDEX_OUT_OF_RANGE, YC("Copy position disallowed"));

		yuint32_t uSampleSize = _YXV_AVUtils::_GetSampleBits(aSampleSrc->u1.sampleDesc.sampleFmt) / 8;
		if (aSampleSrc->uNumPlanars == 1)
		{
			uSampleSize *= aSampleSrc->u1.sampleDesc.uNumChannels;
		}

		yuint32_t uBufSamples = aSampleDst->uSize[0] / uSampleSize;
		_YXC_CHECK_REPORT_NEW_RET(uBufSamples >= uDstIndex + uNumSamplesToCopy, YXC_ERC_BUFFER_NOT_ENOUGH,
			YC("Too many samples to copy in(%d)."), uDstIndex + uNumSamplesToCopy);

		for (yuint32_t i = 0; i < aSampleDst->uNumPlanars; ++i)
		{
			memcpy(aSampleDst->pData[i] + uDstIndex * uSampleSize, aSampleSrc->pData[i] + uSrcIndex * uSampleSize, uNumSamplesToCopy * uSampleSize);
		}

		aSampleDst->uNumSamples = YXCLib::TMax(aSampleDst->uNumSamples, uDstIndex + uNumSamplesToCopy);
		return YXC_ERC_SUCCESS;
	}

	void YXV_PacketUnref(YXV_Packet* packet)
	{
		YXV_Buffer* buffer = packet->buffer;
		packet->buffer = NULL;
		packet->uDataLen = 0;
		packet->pData = NULL;

		YXV_BufferUnref(buffer);
	}

	YXC_Status YXV_VFrameCalcSize(YXV_Frame* f, yuint32_t* pSize)
	{
		yuint32_t uSrcRSArr[YXV_MAX_NUM_PLANARS]; // src row stride array
		yuint32_t uSrcHArr[YXV_MAX_NUM_PLANARS]; // src height array
		yuint32_t uSrcCSArr[YXV_MAX_NUM_PLANARS]; // src channel size array
		yuint32_t uSrcC; // src num channels
		yuint32_t uSrcS; // src total buffer size

		YXV_PicDesc picDesc = f->u1.picDesc;
		YXC_Status rc = _CalculateSampleInfo(f->u1.picDesc.pixFmt, f->u1.picDesc.w, f->u1.picDesc.h, &uSrcC, uSrcRSArr,
			uSrcHArr, uSrcCSArr, &uSrcS);
		_YXC_CHECK_RC_RET(rc);

		*pSize = uSrcS;
		return YXC_ERC_SUCCESS;
	}

	void YXV_VFrameROI(const YXV_Frame* f1, yuint32_t x, yuint32_t y, yuint32_t w, yuint32_t h, YXV_Frame* f2)
	{
		*f2 = *f1;

		for (yuint32_t i = 0; i < f2->uNumPlanars; ++i)
		{
			f2->pData[i] += x / f1->uRatioW[i] * f1->uPixSize[i] + y / f1->uRatioH[i] * (yint64_t)f1->uSize[i];
		}

		f2->u1.picDesc.w = w;
		f2->u1.picDesc.h = h;
		f2->buffer = NULL;
	}

	static void _ConvertBKScalar(YXV_Scalar* scalar, YXV_PixFmt pixFmt, YXV_Scalar* pscalar)
	{
		if (pixFmt & YXV_PIX_FMT_TYPE_YUV_HALF)
		{
			_rgba_to_yuv_scalar(pscalar,scalar);
		}
		else
		{
			memcpy(scalar,pscalar,sizeof(YXV_Scalar));
		}
	}

	void YXV_VFrameBK(YXV_Frame* sample, YXV_Scalar* pscalar)
	{
		YXV_Scalar scalar;
		_ConvertBKScalar(&scalar, sample->u1.picDesc.pixFmt, pscalar);

		for (yuint32_t i = 0; i < sample->uNumPlanars; ++i)
		{
			yuint32_t rh = sample->u1.picDesc.h / sample->uRatioH[i], rw = sample->u1.picDesc.w / sample->uRatioW[i];
			if (sample->pData[i])
			{
				for (yuint32_t j = 0; j < rh; ++j)
				{
					ybool_t bInvert = sample->uSize[i] < 0;

					memset(sample->pData[i] + sample->uSize[i] * j, scalar.byScalar[i], rw * sample->uPixSize[i]);
				}
			}
		}
	}
};

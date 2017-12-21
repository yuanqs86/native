#include <YXC_AVKernel/Codec/HWLib/_YXV_IntelGPU.hpp>
#include <time.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_LocalMM.hpp>

namespace
{
	static void _FreeIntelEncoder(_YXV_HWLib::_IntelEncoder* encoder)
	{
		encoder->Close();
	}

	static void _InitVideoParams(const YXV_HWCodecParam* requested, mfxU16 ioPattern, mfxVideoParam& param)
	{
		memset(&param, 0, sizeof(param));

		param.mfx.CodecId = MFX_CODEC_AVC;
		param.mfx.TargetUsage = MFX_TARGETUSAGE_BEST_SPEED; // trade-off between quality and speed
		param.mfx.TargetKbps = requested->vParam.uBitrate / 1000; // in Kbps
		param.mfx.GopRefDist = 1;  /* disable B-Frames, can't write file. */

		if (requested->vParam.uBitrate == 0) /* VBR */
		{
			param.mfx.RateControlMethod = (mfxU16)MFX_RATECONTROL_VBR;
		}
		else
		{
			param.mfx.RateControlMethod = (mfxU16)MFX_RATECONTROL_CBR;
		}
		ConvertFrameRate(30, &param.mfx.FrameInfo.FrameRateExtN, &param.mfx.FrameInfo.FrameRateExtD);
		param.mfx.EncodedOrder = 0; // binary flag, 0 signals encoder to take frames in display order
		param.IOPattern = ioPattern;

		// frame info parameters
		param.mfx.FrameInfo.FourCC = MFX_FOURCC_NV12;
		param.mfx.FrameInfo.ChromaFormat = MFX_CHROMAFORMAT_YUV420;
		param.mfx.FrameInfo.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
		param.mfx.GopPicSize = 30;
		param.mfx.FrameInfo.Width = MSDK_ALIGN16(requested->vParam.desc.w);
		param.mfx.FrameInfo.Height = (MFX_PICSTRUCT_PROGRESSIVE == param.mfx.FrameInfo.PicStruct) ?
			MSDK_ALIGN16(requested->vParam.desc.h) : MSDK_ALIGN32(requested->vParam.desc.h);

		param.mfx.FrameInfo.CropX = 0;
		param.mfx.FrameInfo.CropY = 0;
		param.mfx.FrameInfo.CropW = requested->vParam.desc.w;
		param.mfx.FrameInfo.CropH = requested->vParam.desc.h;

		// JPEG encoder settings overlap with other encoders settings in mfxInfoMFX structure
		if (MFX_CODEC_JPEG == MFX_CODEC_AVC)
		{
			param.mfx.Interleaved = 1;
			param.mfx.Quality = 50;
			param.mfx.RestartInterval = 0;
			MSDK_ZERO_MEMORY(param.mfx.reserved5);
		}
	}
}

namespace _YXV_HWLib
{
	static inline void convertNV12toYUVpitch(unsigned char *yuv_luma, unsigned char *yuv_cb, unsigned char *yuv_cr,
										  unsigned char *nv12_luma, unsigned char *nv12_chroma, int width, int height,
										  int srcStride, int dstStride)
	{
		if (srcStride == 0) srcStride = width;
		if (dstStride == 0) dstStride = width;

		for (int y = 0; y < height; y++)
		{
			memcpy(yuv_luma + (dstStride * y), nv12_luma + (srcStride * y), width);
		}

		for (int y = 0; y < height / 2; y++)
		{
			for (int x = 0 ; x < width; x = x + 2)
			{
				yuv_cb[(dstStride / 2) * y + (x >> 1)] = nv12_chroma[(y * dstStride) + x];
				yuv_cr[(srcStride / 2) * y + (x >> 1)] = nv12_chroma[(y * dstStride) + x + 1];
			}
		}
	}

	static inline void convertYUVpitchtoNV12(unsigned char *yuv_luma, unsigned char *yuv_cb, unsigned char *yuv_cr,
		unsigned char *nv12_luma, unsigned char *nv12_chroma, int width, int height, int srcStride, int dstStride)
	{
		if (srcStride == 0) srcStride = width;
		if (dstStride == 0) dstStride = width;

		for (int y = 0; y < height; y++)
		{
			memcpy(nv12_luma + (dstStride * y), yuv_luma + (srcStride * y) , width);
		}

		for (int y = 0; y < height / 2; y++)
		{
			for (int x = 0 ; x < width; x = x+2)
			{
				nv12_chroma[(y * dstStride) + x] = yuv_cb[(srcStride / 2) * y + (x >> 1)];
				nv12_chroma[(y * dstStride) + x + 1] = yuv_cr[(srcStride / 2) * y + (x >> 1)];
			}
		}
	}

	YXC_Status _ReportMFXError(mfxStatus ms, const ychar* module, const ychar* file, long line, const ychar* function, const ychar* format, ...)
	{
		ychar szMessage[YXC_BASE_ERROR_BUFFER] = {0};
		ychar szMessage2[200];

		va_list vl;
		va_start(vl, format);
		yh_vsnprintf(szMessage, YXC_BASE_ERROR_BUFFER - 1, format, vl);
		va_end(vl);

		yh_sprintf(szMessage2, YC("Mfx error(%d)"), ms);

		YXC_Status rc = YXC_ReportErrorExFormat(TRUE, YXC_ERROR_SRC_3RD_PARTY, YXC_ERR_CAT_3RD_MFX, ms, module, file, line,
			function, NULL, 0, YC("%s:%s"), szMessage, szMessage2);
		return YXC_ERC_3RD;
	}

	yuint32_t _IntelDev::s_implArr[_IntelDev::MAX_DEV_NUM] = {
		MFX_IMPL_HARDWARE,
		MFX_IMPL_HARDWARE2,
		MFX_IMPL_HARDWARE3,
		MFX_IMPL_HARDWARE4
	};

	_IntelDev::_IntelDev()
	{

	}

	_IntelDev::~_IntelDev()
	{

	}

	YXC_Status _IntelDev::Init(yuint32_t uDevIndex, YXV_HWIFaceType ifaceType)
	{
		_YXC_CHECK_REPORT_NEW_RET(uDevIndex < 4, YXC_ERC_INDEX_OUT_OF_RANGE, YC("Max intel device number(%d)"), 4);

		this->_devIndex = uDevIndex;
		return YXC_ERC_SUCCESS;
	}

	void _IntelDev::Close()
	{

	}

	YXC_Status _IntelDev::CreateEncoder(_HWEncoderBase** ppEncoder)
	{
		_YCHK_MAL_R1(pNvEncoder, _IntelEncoder);
		new (pNvEncoder) _IntelEncoder();

		YXC_Status rcRet;
		YXC_Status rc = pNvEncoder->Init(this);
		_YXC_CHECK_RC_GOTO(rc);

		*ppEncoder = pNvEncoder;
		return YXC_ERC_SUCCESS;
err_ret:
		pNvEncoder->~_IntelEncoder();
		free(pNvEncoder);

		return rcRet;
	}

	YXC_Status _IntelDev::CreateDecoder(_HWDecoderBase** ppDecoder)
	{
		_YCHK_MAL_R1(pDecoder, _IntelDecoder);
		new (pDecoder) _IntelDecoder();

		YXC_Status rcRet;
		YXC_Status rc = pDecoder->Init(this);
		_YXC_CHECK_RC_GOTO(rc);

		*ppDecoder = pDecoder;
		return YXC_ERC_SUCCESS;
err_ret:
		pDecoder->~_IntelDecoder();
		free(pDecoder);

		return rcRet;
	}

	YXC_Status _IntelDev::CreateVPP(_HWVPPBase** ppVPP)
	{
		_YCHK_MAL_R1(pVPP, _IntelVPP);
		new (pVPP) _IntelVPP();

		YXC_Status rcRet;
		YXC_Status rc = pVPP->Init(this);
		_YXC_CHECK_RC_GOTO(rc);

		*ppVPP = pVPP;
		return YXC_ERC_SUCCESS;
err_ret:
		pVPP->~_IntelVPP();
		free(pVPP);

		return rcRet;
	}

	YXC_Status _YXV_IntelGetDeviceNum(yuint32_t* puDevNum)
	{
		YXC_Status rc = YXC_ERC_SUCCESS;
		mfxVersion mfVer;
		mfVer.Minor = 1;
		mfVer.Major = 1;

		yuint32_t uNumDev = 0;
		for (yuint32_t i = 0; i < 4; ++i)
		{
			mfxSession session = {0};

			mfxStatus ms = MFXInit(_IntelDev::s_implArr[i], &mfVer, &session);
			if (ms < MFX_ERR_NONE)
			{
				break;
			}
			MFXClose(session);
			++uNumDev;
		}

		*puDevNum = uNumDev;
		return YXC_ERC_SUCCESS;
	}
}

namespace _YXV_HWLib
{
	_IntelEncoder::_IntelEncoder()
	{
		this->_pOutBuffer = NULL;
		this->_uNumFramesProcessed = 0;
		this->_uNumFramesGenerated = 0;
		this->_stcbOutBuffer = 0;
		this->_uDevIndex = -1;
		pSPS=pPPS=NULL;
		spsSize=ppsSize=0;
	}

	_IntelEncoder::~_IntelEncoder()
	{
		this->Close();
	}

	void _IntelEncoder::GetInfo(YXV_HWEncoderInfo* pInfo)
	{
		sprintf(pInfo->szDevName, "Intel Device %d", this->_uDevIndex);
		pInfo->inputType = YXV_PIX_FMT_YUV420P;
	}

	YXC_Status _IntelEncoder::Init(_HWDevBase* dev)
	{
		mfxVersion mfVer;
		mfVer.Minor = 1;
		mfVer.Major = 1;

		_IntelDev* iDev = (_IntelDev*)dev;
		yuint32_t impl = _IntelDev::s_implArr[iDev->GetDevIndex()];
		mfxStatus ms = MFXInit(impl, &mfVer, &this->_session);
		_YXC_CHECK_MFX_RET(ms, YC("Failed to init device(%d)"), iDev->GetDevIndex());

		this->_uDevIndex = iDev->GetDevIndex();
		return YXC_ERC_SUCCESS;
	}

	void _IntelEncoder::Close()
	{
		if (this->_session)
		{
			for (int i = 0; i < YUV_FRAMES; ++i)
			{
				if (this->_encodeData[i].surface.Data.R)
				{
					this->_sys_alloc.UnlockFrame(this->_allocRes[i].mids[0], &this->_encodeData[i].surface.Data);
				}
				this->_sys_alloc.FreeFrames(&this->_allocRes[i]);

				if (this->_encodeData[i].bitStream.Data)
				{
					free(this->_encodeData[i].bitStream.Data);
					this->_encodeData[i].bitStream.Data = NULL;
				}
			}
			this->_sys_alloc.Close();
			memset(this->_encodeData, 0, sizeof(this->_encodeData));
			if (this->_pOutBuffer)
			{
				free(this->_pOutBuffer);
				this->_pOutBuffer = NULL;
			}
			MFXVideoENCODE_Close(this->_session);
			MFXClose(this->_session);
			this->_session = NULL;
			FreeSPSandPPS();
		}
	}

	YXC_Status _IntelEncoder::Config(const YXV_HWCodecParam* param)
	{
		memcpy(&this->_param, param, sizeof(YXV_HWCodecParam));
		memset(this->_encodeData, 0, sizeof(this->_encodeData));

		_InitVideoParams(param, MFX_IOPATTERN_IN_SYSTEM_MEMORY, this->_vp);

		YXC_Status rc = _InitMfxBuffers(this->_session, &this->_vp);
		_YXC_CHECK_RC_RET(rc);

		mfxStatus ms = MFXVideoENCODE_Init(this->_session, &this->_vp);
		_YXC_CHECK_MFX_WARN_RET(ms, YC("Failed to init intel encoder"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelEncoder::QueuedInput(const ybyte_t* pic_buffer, YXV_FrameType fType, yint64_t timestamp)
	{
		int suf_index = 0;
		mfx_surfaceinfo *pEncData = NULL;
		for (int i = 0; i < YUV_FRAMES; i++)
		{
			if (!this->_encodeData[i].surface.Data.Locked)
			{
				suf_index = i;
				pEncData = this->_encodeData + i;
				break;
			}
		}

		_YXC_CHECK_REPORT_NEW_RET(pEncData != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Can't found encode data now..."));
		ybyte_t* pLuma = (ybyte_t*)pic_buffer;
		ybyte_t* pChromaU = pLuma + this->_param.vParam.desc.w * this->_param.vParam.desc.h;
		ybyte_t* pChromaV = pChromaU + this->_param.vParam.desc.w * this->_param.vParam.desc.h / 4;
		mfxFrameInfo* pInfo = &this->_encodeData[suf_index].surface.Info;
		mfxFrameData* pData = &this->_encodeData[suf_index].surface.Data;

		mfxU16 pitch = pData->Pitch;

		if (_param.vParam.desc.pixFmt != YXV_PIX_FMT_NV12)
		{
			mfxU8* ptr = pData->Y + pInfo->CropX + pInfo->CropY * pData->Pitch;
			mfxU8* ptr2 = pData->UV + pInfo->CropX + (pInfo->CropY / 2) * pitch;
			convertYUVpitchtoNV12(pLuma, pChromaU, pChromaV, ptr, ptr2, this->_param.vParam.desc.w, this->_param.vParam.desc.h,
				this->_param.vParam.desc.w, pData->Pitch);
		}
		else
		{
			mfxU32 h = this->_param.vParam.desc.h;
			mfxU32 w = this->_param.vParam.desc.w;
			for (mfxU32 i = 0; i < h; i++)
			{
				memcpy(pData->Y+i*pitch,pLuma + i * w, w);
			}
			for (mfxU32 i = 0; i < h / 2; i++)
			{
				memcpy(pData->UV+pitch * i,pChromaU + i * w , w);
			}
		}

		pData->TimeStamp = timestamp;
		if (fType == YXV_FRAME_TYPE_I)
		{
			_encodeData[suf_index].surface.Info.PicStruct = 0;
			_Enctrl.FrameType = MFX_FRAMETYPE_I | MFX_FRAMETYPE_IDR | MFX_FRAMETYPE_REF;
		}
		else
		{
			_Enctrl.FrameType = MFX_FRAMETYPE_UNKNOWN;
		}

		mfxStatus ms = MFXVideoENCODE_EncodeFrameAsync(this->_session, NULL, &_encodeData[suf_index].surface,
			&_encodeData[suf_index].bitStream, &this->_syncp[suf_index]);
		_YXC_CHECK_REPORT_NEW_RET(ms != MFX_ERR_MORE_DATA, YXC_ERC_NEED_MORE_DATA, YC("Need more data to encode"));
		_YXC_CHECK_MFX_WARN_RET(ms, YC("_MFXVideoENCODE_EncodeFrameAsync"));

		++this->_uNumFramesGenerated;
		++this->_uNumFramesProcessed;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelEncoder::GetOutput(ybyte_t** ppOutputBuf, yuint32_t* puOutputSize, ybool_t* pbKey, yint64_t* pTimestamp)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_uNumFramesGenerated > 0, YXC_ERC_NO_DATA, YC("No frames has been generated"));

		yuint32_t suf_index = -1;
		for (yuint32_t i = 0; i < YUV_FRAMES; ++i)
		{
			if (this->_syncp[i] != 0)
			{
				suf_index = i;
				break;
			}
		}

		mfxStatus ms = MFXVideoCORE_SyncOperation(this->_session, this->_syncp[suf_index], MFX_INFINITE);
		_YXC_CHECK_MFX_WARN_RET(ms, YC("MFXVideoCORE_SyncOperation"));

		this->_syncp[suf_index] = 0;

		YXCLib::_MakeSureBufferOrFree((void*&)this->_pOutBuffer, this->_stcbOutBuffer, _encodeData[suf_index].bitStream.DataLength);
		_YXC_CHECK_REPORT_NEW_RET(this->_pOutBuffer != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Failed to alloc output bitstream memory"));

		memcpy(this->_pOutBuffer, _encodeData[suf_index].bitStream.Data, _encodeData[suf_index].bitStream.DataLength);

		*ppOutputBuf = this->_pOutBuffer;
		*puOutputSize = this->_encodeData[suf_index].bitStream.DataLength;
		*pTimestamp = this->_encodeData[suf_index].bitStream.TimeStamp;
		*pbKey = (this->_encodeData[suf_index].bitStream.FrameType & MFX_FRAMETYPE_I) ? TRUE : FALSE;
		if(!spsSize && !ppsSize && *pbKey==TRUE)
		{
			YXX_CritLocker locker(m_cLock);
			ybyte_t* ptemp=this->_pOutBuffer;
			int ifound=0,ipos1=0,ipos2=0,ipos3=0;
			for(int i=0;i<this->_encodeData[suf_index].bitStream.DataLength;i++)
			{
				if((ptemp[i]==00)&&(ptemp[i+1]==00)&&(ptemp[i+2]==00)&&(ptemp[i+3]==01))
				{
					ifound++;
					if (ifound==2)
					{
						ipos1=i+4;
					}
					if (ifound==3)
					{
						ipos2=i;
					}
					if(ifound==4)
					{
						ipos3=i;
						FreeSPSandPPS();
						spsSize=ipos2-ipos1;
						ppsSize=ipos3-ipos2-4;
						if (spsSize && ppsSize)
						{
							pSPS=new BYTE[spsSize];
							pPPS=new BYTE[ppsSize];
							memcpy(pSPS,_pOutBuffer+ipos1,spsSize);
							memcpy(pPPS,_pOutBuffer+ipos2+4,ppsSize);
						}
						break;
					}
					i=i+3;
					continue;
				}
			}
		}
		_encodeData[suf_index].bitStream.DataLength = 0;
		_encodeData[suf_index].bitStream.DataOffset = 0;
		--this->_uNumFramesGenerated;
		return YXC_ERC_SUCCESS;
	}

	void _IntelEncoder::GetSpsPps(ybyte_t** ppSps, ybyte_t** ppPps, yuint32_t* pSpsSize, yuint32_t* pPpsSize)
	{
		YXX_CritLocker locker(m_cLock);
		*ppSps=pSPS;
		*ppPps=pPPS;
		*pSpsSize=spsSize;
		*pPpsSize=ppsSize;
	}

	void _IntelEncoder::FreeSPSandPPS()
	{
		if (pSPS)
		{
			delete[] pSPS;
			pSPS=NULL;
			spsSize=0;
		}
		if (pPPS)
		{
			delete[] pPPS;
			pPPS=NULL;
			ppsSize=0;
		}
	}

	YXC_Status _IntelEncoder::_InitMfxBuffers(mfxSession ses, mfxVideoParam* par)
	{
		this->_sys_alloc.Init(NULL);

		mfxStatus ms;
		//mfxFrameAllocRequest allocReq = {0};
		//mfxStatus ms = MFXVideoENCODE_QueryIOSurf(ses, par, &allocReq);
		//_YXC_CHECK_MFX_WARN_RET(ms, YC("MFXVideoENCODE_QueryIOSurf"));

		mfxFrameAllocRequest allocReq = {0};
		allocReq.NumFrameMin = 1;
		allocReq.NumFrameSuggested = 1;
		allocReq.Type = MFX_MEMTYPE_SYSTEM_MEMORY | MFX_MEMTYPE_FROM_ENCODE | MFX_MEMTYPE_PERSISTENT_MEMORY;
		allocReq.Info = par->mfx.FrameInfo;

		for (int i = 0; i < YUV_FRAMES; ++i)
		{
			ms = this->_sys_alloc.AllocFrames(&allocReq, &this->_allocRes[i]);
			_YXC_CHECK_MFX_RET(ms, YC("AllocFrames"));

			mfxFrameSurface1& suf = _encodeData[i].surface;
			ms = this->_sys_alloc.LockFrame(this->_allocRes[i].mids[0], &suf.Data);
			_YXC_CHECK_MFX_RET(ms, YC("LockFrame"));

			suf.Info = allocReq.Info;

			_YCHK_MAL_ARR_R2(this->_encodeData[i].bitStream.Data, mfxU8, this->_param.vParam.uBitrate / 2);
			this->_encodeData[i].bitStream.MaxLength = this->_param.vParam.uBitrate / 2;
		}

		return YXC_ERC_SUCCESS;
	}
}

namespace _YXV_HWLib
{
	_IntelDecoder::_IntelDecoder()
	{
	}

	_IntelDecoder::~_IntelDecoder()
	{
		this->Close();
	}

	void _IntelDecoder::GetInfo(YXV_HWDecoderInfo* pInfo)
	{
		sprintf(pInfo->szDevName, "Intel Device %d", this->_uDevIndex);
		pInfo->inputType = YXV_PIX_FMT_YUV420P;
	}

	YXC_Status _IntelDecoder::Init(_HWDevBase* dev)
	{
		mfxVersion mfVer;
		mfVer.Minor = 1;
		mfVer.Major = 1;

		_IntelDev* iDev = (_IntelDev*)dev;
		yuint32_t impl = _IntelDev::s_implArr[iDev->GetDevIndex()];
		mfxStatus ms = MFXInit(impl, &mfVer, &this->_session);
		_YXC_CHECK_MFX_RET(ms, YC("Failed to init device(%d)"), iDev->GetDevIndex());

		memset(&this->_surface, 0, sizeof(this->_surface));
		memset(&this->_vp, 0, sizeof(this->_vp));
		memset(&this->_bs, 0, sizeof(this->_bs));
		this->_uDevIndex = iDev->GetDevIndex();
		this->_pNvBuffer = NULL;
		this->_uNumFramesGenerated = 0;
		this->_uNumFramesProcessed = 0;

		return YXC_ERC_SUCCESS;
	}

	void _IntelDecoder::Close()
	{
		MFXVideoDECODE_Close(this->_session);

		if (this->_surface.Data.R)
		{
			free(this->_surface.Data.R);
			this->_surface.Data.R = NULL;
		}

		if (this->_pNvBuffer)
		{
			free(this->_pNvBuffer);
			this->_pNvBuffer = NULL;
		}

		if (this->_bs.Data)
		{
			free(this->_bs.Data);
			this->_bs.Data = NULL;
		}
		MFXClose(this->_session);
		this->_session = NULL;
	}

	YXC_Status _IntelDecoder::Config(const YXV_HWCodecParam* param)
	{
		memcpy(&this->_param, param, sizeof(YXV_HWCodecParam));

		_YCHK_MAL_ARR_R2(this->_surface.Data.Y, mfxU8, this->_param.vParam.desc.w * this->_param.vParam.desc.h * 3);// / 2);
		this->_surface.Data.UV = this->_surface.Data.Y + this->_param.vParam.desc.w * this->_param.vParam.desc.h;
		this->_surface.Data.Pitch = this->_param.vParam.desc.w;

		_YCHK_MAL_ARR_R2(this->_pNvBuffer, ybyte_t, this->_param.vParam.desc.w * this->_param.vParam.desc.h * 3);// / 2);
		this->_bs.MaxLength = this->_param.vParam.desc.w * this->_param.vParam.desc.h*3;// / 3;
		_YCHK_MAL_ARR_R2(this->_bs.Data, ybyte_t, this->_bs.MaxLength + param->extData.uExt);

		if (param->extData.uExt > 0)
		{
			_InitVideoParams(param, MFX_IOPATTERN_OUT_SYSTEM_MEMORY, this->_vp);

			this->_vp.mfx.FrameInfo.AspectRatioW = 1;
			this->_vp.mfx.FrameInfo.AspectRatioH = 1;
			this->_vp.mfx.CodecProfile = param->uProfile;
			this->_vp.mfx.CodecLevel = 0;
			this->_vp.AsyncDepth = 1;
			this->_vp.mfx.RateControlMethod = 0;
			this->_vp.mfx.TargetUsage = 0;
			this->_vp.mfx.TargetKbps = 0;

			this->_param.extData.pExt = this->_bs.Data + this->_bs.MaxLength;
			memcpy(this->_param.extData.pExt, param->extData.pExt, param->extData.uExt);

			mfxStatus ms = MFXVideoDECODE_Init(this->_session, &this->_vp);
			_YXC_CHECK_MFX_RET(ms, YC("Failed to init intel encoder"));

			this->_surface.Info = this->_vp.mfx.FrameInfo;
			this->_uNumFramesProcessed++;
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelDecoder::QueuedInput(const YXV_Packet* packet)
	{
		memmove(this->_bs.Data, this->_bs.Data + this->_bs.DataOffset, this->_bs.DataLength);
		if (packet->bKeyPacket && this->_param.extData.uExt > 0)
		{
			memcpy(this->_bs.Data + this->_bs.DataLength, this->_param.extData.pExt,
				this->_param.extData.uExt);
			this->_bs.DataLength += this->_param.extData.uExt;
		}

		memcpy(this->_bs.Data + this->_bs.DataLength, packet->pData, packet->uDataLen);
		this->_bs.TimeStamp = packet->uRefTime;
		this->_bs.DataOffset = 0;
		this->_bs.DataLength = packet->uDataLen + this->_bs.DataLength;

		if (this->_uNumFramesProcessed == 0)
		{
			mfxVideoParam vp = {0};
			vp.mfx.CodecId = MFX_CODEC_AVC;
			mfxStatus ms1 = MFXVideoDECODE_DecodeHeader(this->_session, &this->_bs, &vp);
			_YXC_CHECK_MFX_RET(ms1, YC("MFXVideoDECODE_DecodeHeader"));
			++this->_uNumFramesProcessed;

			this->_vp = vp;
			this->_vp.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;
			mfxStatus ms = MFXVideoDECODE_Init(this->_session, &this->_vp);
			_YXC_CHECK_MFX_RET(ms, YC("MFXVideoDECODE_Init"));

			this->_surface.Info = this->_vp.mfx.FrameInfo;
		}

		mfxFrameSurface1* surface_out = NULL;
		mfxStatus ms = MFXVideoDECODE_DecodeFrameAsync(this->_session, &this->_bs, &this->_surface, &surface_out, &this->_syncp);
		while (ms == MFX_WRN_DEVICE_BUSY || ms == MFX_WRN_VIDEO_PARAM_CHANGED)
		{
			if (ms == MFX_WRN_DEVICE_BUSY)
			{
				YXCLib::OSSleep(1);
			}

			ms = MFXVideoDECODE_DecodeFrameAsync(this->_session, &this->_bs, &this->_surface, &surface_out, &this->_syncp);
		}
		_YXC_CHECK_REPORT_NEW_RET(ms != MFX_ERR_MORE_DATA, YXC_ERC_NEED_MORE_DATA, YC("Need more data to decode"));
		_YXC_CHECK_MFX_RET(ms, YC("_MFXVideoENCODE_EncodeFrameAsync %lld"), packet->uRefTime);

		++this->_uNumFramesGenerated;
		++this->_uNumFramesProcessed;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelDecoder::GetOutput(YXV_Frame* sample)
	{
		_YXC_CHECK_REPORT_NEW_RET(this->_uNumFramesGenerated > 0, YXC_ERC_NO_DATA, YC("No frames has been generated"));

		mfxStatus ms = MFXVideoCORE_SyncOperation(this->_session, this->_syncp, MFX_INFINITE);
		_YXC_CHECK_MFX_RET(ms, YC("MFXVideoCORE_SyncOperation"));

		yuint32_t w = this->_surface.Info.CropW, h = this->_surface.Info.CropH;

		sample->bGotFrame = TRUE;
		sample->fType = YXV_FRAME_TYPE_NONE;
		sample->uRefTime = this->_surface.Data.TimeStamp;
		sample->uSize[0] = w;
		sample->uSize[1] = w / 2;
		sample->uSize[2] = w / 2;
		sample->pData[0] = this->_pNvBuffer;
		sample->pData[1] = this->_pNvBuffer + w * this->_vp.mfx.FrameInfo.CropH;
		sample->pData[2] = sample->pData[1] + w * this->_vp.mfx.FrameInfo.CropH / 4;

		if (_param.vParam.desc.pixFmt!=EV_PIX_FMT_NV12)
		{
			convertNV12toYUVpitch(sample->pData[0], sample->pData[1], sample->pData[2], this->_surface.Data.Y,
				this->_surface.Data.UV, w, h, this->_surface.Data.Pitch, w);
		}
		else
		{
			memcpy(sample->pData[0],this->_surface.Data.Y,w*h);
			memcpy(sample->pData[1],this->_surface.Data.UV,w*h/2);
		}
		--this->_uNumFramesGenerated;
		return YXC_ERC_SUCCESS;
	}
}


namespace _YXV_HWLib
{
	_IntelVPP::_IntelVPP()
	{

	}

	_IntelVPP::~_IntelVPP()
	{

	}

	void _IntelVPP::Close()
	{
		MFXVideoVPP_Close(_session);
		for (int i = 0; i < 2; ++i)
		{
			if (pSurfaces[i].Data.R)
			{
				this->_sys_alloc.UnlockFrame(this->response[i].mids[0], &this->pSurfaces[i].Data);
			}
			this->_sys_alloc.FreeFrames(&this->response[i]);
		}
		this->_sys_alloc.Close();
	}

	YXC_Status _IntelVPP::Init(_HWDevBase* dev)
	{
		mfxVersion mfVer;
		mfVer.Minor = 1;
		mfVer.Major = 1;

		_IntelDev* iDev = (_IntelDev*)dev;
		yuint32_t impl = _IntelDev::s_implArr[iDev->GetDevIndex()];
		mfxStatus ms = MFXInit(impl, &mfVer, &this->_session);
		_YXC_CHECK_MFX_RET(ms, YC("Failed to init device(%d)"), iDev->GetDevIndex());

		memset(&this->_surface, 0, sizeof(this->_surface));
		memset(&this->_vp, 0, sizeof(this->_vp));

		this->_uDevIndex = iDev->GetDevIndex();

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelVPP::Config(const YXV_HWCodecParam* pInParams,const YXV_HWCodecParam* pOutParams)
	{
		_vp.mfx.CodecProfile = pInParams->uProfile;
		_vp.mfx.CodecLevel = 0;
		_vp.mfx.RateControlMethod = 0;
		_vp.mfx.TargetUsage = 0;
		_vp.mfx.TargetKbps = 0;
		_vp.vpp.In.FourCC  = MFX_FOURCC_NV12;
		_vp.vpp.In.CropX = 0;
		_vp.vpp.In.CropY = 0;
		_vp.vpp.In.CropW = pInParams->vParam.desc.w;
		_vp.vpp.In.CropH = pInParams->vParam.desc.h;
		_vp.vpp.In.PicStruct=MFX_PICSTRUCT_PROGRESSIVE;
		_vp.vpp.In.Width = MSDK_ALIGN16(pInParams->vParam.desc.w);
		_vp.vpp.In.Height = (MFX_PICSTRUCT_PROGRESSIVE == _vp.mfx.FrameInfo.PicStruct) ?
			MSDK_ALIGN16(pInParams->vParam.desc.h) : MSDK_ALIGN32(pInParams->vParam.desc.h);
		_vp.vpp.In.PicStruct=MFX_PICSTRUCT_PROGRESSIVE;
		ConvertFrameRate(30, &_vp.vpp.In.FrameRateExtN, &_vp.vpp.In.FrameRateExtD);
		_vp.IOPattern = MFX_IOPATTERN_IN_SYSTEM_MEMORY | MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

		_vp.vpp.Out.Width = MSDK_ALIGN16(pOutParams->vParam.desc.w);
		_vp.vpp.Out.Height = (MFX_PICSTRUCT_PROGRESSIVE == _vp.mfx.FrameInfo.PicStruct) ?
			MSDK_ALIGN16(pOutParams->vParam.desc.h) : MSDK_ALIGN32(pOutParams->vParam.desc.h);
		_vp.vpp.Out.FourCC  = MFX_FOURCC_NV12;
		_vp.vpp.Out.CropX = 0;
		_vp.vpp.Out.CropY = 0;
		_vp.vpp.Out.CropW = pOutParams->vParam.desc.w;
		_vp.vpp.Out.CropH = pOutParams->vParam.desc.h;
		_vp.vpp.Out.PicStruct=MFX_PICSTRUCT_PROGRESSIVE;
		ConvertFrameRate(30, &_vp.vpp.Out.FrameRateExtN, &_vp.vpp.Out.FrameRateExtD);

		InitVPP();
		InitBufferVPP();

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelVPP::InitVPP()
	{

		MFXVideoVPP_Close(_session);
		mfxStatus ms=MFXVideoVPP_Init(_session,&_vp);
		_YXC_CHECK_MFX_RET(ms, YC("MFXVideoVPP_Init"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelVPP::InitBufferVPP()
	{
		this->_sys_alloc.Init(NULL);

		mfxFrameAllocRequest allocReq[2] = {0};
		mfxStatus ms = MFXVideoVPP_QueryIOSurf(_session, &_vp, allocReq);
		_YXC_CHECK_MFX_RET(ms, YC("MFXVideoVPP_QueryIOSurf"));

		for (int i = 0; i < 2; ++i)
		{
			allocReq[i].NumFrameMin = 1;
			allocReq[i].NumFrameSuggested = 1;
			allocReq[i].Type |= MFX_MEMTYPE_SYSTEM_MEMORY;
			ms = this->_sys_alloc.AllocFrames(&allocReq[i], &this->response[i]);
			_YXC_CHECK_MFX_RET(ms, YC("AllocFrames"));

			if (i==0)
			{
				pSurfaces[i].Info=_vp.vpp.In;
			}
			else
				pSurfaces[i].Info=_vp.vpp.Out;
			pSurfaces[i].Data.MemId = this->response[i].mids[i];
			ms = this->_sys_alloc.LockFrame(this->response[i].mids[0], &pSurfaces[i].Data);
			pSurfaces[i].Data.FrameOrder=0;
			pSurfaces[i].Data.TimeStamp=0;
			pSurfaces[i].Data.MemId=NULL;
			pSurfaces[i].Data.DataFlag=0;
			pSurfaces[i].Data.Corrupted=0;
			pSurfaces[i].Data.Locked=0;
			_YXC_CHECK_MFX_RET(ms, YC("LockFrame"));
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _IntelVPP::RunFrameVPP(ybyte_t** pbuffer_in,ybyte_t* pbuffer_out)
	{
		if (pbuffer_in)
		{
			memcpy(pSurfaces[VP_IN].Data.Y,pbuffer_in[0],_vp.vpp.In.CropW*_vp.vpp.In.CropH);
			memcpy(pSurfaces[VP_IN].Data.UV,pbuffer_in[1],(_vp.vpp.In.CropW*_vp.vpp.In.CropH)/2);
		}
		mfxStatus ms=MFXVideoVPP_RunFrameVPPAsync(_session,&pSurfaces[VP_IN],&pSurfaces[VP_OUT],NULL,&syncp);
		if (MFX_ERR_MORE_DATA == ms)
		{

		}
		if (MFX_ERR_MORE_SURFACE == ms)
		{
			ms = MFX_ERR_NONE;
		}
		_YXC_CHECK_MFX_RET(ms, YC("MFXVideoVPP_RunFrameVPPAsync"));

		ms=MFXVideoCORE_SyncOperation(_session, syncp, MSDK_VPP_WAIT_INTERVAL);
		_YXC_CHECK_MFX_RET(ms, YC("MFXVideoCORE_SyncOperation"));
		if (pbuffer_out)
		{
			//	memcpy(pbuffer_out,pSurfaces[VP_OUT].Data.Y,_vp.vpp.Out.CropW*_vp.vpp.Out.CropH);
			//	memcpy(pbuffer_out+_vp.vpp.Out.CropW*_vp.vpp.Out.CropH,pSurfaces[VP_OUT].Data.UV,_vp.vpp.Out.CropW*_vp.vpp.Out.CropH/2);

			mfxFrameData* pData=&pSurfaces[VP_OUT].Data;
			mfxFrameInfo* pInfo=&_vp.vpp.Out;
			mfxU32 i, h, w, pitch;
			mfxU8* ptr,*ptr2;
			if (pInfo->CropH > 0 && pInfo->CropW > 0)
			{
				w = pInfo->CropW;
				h = pInfo->CropH;
			}
			else
			{
				w = pInfo->Width;
				h = pInfo->Height;
			}

			pitch = pData->Pitch;
			//	ptr   = pData->Y + (pInfo->CropX ) + (pInfo->CropY ) * pitch;
			ptr=pData->Y;
			for (i = 0; i < h; i++)
			{
				memcpy(pbuffer_out+w*i,ptr+i * pitch, w);
			}
			ptr2=pbuffer_out+i*w;
			// UV data
			h >>= 1;
			//		ptr  = pData->UV + (pInfo->CropX ) + (pInfo->CropY >> 1) * pitch;
			ptr  = pData->UV;
			for(i = 0; i < h; i++)
			{
				memcpy( ptr2+i*w,ptr+ i * pitch, w);
			}
		}
		return YXC_ERC_SUCCESS;
	}


}

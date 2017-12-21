#define __MODULE__ "EV.Utils.AVBuilderElement"

#include <YXC_AVKernel/YXV_AVBuilder.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/Utils/_YXV_AVBuilderElementBase.hpp>
#include <YXC_AVKernel/YXV_ffmpeg.h>
#include <YXC_AVKernel/YXV_AVScaler.h>
#include <YXC_AVKernel/Utils/_YXV_Common.hpp>
#include <deque>

template <typename T>
static YXC_Status _std_queue_push(std::deque<T>& queue, const T& obj)
{
	try
	{
		queue.push_back(obj);
		return YXC_ERC_SUCCESS;
	}
	catch (std::exception& e)
	{
		_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%@"), e.what());
	}
}

namespace _YXV_AVBuilder
{
	class _AVBuilderText : public _AVBuilderElementBase
	{
	public:
		_AVBuilderText() : _font(NULL)
		{

		}

		~_AVBuilderText()
		{
			if (this->_font)
			{
				YXCLib::TDelete(this->_font);
				this->_font = NULL;
			}
		}

	public:
		YXC_Status Init(YXV_AVSource* pSource)
		{
			YXC_Status rc = _YXV_AVUtils::_VFont::CreateInstance(this->_pFilter->GetFilterType(), &this->_font);
			_YXC_CHECK_RC_RET(rc);

			rc = this->_font->Init(&pSource->textDesc.fontDesc);
			_YXC_CHECK_RC_RET(rc);

			return YXC_ERC_SUCCESS;
		}

		YXC_Status NextFrame(yint64_t i64Duration, YXV_Frame* pSampleA, YXV_Frame* pSampleV)
		{
			if (!this->_CanElementPlay()) return YXC_ERC_SUCCESS;

			double x, y, w, h;
			this->_ConvertWH(x, y, w, h);

			YXC_Status rc = this->_pFilter->DrawText2(x, y, w, h, this->_pSourceDesc->textDesc.pszText, this->_pSourceDesc->textDesc.uTextColor,
				this->_pSourceDesc->textDesc.uTextAlign, this->_font);
			_YXC_CHECK_RC_RET(rc);

			return YXC_ERC_SUCCESS;
		}

	public:
		_YXV_AVUtils::_VFont* _font;
	};

	class _AVBuilderPicture : public _AVBuilderElementBase
	{
	public:
		_AVBuilderPicture() : _pic(NULL)
		{

		}

		~_AVBuilderPicture()
		{
			if (this->_pic)
			{
				YXCLib::TDelete(this->_pic);
				this->_pic = NULL;
			}
		}

		YXC_Status Init(YXV_AVSource* pSource)
		{
			YXC_Status rc = _YXV_AVUtils::_VPicture::CreateInstance(this->_pFilter->GetFilterType(), &this->_pic);
			_YXC_CHECK_RC_RET(rc);

			rc = this->_pic->Init(pSource->imageDesc.imgPath);
			_YXC_CHECK_RC_RET(rc);

			return YXC_ERC_SUCCESS;
		}

		YXC_Status NextFrame(yint64_t i64Duration, YXV_Frame* pSampleA, YXV_Frame* pSampleV)
		{
			if (!this->_CanElementPlay()) return YXC_ERC_SUCCESS;

			double x, y, w, h;
			this->_ConvertWH(x, y, w, h);

			YXC_Status rc = this->_pFilter->DrawPicture(this->_pic, x, y, w, h, &this->_pSourceDesc->imageDesc.picInfo);
			_YXC_CHECK_RC_RET(rc);

			return YXC_ERC_SUCCESS;
		}

	private:
		_YXV_AVUtils::_VPicture* _pic;
	};

	class _AVBuilderAVFile : public _AVBuilderElementBase
	{
	public:
		_AVBuilderAVFile();

		~_AVBuilderAVFile();

	public:
		YXC_Status Init(YXV_AVSource* pSource);

		YXC_Status SeekTo(yint64_t i64Time);

		YXC_Status NextFrame(yint64_t i64Duration, YXV_Frame* pSampleA, YXV_Frame* pSampleV);

		YXC_Status Skip(yint64_t i64Duration);

	private:
		YXC_Status _ReadSamples(yint64_t i64Start, yint64_t i64Next);

		YXC_Status _ReadVSample(yint64_t i64Start, yint64_t i64Next);

		YXC_Status _ReadASample(yint64_t i64Start, yuint32_t uNumSamples);

		YXC_Status _ReadVPacket(YXV_Packet* pVPacket);

		YXC_Status _ReadAPacket(YXV_Packet* pAPacket);

		YXC_Status _FillAudioSample(yint64_t i64Start, YXV_Frame* pAFrame);

		void _ClearBuffers();

	private:
		YXV_Frame _vSampleCur;
		YXV_Frame _vSampleNext;
		YXV_Frame _aSampleCur;
		YXV_Frame _aSampleBuf;

		YXV_FFDec _decoder;
		YXV_FFFormat _inFmt;

		ybool_t _bHasKey;
		ybool_t _bCorrectPos;
		YXCLib::Nullable<yint64_t> _iTotalSamples;

		YXV_VScaler _vScaler;
		YXV_ASampler _aSampler;

		std::deque<YXV_Packet> _vPacketQueue;
		std::deque<YXV_Packet> _aPacketQueue;
	};
}

namespace _YXV_AVBuilder
{
	_AVBuilderElementBase::_AVBuilderElementBase() : _pSourceDesc(NULL), _i64CurrentTime(0)
	{
	}

	_AVBuilderElementBase::~_AVBuilderElementBase()
	{
	}

	YXC_Status _AVBuilderElementBase::CreateElement(_YXV_AVUtils::_VFilter* filter, _YXV_AVUtils::_AFilter* aFilter,
		YXV_AVSource* pSource, _AVBuilderElementBase** ppOutElement)
	{
		_AVBuilderElementBase* pOut = NULL;
		switch (pSource->sourceType)
		{
		case YXV_AV_SOURCE_AV_FILE:
			_YCHK_MAL_R2(pOut, _AVBuilderAVFile);
			new (pOut) _AVBuilderAVFile();
			break;
		case YXV_AV_SOURCE_TEXT:
			_YCHK_MAL_R2(pOut, _AVBuilderText);
			new (pOut) _AVBuilderText();
			break;
		case YXV_AV_SOURCE_PICTURE:
			_YCHK_MAL_R2(pOut, _AVBuilderPicture);
			new (pOut) _AVBuilderPicture();
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_PARAMETER, YC("Invalid source type(%d)"), pSource->sourceType);
		}

		YXCLib::HandleRef<_AVBuilderElementBase*> pOut_res(pOut, YXCLib::TDelete<_AVBuilderElementBase>);

		pOut->_pFilter = filter;
		pOut->_pAFilter = aFilter;
		pOut->_pSourceDesc = pSource;

		YXC_Status rc = pOut->Init(pSource);
		_YXC_CHECK_RC_RET(rc);

		*ppOutElement = pOut_res.Detach();
		return YXC_ERC_SUCCESS;
	}

	void _AVBuilderElementBase::_ConvertWH(double& x, double& y, double& w, double& h)
	{
		YXV_AVSource* source = this->_pSourceDesc;

		x = source->fltX;
		y = source->fltY;
		w = source->fltW;
		h = source->fltH;

		if (source->bXInRatio) x *= this->_pFilter->GetPicDesc().w;
		if (source->bYInRatio) y *= this->_pFilter->GetPicDesc().h;
		if (source->bWInRatio) w *= this->_pFilter->GetPicDesc().w;
		if (source->bHInRatio) h *= this->_pFilter->GetPicDesc().h;

		if (source->bWReverse)
		{
			x -= w;
		}

		if (source->bHReverse)
		{
			y -= h;
		}
	}

	YXC_Status _AVBuilderElementBase::Skip(yint64_t i64Duration)
	{
		this->_i64CurrentTime += i64Duration;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderElementBase::SeekTo(yint64_t i64Time)
	{
		this->_i64CurrentTime = i64Time;
		return YXC_ERC_SUCCESS;
	}

	ybool_t _AVBuilderElementBase::_CanElementPlay()
	{
		if (this->_pSourceDesc->i64Start == 0 && this->_pSourceDesc->i64End == 0) /* Play all time. */
		{
			return TRUE;
		}

		ybool_t bCanPlay = this->_pSourceDesc->i64Start <= this->_i64CurrentTime
			&& this->_pSourceDesc->i64End > this->_i64CurrentTime;
		return bCanPlay;
	}
}

namespace _YXV_AVBuilder
{
	_AVBuilderAVFile::_AVBuilderAVFile() : _decoder(NULL), _inFmt(NULL), _vScaler(NULL), _aSampler(NULL), _iTotalSamples(),
		_bHasKey(FALSE), _bCorrectPos(FALSE)
	{
		memset(&this->_vSampleCur, 0, sizeof(YXV_Frame));
		memset(&this->_vSampleNext, 0, sizeof(YXV_Frame));
		memset(&this->_aSampleCur, 0, sizeof(YXV_Frame));
		memset(&this->_aSampleBuf, 0, sizeof(YXV_Frame));
	}

	_AVBuilderAVFile::~_AVBuilderAVFile()
	{
		this->_ClearBuffers();

		YXV_FrameUnref(&this->_aSampleCur);
		YXV_FrameUnref(&this->_aSampleBuf);
	}

	YXC_Status _AVBuilderAVFile::Init(YXV_AVSource* pSource)
	{
		YXV_VCodecID vCodecId;
		YXV_ACodecID aCodecId;
		YXV_FFormatID fmtId;
		YXC_Status rc = YXV_FFFormatCreateRead(pSource->mfDesc.mfPath, &vCodecId, &aCodecId, &fmtId, &this->_inFmt);
		_YXC_CHECK_RC_RET(rc);

		YXV_VParam vParam;
		YXV_AParam aParam;
		YXV_FFVFormatParam vSpec;
		YXV_FFAFormatParam aSpec;

		rc = YXV_FFDecCreate(vCodecId, aCodecId, &this->_decoder);
		if (vCodecId != YXV_VCODEC_ID_NONE)
		{
			rc = YXV_FFFormatReadParamV(this->_inFmt, &vParam, &vSpec);
			_YXC_CHECK_RC_RET(rc);

			rc = YXV_FFObjConfigV(this->_decoder, &vParam, &vSpec[1]);
			_YXC_CHECK_RC_RET(rc);
		}

		if (aCodecId != YXV_ACODEC_ID_NONE)
		{
			rc = YXV_FFFormatReadParamA(this->_inFmt, &aParam, &aSpec);
			_YXC_CHECK_RC_RET(rc);

			rc = YXV_AFrameAlloc(&aParam.desc, 100000, &this->_aSampleBuf);
			_YXC_CHECK_RC_RET(rc);

			rc = YXV_AFrameAlloc(&aParam.desc, 100000, &this->_aSampleCur);
			_YXC_CHECK_RC_RET(rc);

			this->_aSampleBuf.uNumSamples = 0;
			this->_aSampleCur.uNumSamples = 0;

			rc = YXV_FFObjConfigA(this->_decoder, &aParam, &aSpec[1]);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::NextFrame(yint64_t i64Duration, YXV_Frame* pSampleA, YXV_Frame* pSampleV)
	{
		if (!this->_CanElementPlay())
		{
			return YXC_ERC_SUCCESS;
		}

		yint64_t time_begin = this->_i64CurrentTime, time_end = time_begin + i64Duration;
		time_begin -= this->_pSourceDesc->i64Start;
		time_begin += this->_pSourceDesc->i64StartOff;
		time_end -= this->_pSourceDesc->i64Start;
		time_end += this->_pSourceDesc->i64StartOff;

		if (!this->_bCorrectPos)
		{
			if (time_begin != 0)
			{
				YXC_Status rcSeek = YXV_FFFormatSeek(this->_inFmt, time_begin);
				_YXC_CHECK_RC_RET(rcSeek);
			}
			this->_bCorrectPos = TRUE;
		}

		YXC_Status rc = this->_ReadSamples(time_begin, time_end);
		_YXC_CHECK_RC_RET(rc);

		if (this->_vSampleCur.bGotFrame)
		{
			double x, y, w, h;
			this->_ConvertWH(x, y, w, h);

			if (!this->_vScaler)
			{
				YXV_PicDesc inPicDesc = this->_vSampleCur.u1.picDesc;
				YXV_PicDesc outPicDesc = this->_pFilter->GetPicDesc();
				outPicDesc.w = YXC_Round(w);
				outPicDesc.h = YXC_Round(h);
				rc = YXV_VScalerCreate(&inPicDesc, &outPicDesc, NULL, &this->_vScaler);
				_YXC_CHECK_RC_RET(rc);
			}

			rc = YXV_VScalerScale(this->_vScaler, &this->_vSampleCur, pSampleV);
			_YXC_CHECK_RC_RET(rc);
		}

		if (this->_aSampleCur.uNumSamples > 0)
		{
			if (!this->_aSampler)
			{
				YXV_SampleDesc inSampleDesc = this->_aSampleCur.u1.sampleDesc;
				YXV_SampleDesc outSampleDesc = this->_pAFilter->GetSampleDesc();
				rc = YXV_ASamplerCreate(inSampleDesc.sampleFmt, inSampleDesc.uNumChannels, inSampleDesc.uSampleRate,
					outSampleDesc.sampleFmt, outSampleDesc.uNumChannels, outSampleDesc.uSampleRate, &this->_aSampler);
				_YXC_CHECK_RC_RET(rc);
			}

			if (pSampleA->uNumSamples == 0) /* No mix, direct put when no audio existed. */
			{;
				rc = YXV_ASamplerResample(this->_aSampler, &this->_aSampleCur, pSampleA);
				_YXC_CHECK_RC_RET(rc);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::_ReadSamples(yint64_t i64Start, yint64_t i64Next)
	{
		if (this->_iTotalSamples.IsNull())
		{
			yint64_t total_samples = i64Start / 10000 * this->_aSampleBuf.u1.sampleDesc.uSampleRate / 1000;
			this->_iTotalSamples.Set(total_samples);
		}

		YXC_Status rcV = this->_ReadVSample(i64Start, i64Next);
		if (rcV == YXC_ERC_EOF) rcV = YXC_ERC_SUCCESS;
		_YXC_CHECK_RC_RET(rcV);

		if (this->_pAFilter != NULL)
		{
			YXV_SampleDesc sampleDesc = this->_pAFilter->GetSampleDesc();
			yint64_t uSamplesNeed = i64Next / 10000 * sampleDesc.uSampleRate / 1000; /* Need samples. */
			yint64_t uSamplesToRead = uSamplesNeed - this->_iTotalSamples.Get();
			YXC_Status rcA = this->_ReadASample(i64Start, uSamplesToRead);
			if (rcA == YXC_ERC_EOF) rcA = YXC_ERC_SUCCESS;
			_YXC_CHECK_RC_RET(rcA);

			yuint32_t uSampleCpy = YXCLib::TMin<yuint32_t>(this->_aSampleBuf.uNumSamples, uSamplesToRead);
			this->_aSampleCur.uNumSamples = 0;
			YXV_AFrameCopy(&this->_aSampleCur, 0, &this->_aSampleBuf, 0, uSampleCpy);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::_ReadVPacket(YXV_Packet* pVPacket)
	{
		if (this->_vPacketQueue.size() > 0)
		{
			YXV_Packet pack = this->_vPacketQueue.front();
			this->_vPacketQueue.pop_front();

			*pVPacket = pack;
			return YXC_ERC_SUCCESS;
		}

		while (TRUE)
		{
			YXV_Packet pkt = {0};
			YXC_Status rc = YXV_FFFormatRead(this->_inFmt, &pkt);
			_YXC_CHECK_RC_RET(rc);

			if (pkt.uStreamIndex == 0)
			{
				*pVPacket = pkt;
				return YXC_ERC_SUCCESS;
			}
			else if (pkt.uStreamIndex == 1)
			{
				_std_queue_push(this->_aPacketQueue, pkt);
			}
			else
			{
				YXV_PacketUnref(&pkt);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::_ReadAPacket(YXV_Packet* pAPacket)
	{
		if (this->_aPacketQueue.size() > 0)
		{
			YXV_Packet pack = this->_aPacketQueue.front();
			this->_aPacketQueue.pop_front();

			*pAPacket = pack;
			return YXC_ERC_SUCCESS;
		}

		while (TRUE)
		{
			YXV_Packet pkt = {0};
			YXC_Status rc = YXV_FFFormatRead(this->_inFmt, &pkt);
			_YXC_CHECK_RC_RET(rc);

			if (pkt.uStreamIndex == 0)
			{
				//YXV_PacketUnref(&pkt);
				rc = _std_queue_push(this->_vPacketQueue, pkt);
				_YXC_CHECK_RC_RET(rc);
			}
			else if (pkt.uStreamIndex == 1)
			{
				*pAPacket = pkt;
				return YXC_ERC_SUCCESS;
			}
			else
			{
				YXV_PacketUnref(&pkt);
			}
		}
	}

	YXC_Status _AVBuilderAVFile::_ReadVSample(yint64_t i64Start, yint64_t i64Next)
	{
		if (!this->_vSampleNext.bGotFrame || i64Start >= this->_vSampleNext.uRefTime)
		{
			if (this->_vSampleCur.uRefTime != this->_vSampleNext.uRefTime)
			{
				YXV_FrameUnref(&this->_vSampleCur);
				this->_vSampleCur = this->_vSampleNext;
				YXV_FrameRef(&this->_vSampleCur);
			}

			while (TRUE)
			{
				YXV_Packet pkt;
				YXC_Status rc = this->_ReadVPacket(&pkt);
				_YXC_CHECK_RC_RET(rc);

				YXCLib::HandleRef<YXV_Packet*> pkt_res(&pkt, YXV_PacketUnref);

				if (pkt.bKeyPacket) this->_bHasKey = TRUE;
				if (!this->_bHasKey) continue;

				YXV_Frame vFrame;
				rc = YXV_FFDecProcessV2(this->_decoder, &pkt, &vFrame);

				if (rc == YXC_ERC_SUCCESS && vFrame.bGotFrame)
				{
					if (vFrame.uRefTime > i64Start)
					{
						YXV_FrameUnref(&this->_vSampleNext);
						this->_vSampleNext = vFrame;
						break;
					}
					else
					{
						YXV_FrameUnref(&this->_vSampleCur);
						this->_vSampleCur = vFrame;
					}
				}
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::_ReadASample(yint64_t i64Start, yuint32_t uNumSamples)
	{
		static int i = 0;
		while (uNumSamples > this->_aSampleBuf.uNumSamples)
		{
			YXV_Packet pkt;
			YXC_Status rc = this->_ReadAPacket(&pkt);
			_YXC_CHECK_RC_RET(rc);

			YXCLib::HandleRef<YXV_Packet*> pkt_res(&pkt, YXV_PacketUnref);

			YXV_Frame frame = { 0 };
			rc = YXV_FFDecProcessA(this->_decoder, pkt.pData, pkt.uDataLen, pkt.uRefTime, &frame);
			//_YXC_CHECK_RC_RET(rc);

			if (rc == YXC_ERC_SUCCESS && frame.uNumSamples > 0)
			{
				rc = this->_FillAudioSample(i64Start, &frame);
				YXV_FrameUnref(&frame);
				_YXC_CHECK_RC_RET(rc);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::_FillAudioSample(yint64_t i64Start, YXV_Frame* pAFrame)
	{
		yuint32_t ur = pAFrame->u1.sampleDesc.uSampleRate;
		yint64_t iFrameIndexSrc1 = pAFrame->uRefTime / 10000 * ur / 1000;
		yint64_t iFrameIndexSrc2 = iFrameIndexSrc1 + pAFrame->uNumSamples;

		yint64_t iFrameIndexDst1 = this->_iTotalSamples.Get() + this->_aSampleBuf.uNumSamples;

		yuint32_t uFillThresh = pAFrame->u1.sampleDesc.uSampleRate / 5; /* Max allow 200ms post or first sample. */
		if (iFrameIndexDst1 < iFrameIndexSrc1) /* Delay ? */
		{
			yuint32_t uSamplesDif = iFrameIndexSrc1 - iFrameIndexDst1;
			if (uSamplesDif > uFillThresh || this->_aSampleCur.uNumSamples == 0) /* Fill it by zero data. */
			{
				YXV_Frame aZeroFrame;
				YXC_Status rc = YXV_AFrameAlloc(&pAFrame->u1.sampleDesc, uSamplesDif, &aZeroFrame);
				_YXC_CHECK_RC_RET(rc);

				YXV_FrameZero(&aZeroFrame);
				rc = YXV_AFrameCopy(&this->_aSampleBuf, this->_aSampleBuf.uNumSamples, &aZeroFrame, 0, uSamplesDif);
				YXV_FrameUnref(&aZeroFrame);
				_YXC_CHECK_RC_RET(rc);
			}

			iFrameIndexDst1 = iFrameIndexSrc1;
		}
		else
		{
			yuint32_t uSamplesDif = iFrameIndexDst1 - iFrameIndexSrc1;
			if (uSamplesDif < uFillThresh)
			{
				iFrameIndexDst1 = iFrameIndexSrc1;
			}
		}

		if (iFrameIndexSrc2 < iFrameIndexDst1) /* No samples, return. */
		{
			if (iFrameIndexSrc1 != 0)
			{
				return YXC_ERC_SUCCESS;
			}
			iFrameIndexSrc2 += iFrameIndexDst1;
		}

		yuint32_t uNumSamplesCopy = iFrameIndexSrc2 - iFrameIndexDst1;
		YXC_Status rc = YXV_AFrameCopy(&this->_aSampleBuf, this->_aSampleBuf.uNumSamples, pAFrame, pAFrame->uNumSamples - uNumSamplesCopy,
			uNumSamplesCopy);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void _AVBuilderAVFile::_ClearBuffers()
	{
		YXV_FrameUnref(&this->_vSampleCur);
		YXV_FrameUnref(&this->_vSampleNext);
		this->_iTotalSamples.SetToNull();
		this->_aSampleBuf.uNumSamples = 0;
		this->_bHasKey = FALSE;
		this->_bCorrectPos = FALSE;

		for (size_t i = 0; i < this->_aPacketQueue.size(); ++i)
		{
			YXV_PacketUnref(&this->_aPacketQueue[i]);
		}

		for (size_t i = 0; i < this->_vPacketQueue.size(); ++i)
		{
			YXV_PacketUnref(&this->_vPacketQueue[i]);
		}

		this->_aPacketQueue.clear();
		this->_vPacketQueue.clear();
	}

	YXC_Status _AVBuilderAVFile::SeekTo(yint64_t i64Time)
	{
		this->_ClearBuffers();

		_AVBuilderElementBase::SeekTo(i64Time);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilderAVFile::Skip(yint64_t i64Duration)
	{
		if (!this->_iTotalSamples.IsNull()) /* Has audio samples, start skip here. */
		{
			if (this->_pAFilter)
			{
				YXV_SampleDesc sampleDesc = this->_pAFilter->GetSampleDesc();
				yint64_t uSamples = (this->_i64CurrentTime - this->_pSourceDesc->i64Start + this->_pSourceDesc->i64StartOff + i64Duration)
					/ 10000 * sampleDesc.uSampleRate / 1000; /* Need samples. */
				yint64_t uSamplesToSkip1 = uSamples - this->_iTotalSamples.Get();
				yint64_t uSamplesToSkip = YXCLib::TMin<yuint32_t>(uSamplesToSkip1, this->_aSampleBuf.uNumSamples);

				yuint32_t uSampleSize = _YXV_AVUtils::_GetSampleBits(sampleDesc.sampleFmt) / 8;
				if (!_YXV_AVUtils::_IsPlanarSampleFmt(sampleDesc.sampleFmt))
				{
					uSampleSize *= sampleDesc.uNumChannels;
				}

				for (yuint32_t i = 0; i < this->_aSampleBuf.uNumPlanars; ++i)
				{
					memmove(this->_aSampleBuf.pData[i], this->_aSampleBuf.pData[i] + uSampleSize * uSamplesToSkip,
						(this->_aSampleBuf.uNumSamples - uSamplesToSkip) * uSampleSize);
				}
				this->_aSampleBuf.uNumSamples -= uSamplesToSkip;
				this->_iTotalSamples.Set(this->_iTotalSamples.Get() + uSamplesToSkip1);
			}
		}

		YXC_Status rc = _AVBuilderElementBase::Skip(i64Duration);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}
}

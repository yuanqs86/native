#define __MODULE__ "EV.Utils.AVBuilderImpl"

#include <YXC_AVKernel/YXV_AVBuilder.h>
#include <YXC_AVKernel/Utils/_YXV_AVBuilder.hpp>
#include <YXC_AVKernel/Utils/_YXV_AVBuilderElementBase.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_MMInterface.h>

namespace _YXV_AVBuilder
{
	static void _CloseSourceList(YXV_AVSourceList* pSourceList)
	{
		for (yuint32_t i = 0; i < pSourceList->uNumSources; ++i)
		{
			_AVBuilderElementBase* pElement = _AVBPtr_E(pSourceList->pSources[i].element);
			if (pElement)
			{
				pElement->~_AVBuilderElementBase();
				free(pElement);
				pSourceList->pSources[i].element = NULL;
			}
		}
	}
}

namespace _YXV_AVBuilder
{
	_AVBuilder::_AVBuilder() : _i64CurrentTime(), _pSourceList(NULL), _aFilter(NULL),
		_vFilter(NULL)
	{
		memset(&this->_vSample, 0, sizeof(this->_vSample));
		memset(&this->_aSample, 0, sizeof(this->_aSample));
		memset(&this->_vParam, 0, sizeof(this->_vParam));
		memset(&this->_aParam, 0, sizeof(this->_aParam));
	}

	_AVBuilder::~_AVBuilder()
	{
		this->Destroy();
	}

	YXC_Status _AVBuilder::Create(YXV_VFilterType vfType, yuint32_t uWidth, yuint32_t uHeight, YXV_PixFmt pixFmt, yuint32_t uNumChannels,
		yuint32_t uFreq, YXV_SampleFmt sampleFmt)
	{
		this->_vParam.desc.pixFmt = pixFmt;
		this->_vParam.desc.w = uWidth;
		this->_vParam.desc.h = uHeight;

		this->_aParam.desc.sampleFmt = sampleFmt;
		this->_aParam.desc.uSampleRate = uFreq;
		this->_aParam.desc.uNumChannels = uNumChannels;

		YXC_Status rc = YXV_VFrameAlloc(&this->_vParam.desc, &this->_vSample);
		_YXC_CHECK_RC_RET(rc);

		rc = _YXV_AVUtils::_VFilter::CreateInstance(vfType, &this->_vFilter);
		_YXC_CHECK_RC_RET(rc);

		rc = this->_vFilter->Init(&this->_vSample);
		_YXC_CHECK_RC_RET(rc);

		if (this->_aParam.desc.uNumChannels != 0)
		{
			rc = YXV_AFrameAlloc(&this->_aParam.desc, 100000, &this->_aSample);
			_YXC_CHECK_RC_RET(rc);

			rc = _YXV_AVUtils::_AFilter::CreateInstance(&this->_aFilter);
			_YXC_CHECK_RC_RET(rc);

			rc = this->_aFilter->Init(&this->_aSample);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	void _AVBuilder::Destroy()
	{
		if (this->_pSourceList != NULL)
		{
			this->CloseSources(this->_pSourceList);
			this->_pSourceList = NULL;
		}

		YXV_FrameUnref(&this->_vSample);
		YXV_FrameUnref(&this->_aSample);

		if (this->_aFilter)
		{
			YXCLib::TDelete(this->_aFilter);
			this->_aFilter = NULL;
		}

		if (this->_vFilter)
		{
			YXCLib::TDelete(this->_vFilter);
			this->_vFilter = NULL;
		}
	}

	YXC_Status _AVBuilder::OpenSources(YXV_AVSourceList* pSourceList)
	{
		YXCLib::HandleRef<YXV_AVSourceList*> hRes(pSourceList, _CloseSourceList);
		for (yuint32_t i = 0; i < pSourceList->uNumSources; ++i)
		{
			_AVBuilderElementBase* pEle = NULL;
			YXC_Status rc = _AVBuilderElementBase::CreateElement(this->_vFilter, this->_aFilter, &pSourceList->pSources[i], &pEle);
			_YXC_CHECK_RC_RET(rc);

			pSourceList->pSources[i].element = _AVBHdl_E(pEle);
		}

		this->_pSourceList = pSourceList;
		hRes.Detach();
		return YXC_ERC_SUCCESS;
	}

	void _AVBuilder::CloseSources(YXV_AVSourceList* pSourceList)
	{
		_CloseSourceList(pSourceList);
	}

	void _AVBuilder::FillData(YXV_AVBuilder builder, YXV_Frame* pImgData, YXV_Frame* pAudioData)
	{
		*pImgData = this->_vSample;
		*pAudioData = this->_aSample;
	}

	YXC_Status _AVBuilder::NextFrame(yint64_t i64FrameDuration)
	{
		YXV_FrameZero(&this->_aSample);
		this->_aSample.uNumSamples = 0;

		YXV_FrameZero(&this->_vSample);
		for (yuint32_t i = 0; i < this->_pSourceList->uNumSources; ++i)
		{
			_AVBuilderElementBase* pEleBase = _AVBPtr_E(this->_pSourceList->pSources[i].element);

			YXC_Status rc = pEleBase->NextFrame(i64FrameDuration, &this->_aSample, &this->_vSample);
			_YXC_CHECK_RC_RET(rc);
		}

		if (this->_aFilter != NULL)
		{
			if (this->_aSample.uNumSamples == 0) /* No audio data, fill empty data. */
			{
				this->_aSample.uNumSamples = this->_aFilter->GetSampleDesc().uSampleRate * i64FrameDuration / YXV_REFTIME_PER_SEC;
			}
			this->_aSample.uRefTime = this->_i64CurrentTime;
		}

		this->_vSample.uRefTime = this->_i64CurrentTime;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilder::Seek(yint64_t i64Time)
	{
		for (yuint32_t i = 0; i < this->_pSourceList->uNumSources; ++i)
		{
			_AVBuilderElementBase* pEleBase = _AVBPtr_E(this->_pSourceList->pSources[i].element);

			YXC_Status rc = pEleBase->SeekTo(i64Time);
			_YXC_CHECK_RC_RET(rc);
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AVBuilder::Skip(yint64_t i64Time)
	{
		for (yuint32_t i = 0; i < this->_pSourceList->uNumSources; ++i)
		{
			_AVBuilderElementBase* pEleBase = _AVBPtr_E(this->_pSourceList->pSources[i].element);

			YXC_Status rc = pEleBase->Skip(i64Time);
			_YXC_CHECK_RC_RET(rc);
		}

		this->_i64CurrentTime += i64Time;
		return YXC_ERC_SUCCESS;
	}
}

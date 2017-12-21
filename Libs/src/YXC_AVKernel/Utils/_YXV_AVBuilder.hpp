#ifndef __INNER_INC_YXC_AV_AV_BUILDER_HPP__
#define __INNER_INC_YXC_AV_AV_BUILDER_HPP__

#include <YXC_AVKernel/YXV_AVBuilder.h>
#include <YXC_AVKernel/YXV_AVScaler.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_AVKernel/Utils/_YXV_AFilter.hpp>

namespace _YXV_AVBuilder
{
	class _AVBuilder
	{
	public:
		_AVBuilder();

		~_AVBuilder();

	public:
		YXC_Status Create(YXV_VFilterType vfType, yuint32_t uWidth, yuint32_t uHeight, YXV_PixFmt pixFmt, yuint32_t uNumChannels,
			yuint32_t uFreq, YXV_SampleFmt sampleFmt);

		void Destroy();

		YXC_Status OpenSources(YXV_AVSourceList* pSourceList);

		void CloseSources(YXV_AVSourceList* pSourceList);

		YXC_Status NextFrame(yint64_t i64FrameDuration);

		void FillData(YXV_AVBuilder builder, YXV_Frame* pImgData, YXV_Frame* pAudioData);

		YXC_Status Seek(yint64_t i64Time);

		YXC_Status Skip(yint64_t i64Time);

	protected:
		yint64_t _i64CurrentTime;

		YXV_AVSourceList* _pSourceList;

		YXV_Frame _vSample;
		YXV_Frame _aSample;

		YXV_VParam _vParam;
		YXV_AParam _aParam;

		_YXV_AVUtils::_VFilter* _vFilter;
		_YXV_AVUtils::_AFilter* _aFilter;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_AVBuilder, _AVBuilder, _AVBPtr_B, _AVBHdl_B);
}

#endif /* __INNER_INC_YXC_AV_AV_BUILDER_HPP__ */

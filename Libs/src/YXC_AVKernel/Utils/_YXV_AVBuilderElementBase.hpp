#ifndef __INNER_INC_YXC_AV_AV_BUILDER_ELEMENT_BASE_HPP__
#define __INNER_INC_YXC_AV_AV_BUILDER_ELEMENT_BASE_HPP__

#include <YXC_AVKernel/YXV_AVBuilder.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterBase.hpp>
#include <YXC_AVKernel/Utils/_YXV_AFilter.hpp>

namespace _YXV_AVBuilder
{
	class _AVBuilderElementBase
	{
	public:
		_AVBuilderElementBase();

		virtual ~_AVBuilderElementBase();

	public:
		static YXC_Status CreateElement(_YXV_AVUtils::_VFilter* filter, _YXV_AVUtils::_AFilter* aFilter, YXV_AVSource* pSource,
			_AVBuilderElementBase** ppOutElement);

	public:
		virtual YXC_Status Skip(yint64_t i64Duration);

		virtual YXC_Status SeekTo(yint64_t i64Time);

	public:
		virtual YXC_Status Init(YXV_AVSource* pSource) = 0;

		virtual YXC_Status NextFrame(yint64_t i64Duration, YXV_Frame* pSampleA, YXV_Frame* pSampleV) = 0;

	public:
		void _ConvertWH(double& x, double& y, double& w, double& h);

	protected:
		ybool_t _CanElementPlay();

	protected:
		yint64_t _i64CurrentTime;

		YXV_AVSource* _pSourceDesc;

		_YXV_AVUtils::_VFilter* _pFilter;
		_YXV_AVUtils::_AFilter* _pAFilter;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXV_SourceContent, _AVBuilderElementBase, _AVBPtr_E, _AVBHdl_E);
}

#endif /* __INNER_INC_YXC_AV_AV_ELEMENT_BASE_HPP__ */

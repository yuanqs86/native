#ifndef __INNER_INC_YXC_AV_AUDIO_FILTER_HPP__
#define __INNER_INC_YXC_AV_AUDIO_FILTER_HPP__

#include <YXC_AVKernel/YXV_AVUtils.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_Nullable.hpp>

namespace _YXV_AVUtils
{
	class _AFilter
	{
	public:
		static YXC_Status CreateInstance(_AFilter** ppFilter);

	public:
		YXC_Status Init(YXV_Frame* aFrame);

	public:
		inline const YXV_SampleDesc& GetSampleDesc() const { return this->_desc; }

	protected:
		YXV_SampleDesc _desc;
		YXV_Frame* _sample;
	};
}

#endif /* __INNER_INC_YXC_AV_AUDIO_FILTER_HPP__ */

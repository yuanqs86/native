#define __MODULE__ "EV.Utils.AFilterBase"

#include <YXC_AVKernel/Utils/_YXV_AFilter.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_AVKernel/Utils/_YXV_VFilterGdi.hpp>

namespace _YXV_AVUtils
{
	YXC_Status _AFilter::Init(YXV_Frame* aFrame)
	{
		this->_sample = aFrame;
		this->_desc = aFrame->u1.sampleDesc;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _AFilter::CreateInstance(_AFilter** ppFilter)
	{
		_YCHK_MAL_R1(pFilter, _AFilter);
		new (pFilter) _AFilter();

		*ppFilter = pFilter;
		return YXC_ERC_SUCCESS;
	}
}

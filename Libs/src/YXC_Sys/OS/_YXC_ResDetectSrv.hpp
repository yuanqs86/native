#ifndef __INNER_INC_YXC_SYS_BASE_RESOURCE_DETECT_SERVER_HPP
#define __INNER_INC_YXC_SYS_BASE_RESOURCE_DETECT_SERVER_HPP

#include <YXC_Sys/YXC_NamedPipeIpc.h>

namespace YXC_Inner
{
	struct _ResDetectSrvData
	{
		ybyte_t* pbyBuffer;

		ysize_t stBufferSize;

		ysize_t stRetSize;
	};

	class _ResDetectSrv
	{
	public:
		YXC_Status Create(ybool_t bInDllMain, yuint32_t uNumSrvs);

		void Close();

	private:
		yuint32_t _uNumSrvs;

		_ResDetectSrvData _arrSubSrvs[YXC_NAMED_PIPE_MAX_INSTANCES];

		YXC_NPipeSrv _srv;
	};

	extern _ResDetectSrv gs_hDetectSrv;

	YXC_Status _InitResDetectSrv(ybool_t bInDllMain);

	void _FiniResDetectSrv();
}

#endif /* __INNER_INC_YXC_SYS_BASE_RESOURCE_DETECT_SERVER_HPP */

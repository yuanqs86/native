#define __MODULE__ "EU.DB.Common"

#include <YXC_Sys/YXC_DBCmn.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>

namespace YXCLib
{
	YXC_Status _CreateDBResult(ysize_t stBlockBuf, _DBResult** ppResult)
	{
		_DBResult* pResult = (_DBResult*)::malloc(sizeof(_DBResult));
		_YXC_CHECK_REPORT_NEW_RET(pResult != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc for cc result failed"));

		pResult->SetBaseBuffer(stBlockBuf);

		*ppResult = pResult;
		return YXC_ERC_SUCCESS;
	}

	void _FreeDBResult(_DBResult* result)
	{
		result->Destroy();
		free(result);
	}
}

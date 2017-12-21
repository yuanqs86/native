#define __MODULE__ "EK.Ipc.NamedPipe"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_NamedPipeIpc.h>
#include <YXC_Sys/OS/_YXC_NamedPipe.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>

namespace
{
#if YXC_PLATFORM_WIN
	YXC_Status _FillPipeName(wchar_t* pszDst, yuint32_t uCchDst, const wchar_t* cpszPipeName)
	{
		wchar_t* psz = pszDst, *pszEnd = pszDst + uCchDst;

		YXCLib::_AppendStringChkW(psz, pszEnd, L"\\\\.\\pipe\\", YXC_STRING_ARR_LEN(L"\\\\.\\pipe\\"));

		ybool_t bFullNameGet = YXCLib::_AppendStringChkW(psz, pszEnd, cpszPipeName, YXC_STR_NTS);
		_YXC_CHECK_REPORT_NEW_RET(bFullNameGet, YXC_ERC_SUCCESS, L"Pipe name truncated, src = '%s'", cpszPipeName);

		return YXC_ERC_SUCCESS;
	}
#endif
}

using namespace YXC_Inner;
extern "C"
{
	YXC_Status YXC_NPipeServerCreate(const wchar_t* cpszPipeName, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, void* const* ppSrvDatas,
		YXC_NPipeSrvProc pServiceProc, YXC_NPipeSrv* pPipeSrv)
	{
		return YXC_NPipeServerCreateEx(cpszPipeName, NULL, uNumInstances, uCtrlTimeout, 8192, 8192,
			ppSrvDatas, pServiceProc, 0, pPipeSrv);
	}

	YXC_Status YXC_NPipeServerCreateEx(const wchar_t* cpszPipeName, YXC_KObjectAttr* pSecurity, yuint32_t uNumInstances, yuint32_t uCtrlTimeout, yuint32_t uCbBufferRead,
		yuint32_t uCbBufferWrite, void* const* ppSrvDatas, YXC_NPipeSrvProc pServiceProc, yuint32_t uPipeFlags, YXC_NPipeSrv* pPipeSrv)
	{
		wchar_t szPipeName[YXC_MAX_CCH_PATH];

		_YXC_CHECK_REPORT_NEW_RET(uNumInstances < YXC_NAMED_PIPE_MAX_INSTANCES, YXC_ERC_INVALID_PARAMETER,
			L"Exceed max count of instances, max = %u, require = %d", YXC_NAMED_PIPE_MAX_INSTANCES, uNumInstances);

		YXC_Status rc = _FillPipeName(szPipeName, YXC_MAX_CCH_PATH, cpszPipeName);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Fill pipe name failed");

		_NamedPipeSrv* pSrv = (_NamedPipeSrv*)::malloc(sizeof(_NamedPipeSrv));
		_YXC_CHECK_REPORT_NEW_RET(pSrv != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc for 'pSrv' failed");

		rc = pSrv->Create(szPipeName, pSecurity, uNumInstances, uCtrlTimeout, uCbBufferRead, uCbBufferWrite,
			ppSrvDatas, pServiceProc, uPipeFlags);
		if (rc != YXC_ERC_SUCCESS)
		{
			free(pSrv);
			_YXC_CHECK_REPORT_RET(FALSE, rc, L"Create service named pipe failed");
		}

		*pPipeSrv = (YXC_NPipeSrv)pSrv;
		return YXC_ERC_SUCCESS;
	}

	void YXC_NPipeServerClose(YXC_NPipeSrv pipeSrv)
	{
		_NamedPipeSrv* pSrv = (_NamedPipeSrv*)pipeSrv;

		pSrv->Close();
		free(pSrv);
	}

	YXC_Status YXC_NPipeClientConnect(const wchar_t* cpszPipeName, YXC_KObjectAttr* pSecurity, yuint32_t umsTimeout, YXC_NPipeCli* pClient)
	{
		wchar_t szPipeName[YXC_MAX_CCH_PATH];

		YXC_Status rc = _FillPipeName(szPipeName, YXC_MAX_CCH_PATH, cpszPipeName);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Fill pipe name failed");

		_NamedPipeCli* pCli = (_NamedPipeCli*)::malloc(sizeof(_NamedPipeCli));
		_YXC_CHECK_REPORT_NEW_RET(pCli != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc for 'pCli' failed");

		rc = pCli->Connect(szPipeName, pSecurity, umsTimeout);
		if (rc != YXC_ERC_SUCCESS)
		{
			free(pCli);
			_YXC_CHECK_REPORT_RET(FALSE, rc, L"Connect to service named pipe failed");
		}

		*pClient = (YXC_NPipeCli)pCli;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_NPipeClientCall(YXC_NPipeCli client, const void* pInBuffer, ysize_t stCbInBuffer, void** ppOutBuffer, ysize_t* pstCbOut)
	{
		_NamedPipeCli* pCli = (_NamedPipeCli*)client;

		return pCli->CallProxy(pInBuffer, stCbInBuffer, ppOutBuffer, pstCbOut);
	}

	void YXC_NPipeClientQueryHandle(YXC_NPipeCli client, ybool_t bDuplicate, YXC_KObject* pObject)
	{
		_NamedPipeCli* pCli = (_NamedPipeCli*)client;

		*pObject = pCli->QueryHandle();
	}

	void YXC_NPipeClientClose(YXC_NPipeCli client)
	{
		_NamedPipeCli* pCli = (_NamedPipeCli*)client;

		pCli->Close();
		free(pCli);
	}
};

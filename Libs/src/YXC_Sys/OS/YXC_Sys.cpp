#include <YXC_Sys/YXC_Sys.h>
#include <YXC_Sys/YXC_LocalMM.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <new>
#include <stdio.h>
#include <YXC_Sys/YXC_HandleRef.hpp>
#include <YXC_Sys/Utils/_YXC_TextConverter.hpp>
#include <YXC_Sys/YXC_FilePath.h>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/Utils/_YXC_LoggerBase.hpp>

#define TEST_IMPL(bClear, code, wszMsg, ...) YXC_ReportErrorFormat(bClear, code, __EMODULE__, __EFILE__, __LINE__, __EFUNCTION__, wszMsg, __VA_ARGS__)

#define __MODULE__ "EK.Sys"
#include <YXC_Sys/YXC_ErrMacros.hpp>

#define __SH_FILE_MAPPING_NAME "EK_Sys_gcx"

namespace
{
	typedef void (*_YXC_ErrorBufFreeFunc)(void* p);
	struct _YXC_Error_Buf
	{
		YXC_GuidStrA guidTLS;
		YXC_Error* pErrFirst;

		_YXC_ErrorBufFreeFunc freeFunc;
		YXCLib::EasyMemoryPool emp;
	};

	ychar* _Dup_Str(const ychar* cpszStr, YXCLib::EasyMemoryPool& emp)
	{
		if (cpszStr == NULL)
		{
			cpszStr = YC("");
		}

		size_t stCchStr = yh_strlen(cpszStr);
		ychar* retStr = (ychar*)emp.Alloc((stCchStr + 1) * sizeof(ychar));

		if (retStr == NULL) return NULL;

		memcpy(retStr, cpszStr, (stCchStr + 1) * sizeof(ychar));
		return retStr;
	}

	ychar* _Dup_Format_Str(const ychar* cpszFormat, va_list* args, YXCLib::EasyMemoryPool& emp)
	{
		if (args == NULL)
		{
			return _Dup_Str(cpszFormat, emp);
		}

		if (cpszFormat == NULL)
		{
			cpszFormat = YC("");
		}

		size_t stCchStr = yh_vscprintf(cpszFormat, *args);
		ychar* retStr = (ychar*)emp.Alloc((stCchStr + 1) * sizeof(ychar));

		if (retStr == NULL) return NULL;

		yh_vsprintf(retStr, cpszFormat, *args);
		return retStr;
	}

	YXC_Status _Create_ErrBuffer(_YXC_Error_Buf*& pErrorBuf)
	{
		pErrorBuf = (_YXC_Error_Buf*)malloc(sizeof(_YXC_Error_Buf));
		if (pErrorBuf == NULL) return YXC_ERC_OUT_OF_MEMORY;

		pErrorBuf->pErrFirst = NULL;
		pErrorBuf->emp.Create(FALSE, 1 << 15); // 32K
		pErrorBuf->freeFunc = free;

		return YXC_ERC_SUCCESS;
	}

	ybool_t _Destroy_ErrBuffer(_YXC_Error_Buf* pErrorBuf)
	{
		if (pErrorBuf->freeFunc == free)
		{
			pErrorBuf->emp.Destroy();
			free(pErrorBuf);
			return TRUE;
		}

		return FALSE;
	}

#if YXC_PLATFORM_WIN

	static yuint32_t s_uTlsIndex = 0;
	static YXCLib::OSSharedMemory* s_tls_shm = NULL;
	ybyte_t s_by_buffer_s_tls_shm[sizeof(*s_tls_shm)] = {0};

	YXC_Status _CreateOrGetTls(_YXC_Error_Buf*& pErrBuf)
	{
		pErrBuf = (_YXC_Error_Buf*)TlsGetValue(s_uTlsIndex);
		if (pErrBuf == NULL)
		{
			YXC_Status rc = _Create_ErrBuffer(pErrBuf);
			if (rc != YXC_ERC_SUCCESS) return rc;

			BOOL bRet = ::TlsSetValue(s_uTlsIndex, pErrBuf);
			if (!bRet)
			{
				_Destroy_ErrBuffer(pErrBuf);
				return YXC_ERC_OS;
			}
		}

		return YXC_ERC_SUCCESS;
	}

	_YXC_Error_Buf* _GetTls()
	{
		return (_YXC_Error_Buf*)TlsGetValue(s_uTlsIndex);
	}

#elif YXC_PLATFORM_UNIX

    pthread_key_t s_tls_key;
    typedef void (*pthread_cleanup_func)(void*);

    //__thread ychar s_func[YXC_MAX_FUNC_NAME + 1];
    int _init_tls_key()
    {
        return pthread_key_create(&s_tls_key, (pthread_cleanup_func)_Destroy_ErrBuffer);
    }

    void _free_tls_key(int reserved)
    {
        pthread_key_delete(s_tls_key);
    }

    YXCLib::HandleRef<int> gs_tls_handler(_init_tls_key(), _free_tls_key);

    YXC_Status _CreateOrGetTls(_YXC_Error_Buf*& pErrBuf)
    {
        pErrBuf = (_YXC_Error_Buf*)pthread_getspecific(s_tls_key);
        if (pErrBuf == NULL)
        {
            YXC_Status rc = _Create_ErrBuffer(pErrBuf);
            if (rc != YXC_ERC_SUCCESS)
            {
                return rc;
            }

            int ret = pthread_setspecific(s_tls_key, pErrBuf);
            if (ret != 0)
            {
                _Destroy_ErrBuffer(pErrBuf);
                return YXC_ERC_OS;
            }
        }

        return YXC_ERC_SUCCESS;
    }

    _YXC_Error_Buf* _GetTls()
    {
        return (_YXC_Error_Buf*)pthread_getspecific(s_tls_key);
    }

#endif /* YXC_PLATFORM_WIN */
}

extern "C"
{
	static YXC_Status _YXC_ReportErrorFormat(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsgFormat,
		va_list* vaArgs
	)
	{
		_YXC_Error_Buf* pErrorBuf = NULL;
		YXC_Status rc = _CreateOrGetTls(pErrorBuf);

		if (rc != YXC_ERC_SUCCESS) return rc;

		if (bClearPrevErrors)
		{
			pErrorBuf->pErrFirst = NULL;
			pErrorBuf->emp.Clear();
		}

		ysize_t stOffset = pErrorBuf->emp.GetOffset();
		YXC_Error* pError = (YXC_Error*)pErrorBuf->emp.Alloc(sizeof(YXC_Error));
		if (pError == NULL)
		{
			return YXC_ERC_OUT_OF_MEMORY;
		}

		pError->errSrc = errSrc;
		pError->uCategory = uCategory;
		pError->llErrCode = llErrCode;
		pError->pvAdditional = NULL;
		pError->stAdditionalBufSize = 0;
		pError->iLine = iLine;
		pError->wszModule = _Dup_Str(cpszModule, pErrorBuf->emp);
		if (pError->wszModule != NULL)
		{
			pError->wszMsg = _Dup_Format_Str(cpszMsgFormat, vaArgs, pErrorBuf->emp);
			if (pError->wszMsg != NULL)
			{
				pError->wszFile = _Dup_Str(cpszFile, pErrorBuf->emp);
				if (pError->wszFile != NULL)
				{
					pError->wszFunction = _Dup_Str(cpszFunc, pErrorBuf->emp);
					if (pError->wszFunction != NULL)
					{
						ybyte_t* pAdd = (ybyte_t*)pErrorBuf->emp.Alloc(stAdditionalBufSize);
						if (pAdd != NULL)
						{
							memcpy(pAdd, pvAdditional, stAdditionalBufSize);
							pError->pInnerError = pErrorBuf->pErrFirst;
							pErrorBuf->pErrFirst = pError;

							return YXC_ERC_SUCCESS;
						}
					}
				}
			}
		}

		pErrorBuf->emp.SetOffset(stOffset);
		return YXC_ERC_OUT_OF_MEMORY;
	}

	YXC_Status YXC_ReportErrorFormat(
		ybool_t bClearPrevErrors,
		YXC_Status esError,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		const ychar* cpszMsgFormat,
		...
	)
	{
		va_list vaArgs;
		va_start(vaArgs, cpszMsgFormat);
		YXC_Status rc = _YXC_ReportErrorFormat(bClearPrevErrors, YXC_ERROR_SRC_YXC, 0, esError, cpszModule, cpszFile,
			iLine, cpszFunc, 0, NULL, cpszMsgFormat, &vaArgs);
		va_end(vaArgs);

		return rc;
	}

	YXC_Status YXC_ReportError(
		ybool_t bClearPrevErrors,
		YXC_Status esError,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		const ychar* cpszMsg
	)
	{
		YXC_Status rc = _YXC_ReportErrorFormat(bClearPrevErrors, YXC_ERROR_SRC_YXC, 0, esError, cpszModule, cpszFile,
			iLine, cpszFunc, 0, NULL, cpszMsg, NULL);

		return rc;
	}

	YXC_Status YXC_ReportErrorExFormat(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsgFormat,
		...
	)
	{
		va_list vaArgs;
		va_start(vaArgs, cpszMsgFormat);
		YXC_Status rc = _YXC_ReportErrorFormat(bClearPrevErrors, errSrc, uCategory, llErrCode, cpszModule, cpszFile,
			iLine, cpszFunc, stAdditionalBufSize, pvAdditional, cpszMsgFormat, &vaArgs);
		va_end(vaArgs);

		return rc;
	}

	YXC_Status YXC_ReportErrorExFormat_V(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsgFormat,
		va_list vaList
	)
	{
		va_list va2;
		va_copy(va2, vaList);
		YXC_Status rc = _YXC_ReportErrorFormat(bClearPrevErrors, errSrc, uCategory, llErrCode, cpszModule, cpszFile,
			iLine, cpszFunc, stAdditionalBufSize, pvAdditional, cpszMsgFormat, &va2);

		return rc;
	}

	YXC_Status YXC_ReportErrorEx(
		ybool_t bClearPrevErrors,
		YXC_ErrorSrc errSrc,
		yuint32_t uCategory,
		long long llErrCode,
		const ychar* cpszModule,
		const ychar* cpszFile,
		int iLine,
		const ychar* cpszFunc,
		ysize_t stAdditionalBufSize,
		void* pvAdditional,
		const ychar* cpszMsg
	)
	{
		YXC_Status rc = _YXC_ReportErrorFormat(bClearPrevErrors, errSrc, uCategory, llErrCode, cpszModule, cpszFile,
			iLine, cpszFunc, stAdditionalBufSize, pvAdditional, cpszMsg, NULL);
		return rc;
	}

	YXC_Status YXC_ClearErrors()
	{
		_YXC_Error_Buf* pErrorBuf = NULL;
		YXC_Status rc = _CreateOrGetTls(pErrorBuf);

		if (rc != YXC_ERC_SUCCESS) return rc;

		pErrorBuf->pErrFirst = NULL;
		pErrorBuf->emp.Clear();

		return YXC_ERC_SUCCESS;
	}

	const YXC_Error* YXC_GetLastError()
	{
		_YXC_Error_Buf* pErrorBuf = (_YXC_Error_Buf*)_GetTls();
		if (pErrorBuf == NULL) return NULL;
		return pErrorBuf->pErrFirst;
	}

	const YXC_Error* YXC_GetInnermostError()
	{
		_YXC_Error_Buf* pErrorBuf = (_YXC_Error_Buf*)_GetTls();
		if (pErrorBuf == NULL) return NULL;

		const YXC_Error* pError = pErrorBuf->pErrFirst;
		if (pError == NULL) return NULL;
		while (pError->pInnerError != NULL)
		{
			pError = pError->pInnerError;
		}
		return pError;
	}

	const YXC_Error* YXC_CopyLastError(ysize_t stBuffer, ybyte_t* pBuffer, ysize_t* pstNeeded)
	{
		_YXC_Error_Buf* pErrorBuf = (_YXC_Error_Buf*)_GetTls();
		if (pErrorBuf == NULL)
		{
			if (pstNeeded) *pstNeeded = 0;
			return NULL;
		}

		ysize_t stSize = pErrorBuf->emp.GetOffset();
		if (pstNeeded) *pstNeeded = stSize;
		if (stBuffer >= stSize)
		{
			ybyte_t* pBaseBuffer = pErrorBuf->emp.GetBuffer();
			memcpy(pBuffer, pBaseBuffer, stSize);
			yssize_t ptrDiff = (ybyte_t*)pErrorBuf->pErrFirst - pBaseBuffer;
			yssize_t ptrRel = pBuffer - pBaseBuffer;
			YXC_Error* pError = (YXC_Error*)(pBuffer + ptrDiff);

			while (pError != NULL)
			{
				if (pError->wszFile != NULL) pError->wszFile = (ychar*)((ybyte_t*)pError->wszFile + ptrRel);
				if (pError->wszFunction != NULL) pError->wszFunction = (ychar*)((ybyte_t*)pError->wszFunction + ptrRel);
				if (pError->wszModule != NULL) pError->wszModule = (ychar*)((ybyte_t*)pError->wszModule + ptrRel);
				if (pError->wszMsg != NULL) pError->wszMsg = (ychar*)((ybyte_t*)pError->wszMsg + ptrRel);
				if (pError->pvAdditional != NULL) pError->pvAdditional = (ybyte_t*)pError->pvAdditional + ptrRel;
				if (pError->pInnerError != NULL) pError->pInnerError = (const YXC_Error*)((ybyte_t*)pError->pInnerError + ptrRel);

				pError = (YXC_Error*)pError->pInnerError;
			}
			return (const YXC_Error*)(pBuffer + ptrDiff);
		}

		return NULL;
	}

	const ychar* YXC_GetLastErrorMessage()
	{
		_YXC_Error_Buf* pErrorBuf = (_YXC_Error_Buf*)_GetTls();
		if (pErrorBuf == NULL || pErrorBuf->pErrFirst == NULL) return YC("No Details");
		return pErrorBuf->pErrFirst->wszMsg;
	}

	yint64_t YXC_GetLastOSError()
	{
		const YXC_Error* pError = YXC_GetInnermostError();
		if (pError == NULL || pError->errSrc != YXC_ERROR_SRC_OS)
		{
			return YXCLib::OSGetLastError();
		}
		else
		{
			return pError->llErrCode;
		}
	}

	static void _PromptFatalError(const ychar* pszAlert, const ychar* szExec)
	{
#if YXC_PLATFORM_WIN
		::MessageBoxW(NULL, pszAlert, szExec, MB_OK | MB_ICONERROR);
		if (::IsDebuggerPresent())
		{
			::DebugBreak();
		}
		else
		{
			ExitProcess(0);
		}
#else
#endif /* YXC_PLATFORM_WIN */
	}

	void YXC_FatalAssert(const ychar* szMessage, ...)
	{
		va_list ls;
		va_start(ls, szMessage);

		YXC_FatalAssert_V(szMessage, ls);

		va_end(ls);
	}

	void YXC_FatalAssertSys(const ychar* szMessage, ...)
	{
		va_list ls;
		va_start(ls, szMessage);

		YXC_FatalAssertSys_V(szMessage, ls);

		va_end(ls);
	}

	ysize_t YXC_FormatError(const YXC_Error* errorInfo, ychar* buffer, ysize_t ccBuffer)
	{
		if (errorInfo == NULL)
		{
			buffer[0] = 0;
			return 0;
		}
		ychar* pszEnd = buffer + ccBuffer, *pszMsg = buffer;

		YXCLib::_AppendStringChk(pszMsg, pszEnd, YC("Sys error Information : "), YXC_STR_NTS);

		const YXC_Error* pErr = YXC_GetLastError();
		while (pErr != NULL)
		{
			ychar szSubMsg[YXC_BASE_ERROR_BUFFER];
			yssize_t i = yh_snprintf(szSubMsg, YXC_BASE_ERROR_BUFFER - 1, YC("Src = %d-%u, Code = %lld, Module = %s, Message = %s, ")
				YC("File = %s, Line = %d, Function = %s\n"),
				pErr->errSrc, pErr->uCategory, pErr->llErrCode, pErr->wszModule, pErr->wszMsg,
				pErr->wszFile, pErr->iLine, pErr->wszFunction);
			YXCLib::_AppendStringChk(pszMsg, pszEnd, szSubMsg, i);
			pErr = pErr->pInnerError;
		}

		return pszMsg - buffer;
	}

	void YXC_FatalAssertSys_V(const ychar* szMessage, va_list vaList)
	{
		va_list list2;
		va_copy(list2, vaList);
		int cc = yh_vscprintf(szMessage, list2);

		const ychar head[] = YC("YXCLib kernel assertion: ");
		ychar* szAlert = (ychar*)malloc((cc + YXC_STRING_ARR_LEN(head) + 1 + YXC_BASE_ERROR_BUFFER) * sizeof(ychar));
		if (szAlert == NULL) return;
		YXCLib::HandleRef<void*> szAlert_res(szAlert, free);

		yh_strcpy(szAlert, head);
		yh_vsprintf(szAlert + YXC_STRING_ARR_LEN(head), szMessage, list2);

		ychar* pszMsg = szAlert + YXC_STRING_ARR_LEN(head) + cc, *pszEnd = pszMsg + YXC_BASE_ERROR_BUFFER;
		YXCLib::_AppendStringChk(pszMsg, pszEnd, YC("\n"), YXC_STR_NTS);

		const YXC_Error* pErr = YXC_GetLastError();
		YXC_FormatError(pErr, pszMsg, YXC_BASE_ERROR_BUFFER);

		YXC_FPath szApp = {0};
		YXC_FPathFindModulePath(szApp, NULL);
		const ychar* executable = YXC_FPathFindFileSpec(szApp);

		_PromptFatalError(szAlert, executable);
	}

	void YXC_FatalAssert_V(const ychar* szMessage, va_list vaList)
	{
		va_list list2;
		va_copy(list2, vaList);

		int cc = yh_vscprintf(szMessage, list2);

		YXC_FPath szApp;
		YXC_FPathFindModulePath(szApp, NULL);
		const ychar* executable = YXC_FPathFindFileSpec(szApp);

		const ychar head[] = YC("YXCLib kernel assertion: ");
		ychar* szAlert = (ychar*)malloc((cc + YXC_STRING_ARR_LEN(head) + 1) * sizeof(ychar));
		if (szAlert == NULL) return;

		YXCLib::HandleRef<void*> szAlert_res(szAlert, free);

		yh_strcpy(szAlert, head);
		yh_vsprintf(szAlert + YXC_STRING_ARR_LEN(head), szMessage, list2);

		_PromptFatalError(szAlert, executable);
	}

#if YXC_PLATFORM_WIN
	int __cdecl YXC_NewHandler(size_t stAllocSize)
	{
		ULONG_PTR argument = stAllocSize;
		::RaiseException(YXC_BAD_ALLOC_CODE, 0, 1, &argument);

		return 0;
	}
#endif /* YXC_PLATFORM_WIN */

	void YXC_FastCopy(void* pDst, const void* pSrc, ysize_t stSize)
	{
		yuintptr_t upDst = (yuintptr_t)pDst;
		yuintptr_t upSrc = (yuintptr_t)pSrc;

		ysize_t stRx = stSize / sizeof(ysize_t);
		ysize_t stRemain = stSize % sizeof(ysize_t);

		ysize_t* piDst = (ysize_t*)upDst;
		ysize_t* piSrc = (ysize_t*)upSrc;
		for (ysize_t i = 0; i < stRx; ++i)
		{
			piDst[i] = piSrc[i];
		}

		ybyte_t* pbDst = (ybyte_t*)(upDst + stRx * sizeof(ysize_t));
		ybyte_t* pbSrc = (ybyte_t*)(upSrc + stRx * sizeof(ysize_t));
		for (ysize_t i = 0; i < stRemain; ++i)
		{
			pbDst[i] = pbSrc[i];
		}
	}

	void YXC_GuidToGuidStrA(const YXC_Guid* guid, YXC_GuidStrA guidStr)
	{
		sprintf(guidStr, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid->Data1, guid->Data2, guid->Data3,
			guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
			guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]
		);
	}

	void YXC_GuidStrToGuidA(const YXC_GuidStrA guidStr, YXC_Guid* guid)
	{
		yuint32_t u1, u2, u3, u4;
		yuint64_t lu;
		sscanf(guidStr, "%x-%x-%x-%x-%llx", &u1, &u2, &u3, &u4, &lu);

		guid->Data1 = u1;
		guid->Data2 = u2;
		guid->Data3 = u3;
		guid->Data4[0] = (ybyte_t)(u4 >> 8);
		guid->Data4[1] = (ybyte_t)(u4 & 0xff);
		guid->Data4[2] = (ybyte_t)(lu >> 40);
		guid->Data4[3] = (ybyte_t)((lu >> 32) & 0xff);
		guid->Data4[4] = (ybyte_t)((lu >> 24) & 0xff);
		guid->Data4[5] = (ybyte_t)((lu >> 16) & 0xff);
		guid->Data4[6] = (ybyte_t)((lu >> 8) & 0xff);
		guid->Data4[7] = lu & 0xff;
	}

	void YXC_GuidToGuidStrW(const YXC_Guid* guid, YXC_GuidStrW guidStr)
	{
		yxcwrap_swprintf(guidStr, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", guid->Data1, guid->Data2, guid->Data3,
			guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
			guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]
		);
	}

	void YXC_GuidStrToGuidW(const YXC_GuidStrW guidStr, YXC_Guid* guid)
	{
		yuint32_t u1, u2, u3, u4;
		yuint64_t lu;
		swscanf(guidStr, L"%x-%x-%x-%x-%llx", &u1, &u2, &u3, &u4, &lu);

		guid->Data1 = u1;
		guid->Data2 = u2;
		guid->Data3 = u3;
		guid->Data4[0] = (ybyte_t)(u4 >> 8);
		guid->Data4[1] = (ybyte_t)(u4 & 0xff);
		guid->Data4[2] = (ybyte_t)(lu >> 40);
		guid->Data4[3] = (ybyte_t)((lu >> 32) & 0xff);
		guid->Data4[4] = (ybyte_t)((lu >> 24) & 0xff);
		guid->Data4[5] = (ybyte_t)((lu >> 16) & 0xff);
		guid->Data4[6] = (ybyte_t)((lu >> 8) & 0xff);
		guid->Data4[7] = lu & 0xff;
	}

	const char* YXC_GuidStrTLS(const YXC_Guid* guid)
	{
		_YXC_Error_Buf* pErrorBuf = NULL;
		YXC_Status rc = _CreateOrGetTls(pErrorBuf);
		if (rc != YXC_ERC_SUCCESS) return "NULL";

		YXC_GuidToGuidStrA(guid, pErrorBuf->guidTLS);
		return pErrorBuf->guidTLS;
	}
};

#if YXC_PLATFORM_WIN

namespace YXC_Inner {
	ybool_t gs_bIsLittleEndianCPU;
}

namespace YXC_Inner
{
	void _EnableCrashReport();
	void _DisableCrashReport();
	//void _InitHeapSection();
	//void _DeleteHeapSection();
	//YXC_Status _InitHeapDetect(ybool_t bInDllMain);
	//YXC_Status _InitGdiDetect(ybool_t bInDllMain);
	//void _FiniHeapDetect();
	//void _FiniGdiDetect();

	BOOL _PNCMPDllInitHandler(unsigned int reason);
	BOOL _NetDllInitHandler(unsigned int reason);
	BOOL _LogDllInitHandler(unsigned int reason);
	BOOL _HeapDllInitHandler(unsigned int reason);
	BOOL _CoreDllInitHandler(unsigned int reason)
	{
		switch (reason)
		{
		case DLL_PROCESS_ATTACH:
			gs_bIsLittleEndianCPU = YXCLib::_IsLittleEndianCPU();
			_TextConverter::InitConverter();
			break;
		case DLL_PROCESS_DETACH:
			YXC_Inner::_DisableCrashReport();
			//YXC_Inner::_DeleteHeapSection();
			//YXC_Inner::_FiniHeapDetect();
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		}

		return TRUE;
	}

	BOOL _MainDllInitHandler(unsigned int reason)
	{
		BOOL bRet = FALSE;
		// Specify the dll main call sequency.
		if (reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH)
		{
			BOOL bRetCore = YXC_Inner::_CoreDllInitHandler(reason);
			BOOL bRetLog = YXC_Inner::_LogDllInitHandler(reason);
			BOOL bRetHeap = YXC_Inner::_HeapDllInitHandler(reason);
			BOOL bRetNet = YXC_Inner::_NetDllInitHandler(reason);
			BOOL bRetPNCMP = YXC_Inner::_PNCMPDllInitHandler(reason);
			bRet = bRetCore && bRetNet && bRetPNCMP && bRetLog;
		}
		else
		{
			BOOL bRetPNCMP = YXC_Inner::_PNCMPDllInitHandler(reason);
			BOOL bRetNet = YXC_Inner::_NetDllInitHandler(reason);
			BOOL bRetHeap = YXC_Inner::_HeapDllInitHandler(reason);
			BOOL bRetLog = YXC_Inner::_LogDllInitHandler(reason);
			BOOL bRetCore = YXC_Inner::_CoreDllInitHandler(reason);
			bRet = bRetCore && bRetNet && bRetPNCMP && bRetLog;
		}

		if (!bRet)
		{
			MessageBoxW(NULL, ::YXC_GetLastErrorMessage(), L"YXC_Sys_Loading", MB_OK | MB_ICONERROR);
		}

		return bRet;
	}
}

#if 0
BOOL WINAPI DllMain(HANDLE dllHandle, unsigned int reason, void* lpReserved)
{
	return YXC_Inner::_MainDllInitHandler(reason);
}
#elif YXC_EXPORTS_FLAG == YXC_EXPORTS_DISPATCH_DLL
BOOL WINAPI DllMain(HANDLE dllHandle, unsigned int reason, void* lpReserved)
{
	return YXC_Inner::_CoreDllInitHandler(reason);
}
#else
namespace YXC_Inner
{
	static BOOL gs_bInit =  YXC_Inner::_MainDllInitHandler(DLL_PROCESS_ATTACH);
	static YXCLib::HandleRef<yuint32_t, FALSE> gs_dllFinalizer(DLL_PROCESS_DETACH, (YXCLib::HandleRef<yuint32_t, FALSE>::DestroyFunc)YXC_Inner::_MainDllInitHandler);
}
#endif /* YXC_EXPORTS_LEVEL */

static DWORD _GetTlsIndex()
{
	static DWORD dwRet = 0;
	if (s_tls_shm == NULL)
	{
		s_tls_shm = (YXCLib::OSSharedMemory*)s_by_buffer_s_tls_shm;
		new (s_tls_shm) YXCLib::OSSharedMemory();

		char szShmName[256];
		//lstrcpyA(szShmName, __SH_FILE_MAPPING_NAME);
		//lstrcatA(szShmName, "-");
		wsprintfA(szShmName, "%s-%d", __SH_FILE_MAPPING_NAME, GetCurrentProcessId());

		ybool_t bCreatedNew;
		YXC_Status rc = s_tls_shm->CreateA(szShmName, sizeof(DWORD), &bCreatedNew);
		if (rc != YXC_ERC_SUCCESS)
		{
			//_YXC_KREPORT_ERR(YC("Failed to create shared memory"));
			return -1;
		}

		DWORD* pSharedTls = NULL;
		rc = s_tls_shm->Map(0, sizeof(DWORD), (void**)&pSharedTls);
		if (rc != YXC_ERC_SUCCESS)
		{
			//_YXC_KREPORT_ERR(YC("Failed to map shared memory pointer"));
			return -1;
		}

		if (bCreatedNew)
		{
			dwRet = TlsAlloc();
			if (dwRet == 0) dwRet = -1;
			*pSharedTls = dwRet;
		}
		else
		{
			dwRet = *pSharedTls;
		}
		s_tls_shm->Unmap(pSharedTls);
		return dwRet;
	}

	return dwRet;
}

static void WINAPI TlsMain(HANDLE dllHandle, DWORD reason, void* lpReserved)
{
	_YXC_Error_Buf* pErrorBuf = NULL;

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		s_uTlsIndex = _GetTlsIndex();
		break;
	case DLL_PROCESS_DETACH:
		pErrorBuf = (_YXC_Error_Buf*)TlsGetValue(s_uTlsIndex);
		if (pErrorBuf != NULL)
		{
			ybool_t bDestroy = _Destroy_ErrBuffer(pErrorBuf);
			if (bDestroy)
			{
				TlsSetValue(s_uTlsIndex, NULL);
			}
		}

		if (s_tls_shm != NULL)
		{
			s_tls_shm->~OSSharedMemory();
		}
		//::TlsFree(s_uTlsIndex);

		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		pErrorBuf = (_YXC_Error_Buf*)TlsGetValue(s_uTlsIndex);
		if (pErrorBuf != NULL)
		{
			ybool_t bDestroy = _Destroy_ErrBuffer(pErrorBuf);
			if (bDestroy)
			{
				TlsSetValue(s_uTlsIndex, NULL);
			}
		}
		break;
	}

	return;
}

extern "C"
{
	PIMAGE_TLS_CALLBACK my_tls_callbacktbl[2] = { TlsMain, 0 };
}

#ifdef _M_IX86
#pragma comment(linker, "/INCLUDE:__tls_used")
#else
#pragma comment(linker, "/INCLUDE:_tls_used")
#endif /* _M_IX86 */

#else

namespace YXC_Inner {
	ybool_t gs_bIsLittleEndianCPU = YXCLib::_IsLittleEndianCPU();
}

#endif /* YXC_PLATFORM_WIN */

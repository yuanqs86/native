#define __MODULE__ "EK.ShareMemory.Passway"

#include <YXC_Sys/YXC_ShmPassway.h>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_Locker.hpp>

#define _YXC_PASSWAY_SHM_DESC L"__yxc_pw_shm_desc_x3__"
#define _YXC_PASSWAY_MUTEX_DESC L"__yxc_pw_mutex_desc_x3__"
#define _YXC_PASSWAY_SEM_DESC L"__yxc_pw_sem_desc_x3__"

namespace
{
	struct _ShmHead
	{
		volatile yuint32_t uStarted;
		volatile yuint32_t u32CbCurrent;
		volatile yuint32_t u32ShmMax;
	};

	struct _ShmPassway
	{
		YXX_Mutex pwMutex;
		YXCLib::OSSharedMemory sharedMem;
		_ShmHead* pMappedHead;

		void _FillNames(wchar_t* pszShmPW, wchar_t* pszMutex, yuint32_t uCchPath, const wchar_t* pszShmName)
		{
			wchar_t* pszWrapEnd = pszShmPW + uCchPath;
			wchar_t* pszMutexEnd = pszMutex + uCchPath;
			YXCLib::_AppendStringChkW(pszShmPW, pszWrapEnd, _YXC_PASSWAY_SHM_DESC, YXC_STR_NTS);
			YXCLib::_AppendStringChkW(pszShmPW, pszWrapEnd, pszShmName, YXC_STR_NTS);

			YXCLib::_AppendStringChkW(pszMutex, pszMutexEnd, _YXC_PASSWAY_MUTEX_DESC, YXC_STR_NTS);
			YXCLib::_AppendStringChkW(pszMutex, pszMutexEnd, pszShmName, YXC_STR_NTS);
		}

		ybool_t _ShouldRemap(yuint32_t uCbNeed, yuint32_t uCbMapped)
		{
			return uCbNeed > uCbMapped || uCbNeed < uCbMapped * 2;
		}

		YXC_Status CreateW(const wchar_t* pszShmName, yuint32_t uInitSize, yuint32_t stPWMaxSize)
		{
			wchar_t szShmPW[YXC_MAX_CCH_PATH], szMutexName[YXC_MAX_CCH_PATH];
			_FillNames(szShmPW, szMutexName, YXC_MAX_CCH_PATH, pszShmName);

			_YXC_CHECK_REPORT_NEW_RET(uInitSize < stPWMaxSize, YXC_ERC_INVALID_PARAMETER, L"Initialize size must be less tha max size");

			YXC_Status rcRet = YXC_ERC_UNKNOWN;

			ybool_t bCreatedNewMutex = FALSE, bShmCreated = FALSE;
			YXC_Status rc = this->pwMutex.NamedCreateW(NULL, szMutexName, 0, &bCreatedNewMutex);
			_YXC_CHECK_OS_RET(rc == YXC_ERC_SUCCESS, L"Create mutex '%s' failed", szMutexName);
			_YXC_CHECK_OS_GOTO(bCreatedNewMutex, L"Can't create mutex '%s', opened by another process", szMutexName);

			ybool_t bCreatedNewShm = FALSE;
			rc = this->sharedMem.CreateW(szShmPW, stPWMaxSize + sizeof(_ShmHead), &bCreatedNewShm);
			_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

			bShmCreated = TRUE;
			_YXC_CHECK_OS_GOTO(bCreatedNewShm, L"Can't create shared memory '%s', opened by another process", szShmPW);

			rc = sharedMem.Map(0, sizeof(_ShmHead), (void**)&this->pMappedHead);
			_YXC_CHECK_OS_GOTO(rc == YXC_ERC_SUCCESS, L"Map header part of shared memory failed");

			this->pMappedHead->u32CbCurrent = uInitSize;
			this->pMappedHead->u32ShmMax = stPWMaxSize;
			YXCLib::Interlocked::Exchange(&this->pMappedHead->uStarted, 1);

			return YXC_ERC_SUCCESS;
err_ret:
			pwMutex.Destroy();
			if (bShmCreated) sharedMem.Close();
			return rcRet;
		}

		YXC_Status OpenW(const wchar_t* pszShmName, yuint32_t* pShmMax)
		{
			wchar_t szShmPW[YXC_MAX_CCH_PATH], szMutexName[YXC_MAX_CCH_PATH];
			_FillNames(szShmPW, szMutexName, YXC_MAX_CCH_PATH, pszShmName);

			YXC_Status rc = pwMutex.OpenW(0, szMutexName);
			_YXC_CHECK_OS_RET(rc == YXC_ERC_SUCCESS, L"Open mutex '%s' failed", szMutexName);

			ybool_t bShmCreated = FALSE;
			this->pMappedHead = NULL;

			YXC_Status rcRet = YXC_ERC_UNKNOWN;
			rc = this->sharedMem.OpenW(szShmPW);
			_YXC_CHECK_OS_GOTO(rc == YXC_ERC_SUCCESS, L"Can't open shared memory '%s'", szShmPW);

			bShmCreated = TRUE;
			rc = sharedMem.Map(0, sizeof(_ShmHead), (void**)&pMappedHead);
			_YXC_CHECK_OS_GOTO(rc == YXC_ERC_SUCCESS, L"Map header part of shared memory failed");

			yuint32_t uMaxSpinCount = 100000, uSpinCount = 0;;
			yuint32_t uInited = 0;
			while (uSpinCount++ < uMaxSpinCount)
			{
				uInited = YXCLib::Interlocked::ExchangeAdd(&this->pMappedHead->uStarted, 0);
				if (uInited == 1) break;
			}
			_YXC_CHECK_OS_GOTO(uInited == 1, L"Can't wait for initialized after %d spin times", uSpinCount);

			*pShmMax = pMappedHead->u32ShmMax;
			return YXC_ERC_SUCCESS;
err_ret:
			if (this->pMappedHead != NULL) sharedMem.Unmap(this->pMappedHead);
			if (bShmCreated) this->sharedMem.Close();
			this->pwMutex.Destroy();
			return rcRet;
		}

		YXC_Status Map(yuint32_t uNumBytesToMap, void** ppMappedPtr)
		{
			_YXC_CHECK_REPORT_NEW_RET(uNumBytesToMap < this->pMappedHead->u32ShmMax, YXC_ERC_BUFFER_NOT_ENOUGH, L"Can't map(%d bytes), max size(%d bytes)",
				uNumBytesToMap, this->pMappedHead->u32ShmMax);

			void* ptr = NULL;
			YXC_Status rc = this->sharedMem.Map(0, uNumBytesToMap + sizeof(_ShmHead), &ptr);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Map for %d bytes failed", uNumBytesToMap + sizeof(_ShmHead));

			*ppMappedPtr = (ybyte_t*)ptr + sizeof(_ShmHead);
			return YXC_ERC_SUCCESS;
		}

		YXC_Status SetSize(yuint32_t uSize)
		{
			_YXC_CHECK_REPORT_NEW_RET(uSize < this->pMappedHead->u32ShmMax, YXC_ERC_BUFFER_NOT_ENOUGH, L"Can't inflate to size(%d bytes), max size(%d bytes)",
				uSize, this->pMappedHead->u32ShmMax);

			this->pMappedHead->u32CbCurrent = uSize;
			return YXC_ERC_SUCCESS;
		}

		yuint32_t GetSize()
		{
			return this->pMappedHead->u32CbCurrent;
		}

		void Unmap(void* pPtr)
		{
			ybyte_t* pRawPtr = (ybyte_t*)pPtr - sizeof(_ShmHead);
			this->sharedMem.Unmap(pRawPtr);
		}

		void Close()
		{
			this->sharedMem.Unmap(this->pMappedHead);
			this->sharedMem.Close();
			this->pwMutex.Destroy();
		}
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_ShmPassway, _ShmPassway, _ShmPtr, _ShmHdl)
}

extern "C"
{
	YXC_Status YXC_ShmPWCreate(const wchar_t* pszShmName, yuint32_t uInitSize, yuint32_t uPWMaxSize, YXC_ShmPassway* pPassway)
	{
		_ShmPassway* pPassway2 = (_ShmPassway*)::malloc(sizeof(_ShmPassway));
		_YXC_CHECK_REPORT_NEW_RET(pPassway != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc for shared memory passway failed");

		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		YXC_Status rc = pPassway2->CreateW(pszShmName, uInitSize, uPWMaxSize);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		*pPassway = _ShmHdl(pPassway2);
		return YXC_ERC_SUCCESS;

err_ret:
		free(pPassway2);
		return rcRet;
	}

	YXC_Status YXC_ShmPWOpen(const wchar_t* pszShmName, YXC_ShmPassway* pPassway, yuint32_t* puPWMaxSize)
	{
		_ShmPassway* pPassway2 = (_ShmPassway*)::malloc(sizeof(_ShmPassway));
		_YXC_CHECK_REPORT_NEW_RET(pPassway != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc for shared memory passway failed");

		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		YXC_Status rc = pPassway2->OpenW(pszShmName, puPWMaxSize);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		*pPassway = _ShmHdl(pPassway2);
		return YXC_ERC_SUCCESS;
err_ret:
		free(pPassway2);
		return rcRet;
	}

	YXC_Status YXC_ShmPWLock(YXC_ShmPassway passway, yuint32_t stmsTimeout)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		return pPassway->pwMutex.Lock(stmsTimeout);
	}

	YXC_Status YXC_ShmPWUnlock(YXC_ShmPassway passway)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		return pPassway->pwMutex.Unlock();
	}

	YXC_Status YXC_ShmPWMap(YXC_ShmPassway passway, yuint32_t uNumBytesToMap, void** ppAddr)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		YXC_Status rc = pPassway->Map(uNumBytesToMap, ppAddr);
		return rc;
	}

	void YXC_ShmPWUnmap(YXC_ShmPassway passway, void* pMappedAddr)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		pPassway->Unmap(pMappedAddr);
	}

	void YXC_ShmPWGetSize(YXC_ShmPassway passway, yuint32_t* puCurrentSize)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		*puCurrentSize = pPassway->GetSize();
	}

	YXC_Status YXC_ShmPWSetSize(YXC_ShmPassway passway, yuint32_t uNewSize)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		return pPassway->SetSize(uNewSize);
	}

	void YXC_ShmPWClose(YXC_ShmPassway passway)
	{
		_ShmPassway* pPassway = _ShmPtr(passway);
		pPassway->Close();
		free(pPassway);
	}
};

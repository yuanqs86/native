#define __MODULE__ "EK.Profile"

#include <YXC_Sys/YXC_Profile.h>
#include <YXC_Sys/Utils/_YXC_ProfileImpl.hpp>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_MMInterface.h>

using namespace YXC_InnerProfile;

extern "C"
{
	YXC_Status YXC_ProfileCreateW(ybool_t bAutoDiskSync, ybool_t bThreadSafe, YXC_ProfileW* pProfile)
	{
		_YCHK_MAL_R1(pProfileImpl, _Profile<wchar_t>);

		try
		{
			new (pProfileImpl) _Profile<wchar_t>(bAutoDiskSync, bThreadSafe);
		}
		catch (const std::exception& e)
		{
			free(pProfileImpl);
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%s"), e.what());
		}

		*pProfile = _PfHdlW(pProfileImpl);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_ProfileLoadW(YXC_ProfileW hProfile, const wchar_t* pszProfilePath)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->Load(pszProfilePath);
	}

	YXC_Status YXC_ProfileLoadContentW(YXC_ProfileW hProfile, const wchar_t* pszContent)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->LoadContent(pszContent);
	}

	YXC_Status YXC_ProfileSaveW(const YXC_ProfileW hProfile, const wchar_t* pszDstPath)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->Save(pszDstPath);
	}

	YXC_Status YXC_ProfileSaveContentW(YXC_ProfileW hProfile, wchar_t** pszContent, yuint32_t* puCcContent)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->SaveContent(pszContent, puCcContent);
	}

	void YXC_ProfileFreeContentW(wchar_t* pszContent)
	{
		YXC_MMCFreeData(pszContent);
	}

	YXC_Status YXC_ProfileGetSectionNamesW(const YXC_ProfileW hProfile, yuint32_t uCchRet, wchar_t* pszRet, yuint32_t* puCchNeeded)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->GetSectionNames(uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileGetSectionKeysW(YXC_ProfileW hProfile, const wchar_t* pszSection, yuint32_t uCchRet, wchar_t* pszRet,
		yuint32_t* puCchNeeded)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->GetSectionKeys(pszSection, uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileGetSectionW(const YXC_ProfileW hProfile, const wchar_t* pszSection, yuint32_t uCchRet,
		wchar_t* pszRet, yuint32_t* puCchNeeded)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->GetSection(pszSection, uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileAddSectionW(YXC_ProfileW hProfile, const wchar_t* pszSection)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->AddSection(pszSection);
	}

	YXC_Status YXC_ProfileDeleteSectionW(YXC_ProfileW hProfile, const wchar_t* pszSection)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->DeleteSection(pszSection);
	}

	YXC_Status YXC_ProfileSetSectionW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszStrSet)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->SetSection(pszSection, pszStrSet);
	}

	YXC_Status YXC_ProfileGetStringW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, const wchar_t* pszDefRet,
		yuint32_t uCchRet, wchar_t* pszRet, yuint32_t* puCchNeeded)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->GetString(pszSection, pszKey, pszDefRet, uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileGetNumberW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, yint64_t* pNumber)
	{
		wchar_t szRet[30];
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		YXC_Status rc = pProfileImpl->GetString(pszSection, pszKey, L"0", YXC_STRING_ARR_LEN(szRet), szRet, NULL);
		if (rc == YXC_ERC_SUCCESS)
		{
			int find = swscanf(szRet, L"%lld", pNumber);
			_YXC_CHECK_REPORT_NEW_RET(find, YXC_ERC_INVALID_NUMBER_FORMAT, YC("Invalid number val %ls"), szRet);
			return YXC_ERC_SUCCESS;
		}

		return rc;
	}

	YXC_Status YXC_ProfileSetStringW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, const wchar_t* pszVal)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->SetString(pszSection, pszKey, pszVal);
	}

	YXC_Status YXC_ProfileSetNumberW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, yint64_t number)
	{
		wchar_t szRet[30];
		yxcwrap_swprintf(szRet, L"%lld", number);

		YXC_Status rc = YXC_ProfileSetStringW(hProfile, pszSection, pszKey, szRet);
		return rc;
	}

	YXC_Status YXC_ProfileDeleteStringW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);
		return pProfileImpl->DeleteString(pszSection, pszKey);
	}

	void YXC_ProfileCloseW(YXC_ProfileW hProfile)
	{
		_Profile<wchar_t>* pProfileImpl = _PfPtrW(hProfile);

		pProfileImpl->~_Profile<wchar_t>();
		free(pProfileImpl);
	}
}

extern "C"
{
	YXC_Status YXC_ProfileCreateA(ybool_t bAutoDiskSync, ybool_t bThreadSafe, YXC_ProfileA* pProfile)
	{
		_YCHK_MAL_R1(pProfileImpl, _Profile<char>);

		try
		{
			new (pProfileImpl) _Profile<char>(bAutoDiskSync, bThreadSafe);
		}
		catch (const std::exception& e)
		{
			free(pProfileImpl);
			_YXC_REPORT_NEW_RET(YXC_ERC_C_RUNTIME, YC("%S"), e.what());
		}

		*pProfile = _PfHdlA(pProfileImpl);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_ProfileLoadA(YXC_ProfileA hProfile, const char* pszProfilePath)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->Load(pszProfilePath);
	}

	YXC_Status YXC_ProfileLoadContentA(YXC_ProfileA hProfile, const char* pszContent)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->LoadContent(pszContent);
	}

	YXC_Status YXC_ProfileSaveA(const YXC_ProfileA hProfile, const char* pszDstPath)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->Save(pszDstPath);
	}

	YXC_Status YXC_ProfileSaveContentA(YXC_ProfileA hProfile, char** pszContent, yuint32_t* puCcContent)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->SaveContent(pszContent, puCcContent);
	}

	void YXC_ProfileFreeContentA(char* pszContent)
	{
		YXC_MMCFreeData(pszContent);
	}

	YXC_Status YXC_ProfileGetSectionNamesA(const YXC_ProfileA hProfile, yuint32_t uCchRet, char* pszRet, yuint32_t* puCchNeeded)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->GetSectionNames(uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileGetSectionKeysA(YXC_ProfileA hProfile, const char* pszSection, yuint32_t uCchRet, char* pszRet,
		yuint32_t* puCchNeeded)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->GetSectionKeys(pszSection, uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileGetSectionA(const YXC_ProfileA hProfile, const char* pszSection, yuint32_t uCchRet,
		char* pszRet, yuint32_t* puCchNeeded)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->GetSection(pszSection, uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileAddSectionA(YXC_ProfileA hProfile, const char* pszSection)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->AddSection(pszSection);
	}

	YXC_Status YXC_ProfileDeleteSectionA(YXC_ProfileA hProfile, const char* pszSection)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->DeleteSection(pszSection);
	}

	YXC_Status YXC_ProfileSetSectionA(YXC_ProfileA hProfile, const char* pszSection, const char* pszStrSet)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->SetSection(pszSection, pszStrSet);
	}

	YXC_Status YXC_ProfileGetStringA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, const char* pszDefRet,
		yuint32_t uCchRet, char* pszRet, yuint32_t* puCchNeeded)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->GetString(pszSection, pszKey, pszDefRet, uCchRet, pszRet, puCchNeeded);
	}

	YXC_Status YXC_ProfileGetNumberA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, yint64_t* pNumber)
	{
		char szRet[30];
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		YXC_Status rc = pProfileImpl->GetString(pszSection, pszKey, "0", YXC_STRING_ARR_LEN(szRet), szRet, NULL);
	    if (rc == YXC_ERC_SUCCESS)
		{
			int find = sscanf(szRet, "%lld", pNumber);
			_YXC_CHECK_REPORT_NEW_RET(find, YXC_ERC_INVALID_NUMBER_FORMAT, YC("Invalid number val %@"), szRet);
			return YXC_ERC_SUCCESS;
		}

		return rc;
	}

	YXC_Status YXC_ProfileSetStringA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, const char* pszVal)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->SetString(pszSection, pszKey, pszVal);
	}

	YXC_Status YXC_ProfileSetNumberA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, yint64_t number)
	{
		char szRet[30];
		sprintf(szRet, "%lld", number);

		YXC_Status rc = YXC_ProfileSetStringA(hProfile, pszSection, pszKey, szRet);
		return rc;
	}

	YXC_Status YXC_ProfileDeleteStringA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);
		return pProfileImpl->DeleteString(pszSection, pszKey);
	}

	void YXC_ProfileCloseA(YXC_ProfileA hProfile)
	{
		_Profile<char>* pProfileImpl = _PfPtrA(hProfile);

		pProfileImpl->~_Profile<char>();
		free(pProfileImpl);
	}
}

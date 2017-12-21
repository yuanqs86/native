/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_SYS_BASE_PROFILE_H__
#define __INC_YXC_SYS_BASE_PROFILE_H__

#include <YXC_Sys/YXC_Sys.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	YXC_DECLARE_STRUCTURE_HANDLE(YXC_ProfileA);
	YXC_DECLARE_STRUCTURE_HANDLE(YXC_ProfileW);

	YXC_API(YXC_Status) YXC_ProfileCreateA(ybool_t bAutoDiskSync, ybool_t bThreadSafe, YXC_ProfileA* pProfile);

	YXC_API(YXC_Status) YXC_ProfileLoadA(YXC_ProfileA hProfile, const char* pszProfilePath);

	YXC_API(YXC_Status) YXC_ProfileLoadContentA(YXC_ProfileA hProfile, const char* pszContent);

	YXC_API(YXC_Status) YXC_ProfileSaveA(const YXC_ProfileA hProfile, const char* pszDstPath);

	YXC_API(YXC_Status) YXC_ProfileSaveContentA(const YXC_ProfileA hProfile, char** pszContent, yuint32_t* puCcContent);

	YXC_API(void) YXC_ProfileFreeContentA(char* content);

	YXC_API(YXC_Status) YXC_ProfileGetSectionNamesA(const YXC_ProfileA hProfile, yuint32_t uCchRet, char* pszRet,
		yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileGetSectionKeysA(YXC_ProfileA hProfile, const char* pszSection,
		yuint32_t uCchRet, char* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileGetSectionA(const YXC_ProfileA hProfile, const char* pszSection, yuint32_t uCchRet,
		char* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileAddSectionA(YXC_ProfileA hProfile, const char* pszSection);

	YXC_API(YXC_Status) YXC_ProfileSetSectionA(YXC_ProfileA hProfile, const char* pszSection, const char* pszStrSet);

	YXC_API(YXC_Status) YXC_ProfileDeleteSectionA(YXC_ProfileA hProfile, const char* pszSection);

	YXC_API(YXC_Status) YXC_ProfileGetStringA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey,
		const char* pszDefRet, yuint32_t uCchRet, char* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileGetNumberA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, yint64_t* pNumber);

	YXC_API(YXC_Status) YXC_ProfileSetStringA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, const char* pszVal);

	YXC_API(YXC_Status) YXC_ProfileSetNumberA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey, yint64_t number);

	YXC_API(YXC_Status) YXC_ProfileDeleteStringA(YXC_ProfileA hProfile, const char* pszSection, const char* pszKey);

	YXC_API(void) YXC_ProfileCloseA(YXC_ProfileA hProfile);

	YXC_API(YXC_Status) YXC_ProfileCreateW(ybool_t bAutoDiskSync, ybool_t bThreadSafe, YXC_ProfileW* pProfile);

	YXC_API(YXC_Status) YXC_ProfileLoadW(YXC_ProfileW hProfile, const wchar_t* pszProfilePath);

	YXC_API(YXC_Status) YXC_ProfileLoadContentW(YXC_ProfileW hProfile, const wchar_t* pszContent);

	YXC_API(YXC_Status) YXC_ProfileSaveW(const YXC_ProfileW hProfile, const wchar_t* pszDstPath);

	YXC_API(YXC_Status) YXC_ProfileSaveContentW(const YXC_ProfileW hProfile, wchar_t** pszContent, yuint32_t* puCcContent);

	YXC_API(void) YXC_ProfileFreeContentW(wchar_t* content);

	YXC_API(YXC_Status) YXC_ProfileGetSectionNamesW(const YXC_ProfileW hProfile, yuint32_t uCchRet, wchar_t* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileGetSectionKeysW(YXC_ProfileW hProfile, const wchar_t* pszSection,
		yuint32_t uCchRet, wchar_t* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileGetSectionW(const YXC_ProfileW hProfile, const wchar_t* pszSection, yuint32_t uCchRet, wchar_t* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileAddSectionW(YXC_ProfileW hProfile, const wchar_t* pszSection);

	YXC_API(YXC_Status) YXC_ProfileSetSectionW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszStrSet);

	YXC_API(YXC_Status) YXC_ProfileDeleteSectionW(YXC_ProfileW hProfile, const wchar_t* pszSection);

	YXC_API(YXC_Status) YXC_ProfileGetStringW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey,
		const wchar_t* pszDefRet, yuint32_t uCchRet, wchar_t* pszRet, yuint32_t* puCchNeeded);

	YXC_API(YXC_Status) YXC_ProfileGetNumberW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, yint64_t* pNumber);

	YXC_API(YXC_Status) YXC_ProfileSetStringW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, const wchar_t* pszVal);

	YXC_API(YXC_Status) YXC_ProfileSetNumberW(YXC_ProfileW hProfile, const wchar_t* pszSection, const wchar_t* pszKey, yint64_t number);

	YXC_API(YXC_Status) YXC_ProfileDeleteStringW(YXC_ProfileW handle, const wchar_t* pszSection, const wchar_t* pszKey);

	YXC_API(void) YXC_ProfileCloseW(YXC_ProfileW hProfile);

#if YCHAR_WCHAR_T
    typedef YXC_ProfileW YXC_Profile;

#define YXC_ProfileCreate YXC_ProfileCreateW
#define YXC_ProfileLoad YXC_ProfileLoadW
#define YXC_ProfileLoadContent YXC_ProfileLoadContentW
#define YXC_ProfileSave YXC_ProfileSaveW
#define YXC_ProfileSaveContent YXC_ProfileSaveContentW
#define YXC_ProfileFreeContent YXC_ProfileFreeContentW
#define YXC_ProfileGetSectionNames YXC_ProfileGetSectionNamesW
#define YXC_ProfileGetSectionKeys YXC_ProfileGetSectionKeysW
#define YXC_ProfileGetSection YXC_ProfileGetSectionW
#define YXC_ProfileAddSection YXC_ProfileAddSectionW
#define YXC_ProfileSetSection YXC_ProfileSetSectionW
#define YXC_ProfileDeleteSection YXC_ProfileDeleteSectionW
#define YXC_ProfileGetString YXC_ProfileGetStringW
#define YXC_ProfileGetNumber YXC_ProfileGetNumberW
#define YXC_ProfileSetString YXC_ProfileSetStringW
#define YXC_ProfileSetNumber YXC_ProfileSetNumberW
#define YXC_ProfileDeleteString YXC_ProfileDeleteStringW
#define YXC_ProfileClose YXC_ProfileCloseW

#else
    typedef YXC_ProfileA YXC_Profile;

#define YXC_ProfileCreate YXC_ProfileCreateA
#define YXC_ProfileLoad YXC_ProfileLoadA
#define YXC_ProfileLoadContent YXC_ProfileLoadContentA
#define YXC_ProfileSave YXC_ProfileSaveA
#define YXC_ProfileSaveContent YXC_ProfileSaveContentA
#define YXC_ProfileFreeContent YXC_ProfileFreeContentA
#define YXC_ProfileGetSectionNames YXC_ProfileGetSectionNamesA
#define YXC_ProfileGetSectionKeys YXC_ProfileGetSectionKeysA
#define YXC_ProfileGetSection YXC_ProfileGetSectionA
#define YXC_ProfileAddSection YXC_ProfileAddSectionA
#define YXC_ProfileSetSection YXC_ProfileSetSectionA
#define YXC_ProfileDeleteSection YXC_ProfileDeleteSectionA
#define YXC_ProfileGetString YXC_ProfileGetStringA
#define YXC_ProfileGetNumber YXC_ProfileGetNumberA
#define YXC_ProfileSetString YXC_ProfileSetStringA
#define YXC_ProfileSetNumber YXC_ProfileSetNumberA
#define YXC_ProfileDeleteString YXC_ProfileDeleteStringA
#define YXC_ProfileClose YXC_ProfileCloseA

#endif /* YCHAR_WCHAR_T */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_SYS_BASE_PROFILE_H__ */

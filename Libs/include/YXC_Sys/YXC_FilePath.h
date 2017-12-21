
/* ****************************************************************************** *\

Author : Qiushi Yuan
Copyright(c) 2008-2015 Qiushi Yuan. All Rights Reserved.
This software is supplied under the terms, you cannot copy or disclose except agreement with the author. 

\* ****************************************************************************** */
#ifndef __INC_YXC_BASE_FILE_PATH_H__
#define __INC_YXC_BASE_FILE_PATH_H__

#include <YXC_Sys/YXC_Sys.h>

#define YXC_FPATH_ATTR_DIR 0x1
#define YXC_FPATH_ATTR_FILE 0x2
#define YXC_FPATH_ATTR_HIDDEN 0x4
#define YXC_FPATH_ATTR_READONLY 0x10
#define YXC_FPATH_ATTR_SYSTEM 0x20

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	typedef enum __YXC_FPATH_SPECIAL_DIR
	{
		YXC_FPATH_SDIR_UNKNOWN = 0,
		YXC_FPATH_SDIR_DOCUMENTS = 1,
		YXC_FPATH_SDIR_APPDATA = 2,
		/* YXC_FPATH_SDIR_DESKTOP = 3, */
		YXC_FPATH_SDIR_BINARIES = 4,
		YXC_FPATH_SDIR_APPLICATION = 5,
		YXC_FPATH_SDIR_PUBLIC = 6,
	}YXC_FPathSpecDir;

    YXC_API(YXC_Status) YXC_FPathAttributeA(const char* path, yuint32_t* puAttribute);

    YXC_API(YXC_Status) YXC_FPathCombineA(char* path_dst, const char* path1, const char* path2);

    YXC_API(YXC_Status) YXC_FPathAppendA(char* path_dst, const char* path_to_append);

    YXC_API(YXC_Status) YXC_FPathCatA(char* path_dst, const char* path_to_cat);

    YXC_API(YXC_Status) YXC_FPathCopyA(char* path_dst, const char* path_to_copy);

    YXC_API(YXC_Status) YXC_FPathAddExtensionA(char* path, const char* ext);

    YXC_API(YXC_Status) YXC_FPathRemoveExtensionA(char* path);

    YXC_API(const char*) YXC_FPathFindFileSpecA(const char* path);

    YXC_API(const char*) YXC_FPathFindExtensionA(const char* path);

    YXC_API(YXC_Status) YXC_FPathRemoveFileSpecA(char* path);

    YXC_API(YXC_Status) YXC_FPathExactA(char* path);

	YXC_API(ybool_t) YXC_FPathExistsA(const char* path);

    YXC_API(YXC_Status) YXC_FPathFindRootA(const char* path, char* root);

    YXC_API(YXC_Status) YXC_FPathFindModulePathA(char* path, const char* module);

    YXC_API(YXC_Status) YXC_FPathFindSpecDirA(char* path_base, YXC_FPathSpecDir dir);

    YXC_API(YXC_Status) YXC_FPathCreateDirA(const char* pszPath, YXC_KObjectAttr* attr, ybool_t bCreateAll);

    YXC_API(YXC_Status) YXC_FPathDeleteA(const char* pszPath, ybool_t bFileDelete);

	YXC_API(YXC_Status) YXC_FPathAttributeW(const wchar_t* path, yuint32_t* puAttribute);

	YXC_API(YXC_Status) YXC_FPathCombineW(wchar_t* path_dst, const wchar_t* path1, const wchar_t* path2);

	YXC_API(YXC_Status) YXC_FPathAppendW(wchar_t* path_dst, const wchar_t* path_to_append);

	YXC_API(YXC_Status) YXC_FPathCatW(wchar_t* path_dst, const wchar_t* path_to_cat);

	YXC_API(YXC_Status) YXC_FPathCopyW(wchar_t* path_dst, const wchar_t* path_to_copy);

	YXC_API(YXC_Status) YXC_FPathAddExtensionW(wchar_t* path, const wchar_t* ext);

	YXC_API(YXC_Status) YXC_FPathRemoveExtensionW(wchar_t* path);

	YXC_API(const wchar_t*) YXC_FPathFindFileSpecW(const wchar_t* path);

	YXC_API(const wchar_t*) YXC_FPathFindExtensionW(const wchar_t* path);

	YXC_API(YXC_Status) YXC_FPathRemoveFileSpecW(wchar_t* path);

	YXC_API(YXC_Status) YXC_FPathExactW(wchar_t* path);

	YXC_API(ybool_t) YXC_FPathExistsW(const wchar_t* path);

	YXC_API(YXC_Status) YXC_FPathFindRootW(const wchar_t* path, wchar_t* root);

	YXC_API(YXC_Status) YXC_FPathFindModulePathW(wchar_t* path, const wchar_t* module);

	YXC_API(YXC_Status) YXC_FPathFindSpecDirW(wchar_t* path_base, YXC_FPathSpecDir dir);

	YXC_API(YXC_Status) YXC_FPathCreateDirW(const wchar_t* pszPath, YXC_KObjectAttr* attr, ybool_t bCreateAll);

	YXC_API(YXC_Status) YXC_FPathDeleteW(const wchar_t* pszPath, ybool_t bFileDelete);

#if YCHAR_WCHAR_T
#define YXC_FPathAttribute(_EM_path, _EM_puAttribute) YXC_FPathAttributeW(_EM_path, _EM_puAttribute)
#define YXC_FPathCombine(_EM_path_dst, _EM_path1, _EM_path2) YXC_FPathCombineW(_EM_path_dst, _EM_path1, _EM_path2)
#define YXC_FPathAppend(_EM_path_dst, _EM_path_to_append) YXC_FPathAppendW(_EM_path_dst, _EM_path_to_append)
#define YXC_FPathCat(_EM_path_dst, _EM_path_to_cat) YXC_FPathCatW(_EM_path_dst, _EM_path_to_cat)
#define YXC_FPathCopy(_EM_path_dst, _EM_path_to_copy) YXC_FPathCopyW(_EM_path_dst, _EM_path_to_copy)
#define YXC_FPathAddExtension(_EM_path_dst, _EM_ext) YXC_FPathAddExtensionW(_EM_path_dst, _EM_ext)
#define YXC_FPathRemoveExtension(_EM_path_dst) YXC_FPathRemoveExtensionW(_EM_path_dst)
#define YXC_FPathFindFileSpec(_EM_path) YXC_FPathFindFileSpecW(_EM_path)
#define YXC_FPathFindExtension(_EM_path) YXC_FPathFindExtensionW(_EM_path)
#define YXC_FPathRemoveFileSpec(_EM_path) YXC_FPathRemoveFileSpecW(_EM_path)
#define YXC_FPathExact(_EM_path) YXC_FPathExactW(_EM_path)
#define YXC_FPathExists(_EM_path) YXC_FPathExistsW(_EM_path)
#define YXC_FPathFindRoot(_EM_path, _EM_root) YXC_FPathFindRootW(_EM_path, _EM_root)
#define YXC_FPathFindModulePath(_EM_path, _EM_module) YXC_FPathFindModulePathW(_EM_path, _EM_module)
#define YXC_FPathFindSpecDir(_EM_path_base, _EM_dir) YXC_FPathFindSpecDirW(_EM_path_base, _EM_dir)
#define YXC_FPathCreateDir(_EM_pszPath, _EM_attr, _EM_bCreateAll) YXC_FPathCreateDirW(_EM_pszPath, _EM_attr, _EM_bCreateAll)
#define YXC_FPathDelete(_EM_pszPath, _EM_bFileDelete) YXC_FPathDeleteW(_EM_pszPath, _EM_bFileDelete)
#else
#define YXC_FPathAttribute(_EM_path, _EM_puAttribute) YXC_FPathAttributeA(_EM_path, _EM_puAttribute)
#define YXC_FPathCombine(_EM_path_dst, _EM_path1, _EM_path2) YXC_FPathCombineA(_EM_path_dst, _EM_path1, _EM_path2)
#define YXC_FPathAppend(_EM_path_dst, _EM_path_to_append) YXC_FPathAppendA(_EM_path_dst, _EM_path_to_append)
#define YXC_FPathCat(_EM_path_dst, _EM_path_to_cat) YXC_FPathCatA(_EM_path_dst, _EM_path_to_cat)
#define YXC_FPathCopy(_EM_path_dst, _EM_path_to_copy) YXC_FPathCopyA(_EM_path_dst, _EM_path_to_copy)
#define YXC_FPathAddExtension(_EM_path_dst, _EM_ext) YXC_FPathAddExtensionA(_EM_path_dst, _EM_ext)
#define YXC_FPathRemoveExtension(_EM_path_dst) YXC_FPathRemoveExtensionA(_EM_path_dst)
#define YXC_FPathFindFileSpec(_EM_path) YXC_FPathFindFileSpecA(_EM_path)
#define YXC_FPathFindExtension(_EM_path) YXC_FPathFindExtensionA(_EM_path)
#define YXC_FPathRemoveFileSpec(_EM_path) YXC_FPathRemoveFileSpecA(_EM_path)
#define YXC_FPathExact(_EM_path) YXC_FPathExactA(_EM_path)
#define YXC_FPathExists(_EM_path) YXC_FPathExistsA(_EM_path)
#define YXC_FPathFindRoot(_EM_path, _EM_root) YXC_FPathFindRootA(_EM_path, _EM_root)
#define YXC_FPathFindModulePath(_EM_path, _EM_module) YXC_FPathFindModulePathA(_EM_path, _EM_module)
#define YXC_FPathFindSpecDir(_EM_path_base, _EM_dir) YXC_FPathFindSpecDirA(_EM_path_base, _EM_dir)
#define YXC_FPathCreateDir(_EM_pszPath, _EM_attr, _EM_bCreateAll) YXC_FPathCreateDirA(_EM_pszPath, _EM_attr, _EM_bCreateAll)
#define YXC_FPathDelete(_EM_pszPath, _EM_bFileDelete) YXC_FPathDeleteA(_EM_pszPath, _EM_bFileDelete)
#endif /* YCHAR_WCHAR_T */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INC_YXC_BASE_FILE_PATH_H__ */

#define __MODULE__ "YXK.FilePath"

#include <YXC_Sys/YXC_FilePath.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_OSUtil.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>

#if YXC_PLATFORM_APPLE
#include <mach-o/dyld_images.h>
#include <mach-o/dyld.h>
#endif /* YXC_PLATFORM_APPLE */

#if YXC_PLATFORM_WIN
#include <sys/stat.h>
#include <ShlObj.h>
#else
#include <sys/stat.h>
#endif /* YXC_PLATFORM_WIN */

namespace YXC_Inner
{
    YXC_Status _YXC_FPathFindSpecDir_IOS(ychar* path_base, YXC_FPathSpecDir dir);
}

using YXCLib::_ChTraits;

namespace
{
	template <typename C>
	struct _PTraits
	{
		const static ychar _EPATH_SEP[];
	};

	template <>
	struct _PTraits<wchar_t>
	{
		const static wchar_t _EPATH_SEP[];
	};

	template <>
	struct _PTraits<char>
	{
		const static char _EPATH_SEP[];
	};

#if YXC_PLATFORM_WIN
	const wchar_t _PTraits<wchar_t>::_EPATH_SEP[] = L"\\";
	const char _PTraits<char>::_EPATH_SEP[] = "\\";
#else
	const wchar_t _PTraits<wchar_t>::_EPATH_SEP[] = L"/";
	const char _PTraits<char>::_EPATH_SEP[] = "/";
#endif

	template <typename C>
    ybool_t _AppendPath(C*& path_s, C*& path_e, const C* path_to_append)
    {
        ybool_t bFullAppend = YXCLib::_AppendStringChkT<C>(path_s, path_e, _PTraits<C>::_EPATH_SEP, 1);
        if (!bFullAppend) return FALSE;

        bFullAppend = YXCLib::_AppendStringChkT<C>(path_s, path_e, path_to_append, YXC_STR_NTS);
        return bFullAppend;
    }

	template <typename C>
    ybool_t _IsSlash(const C* p)
    {
        return *p == '/' || *p == '\\';
    }

    int _ExecDeletePath(ybool_t bIsDir, const ychar* p)
    {
        if (bIsDir)
        {
#if YXC_PLATFORM_WIN
			BOOL bRet = ::RemoveDirectoryW(p);
			return bRet ? 0 : GetLastError();
#else
            return rmdir(p);
#endif /* YXC_PLATFORM_WIN */
        }

#if YXC_PLATFORM_WIN
		BOOL bRet = ::DeleteFileW(p);
		return bRet ? 0 : GetLastError();
#else
        return remove(p);
#endif /* YXC_PLATFORM_WIN */
    }
}

namespace
{
    YXC_Status _YXC_FPathAttribute(const ychar* path, yuint32_t* puAttribute)
    {
        yuint32_t uAttrRet = 0;

#if YXC_PLATFORM_WIN
		struct _stat64 st;
		int ret = _wstat64(path, &st);
		_YXC_CHECK_OS_RET(ret == 0, YC("Failed call to stat"));

		if (st.st_mode & S_IFDIR) uAttrRet |= YXC_FPATH_ATTR_DIR;
		if (st.st_mode & S_IFREG) uAttrRet |= YXC_FPATH_ATTR_FILE;
#else
        struct stat st;
        int ret = stat(path, &st);

        _YXC_CHECK_OS_RET(ret == 0, YC("Failed call to stat"));

        if (S_ISDIR(st.st_mode)) uAttrRet |= YXC_FPATH_ATTR_DIR;
        if (S_ISREG(st.st_mode)) uAttrRet |= YXC_FPATH_ATTR_FILE;
#endif /* YXC_PLATFORM_WIN */

		*puAttribute = uAttrRet;
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathCombine(C* path_dst, const C* path1, const C* path2)
    {
        C* path_s = path_dst, *path_e = path_dst + YXC_MAX_CCH_PATH;
        ybool_t bFullAppend = YXCLib::_AppendStringChkT<C>(path_s, path_e, path1, YXC_STR_NTS);
        _YXC_CHECK_REPORT_NEW_RET(bFullAppend, YXC_ERC_EXCEED_MAX_COUNT, YC("path1 '%s' truncated"), path1);

		if (path_s > path_dst && ::_IsSlash(path_s - 1)) --path_s;

        bFullAppend = _AppendPath(path_s, path_e, path2);
        _YXC_CHECK_REPORT_NEW_RET(bFullAppend, YXC_ERC_EXCEED_MAX_COUNT, YC("path2 '%s' truncated"), path2);
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathAppend(C* path_dst, const C* path_to_append)
    {
        ysize_t o_str_len = _ChTraits<C>::strlen(path_dst);
        C* path_s = path_dst + o_str_len, *path_e = path_dst + YXC_MAX_CCH_PATH;

        ybool_t bFullAppend = _AppendPath(path_s, path_e, path_to_append);
        _YXC_CHECK_REPORT_NEW_RET(bFullAppend, YXC_ERC_EXCEED_MAX_COUNT, YC("path_to_append exceeds"));
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathCat(C* path_dst, const C* path_to_cat)
    {
        ysize_t o_str_len = _ChTraits<C>::strlen(path_dst);
        C* path_s = path_dst + o_str_len, *path_e = path_dst + YXC_MAX_CCH_PATH;

        ybool_t bFullAppend = YXCLib::_AppendStringChkT(path_s, path_e, path_to_cat, YXC_STR_NTS);
        _YXC_CHECK_REPORT_NEW_RET(bFullAppend, YXC_ERC_EXCEED_MAX_COUNT, YC("path_to_cat exceeds"));
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathCopy(C* path_dst, const C* path_to_copy)
    {
        C* path_s = path_dst, *path_e = path_dst + YXC_MAX_CCH_PATH;
        ybool_t bFullAppend = YXCLib::_AppendStringChkT(path_s, path_e, path_to_copy, YXC_STR_NTS);
        _YXC_CHECK_REPORT_NEW_RET(bFullAppend, YXC_ERC_EXCEED_MAX_COUNT, YC("path_to_copy exceeds"));

        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathAddExtension(C* path, const C* ext)
    {
        return _YXC_FPathCat(path, ext);
    }

	template <typename C>
    const C* _YXC_FPathFindExtension(const C* path)
    {
        ysize_t o_str_len = _ChTraits<C>::strlen(path);
        const C* pExtFound = NULL;

        for (const C* p = path + o_str_len - 1; p >= path; --p)
        {
            if (*p == '.') /* Find first extension. */
            {
                pExtFound = p;
                break;
            }
            else if (_IsSlash(p)) /* path slash, no extension found. */
            {
                break;
            }
        }

        _YXC_CHECK_REPORT_NEW_RET2(pExtFound != NULL, YXC_ERC_INVALID_PARAMETER, path + o_str_len, YC("No extenstion found"));
        return pExtFound;
    }

	template <typename C>
    YXC_Status _YXC_FPathRemoveExtension(C* path)
    {
        C* ext = (C*)_YXC_FPathFindExtension((C*)path);
        _YXC_CHECK_REPORT_NEW_RET(ext != NULL, YXC_ERC_INVALID_PARAMETER, YC("No extenstion was found"));

        *ext = 0;
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    const C* _YXC_FPathFindFileSpec(const C* path)
	{
		ysize_t o_str_len = _ChTraits<C>::strlen(path);
        const C* pFileSpec = NULL;

        for (const C* p = path + o_str_len - 1; p >= path; --p)
        {
            if (_IsSlash(p)) /* path slash, no extension found. */
            {
                pFileSpec = p + 1;
                break;
            }
        }

        _YXC_CHECK_REPORT_NEW_RET2(pFileSpec != NULL, YXC_ERC_INVALID_PATH, NULL, YC("Invalid path(%s)"), path);
        return pFileSpec;
    }

	template <typename C>
    YXC_Status _YXC_FPathRemoveFileSpec(C* path)
    {
		C* fSpec = (C*)_YXC_FPathFindFileSpec(path);
		_YXC_CHECK_REPORT_NEW_RET(fSpec != NULL, YXC_ERC_INVALID_PATH, YC("No file was found"));

        *(fSpec - 1) = 0; /* remove path slash. */
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathExact(C* path)
    {
        ysize_t o_str_len = _ChTraits<C>::strlen(path);
        C* dst = path;
        for (C* p = path; p < path + o_str_len; ++p)
        {
			C val = *p;
            if (_IsSlash(p))
            {
                C* p2 = p;
                while (_IsSlash(p2 + 1))
                {
                    ++p2;
                }
                p = p2;
				val = _PTraits<C>::_EPATH_SEP[0];
            }
            *dst++ = val;
        }
		*dst = 0;
        return YXC_ERC_SUCCESS;
    }

	template <typename C>
    YXC_Status _YXC_FPathFindRoot(const C* path, C* root)
    {
        yint32_t uXCount = 0;
#if YXC_PLATFORM_WIN
        const yuint32_t uSplitReq = 1;
#elif YXC_PLATFORM_UNIX
        const yuint32_t uSplitReq = 2;
#endif /* YXC_PLATFORM_WIN */
        for (const C* p = path; *p != 0 && p - path < YXC_MAX_CCH_PATH; ++p)
        {
            if (_IsSlash(p))
            {
                ++uXCount;
            }

            *root = *p;
            if (uXCount == uSplitReq) /* Root path is found. */
            {
                return YXC_ERC_SUCCESS;
            }
        }

        *root = 0; /* Root path not found. */
        _YXC_REPORT_NEW_RET(YXC_ERC_INVALID_PATH, YC("Invalid path('%s')"), path);
    }

    YXC_Status _YXC_FPathCreateDir(const ychar* pszPath, YXC_KObjectAttr* attr, ybool_t bCreateAll)
    {
#if YXC_PLATFORM_UNIX
        ychar szSys[YXC_MAX_CCH_PATH + 100];

		if (bCreateAll)
		{
			sprintf(szSys, "mkdir -p %s", pszPath);
		}
		else
		{
			sprintf(szSys, "mkdir %s", pszPath);
		}
        int ret = system(szSys);
        //int ret = mkdir(pszPath, attr);
        _YXC_CHECK_OS_RET(ret == 0, YC("Failed to make dir ('%s')"), pszPath);

        return YXC_ERC_SUCCESS;
#else
		if (bCreateAll)
		{
			int iRet = SHCreateDirectory(NULL, pszPath);
			_YXC_CHECK_REPORT_NEW_RET(iRet != ERROR_ALREADY_EXISTS, YXC_ERC_ALREADY_EXISTS, YC("Directory already exists"));
			_YXC_CHECK_OS_RET(iRet == 0, YC("Failed to make dir ('%s')"), pszPath);
		}
		else
		{
			BOOL bRet = ::CreateDirectoryW(pszPath, NULL);
			if (!bRet)
			{
				_YXC_CHECK_REPORT_NEW_RET(::GetLastError() != ERROR_ALREADY_EXISTS, YXC_ERC_ALREADY_EXISTS, YC("Directory already exists"));
				_YXC_REPORT_OS_RET(YC("Failed to make dir ('%s')"), pszPath);
			}
		}

		return YXC_ERC_SUCCESS;
#endif /* YXC_PLATFORM_UNIX */
    }

    YXC_Status _YXC_FPathDelete(const ychar* pszPath, ybool_t bIsDir)
    {
        if (!bIsDir)
        {
            int r = _ExecDeletePath(FALSE, pszPath);
            _YXC_CHECK_OS_RET(r == 0, YC("Failed to delete file ('%s)"), pszPath);
        }
        else
        {
            YXCLib::OSFileFinder ff;
            YXC_Status rc = ff.Open(pszPath);
            _YXC_CHECK_RC_RET(rc);

            if (ff.IsDir())
            {
                while (TRUE)
                {
                    YXCLib::OSFileAttr fAttr;
                    rc = ff.Next(&fAttr);

                    if (rc == YXC_ERC_NO_DATA) /* end of directory. */
                    {
                        break;
                    }

                    _YXC_CHECK_RC_RET(rc);
                    rc = YXC_FPathDelete(fAttr.osFilePath, fAttr.bIsDir);
                    _YXC_CHECK_RC_RET(rc);
                }

                int r = _ExecDeletePath(TRUE, pszPath);
                _YXC_CHECK_OS_RET(r == 0, YC("Failed to delete dir('%s')"), pszPath);
            }
            else
            {
                int r = _ExecDeletePath(FALSE, pszPath);
                _YXC_CHECK_OS_RET(r == 0, YC("Failed to delete file ('%s)"), pszPath);
            }
        }
        return YXC_ERC_SUCCESS;
    }

    YXC_Status _YXC_FPathFindSpecDir(ychar* path_base, YXC_FPathSpecDir dir)
    {
        if (dir == YXC_FPATH_SDIR_APPLICATION)
        {
            return YXC_FPathFindModulePath(path_base, NULL);
        }
#if YXC_PLATFORM_WIN
		BOOL bRet = FALSE;
		switch (dir)
		{
		case YXC_FPATH_SDIR_DOCUMENTS:
			bRet = SHGetSpecialFolderPathW(NULL, path_base, CSIDL_MYDOCUMENTS, TRUE);
			break;
		case YXC_FPATH_SDIR_APPDATA:
			bRet = SHGetSpecialFolderPathW(NULL, path_base, CSIDL_LOCAL_APPDATA, TRUE);
			break;
		case YXC_FPATH_SDIR_BINARIES:
			bRet = SHGetSpecialFolderPathW(NULL, path_base, CSIDL_SYSTEM, TRUE);
			break;
		case YXC_FPATH_SDIR_PUBLIC:
			bRet = SHGetSpecialFolderPathW(NULL, path_base, CSIDL_COMMON_DOCUMENTS, TRUE);
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_PATH, YC("Invalid special directory to find"));
			break;
		}
		_YXC_CHECK_OS_RET(bRet, YC("SHGetSpecialFolderPathW %d"), dir);
		return YXC_ERC_SUCCESS;
#elif YXC_PLATFORM_APPLE

    #if YXC_PLATFORM_IOS
        return YXC_Inner::_YXC_FPathFindSpecDir_IOS(path_base, dir);
    #else
        ychar* pszHome = getenv("HOME");
        switch (dir)
        {
            case YXC_FPATH_SDIR_DOCUMENTS:
                YXC_FPathCombine(path_base, pszHome, YC("Documents"));
                break;
            case YXC_FPATH_SDIR_APPDATA:
                YXC_FPathCombine(path_base, pszHome, YC("AppData"));
                break;
            case YXC_FPATH_SDIR_BINARIES:
                YXC_FPathCopy(path_base, YC("/bin"));
                break;
            default:
                _YXC_REPORT_NEW_RET(YXC_ERC_INVALID_PATH, YC("Invalid special directory to find"));
                break;
        }
        return YXC_ERC_SUCCESS;
    #endif /* YXC_PLATFORM_IOS */

#elif YXC_PLATFORM_LINUX
#endif /* YXC_PLATFORM_WIN */
    }

    YXC_Status _YXC_FPathFindModulePath(ychar* path, const ychar* module)
    {
#if YXC_PLATFORM_APPLE
        uint32_t uNumModules = _dyld_image_count();
        if (module == NULL) /* First argument for exe. */
        {
            YXC_FPathCopy(path, _dyld_get_image_name(0));
            return YXC_ERC_SUCCESS;
        }
        for (yuint32_t i = 0; i < uNumModules; ++i)
        {
            const char* name = _dyld_get_image_name(i);

            if (yh_strstr(name, module) != NULL) /* Is this module? */
            {
                YXC_FPathCopy(path, name);
                return YXC_ERC_SUCCESS;
            }
        }

        _YXC_REPORT_NEW_RET(YXC_ERC_KEY_NOT_FOUND, YC("Can't find module('%s')"), module);
#elif YXC_PLATFORM_WIN
		HMODULE h = GetModuleHandleW(module);
		_YXC_CHECK_OS_RET(h != NULL, YC("Can't find module('%s')"), module);

		DWORD dwRet = ::GetModuleFileNameW(h, path, YXC_MAX_CCH_PATH);
		_YXC_CHECK_OS_RET(dwRet > 0, YC("GetModuleFileNameW('%s')"), module);

		return YXC_ERC_SUCCESS;
#else
		_YXC_CHECK_REPORT_NEW_RET(module == NULL, YXC_ERC_NOT_SUPPORTED, YC("Only support exe path"));

		int val = readlink("/proc/self/exe", path, YXC_MAX_CCH_PATH);
		_YXC_CHECK_OS_RET(val > 0, YC("readlink"));

		//char* path_end = strrchr(processdir,  '/');
		//_YXC_CHECK_REPORT_NEW_RET(path_end != NULL, YXC_ERC_INVALID_PATH, YC("Invalid process dir"));

		return YXC_ERC_SUCCESS;
#endif /* YXC_PLATFORM_APPLE */
    }
}

extern "C"
{
	YXC_Status YXC_FPathAttributeA(const char* path, yuint32_t* puAttribute)
	{
#if YCHAR_WCHAR_T
		YXC_FPath p;
		YXC_TECharToEChar(path, YXC_STR_NTS, p, YXC_MAX_CCH_PATH, NULL, NULL);
		return _YXC_FPathAttribute(p, puAttribute);
#else
		return _YXC_FPathAttribute(path, puAttribute);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status YXC_FPathAttributeW(const wchar_t* path, yuint32_t* puAttribute)
	{
#if YCHAR_WCHAR_T
		return _YXC_FPathAttribute(path, puAttribute);
#else
		YXC_FPath p;
		YXC_TEWCharToEChar(path, YXC_STR_NTS, p, YXC_MAX_CCH_PATH, NULL, NULL);
		return _YXC_FPathAttribute(p, puAttribute);
#endif /* YCHAR_WCHAR_T */
	}

	YXC_Status YXC_FPathCombineA(char* path_dst, const char* path1, const char* path2)
	{
		return _YXC_FPathCombine<char>(path_dst, path1, path2);
	}

	YXC_Status YXC_FPathCombineW(wchar_t* path_dst, const wchar_t* path1, const wchar_t* path2)
	{
		return _YXC_FPathCombine<wchar_t>(path_dst, path1, path2);
	}

	YXC_Status YXC_FPathAppendA(char* path_dst, const char* path_to_append)
	{
		return _YXC_FPathAppend<char>(path_dst, path_to_append);
	}

	YXC_Status YXC_FPathAppendW(wchar_t* path_dst, const wchar_t* path_to_append)
	{
		return _YXC_FPathAppend<wchar_t>(path_dst, path_to_append);
	}

	YXC_Status YXC_FPathCatA(char* path_dst, const char* path_to_cat)
	{
		return _YXC_FPathCat<char>(path_dst, path_to_cat);
	}

	YXC_Status YXC_FPathCatW(wchar_t* path_dst, const wchar_t* path_to_cat)
	{
		return _YXC_FPathCat<wchar_t>(path_dst, path_to_cat);
	}

	YXC_Status YXC_FPathCopyA(char* path_dst, const char* path_to_copy)
	{
		return _YXC_FPathCopy<char>(path_dst, path_to_copy);
	}

	YXC_Status YXC_FPathCopyW(wchar_t* path_dst, const wchar_t* path_to_copy)
	{
		return _YXC_FPathCopy<wchar_t>(path_dst, path_to_copy);
	}

	YXC_Status YXC_FPathAddExtensionA(char* path, const char* ext)
	{
		return _YXC_FPathAddExtension<char>(path, ext);
	}

	YXC_Status YXC_FPathAddExtensionW(wchar_t* path, const wchar_t* ext)
	{
		return _YXC_FPathAddExtension<wchar_t>(path, ext);
	}

	YXC_Status YXC_FPathRemoveExtensionA(char* path)
	{
		return _YXC_FPathRemoveExtension<char>(path);
	}

	YXC_Status YXC_FPathRemoveExtensionW(wchar_t* path)
	{
		return _YXC_FPathRemoveExtension<wchar_t>(path);
	}

	const char* YXC_FPathFindFileSpecA(const char* path)
	{
		return _YXC_FPathFindFileSpec<char>(path);
	}

	const wchar_t* YXC_FPathFindFileSpecW(const wchar_t* path)
	{
		return _YXC_FPathFindFileSpec<wchar_t>(path);
	}

	const char* YXC_FPathFindExtensionA(const char* path)
	{
		return _YXC_FPathFindExtension<char>(path);
	}

	const wchar_t* YXC_FPathFindExtensionW(const wchar_t* path)
	{
		return _YXC_FPathFindExtension<wchar_t>(path);
	}

	YXC_Status YXC_FPathRemoveFileSpecA(char* path)
	{
		return _YXC_FPathRemoveFileSpec<char>(path);
	}

	YXC_Status YXC_FPathRemoveFileSpecW(wchar_t* path)
	{
		return _YXC_FPathRemoveFileSpec<wchar_t>(path);
	}

	YXC_Status YXC_FPathExactA(char* path)
	{
		return _YXC_FPathExact<char>(path);
	}

	ybool_t YXC_FPathExistsA(const char* path)
	{
		yuint32_t uAttr = 0;
		YXC_Status rc = YXC_FPathAttributeA(path, &uAttr);

		return (uAttr & (YXC_FPATH_ATTR_DIR | YXC_FPATH_ATTR_FILE)) != 0;
	}

	YXC_Status YXC_FPathExactW(wchar_t* path)
	{
		return _YXC_FPathExact<wchar_t>(path);
	}

	ybool_t YXC_FPathExistsW(const wchar_t* path)
	{
		yuint32_t uAttr = 0;
		YXC_Status rc = YXC_FPathAttributeW(path, &uAttr);

		return uAttr & (YXC_FPATH_ATTR_DIR | YXC_FPATH_ATTR_FILE);
	}

	YXC_Status YXC_FPathFindRootA(const char* path, char* root)
	{
		return _YXC_FPathFindRoot<char>(path, root);
	}

	YXC_Status YXC_FPathFindRootW(const wchar_t* path, wchar_t* root)
	{
		return _YXC_FPathFindRoot<wchar_t>(path, root);
	}

	YXC_Status YXC_FPathFindModulePathA(char* path, const char* module)
	{
#if YCHAR_WCHAR_T
		YXC_FPath eModule, ePath;
		ychar* module2 = NULL;
		if (module != NULL)
		{
			YXC_TECharToEChar(module, YXC_STR_NTS, eModule, YXC_MAX_CCH_PATH, NULL, NULL);
			module2 = eModule;
		}

		YXC_Status rc = _YXC_FPathFindModulePath(ePath, module2);
		_YXC_CHECK_RC_RET(rc);

		YXC_TEECharToChar(ePath, YXC_STR_NTS, path, YXC_MAX_CCH_PATH, NULL, NULL);
#else
		YXC_Status rc = _YXC_FPathFindModulePath(path, module);
		_YXC_CHECK_RC_RET(rc);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathFindModulePathW(wchar_t* path, const wchar_t* module)
	{
#if YCHAR_WCHAR_T
		YXC_Status rc = _YXC_FPathFindModulePath(path, module);
		_YXC_CHECK_RC_RET(rc);
#else
		YXC_FPath eModule, ePath;
		ychar* module2 = NULL;
		if (module != NULL)
		{
			YXC_TEWCharToEChar(module, YXC_STR_NTS, module2, YXC_MAX_CCH_PATH, NULL, NULL);
			module2 = eModule;
		}

		YXC_Status rc = _YXC_FPathFindModulePath(ePath, module2);
		_YXC_CHECK_RC_RET(rc);

		YXC_TEECharToWChar(ePath, YXC_STR_NTS, path, YXC_MAX_CCH_PATH, NULL, NULL);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathFindSpecDirA(char* path_base, YXC_FPathSpecDir dir)
	{
#if YCHAR_WCHAR_T
		YXC_FPath ePathBase;

		YXC_Status rc = _YXC_FPathFindSpecDir(ePathBase, dir);
		_YXC_CHECK_RC_RET(rc);

		YXC_TEECharToChar(ePathBase, YXC_STR_NTS, path_base, YXC_MAX_CCH_PATH, NULL, NULL);
#else
		YXC_Status rc = _YXC_FPathFindSpecDir(path_base, dir);
		_YXC_CHECK_RC_RET(rc);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathFindSpecDirW(wchar_t* path_base, YXC_FPathSpecDir dir)
	{
#if YCHAR_WCHAR_T
		YXC_Status rc = _YXC_FPathFindSpecDir(path_base, dir);
		_YXC_CHECK_RC_RET(rc);
#else
		YXC_FPath ePathBase;

		YXC_Status rc = _YXC_FPathFindSpecDir(ePathBase, dir);
		_YXC_CHECK_RC_RET(rc);

		YXC_TEECharToWChar(ePathBase, YXC_STR_NTS, path_base, YXC_MAX_CCH_PATH, NULL, NULL);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathCreateDirA(const char* pszPath, YXC_KObjectAttr* attr, ybool_t bCreateAll)
	{
#if YCHAR_WCHAR_T
		YXC_FPath ePath;
		YXC_TECharToEChar(pszPath, YXC_STR_NTS, ePath, YXC_MAX_CCH_PATH, NULL, NULL);

		YXC_Status rc = _YXC_FPathCreateDir(ePath, attr, bCreateAll);
		_YXC_CHECK_RC_RET(rc);
#else
		YXC_Status rc = _YXC_FPathCreateDir(pszPath, attr, bCreateAll);
		_YXC_CHECK_RC_RET(rc);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathCreateDirW(const wchar_t* pszPath, YXC_KObjectAttr* attr, ybool_t bCreateAll)
	{
#if YCHAR_WCHAR_T
		YXC_Status rc = _YXC_FPathCreateDir(pszPath, attr, bCreateAll);
		_YXC_CHECK_RC_RET(rc);
#else
		YXC_FPath ePath;
		YXC_TEWCharToEChar(pszPath, YXC_STR_NTS, ePath, YXC_MAX_CCH_PATH, NULL, NULL);

		YXC_Status rc = _YXC_FPathCreateDir(ePath, attr, bCreateAll);
		_YXC_CHECK_RC_RET(rc);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathDeleteA(const char* pszPath, ybool_t bFileDelete)
	{
#if YCHAR_WCHAR_T
		YXC_FPath ePath;
		YXC_TECharToEChar(pszPath, YXC_STR_NTS, ePath, YXC_MAX_CCH_PATH, NULL, NULL);

		YXC_Status rc = _YXC_FPathDelete(ePath, bFileDelete);
		_YXC_CHECK_RC_RET(rc);
#else
		YXC_Status rc = _YXC_FPathDelete(pszPath, bFileDelete);
		_YXC_CHECK_RC_RET(rc);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}

	YXC_Status YXC_FPathDeleteW(const wchar_t* pszPath, ybool_t bFileDelete)
	{
#if YCHAR_WCHAR_T
		YXC_Status rc = _YXC_FPathDelete(pszPath, bFileDelete);
		_YXC_CHECK_RC_RET(rc);
#else
		YXC_FPath ePath;
		YXC_TEWCharToEChar(pszPath, YXC_STR_NTS, ePath, YXC_MAX_CCH_PATH, NULL, NULL);

		YXC_Status rc = _YXC_FPathDelete(ePath, bFileDelete);
		_YXC_CHECK_RC_RET(rc);
#endif /* YCHAR_WCHAR_T */

		return YXC_ERC_SUCCESS;
	}
};

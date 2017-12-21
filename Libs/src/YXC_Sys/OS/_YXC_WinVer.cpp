#include <YXC_Sys/OS/_YXC_WinVer.hpp>

namespace YXC_Inner
{
	///////////////////////////////////////////////////////////////////////////////
	// ctor
	_MiniVersion::_MiniVersion(LPCTSTR lpszPath)
	{
		ZeroMemory(_szPath, sizeof(_szPath));

		if (lpszPath && lpszPath[0] != 0)
		{
			lstrcpyn(_szPath, lpszPath, sizeof(_szPath)-1);
		}
		else
		{
		}

		_pData = NULL;
		_dwHandle = 0;

		for (int i = 0; i < 4; i++)
		{
			_wFileVersion[i] = 0;
			_wProductVersion[i] = 0;
		}

		_dwFileFlags = 0;
		_dwFileOS = 0;
		_dwFileType = 0;
		_dwFileSubtype = 0;

		ZeroMemory(_szCompanyName, sizeof(_szCompanyName));
		ZeroMemory(_szProductName, sizeof(_szProductName));
		ZeroMemory(_szFileDescription, sizeof(_szFileDescription));

		Init();
	}


	///////////////////////////////////////////////////////////////////////////////
	// Init
	BOOL _MiniVersion::Init()
	{
		DWORD dwHandle;
		DWORD dwSize;
		BOOL rc;

		dwSize = ::GetFileVersionInfoSize(_szPath, &dwHandle);
		if (dwSize == 0)
			return FALSE;

		_pData = new BYTE [dwSize + 1];
		ZeroMemory(_pData, dwSize+1);

		rc = ::GetFileVersionInfoW(_szPath, dwHandle, dwSize, _pData);
		if (!rc)
			return FALSE;

		// get fixed info

		VS_FIXEDFILEINFO FixedInfo;

		if (GetFixedInfo(FixedInfo))
		{
			_wFileVersion[0] = HIWORD(FixedInfo.dwFileVersionMS);
			_wFileVersion[1] = LOWORD(FixedInfo.dwFileVersionMS);
			_wFileVersion[2] = HIWORD(FixedInfo.dwFileVersionLS);
			_wFileVersion[3] = LOWORD(FixedInfo.dwFileVersionLS);

			_wProductVersion[0] = HIWORD(FixedInfo.dwProductVersionMS);
			_wProductVersion[1] = LOWORD(FixedInfo.dwProductVersionMS);
			_wProductVersion[2] = HIWORD(FixedInfo.dwProductVersionLS);
			_wProductVersion[3] = LOWORD(FixedInfo.dwProductVersionLS);

			_dwFileFlags   = FixedInfo.dwFileFlags;
			_dwFileOS      = FixedInfo.dwFileOS;
			_dwFileType    = FixedInfo.dwFileType;
			_dwFileSubtype = FixedInfo.dwFileSubtype;
		}
		else
			return FALSE;

		// get string info

		GetStringInfo(L"CompanyName",     _szCompanyName);
		GetStringInfo(L"FileDescription", _szFileDescription);
		GetStringInfo(L"ProductName",     _szProductName);

		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// Release
	void _MiniVersion::Release()
	{
		// do this manually, because we can't use objects requiring
		// a dtor within an exception handler
		if (_pData)
			delete [] _pData;
		_pData = NULL;
	}


	///////////////////////////////////////////////////////////////////////////////
	// GetFileVersion
	BOOL _MiniVersion::GetFileVersion(WORD * pwVersion)
	{
		for (int i = 0; i < 4; i++)
			*pwVersion++ = _wFileVersion[i];
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetProductVersion
	BOOL _MiniVersion::GetProductVersion(WORD * pwVersion)
	{
		for (int i = 0; i < 4; i++)
			*pwVersion++ = _wProductVersion[i];
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetFileFlags
	BOOL _MiniVersion::GetFileFlags(DWORD& rdwFlags)
	{
		rdwFlags = _dwFileFlags;
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetFileOS
	BOOL _MiniVersion::GetFileOS(DWORD& rdwOS)
	{
		rdwOS = _dwFileOS;
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetFileType
	BOOL _MiniVersion::GetFileType(DWORD& rdwType)
	{
		rdwType = _dwFileType;
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetFileSubtype
	BOOL _MiniVersion::GetFileSubtype(DWORD& rdwType)
	{
		rdwType = _dwFileSubtype;
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetCompanyName
	BOOL _MiniVersion::GetCompanyName(LPTSTR lpszCompanyName, int nSize)
	{
		if (!lpszCompanyName)
			return FALSE;
		ZeroMemory(lpszCompanyName, nSize);
		lstrcpyn(lpszCompanyName, _szCompanyName, nSize-1);
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetFileDescription
	BOOL _MiniVersion::GetFileDescription(LPTSTR lpszFileDescription, int nSize)
	{
		if (!lpszFileDescription)
			return FALSE;
		ZeroMemory(lpszFileDescription, nSize);
		lstrcpyn(lpszFileDescription, _szFileDescription, nSize-1);
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetProductName
	BOOL _MiniVersion::GetProductName(LPTSTR lpszProductName, int nSize)
	{
		if (!lpszProductName)
			return FALSE;
		ZeroMemory(lpszProductName, nSize);
		lstrcpyn(lpszProductName, _szProductName, nSize-1);
		return TRUE;
	}

	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	//
	// protected methods
	//
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////
	// GetFixedInfo
	BOOL _MiniVersion::GetFixedInfo(VS_FIXEDFILEINFO& rFixedInfo)
	{
		BOOL rc;
		UINT nLength;
		VS_FIXEDFILEINFO *pFixedInfo = NULL;

		if (!_pData)
			return FALSE;

		if (_pData)
			rc = ::VerQueryValue(_pData, L"\\", (void **) &pFixedInfo, &nLength);
		else
			rc = FALSE;

		if (rc)
			memcpy (&rFixedInfo, pFixedInfo, sizeof (VS_FIXEDFILEINFO));

		return rc;
	}

	///////////////////////////////////////////////////////////////////////////////
	// GetStringInfo
	BOOL _MiniVersion::GetStringInfo(const wchar_t* lpszKey, wchar_t* lpszReturnValue)
	{
		BOOL rc;
		DWORD *pdwTranslation;
		UINT nLength;
		LPTSTR lpszValue;

		if (_pData == NULL)
			return FALSE;

		if (!lpszReturnValue)
			return FALSE;

		if (!lpszKey)
			return FALSE;

		*lpszReturnValue = 0;

		rc = ::VerQueryValue(_pData, L"\\VarFileInfo\\Translation", (void**) &pdwTranslation, &nLength);
		if (!rc)
			return FALSE;

		wchar_t szKey[2000];
		wsprintfW(szKey, L"\\StringFileInfo\\%04x%04x\\%s", LOWORD(*pdwTranslation), HIWORD(*pdwTranslation), lpszKey);

		rc = ::VerQueryValueW(_pData, szKey, (void**) &lpszValue, &nLength);

		if (!rc)
			return FALSE;

		lstrcpy(lpszReturnValue, lpszValue);

		return TRUE;
	}
	// from winbase.h
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s             0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS      1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT           2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif


/*
    This table has been assembled from Usenet postings, personal
    observations, and reading other people's code.  Please feel
    free to add to it or correct it.

         dwPlatFormID  dwMajorVersion  dwMinorVersion  dwBuildNumber
95             1              4               0             950
95 SP1         1              4               0        >950 && <=1080
95 OSR2        1              4             <10           >1080
98             1              4              10            1998
98 SP1         1              4              10       >1998 && <2183
98 SE          1              4              10          >=2183
ME             1              4              90            3000

NT 3.51        2              3              51
NT 4           2              4               0            1381
2000           2              5               0            2195
XP             2              5               1            2600
2003 Server    2              5               2            3790

CE             3

*/

	///////////////////////////////////////////////////////////////////////////////
	// _GetWinVer
	BOOL _GetWinVer(wchar_t* pszVersion, int *nVersion, wchar_t* pszMajorMinorBuild)
	{
		if (!pszVersion || !nVersion || !pszMajorMinorBuild)
			return FALSE;
		lstrcpy(pszVersion, _YXC_WUNKNOWNSTR);
		*nVersion = _YXC_WUNKNOWN;

		OSVERSIONINFOEXW osinfo;
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

		if (!GetVersionExW((LPOSVERSIONINFOW)&osinfo))
			return FALSE;

		DWORD dwPlatformId   = osinfo.dwPlatformId;
		DWORD dwMinorVersion = osinfo.dwMinorVersion;
		DWORD dwMajorVersion = osinfo.dwMajorVersion;
		DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;	// Win 95 needs this

		wsprintfW(pszMajorMinorBuild, L"%u.%u.%u", dwMajorVersion, dwMinorVersion, dwBuildNumber);

		if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
		{
			if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
			{
				lstrcpyW(pszVersion, _YXC_W95STR);
				*nVersion = _YXC_W95;
			}
			else if ((dwMinorVersion < 10) &&
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
			{
				lstrcpyW(pszVersion, _YXC_W95SP1STR);
				*nVersion = _YXC_W95SP1;
			}
			else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
			{
				lstrcpyW(pszVersion, _YXC_W95OSR2STR);
				*nVersion = _YXC_W95OSR2;
			}
			else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
			{
				lstrcpyW(pszVersion, _YXC_W98STR);
				*nVersion = _YXC_W98;
			}
			else if ((dwMinorVersion == 10) &&
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
			{
				lstrcpyW(pszVersion, _YXC_W98SP1STR);
				*nVersion = _YXC_W98SP1;
			}
			else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
			{
				lstrcpyW(pszVersion, _YXC_W98SESTR);
				*nVersion = _YXC_W98SE;
			}
			else if (dwMinorVersion == 90)
			{
				lstrcpyW(pszVersion, _YXC_WMESTR);
				*nVersion = _YXC_WME;
			}
		}
		else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
			{
				lstrcpy(pszVersion, _YXC_WNT351STR);
				*nVersion = _YXC_WNT351;
			}
			else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
			{
				lstrcpy(pszVersion, _YXC_WNT4STR);
				*nVersion = _YXC_WNT4;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
			{
				lstrcpy(pszVersion, _YXC_W2KSTR);
				*nVersion = _YXC_W2K;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
			{
				lstrcpy(pszVersion, _YXC_WXPSTR);
				*nVersion = _YXC_WXP;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
			{
				lstrcpy(pszVersion, _YXC_W2003SERVERSTR);
				*nVersion = _YXC_W2003SERVER;
			}
			else if ((dwMajorVersion == 6) && (dwMinorVersion == 0))
			{
				lstrcpy(pszVersion, _YXC_WVISTASTR);
				*nVersion = _YXC_WVISTA;
			}
			else if ((dwMajorVersion == 6) && (dwMinorVersion == 1))
			{
				if (osinfo.wProductType == VER_NT_WORKSTATION)
				{
					lstrcpy(pszVersion, _YXC_WWIN7STR);
					*nVersion = _YXC_WWIN7;
				}
				else
				{
					lstrcpy(pszVersion, _YXC_W2008SERVERSTR);
					*nVersion = _YXC_W2008SERVER;
				}
			}
		}
		else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
		{
			lstrcpy(pszVersion, _YXC_WCESTR);
			*nVersion = _YXC_WCE;
		}
		return TRUE;
	}
}

#ifndef __INNER_INC_YXC_SYS_BASE_WINDOWS_VERSION_HPP__
#define __INNER_INC_YXC_SYS_BASE_WINDOWS_VERSION_HPP__

#include <windows.h>

#define _YXC_WUNKNOWNSTR		L"unknown Windows version"

#define _YXC_W95STR			L"Windows 95"
#define _YXC_W95SP1STR		L"Windows 95 SP1"
#define _YXC_W95OSR2STR		L"Windows 95 OSR2"
#define _YXC_W98STR			L"Windows 98"
#define _YXC_W98SP1STR		L"Windows 98 SP1"
#define _YXC_W98SESTR		L"Windows 98 SE"
#define _YXC_WMESTR			L"Windows ME"

#define _YXC_WNT351STR		L"Windows NT 3.51"
#define _YXC_WNT4STR			L"Windows NT 4"
#define _YXC_W2KSTR			L"Windows 2000"
#define _YXC_WXPSTR			L"Windows XP"
#define _YXC_W2003SERVERSTR	L"Windows 2003 Server"
#define _YXC_WVISTASTR	L"Windows Vista"
#define _YXC_WWIN7STR	L"Windows 7"
#define _YXC_W2008SERVERSTR	L"Windows 2008 Server"

#define _YXC_WCESTR			L"Windows CE"

#define _YXC_WUNKNOWN	0

#define _YXC_W9XFIRST	1
#define _YXC_W95			1
#define _YXC_W95SP1		2
#define _YXC_W95OSR2		3
#define _YXC_W98			4
#define _YXC_W98SP1		5
#define _YXC_W98SE		6
#define _YXC_WME			7
#define _YXC_W9XLAST		99

#define _YXC_WNTFIRST	101
#define _YXC_WNT351		101
#define _YXC_WNT4		102
#define _YXC_W2K			103
#define _YXC_WXP			104
#define _YXC_W2003SERVER	105
#define _YXC_WVISTA		106
#define _YXC_WWIN7		107
#define _YXC_W2008SERVER	108
#define _YXC_WNTLAST		199

#define _YXC_WCEFIRST	201
#define _YXC_WCE			201
#define _YXC_WCELAST		299

namespace YXC_Inner
{
	BOOL _GetWinVer(wchar_t* pszVersion, int *nVersion, wchar_t* pszMajorMinorBuild);

	class _MiniVersion
	{
		// constructors
	public:
		_MiniVersion(const wchar_t* lpszPath = NULL);

		BOOL Init();
		void Release();

		// operations
	public:

		// attributes
	public:
		// fixed info
		BOOL GetFileVersion(WORD *pwVersion);
		BOOL GetProductVersion(WORD* pwVersion);
		BOOL GetFileFlags(DWORD& rdwFlags);
		BOOL GetFileOS(DWORD& rdwOS);
		BOOL GetFileType(DWORD& rdwType);
		BOOL GetFileSubtype(DWORD& rdwType);

		// string info
		BOOL GetCompanyName(LPTSTR lpszCompanyName, int nSize);
		BOOL GetFileDescription(LPTSTR lpszFileDescription, int nSize);
		BOOL GetProductName(LPTSTR lpszProductName, int nSize);

		// implementation
	protected:
		BOOL GetFixedInfo(VS_FIXEDFILEINFO& rFixedInfo);
		BOOL GetStringInfo(const wchar_t* lpszKey, wchar_t* lpszValue);

		BYTE* _pData;
		DWORD _dwHandle;
		WORD  _wFileVersion[4];
		WORD  _wProductVersion[4];
		DWORD _dwFileFlags;
		DWORD _dwFileOS;
		DWORD _dwFileType;
		DWORD _dwFileSubtype;

		TCHAR _szPath[MAX_PATH*2];
		TCHAR _szCompanyName[MAX_PATH*2];
		TCHAR _szProductName[MAX_PATH*2];
		TCHAR _szFileDescription[MAX_PATH*2];
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_WINDOWS_VERSION_HPP__ */

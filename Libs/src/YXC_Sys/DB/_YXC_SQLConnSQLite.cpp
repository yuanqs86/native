#define __MODULE__ "EK.DB.SQLITE"

#include <stdio.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnSQLite.hpp>
#include <YXC_Sys/DB/_YXC_SQLStatSQLite.hpp>

namespace sqlite
{
	void _InitSQLite();
}

namespace YXC_Inner
{
	_SQLConnSQLite::_SQLConnSQLite()
	{
		if (sqlite::_sqlite3_open == NULL)
		{
			sqlite::_InitSQLite();
		}

	}

	_SQLConnSQLite::~_SQLConnSQLite()
	{

	}

	YXC_Status _SQLConnSQLite::Init(ybool_t bAutoCommit)
	{
		this->_bAutoCommit = bAutoCommit;
		this->_dbName = NULL;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLConnSQLite::ConnectToDBServer(const wchar_t* cpszTarget, yuint32_t uTargetPort, const wchar_t* cpszDriver,
		const wchar_t* cpszInitDB, const wchar_t* cpszUser, const wchar_t* cpszPwd)
	{
		ysize_t len = wcslen(cpszTarget);
		_YCHK_MAL_STRW_R2(this->_dbName, len);
		memcpy(this->_dbName, cpszTarget, (len + 1) * sizeof(wchar_t));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLConnSQLite::AllocStatement(_SQLStatBase** ppStatement)
	{
		YXC_Status rc = _SQLStatSQLite::CreateStatement(this->_dbName, this->_bAutoCommit, (_SQLStatSQLite**)ppStatement); //_SQLStatMSSQL*)malloc(sizeof(_SQLStatMSSQL));
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void _SQLConnSQLite::FreeStatement(_SQLStatBase* pStat)
	{
		_SQLStatSQLite::FreeStatement((_SQLStatSQLite*)pStat);
	}

	void _SQLConnSQLite::Free()
	{
		if (this->_dbName != NULL)
		{
			free(this->_dbName);
		}
	}
}

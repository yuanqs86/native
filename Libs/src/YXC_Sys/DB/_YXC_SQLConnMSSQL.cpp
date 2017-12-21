#define __MODULE__ "EK.DB.MSSQL"

#include <stdio.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnMSSQL.hpp>
#include <YXC_Sys/DB/_YXC_SQLStatMSSQL.hpp>

namespace YXC_Inner
{
	_SQLConnMSSQL::_SQLConnMSSQL()
	{

	}

	_SQLConnMSSQL::~_SQLConnMSSQL()
	{

	}

	YXC_Status _SQLConnMSSQL::Init(ybool_t bAutoCommit)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		SQLHENV hEnv = SQL_NULL_HANDLE;
		SQLHDBC hDbc = SQL_NULL_HANDLE;

		SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
		_YXC_CHECK_MSSQL_GOTO(retcode, SQL_HANDLE_ENV, hEnv, L"Alloc SQL environment failed");

		retcode = SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
		_YXC_CHECK_MSSQL_GOTO(retcode, SQL_HANDLE_ENV, hEnv, L"Set ODBC version to SQL_OV_ODBC3 failed");

		retcode = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
		_YXC_CHECK_MSSQL_GOTO(retcode, SQL_HANDLE_ENV, hEnv, L"Alloc SQL connection handle failed");

		SQLPOINTER nCommitType = (SQLPOINTER)(bAutoCommit ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF);
		retcode = SQLSetConnectAttrW(hDbc, SQL_AUTOCOMMIT, (SQLPOINTER)nCommitType, 0);
		_YXC_CHECK_MSSQL_GOTO(retcode, SQL_HANDLE_DBC, hDbc, L"Set SQL connection attribute failed");

		retcode = SQLSetConnectAttr(hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)4, 0);
		_YXC_CHECK_MSSQL_GOTO(retcode, SQL_HANDLE_DBC, hDbc, L"Set SQL connection timeout failed");

		this->_hEnv = hEnv;
		this->_hDbc = hDbc;

		return YXC_ERC_SUCCESS;
err_ret:
		if (hDbc) SQLFreeConnect(hDbc);
		if (hEnv) SQLFreeEnv(hEnv);

		return rcRet;
	}

	YXC_Status _SQLConnMSSQL::ConnectToDBServer(const wchar_t* cpszTarget, yuint32_t uTargetPort, const wchar_t* cpszDriver,
		const wchar_t* cpszInitDB, const wchar_t* cpszUser, const wchar_t* cpszPwd)
	{
		wchar_t szConn[MAX_PATH];
		if (cpszUser == NULL)
		{
			swprintf_s(szConn, L"DRIVER=%s;SERVER=%s;DATABASE=%s;Trusted_Connection=TRUE", cpszDriver, cpszTarget, cpszInitDB);
		}
		else
		{
			swprintf_s(szConn, L"DRIVER=%s;SERVER=%s;DATABASE=%s;UID=%s;PWD=%s", cpszDriver, cpszTarget, cpszInitDB, cpszUser, cpszPwd);
		}
		wchar_t szConnOut[256];
		SQLSMALLINT iConnOut = 256;

		SQLRETURN retcode = ::SQLDriverConnectW(this->_hDbc, NULL, szConn, SQL_NTS, szConnOut, iConnOut,
			&iConnOut, SQL_DRIVER_NOPROMPT);

		//if (retcode == 1) /* MSSQL memory leak bug, get info out here */
		//{
		//	_YXC_MSSQL_REPORT(retcode, SQL_HANDLE_DBC, this->_hDbc, L"Connect with info");
		//	YXC_ClearErrors();
		//}

		_YXC_CHECK_MSSQL_RET(retcode, SQL_HANDLE_DBC, this->_hDbc, L"Connect to SQL server failed");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLConnMSSQL::AllocStatement(_SQLStatBase** ppStatement)
	{
		SQLHSTMT stmt = SQL_NULL_HSTMT;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		SQLRETURN retcode = ::SQLAllocStmt(this->_hDbc, &stmt);

		_YXC_CHECK_MSSQL_RET(retcode, SQL_HANDLE_DBC, this->_hDbc, L"Alloc for SQL statement failed");

		YXC_Status rc = _SQLStatMSSQL::CreateStatement(this->_hDbc, stmt, (_SQLStatMSSQL**)ppStatement); //_SQLStatMSSQL*)malloc(sizeof(_SQLStatMSSQL));
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		return YXC_ERC_SUCCESS;
err_ret:
		::SQLFreeHandle(SQL_HANDLE_STMT, stmt);
		return rcRet;
	}

	void _SQLConnMSSQL::FreeStatement(_SQLStatBase* pStat)
	{
		_SQLStatMSSQL::FreeStatement((_SQLStatMSSQL*)pStat);
	}

	void _SQLConnMSSQL::Free()
	{
		::SQLDisconnect(this->_hDbc);
		::SQLFreeHandle(SQL_HANDLE_DBC, this->_hDbc);
		::SQLFreeHandle(SQL_HANDLE_ENV, this->_hEnv);
	}
}

#define __MODULE__ "EK.DB.MYSQL"

#include <stdio.h>

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnMySQL.hpp>
#include <YXC_Sys/DB/_YXC_SQLStatMySQL.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>

namespace mysql
{
	void _InitMySQL();
}

namespace YXC_Inner
{
	_SQLConnMySQL::_SQLConnMySQL()
	{
		if (mysql::_mysql_init == NULL)
		{
			mysql::_InitMySQL();
		}
	}

	_SQLConnMySQL::~_SQLConnMySQL()
	{

	}

	YXC_Status _SQLConnMySQL::Init(ybool_t bAutoCommit)
	{
		this->_mysql = mysql::_mysql_init(NULL);
		_YXC_CHECK_REPORT_RET_EX(this->_mysql != NULL, YXC_ERROR_SRC_SQL, YXC_ERR_CAT_SQL_MYSQL, YXC_ERC_SQL, YXC_ERC_SQL,
			YC("_mysql_init"));

		this->_bAutoCommit = bAutoCommit;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLConnMySQL::ConnectToDBServer(const wchar_t* cpszTarget, yuint32_t uTargetPort, const wchar_t* cpszDriver,
		const wchar_t* cpszInitDB, const wchar_t* cpszUser, const wchar_t* cpszPwd)
	{
		yuint32_t uConverted;
		YXC_FPathA target, usr, pwd, db;
		const char* pcTarget = NULL, *pcUsr = NULL, *pcPwd = NULL, *pcDb = NULL;

		if (cpszTarget)
		{
			YXC_TEWCharToUTF8(cpszTarget, YXC_STR_NTS, (yuint8_t*)target, YXC_MAX_CCH_PATH, &uConverted, NULL);
			pcTarget = target;
		}

		if (cpszUser)
		{
			YXC_TEWCharToUTF8(cpszUser, YXC_STR_NTS, (yuint8_t*)usr, YXC_MAX_CCH_PATH, &uConverted, NULL);
			pcUsr = usr;
		}

		if (cpszPwd)
		{
			YXC_TEWCharToUTF8(cpszPwd, YXC_STR_NTS, (yuint8_t*)pwd, YXC_MAX_CCH_PATH, &uConverted, NULL);
			pcPwd = pwd;
		}

		if (cpszInitDB)
		{
			YXC_TEWCharToUTF8(cpszInitDB, YXC_STR_NTS, (yuint8_t*)db, YXC_MAX_CCH_PATH, &uConverted, NULL);
			pcDb = db;
		}

		MYSQL* sql = mysql::_mysql_real_connect(this->_mysql, pcTarget, pcUsr, pcPwd, pcDb, 0, NULL, 0);
		_YXC_CHECK_MYSQL_RETB(sql != NULL, _YXC_MYSQL_HDLTYPE_HDBC, this->_mysql, YC("_mysql_real_connect"));

		int x_commit = mysql::_mysql_autocommit(this->_mysql, this->_bAutoCommit);
		_YXC_CHECK_MYSQL_RET(x_commit, _YXC_MYSQL_HDLTYPE_HDBC, this->_mysql, YC("_mysql_autocommit"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLConnMySQL::AllocStatement(_SQLStatBase** ppStatement)
	{
		MYSQL_STMT* stmt = mysql::_mysql_stmt_init(this->_mysql);
		_YXC_CHECK_MYSQL_RETB(stmt != NULL, _YXC_MYSQL_HDLTYPE_HDBC, this->_mysql, YC("_mysql_stmt_init"));

		YXC_Status rc = _SQLStatMySQL::CreateStatement(this->_mysql, stmt, (_SQLStatMySQL**)ppStatement); //_SQLStatMSSQL*)malloc(sizeof(_SQLStatMSSQL));
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	void _SQLConnMySQL::FreeStatement(_SQLStatBase* pStat)
	{
		_SQLStatMySQL::FreeStatement((_SQLStatMySQL*)pStat);
	}

	void _SQLConnMySQL::Free()
	{
		if (this->_mysql != NULL)
		{
			mysql::_mysql_close(this->_mysql);
		}
	}
}

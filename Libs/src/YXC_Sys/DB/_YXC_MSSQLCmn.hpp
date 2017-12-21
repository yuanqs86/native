#ifndef __INNER_INC_YXC_SYS_BASE_MSSQL_COMMON_HPP
#define __INNER_INC_YXC_SYS_BASE_MSSQL_COMMON_HPP

#include <sqlext.h>
#include <YXC_Sys/YXC_Database.h>

#define _YXC_MSSQL_MAX_DIAG_SIZE 512

namespace YXC_Inner
{
	void _ReportMSSQLError(SQLRETURN code, const wchar_t* cpszModule, const wchar_t* cpszFile, int iLine, const wchar_t* cpszFunc,
		const wchar_t* cpszMsg, SQLSMALLINT fHandleType, SQLHANDLE handle);

	YXC_Status _GetSqlTypeAndCType(ybyte_t byDataType, SQLSMALLINT& rSQLType, SQLSMALLINT& rCType, SQLULEN& uColDef);

	YXC_Status _GetDBTypeBySqlType(SQLSMALLINT sqlType, ybyte_t& rDataType);

	YXC_Status _GetSqlDefBufSize(ybyte_t byDataType, yuint32_t& uDefBufSize);
}

#define _YXC_MSSQL_REPORT(rc, fType, hdl, wszMsg, ...) YXC_Inner::_ReportMSSQLError(rc, __EMODULE__, __EFILE__, __LINE__, \
	__EFUNCTION__, wszMsg, fType, hdl)

#define _YXC_CHECK_MSSQL_RET(rc, fType, hdl, wszMsg, ...)				\
	do {																\
		SQLRETURN code = (rc);											\
		if (code != SQL_SUCCESS && code != SQL_SUCCESS_WITH_INFO) {		\
			_YXC_MSSQL_REPORT(rc, fType, hdl, wszMsg, ##__VA_ARGS__);		\
			return YXC_ERC_SQL;											\
		}																\
	} while (0)

#define _YXC_CHECK_MSSQL_GOTO(rc, fType, hdl,wszMsg,  ...)				\
	do {																\
		SQLRETURN code = (rc);											\
		if (code != SQL_SUCCESS && code != SQL_SUCCESS_WITH_INFO) {		\
			_YXC_MSSQL_REPORT(rc, fType, hdl, wszMsg, ##__VA_ARGS__);		\
			__VARIABLE_RC__ = YXC_ERC_SQL; goto __LABEL__;				\
		}																\
	} while (0)

#endif /* __INNER_INC_YXC_SYS_BASE_MSSQL_COMMON_HPP */

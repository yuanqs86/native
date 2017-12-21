#define __MODULE__ "EK.DB.MSSQL"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/DB/_YXC_MSSQLCmn.hpp>

namespace
{
	struct MSSQLColTypeDesc
	{
		SQLSMALLINT cType;
		SQLSMALLINT sqlType;
		ybool_t bFixedSizeType;
		yuint32_t uDefBufSize;
	};

	static MSSQLColTypeDesc gs_colHeaderDescs[] = {
		{ SQL_C_STINYINT, SQL_TINYINT, TRUE, sizeof(yint8_t) },
		{ SQL_C_SSHORT, SQL_SMALLINT, TRUE, sizeof(yint16_t) },
		{ SQL_C_SLONG, SQL_INTEGER, TRUE, sizeof(yint32_t) },
		{ SQL_C_SBIGINT, SQL_BIGINT, TRUE, sizeof(yint64_t) },
		{ SQL_C_UTINYINT, SQL_TINYINT, TRUE, sizeof(yuint8_t) },
		{ SQL_C_USHORT, SQL_SMALLINT, TRUE, sizeof(yuint16_t) },
		{ SQL_C_ULONG, SQL_INTEGER, TRUE, sizeof(yuint32_t) },
		{ SQL_C_UBIGINT, SQL_BIGINT, TRUE, sizeof(yuint64_t) },
		{ SQL_C_BIT, SQL_BIT, TRUE, sizeof(ybool_t) },
		{ SQL_C_FLOAT, SQL_REAL, TRUE, sizeof(float) },
		{ SQL_C_DOUBLE, SQL_FLOAT, TRUE, sizeof(double) },
		{ SQL_C_CHAR, SQL_CHAR, TRUE, 256 * sizeof(char) },
		{ SQL_C_CHAR, SQL_VARCHAR, FALSE, 256 * sizeof(char) },
		{ SQL_C_CHAR, SQL_LONGVARCHAR, FALSE, 4096 * sizeof(char),  },
		{ SQL_C_WCHAR, SQL_WCHAR, TRUE, 256 * sizeof(wchar_t) },
		{ SQL_C_WCHAR, SQL_WVARCHAR, FALSE, 256 * sizeof(wchar_t) },
		{ SQL_C_WCHAR, SQL_WLONGVARCHAR, FALSE, 4096 * sizeof(wchar_t) },
		{ SQL_C_BINARY, SQL_BINARY, TRUE, 1024 },
		{ SQL_C_BINARY, SQL_VARBINARY, FALSE, 1024, },
		{ SQL_C_BINARY, SQL_LONGVARBINARY, FALSE, 32768 },
		{ SQL_C_TYPE_DATE, SQL_TYPE_DATE, TRUE, sizeof(SQL_DATE_STRUCT) },
		{ SQL_C_TYPE_TIME, SQL_TYPE_TIME, TRUE, sizeof(SQL_TIME_STRUCT) },
		{ SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, TRUE, sizeof(SQL_TIMESTAMP_STRUCT) },
		{ SQL_C_BINARY, SQL_DATETIME, TRUE, sizeof(SQL_DATE_STRUCT) + sizeof(SQL_TIME_STRUCT) },
	};
}

namespace YXC_Inner
{
	void _ReportMSSQLError(SQLRETURN code, const wchar_t* cpszModule, const wchar_t* cpszFile, int iLine, const wchar_t* cpszFunc,
		const wchar_t* cpszMsg, SQLSMALLINT fHandleType, SQLHANDLE handle)
	{
		SQLWCHAR szSQLState[20] = {0};
		SQLINTEGER  nNativeError;
		SQLSMALLINT nMsgLen;
		SQLWCHAR szMessage[SQL_MAX_MESSAGE_LENGTH] = {0};
		wchar_t szMessage2[YXC_BASE_ERROR_BUFFER] = {0};
		SQLSMALLINT nRecNumber = 1;
		wchar_t* pszMsg = szMessage2;
		wchar_t* pszEnd = pszMsg + YXC_BASE_ERROR_BUFFER;

		wchar_t szStatDesc[] = L"SQL State : ";
		wchar_t szDiagDesc[] = L", Diag : ";

		YXCLib::_AppendStringChkW(pszMsg, pszEnd, cpszMsg, YXC_STR_NTS);
		BOOL bFirstLine = FALSE;

		while (TRUE)
		{
			SQLRETURN retcode = SQLGetDiagRecW(fHandleType, handle, nRecNumber++, szSQLState,
				&nNativeError, szMessage, sizeof(szMessage), &nMsgLen);

			if (retcode == SQL_NO_DATA) break;

			if (!YXCLib::_AppendStringChkW(pszMsg, pszEnd, L"\n", YXC_STRING_ARR_LEN(L"\n")))
			{
				break;
			}

			if (!YXCLib::_AppendStringChkW(pszMsg, pszEnd, szStatDesc, YXC_STRING_ARR_LEN(szStatDesc))) break;
			if (!YXCLib::_AppendStringChkW(pszMsg, pszEnd, szSQLState, YXC_STR_NTS)) break;
			if (!YXCLib::_AppendStringChkW(pszMsg, pszEnd, szDiagDesc, YXC_STRING_ARR_LEN(szDiagDesc))) break;
			if (!YXCLib::_AppendStringChkW(pszMsg, pszEnd, szMessage, nMsgLen)) break;
		}

		YXC_DBConnType connType = YXC_DB_CONNECTION_TYPE_MSSQL;
		YXC_ReportErrorExFormat(TRUE, YXC_ERROR_SRC_SQL, YXC_ERR_CAT_SQL_ODBC, code, cpszModule, cpszFile, iLine, cpszFunc,
			sizeof(YXC_DBConnType), &connType, L"%s", szMessage2);
	}

	YXC_Status _GetSqlTypeAndCType(ybyte_t byDataType, SQLSMALLINT& rSQLType, SQLSMALLINT& rCType, SQLULEN& uColDef)
	{
		_YXC_CHECK_REPORT_NEW_RET(byDataType <= YXC_DB_DATA_TYPE_MAX, YXC_ERC_INVALID_PARAMETER, YC("Invalid data type to switch"));

		MSSQLColTypeDesc& rDesc = gs_colHeaderDescs[byDataType];

		rSQLType = rDesc.sqlType;
		rCType = rDesc.cType;

		if (rDesc.bFixedSizeType)
		{
			uColDef = -1;
		}
		else
		{
			uColDef = rDesc.uDefBufSize;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _GetDBTypeBySqlType(SQLSMALLINT sqlType, ybyte_t& rDataType)
	{
		switch (sqlType)
		{
		case SQL_TINYINT:
			rDataType = YXC_DB_DATA_TYPE_U8;
			break;
		case SQL_SMALLINT:
			rDataType = YXC_DB_DATA_TYPE_U16;
			break;
		case SQL_INTEGER:
			rDataType = YXC_DB_DATA_TYPE_I32;
			break;
		case SQL_BIGINT:
			rDataType = YXC_DB_DATA_TYPE_I64;
			break;
		case SQL_REAL:
			rDataType = YXC_DB_DATA_TYPE_F32;
			break;
		case SQL_FLOAT:
			rDataType = YXC_DB_DATA_TYPE_F64;
			break;
		case SQL_CHAR:
			rDataType = YXC_DB_DATA_TYPE_ASTRING;
			break;
		case SQL_VARCHAR:
		case SQL_LONGVARCHAR:
			rDataType = YXC_DB_DATA_TYPE_VASTRING;
			break;
		case SQL_WCHAR:
			rDataType = YXC_DB_DATA_TYPE_WSTRING;
			break;
		case SQL_WVARCHAR:
		case SQL_WLONGVARCHAR:
			rDataType = YXC_DB_DATA_TYPE_VWSTRING;
			break;
		case SQL_BINARY:
			rDataType = YXC_DB_DATA_TYPE_BINARY;
			break;
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
			rDataType = YXC_DB_DATA_TYPE_VBINARY;
			break;
		case SQL_TYPE_DATE:
			rDataType = YXC_DB_DATA_TYPE_DATE;
			break;
		case SQL_TYPE_TIME:
			rDataType = YXC_DB_DATA_TYPE_TIME;
			break;
		case SQL_TYPE_TIMESTAMP:
			rDataType = YXC_DB_DATA_TYPE_TIMESTAMP;
			break;
		case SQL_DATETIME:
			rDataType = YXC_DB_DATA_TYPE_DATETIME;
			break;
		default:
			_YXC_REPORT_NEW_ERR(YXC_ERC_INVALID_PARAMETER, YC("Invalid Sql type to convert %d"), sqlType);
			break;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _GetSqlDefBufSize(ybyte_t byDataType, yuint32_t& uDefBufSize)
	{
		_YXC_CHECK_REPORT_NEW_RET(byDataType <= YXC_DB_DATA_TYPE_MAX, YXC_ERC_INVALID_PARAMETER, YC("Invalid data type to switch"));

		MSSQLColTypeDesc& rDesc = gs_colHeaderDescs[byDataType];
		uDefBufSize = rDesc.uDefBufSize;

		return YXC_ERC_SUCCESS;
	}
}

#define __MODULE__ "EK.DB.MSSQL"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnMSSQL.hpp>
#include <YXC_Sys/DB/_YXC_SQLStatMSSQL.hpp>
#include <YXC_Sys/DB/_YXC_MSSQLCmn.hpp>

#include <new>

namespace YXC_Inner
{
	_SQLStatMSSQL::_SQLStatMSSQL()
	{

	}

	_SQLStatMSSQL::~_SQLStatMSSQL()
	{

	}

	YXC_Status _SQLStatMSSQL::CreateStatement(SQLHDBC dbc, SQLHSTMT hStmt, _SQLStatMSSQL** ppStat)
	{
		SQLRETURN ret = SQLSetStmtAttrW(hStmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC,
			SQL_IS_INTEGER);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, hStmt, L"Set cursor type of statement failed");

		_SQLStatMSSQL* pStat = (_SQLStatMSSQL*)::malloc(sizeof(_SQLStatMSSQL));
		_YXC_CHECK_REPORT_NEW_RET(pStat != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc for mssql stat failed");

		new (pStat) _SQLStatMSSQL();
		pStat->_stmt = hStmt;
		pStat->_dbc = dbc;
		pStat->_pBindInfos = NULL;
		pStat->_uNumBindCols = 0;
		pStat->_uNumBindColsBuf = 0;
		pStat->_pCellsPtr = NULL;

		*ppStat = pStat;
		return YXC_ERC_SUCCESS;
	}

	void _SQLStatMSSQL::FreeStatement(_SQLStatMSSQL* pStat)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, pStat->_stmt);

		pStat->_FreeColBindInfos(pStat->_uNumBindCols - 1);
		if (pStat->_pBindInfos)
		{
			free(pStat->_pBindInfos);
		}

		pStat->~_SQLStatMSSQL();
		free(pStat);
	}

	YXC_Status _SQLStatMSSQL::Prepare(const wchar_t* pszStmt)
	{
		SQLRETURN ret = ::SQLPrepareW(this->_stmt, (wchar_t*)pszStmt, SQL_NTS);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Prepare for sql statement failed");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::ExecuteDirect(const wchar_t* pszStmt, ysize_t* pstAffectedRows)
	{
		SQLRETURN ret = ::SQLExecDirectW(this->_stmt, (wchar_t*)pszStmt, SQL_NTS);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Execute sql statement directly failed");

		this->_GetAffectedRowNum(pstAffectedRows);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::Reset()
	{
		SQLRETURN ret = ::SQLFreeStmt(this->_stmt, SQL_RESET_PARAMS);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Reset params for sql statement failed");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::CloseCursor()
	{
		SQLRETURN ret = ::SQLFreeStmt(this->_stmt, SQL_CLOSE);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Close cursor for sql statement failed");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::Execute(ysize_t* pstAffectedRows)
	{
		SQLRETURN ret = ::SQLExecute(this->_stmt);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Execute sql statement failed");

		this->_GetAffectedRowNum(pstAffectedRows);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::MoreResults(ysize_t* pstAffectedRows)
	{
		SQLRETURN ret = ::SQLMoreResults(this->_stmt);
		if (ret == SQL_NO_DATA) return YXC_ERC_NO_DATA;
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Fetch more results failed");

		this->_GetAffectedRowNum(pstAffectedRows);
		return YXC_ERC_SUCCESS;
	}

	void _SQLStatMSSQL::_GetAffectedRowNum(ysize_t* pstAffectedRows)
	{
		if (pstAffectedRows != NULL)
		{
			SQLLEN affectCount = 0;
			SQLRETURN ret2 = ::SQLRowCount(this->_stmt, &affectCount);
			if (ret2 == SQL_SUCCESS)
			{
				*pstAffectedRows = affectCount;
			}
		}
	}

	YXC_Status _SQLStatMSSQL::_BindParameter(SQLSMALLINT uParamType, ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData)
	{
		SQLSMALLINT sqlType, cType;

		SQLULEN uBufSize = *pstDataLen;
		if (uBufSize == YXC_DB_NULL_DATA)
		{
			*pstDataLen = SQL_NULL_DATA;
			uBufSize = 0;
		}

		SQLULEN iColDef = uBufSize;
		YXC_Status rc = _GetSqlTypeAndCType(byDataType, sqlType, cType, iColDef);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		SQLSMALLINT sqlType2, scale, nullable;
		SQLULEN paramDesc;
		SQLRETURN ret = ::SQLDescribeParam(this->_stmt, uColIndex + 1, &sqlType2, &paramDesc, &scale, &nullable);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Describe sql parameter(%d) failed", uColIndex);

		ret = ::SQLBindParameter(this->_stmt, uColIndex + 1, uParamType, cType, sqlType,
			paramDesc, 0, (SQLPOINTER)pData, uBufSize, (SQLLEN*)pstDataLen);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Bind sql parameter(%d) failed", uColIndex);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::BindInputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, const void* pData)
	{
		return _BindParameter(SQL_PARAM_INPUT, byDataType, uColIndex, pstDataLen, (void*)pData);
	}

	YXC_Status _SQLStatMSSQL::BindOutputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData)
	{
		return _BindParameter(SQL_PARAM_INPUT_OUTPUT, byDataType, uColIndex, pstDataLen, pData);
	}

	YXC_Status _SQLStatMSSQL::QueryResultInfo(yuint32_t* puNumHeaders, YXC_DBRTColHeader* pHeaders)
	{
		SQLSMALLINT colType, colNameLen;
		SQLULEN colDef;
		SQLSMALLINT scale, nullable;
		SQLSMALLINT resultCount;

		SQLRETURN ret = ::SQLNumResultCols(this->_stmt, &resultCount);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Get result cols number failed");

		if (*puNumHeaders >= resultCount)
		{
			*puNumHeaders = resultCount;
			for (SQLSMALLINT i = 0; i < resultCount; ++i)
			{
				SQLRETURN ret = ::SQLDescribeColW(this->_stmt, i + 1, pHeaders[i].szColName, YXC_DB_MAX_COL_NAME,
					&colNameLen, &colType, &colDef, &scale, &nullable);
				_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Describe result col failed");

				YXC_Status rc = _GetDBTypeBySqlType(colType, pHeaders[i].byDataType);
				_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

				pHeaders[i].bCanBeNull = nullable == SQL_NULLABLE;
				pHeaders[i].uDataMaxLen = colDef;
			}

			return YXC_ERC_SUCCESS;
		}
		else
		{
			*puNumHeaders = resultCount;
			return YXC_ERC_BUFFER_NOT_ENOUGH;
		}
	}

	YXC_Status _SQLStatMSSQL::BindResultCols(yuint32_t uNumCols, const YXC_DBRTColBindInfo* pBindInfos,
		const YXC_DBRTCell** ppCellsPtr)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		yuint32_t uIndex = 0;
		this->_FreeColBindInfos(this->_uNumBindCols - 1);
		if (this->_uNumBindColsBuf < uNumCols)
		{
			free(this->_pBindInfos);
			this->_pBindInfos = (_MSSQLColBindInfo*)::calloc(uNumCols, sizeof(_MSSQLColBindInfo) + sizeof(YXC_DBRTCell));
			this->_pCellsPtr = (YXC_DBRTCell*)(this->_pBindInfos + uNumCols);
			_YXC_CHECK_REPORT_NEW_GOTO(this->_pBindInfos != NULL, YXC_ERC_OUT_OF_MEMORY,
				L"Alloc new column bind infos failed");

			this->_uNumBindColsBuf = uNumCols;
		}
		this->_uNumBindCols = uNumCols;

		for (; uIndex < uNumCols; ++uIndex)
		{
			_MSSQLColBindInfo& rDstInfo = this->_pBindInfos[uIndex];
			const YXC_DBRTColBindInfo& rSrcInfo = pBindInfos[uIndex];

			YXC_Status rc = this->_CopyColBindInfo(rDstInfo, rSrcInfo, uIndex);
			_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Copy column bind info failed, index = %u", uIndex);
		}

		*ppCellsPtr = this->_pCellsPtr;
		return YXC_ERC_SUCCESS;
err_ret:
		this->_FreeColBindInfos(uIndex);
		this->_uNumBindCols = 0;
		return rcRet;
	}

	YXC_Status _SQLStatMSSQL::RebindResultCol(yuint32_t uColIndex, const YXC_DBRTColBindInfo* pBindInfo)
	{
		_YXC_CHECK_REPORT_NEW_RET(uColIndex < this->_uNumBindCols, YXC_ERC_INDEX_OUT_OF_RANGE,
			L"Rebind col : invalid col index %d", uColIndex);

		_MSSQLColBindInfo& rBindInfo = this->_pBindInfos[uColIndex];
		return this->_AssignColBindInfo(rBindInfo, *pBindInfo, uColIndex);
	}

	void _SQLStatMSSQL::_FreeColBindInfos(yuint32_t uLastBindIndex)
	{
		for (yuint32_t i = 0; i <= uLastBindIndex && i < this->_uNumBindCols; ++i)
		{
			_MSSQLColBindInfo& rDstInfo = this->_pBindInfos[i];
			if (rDstInfo.pMemory && rDstInfo.bAutoManage)
			{
				free(rDstInfo.pMemory);
			}
		}

		this->_uNumBindCols = 0;
	}

	YXC_Status _SQLStatMSSQL::_CopyColBindInfo(_MSSQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo,
		yuint32_t uIndex)
	{
		rDstInfo.byDataType = rSrcInfo.byDataType;
		if (rSrcInfo.pMemory == NULL)
		{
			yuint32_t uBufSize = rSrcInfo.uBufferSize;
			if (uBufSize == 0)
			{
				YXC_Status rc = _GetSqlDefBufSize(rDstInfo.byDataType, uBufSize);
				_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);
			}

			rDstInfo.uBufferSize = uBufSize;
			rDstInfo.bAutoManage = TRUE;
			rDstInfo.pMemory = ::malloc(uBufSize);
			_YXC_CHECK_REPORT_NEW_RET(rDstInfo.pMemory != NULL, YXC_ERC_OUT_OF_MEMORY,
				L"Alloc memory for cell bind data failed, index = %u", uIndex);
		}
		else
		{
			rDstInfo.uBufferSize = rSrcInfo.uBufferSize;
			rDstInfo.bAutoManage = FALSE;
			rDstInfo.pMemory = rSrcInfo.pMemory;
		}

		return this->_BindCol(uIndex, rDstInfo, &rDstInfo.strlenOrInd);
	}

	YXC_Status _SQLStatMSSQL::_AssignColBindInfo(_MSSQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo,
		yuint32_t uIndex)
	{
		_MSSQLColBindInfo newInfo;

		newInfo.byDataType = rSrcInfo.byDataType;
		if (rSrcInfo.pMemory == NULL)
		{
			yuint32_t uBufSize = rSrcInfo.uBufferSize;
			if (uBufSize == 0)
			{
				YXC_Status rc = _GetSqlDefBufSize(rDstInfo.byDataType, uBufSize);
				_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);
			}
			newInfo.bAutoManage = TRUE;

			if (rDstInfo.bAutoManage && rDstInfo.uBufferSize > uBufSize)
			{
				newInfo.uBufferSize = rDstInfo.uBufferSize;
				newInfo.pMemory = rDstInfo.pMemory;
			}
			else
			{
				newInfo.uBufferSize = uBufSize;
				newInfo.pMemory = ::malloc(uBufSize);
				_YXC_CHECK_REPORT_NEW_RET(rDstInfo.pMemory != NULL, YXC_ERC_OUT_OF_MEMORY,
					L"Alloc memory for cell bind data failed, index = %u", uIndex);
			}
		}
		else
		{
			newInfo.uBufferSize = rSrcInfo.uBufferSize;
			newInfo.bAutoManage = FALSE;
			newInfo.pMemory = rSrcInfo.pMemory;
		}

		YXC_Status rc = this->_BindCol(uIndex, newInfo, &rDstInfo.strlenOrInd);
		if (rc == YXC_ERC_SUCCESS)
		{
			if (rDstInfo.bAutoManage && rDstInfo.pMemory != newInfo.pMemory)
			{
				free(rDstInfo.pMemory);
			}
			rDstInfo = newInfo;
		}
		else
		{
			if (newInfo.bAutoManage && newInfo.pMemory != NULL && newInfo.pMemory != rDstInfo.pMemory)
			{
				free(newInfo.pMemory);
			}
		}

		return rc;
	}

	YXC_Status _SQLStatMSSQL::_BindCol(yuint32_t uIndex, _MSSQLColBindInfo& rDstInfo, SQLLEN* pStrlenOrInd)
	{
		SQLSMALLINT sqlType, cType;
		SQLULEN uColDef;
		YXC_Status rc = _GetSqlTypeAndCType(rDstInfo.byDataType, sqlType, cType, uColDef);
		_YXC_CHECK_RET(rc == YXC_ERC_SUCCESS, rc);

		SQLRETURN ret = ::SQLBindCol(this->_stmt, uIndex + 1, cType, rDstInfo.pMemory,
			rDstInfo.uBufferSize, pStrlenOrInd);
		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Bind column data pointer failed");

		this->_pCellsPtr[uIndex].pCellData = rDstInfo.pMemory;
		this->_pCellsPtr[uIndex].uBufferLen = rDstInfo.uBufferSize;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::FetchNextRow()
	{
		SQLRETURN ret = ::SQLFetch(this->_stmt);

		YXC_Status rc = this->_CheckFetchResult(ret);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Fetch next row failed");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::FetchRow(yuint32_t uRowOffset)
	{
		SQLRETURN ret = ::SQLFetchScroll(this->_stmt, SQL_FETCH_RELATIVE, uRowOffset);

		YXC_Status rc = this->_CheckFetchResult(ret);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, L"Fetch offset row failed");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::RefetchCurrentRow()
	{
		return this->FetchRow(0);
	}

	YXC_Status _SQLStatMSSQL::EndTrans(ybool_t bCommit)
	{
		SQLRETURN retcode = ::SQLEndTran(SQL_HANDLE_DBC, this->_dbc, bCommit ? SQL_COMMIT : SQL_ROLLBACK);

		_YXC_CHECK_MSSQL_RET(retcode, SQL_HANDLE_DBC, this->_dbc, L"End transaction(commit = %d) failed", bCommit);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMSSQL::_CheckFetchResult(SQLRETURN ret)
	{
		if (ret == SQL_NO_DATA)
		{
			_YXC_REPORT_NEW_ERR(YXC_ERC_NO_DATA, L"No more result to fetch");
			return YXC_ERC_NO_DATA;
		}

		_YXC_CHECK_MSSQL_RET(ret, SQL_HANDLE_STMT, this->_stmt, L"Fetch result is not successful");

		for (yuint32_t i = 0; i < this->_uNumBindCols; ++i)
		{
			this->_pCellsPtr[i].uDataLen = this->_pBindInfos[i].strlenOrInd;
			this->_pCellsPtr[i].bIsNullVal = this->_pBindInfos[i].strlenOrInd == SQL_NULL_DATA;
		}

		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			wchar_t szSqlState[6];
			SQLINTEGER nativeError;
			wchar_t szErrMsg[_YXC_MSSQL_MAX_DIAG_SIZE];
			SQLSMALLINT iCchErr;
			::SQLGetDiagRecW(SQL_HANDLE_STMT, this->_stmt, 1, szSqlState, &nativeError,
				szErrMsg, _YXC_MSSQL_MAX_DIAG_SIZE, &iCchErr);

			YXC_Status rc = YXC_ERC_SUCCESS;

			if (wcsicmp(szSqlState, L"01004") == 0)
			{
				rc = YXC_ERC_DATA_TRUNCATED;
			}
			else if (wcsicmp(szSqlState, L"01S07") == 0)
			{
				rc = YXC_ERC_FRACTION_TRUNCATED;
			}
			else if (wcsicmp(szSqlState, L"07006") == 0)
			{
				rc = YXC_ERC_INVALID_TYPE;
			}

			if (rc != YXC_ERC_SUCCESS)
			{
				_YXC_MSSQL_REPORT(SQL_SUCCESS_WITH_INFO, SQL_HANDLE_STMT, this->_stmt, L"Fetch result failed");
				return rc;
			}

			return YXC_ERC_SUCCESS;
		}

		return YXC_ERC_SUCCESS;
	}
}

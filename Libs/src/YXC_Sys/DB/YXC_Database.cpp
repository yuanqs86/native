#define __MODULE__ "EK.DB"

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnMSSQL.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnSQLite.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnMySQL.hpp>
#include <YXC_Sys/DB/_YXC_MSSQLCmn.hpp>

#include <new>

using namespace YXC_Inner;

extern "C"
{
	YXC_Status YXC_DBConnCreate(YXC_DBConnType connType, ybool_t bAutoCommit, YXC_DBConn* pDbc)
	{
		_SQLConnBase* pConnBase = NULL;
		YXC_Status rcRet = YXC_ERC_UNKNOWN;

		switch (connType)
		{
		case YXC_DB_CONNECTION_TYPE_MSSQL:
			_YCHK_MAL_R2(pConnBase, _SQLConnMSSQL);
			new (pConnBase) _SQLConnMSSQL();
			break;
		case YXC_DB_CONNECTION_TYPE_SQLITE3:
			_YCHK_MAL_R2(pConnBase, _SQLConnSQLite);
			new (pConnBase) _SQLConnSQLite();
			break;
		case YXC_DB_CONNECTION_TYPE_MYSQL:
			_YCHK_MAL_R2(pConnBase, _SQLConnMySQL);
			new (pConnBase) _SQLConnMySQL();
			break;
		default:
			_YXC_REPORT_NEW_ERR(YXC_ERC_NOT_SUPPORTED, L"Not supported connection type %d", (int)connType);
			return YXC_ERC_NOT_SUPPORTED;
		}

		YXC_Status rc = pConnBase->Init(bAutoCommit);
		_YXC_CHECK_GOTO(rc == YXC_ERC_SUCCESS, rc);

		*pDbc = _SQLConnHdl(pConnBase);
		return YXC_ERC_SUCCESS;
err_ret:
		if (pConnBase != NULL)
		{
			pConnBase->~_SQLConnBase();
			free(pConnBase);
		}
		return rcRet;
	}

	YXC_Status YXC_DBConnAllocStatement(YXC_DBConn dbc, YXC_DBStat* pDbs)
	{
		_SQLConnBase* pConn = _SQLConnPtr(dbc);

		return pConn->AllocStatement((_SQLStatBase**)pDbs);
	}

	void YXC_DBConnClose(YXC_DBConn dbc)
	{
		_SQLConnBase* pConn = _SQLConnPtr(dbc);

		pConn->Free();
		pConn->~_SQLConnBase();
		free(pConn);
	}

	YXC_Status YXC_DBConnConnect(YXC_DBConn dbc, const wchar_t* cpszTarget, yuint32_t uTargetPort, const wchar_t* cpszDriver,
		const wchar_t* cpszInitDB, const wchar_t* cpszUser, const wchar_t* cpszPwd)
	{
		_SQLConnBase* pConn = _SQLConnPtr(dbc);

		return pConn->ConnectToDBServer(cpszTarget, uTargetPort, cpszDriver, cpszInitDB, cpszUser, cpszPwd);
	}

	YXC_Status YXC_DBStatPrepare(YXC_DBStat dbs, const wchar_t* cpszPrep)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->Prepare(cpszPrep);
	}

	YXC_Status YXC_DBStatBindInputParam(YXC_DBStat dbs, ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, const void* cpData)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->BindInputParameter(byDataType, uColIndex, pstDataLen, cpData);
	}

	YXC_Status YXC_DBStatBindOutputParam(YXC_DBStat dbs, ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->BindOutputParameter(byDataType, uColIndex, pstDataLen, pData);
	}

	YXC_Status YXC_DBStatExecute(YXC_DBStat dbs, ysize_t* pstAffectedRows)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->Execute(pstAffectedRows);
	}

	YXC_Status YXC_DBStatNextResult(YXC_DBStat dbs, ysize_t* pstAffectedRows)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->MoreResults(pstAffectedRows);
	}

	YXC_Status YXC_DBStatResetParams(YXC_DBStat dbs)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->Reset();
	}

	YXC_Status YXC_DBStatCloseCursor(YXC_DBStat dbs)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->CloseCursor();
	}

	YXC_Status YXC_DBStatExecDirect(YXC_DBStat dbs, const wchar_t* cpszStmt, ysize_t* pstAffectedRows)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->ExecuteDirect(cpszStmt, pstAffectedRows);
	}

	YXC_Status YXC_DBStatQueryResultInfo(YXC_DBStat dbs, yuint32_t* puNumHeaders, YXC_DBRTColHeader* pHeaders)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->QueryResultInfo(puNumHeaders, pHeaders);
	}

	YXC_Status YXC_DBStatBindMemory(YXC_DBStat dbs, yuint32_t uNumColumns, const YXC_DBRTColBindInfo* pBindInfos,
		const YXC_DBRTCell** ppCellsPtr)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->BindResultCols(uNumColumns, pBindInfos, ppCellsPtr);
	}

	YXC_Status YXC_DBStatFetchNextRow(YXC_DBStat dbs)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->FetchNextRow();
	}

	YXC_Status YXC_DBStatRefetchCurrentRow(YXC_DBStat dbs)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->RefetchCurrentRow();
	}

	YXC_Status YXC_DBStatRebindCol(YXC_DBStat dbs, yuint32_t uColIndex, const YXC_DBRTColBindInfo* pBindInfo)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->RebindResultCol(uColIndex, pBindInfo);
	}

	void YXC_DBConnFreeStatement(YXC_DBConn dbc, YXC_DBStat dbs)
	{
		_SQLConnBase* pConn = _SQLConnPtr(dbc);
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		pConn->FreeStatement(pStat);
	}

	YXC_Status YXC_DBStatEndTrans(YXC_DBStat dbs, ybool_t bCommit)
	{
		_SQLStatBase* pStat = _SQLStatPtr(dbs);

		return pStat->EndTrans(bCommit);
	}
};

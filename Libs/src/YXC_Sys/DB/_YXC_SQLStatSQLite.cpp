#define __MODULE__ "EK.DB.SQLite"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnSQLite.hpp>
#include <YXC_Sys/DB/_YXC_SQLStatSQLite.hpp>
#include <YXC_Sys/DB/_YXC_SQLiteCmn.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>

#include <new>

namespace YXC_Inner
{
	_SQLStatSQLite::_SQLStatSQLite()
	{

	}

	_SQLStatSQLite::~_SQLStatSQLite()
	{

	}

	static void __LocalFreeStatement(_SQLStatSQLite* pStat)
	{
		pStat->~_SQLStatSQLite();
		free(pStat);
	}

	YXC_Status _SQLStatSQLite::CreateStatement(const wchar_t* pszDbName, ybool_t bAutoCommit, _SQLStatSQLite** ppStat)
	{
		_YCHK_MAL_R1(pStat, _SQLStatSQLite);
		new (pStat) _SQLStatSQLite();

		YXCLib::HandleRef<_SQLStatSQLite*> pStat_res(pStat, __LocalFreeStatement);

		char szDb[YXC_MAX_CCH_PATH];
		yuint32_t uCcConverted;
		YXC_TEWCharToUTF8(pszDbName, YXC_STR_NTS, (yuint8_t*)szDb, YXC_MAX_CCH_PATH - 1, &uCcConverted, NULL);

		int sql_ret = sqlite::_sqlite3_open(szDb, &pStat->_db);
		_YXC_CHECK_SQLITE_RET(sql_ret, NULL, L"Failed to open db '%S'", szDb);

		pStat->_stmt = NULL;
		pStat->_uNumBindCols = 0;
		pStat->_uNumBindColsBuf = 0;
		pStat->_pCellsPtr = NULL;
		pStat->_pBindInfos = NULL;
		pStat->_bAutoCommit = bAutoCommit;
		pStat->_uLastExecution = SQLITE_OK;

		*ppStat = pStat_res.Detach();
		return YXC_ERC_SUCCESS;
	}

	void _SQLStatSQLite::FreeStatement(_SQLStatSQLite* pStat)
	{
		pStat->_FreeColBindInfos(pStat->_uNumBindCols - 1);
		if (pStat->_pBindInfos)
		{
			free(pStat->_pBindInfos);
		}

		if (pStat->_stmt)
		{
			sqlite::_sqlite3_finalize(pStat->_stmt);
		}

		if (pStat->_db)
		{
			sqlite::_sqlite3_close(pStat->_db);
		}

		pStat->~_SQLStatSQLite();
		free(pStat);
	}

	YXC_Status _SQLStatSQLite::Prepare(const wchar_t* pszStmt)
	{
		this->CloseCursor(); /* Close statement before. */

		if (!this->_bAutoCommit)
		{
			YXC_Status rc = this->_ExecuteSql("BEGIN TRANSACTION");
			_YXC_CHECK_RC_RET(rc);
		}

		yuint32_t stCh = (yuint32_t)wcslen(pszStmt);
		_YCHK_MAL_STRA_R1(sqlStrA, stCh * 4); /* Enough memory prepared. */

		YXCLib::HandleRef<void*> sqlstr_res(sqlStrA, free);

		yuint32_t uConverted;
		YXC_Status rc = YXC_TEWCharToUTF8(pszStmt, stCh, (yuint8_t*)sqlStrA, stCh * 4, &uConverted, NULL);
		_YXC_CHECK_RC_RET(rc);

		int ret = sqlite::_sqlite3_prepare(this->_db, sqlStrA, -1, &this->_stmt, NULL);
		_YXC_CHECK_SQLITE_RET(ret, this->_db, L"_sqlite3_prepare");

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::ExecuteDirect(const wchar_t* pszStmt, ysize_t* pstAffectedRows)
	{
		YXC_Status rc = this->Prepare(pszStmt);
		_YXC_CHECK_RC_RET(rc);

		rc = this->Execute(pstAffectedRows);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::Reset()
	{
		int ret = sqlite::_sqlite3_reset(this->_stmt);
		_YXC_CHECK_SQLITE_RET(ret, this->_db, YC("_sqlite3_reset"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::CloseCursor()
	{
		if (this->_stmt)
		{
			sqlite::_sqlite3_finalize(this->_stmt);
			this->_stmt = NULL;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::Execute(ysize_t* pstAffectedRows)
	{
		int n = sqlite::_sqlite3_step(this->_stmt);
		if (n == SQLITE_ROW || n == SQLITE_DONE)
		{
			this->_uLastExecution = n;
			this->_GetAffectedRowNum(pstAffectedRows);
			return YXC_ERC_SUCCESS;
		}

		_YXC_CHECK_SQLITE_RET(n, this->_db, YC("_sqlite3_step"));

		this->_GetAffectedRowNum(pstAffectedRows);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::MoreResults(ysize_t* pstAffectedRows)
	{
		return this->Execute(pstAffectedRows);
		// _YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, L"sqlite don't support this API");
	}

	void _SQLStatSQLite::_GetAffectedRowNum(ysize_t* pstAffectedRows)
	{
		if (pstAffectedRows != NULL)
		{
			int ret = sqlite::_sqlite3_changes(this->_db);
			*pstAffectedRows = ret;
		}
	}

	YXC_Status _SQLStatSQLite::BindInputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, const void* pData)
	{
		++uColIndex;
		int ret;
		if (*pstDataLen == YXC_DB_NULL_DATA)
		{
			ret = sqlite::_sqlite3_bind_null(this->_stmt, uColIndex);
		}
		else
		{
			switch (byDataType)
			{
			case YXC_DB_DATA_TYPE_I8:
			case YXC_DB_DATA_TYPE_BIT:
				ret = sqlite::_sqlite3_bind_int(this->_stmt, uColIndex, *(yint8_t*)pData);
				break;
			case YXC_DB_DATA_TYPE_U8:
				ret = sqlite::_sqlite3_bind_int(this->_stmt, uColIndex, *(yuint8_t*)pData);
				break;
			case YXC_DB_DATA_TYPE_I16:
				ret = sqlite::_sqlite3_bind_int(this->_stmt, uColIndex, *(yint16_t*)pData);
				break;
			case YXC_DB_DATA_TYPE_U16:
				ret = sqlite::_sqlite3_bind_int(this->_stmt, uColIndex, *(yuint16_t*)pData);
				break;
			case YXC_DB_DATA_TYPE_I32:
			case YXC_DB_DATA_TYPE_U32:
				ret = sqlite::_sqlite3_bind_int(this->_stmt, uColIndex, *(yint32_t*)pData);
				break;
			case YXC_DB_DATA_TYPE_I64:
			case YXC_DB_DATA_TYPE_U64:
				ret = sqlite::_sqlite3_bind_int64(this->_stmt, uColIndex, *(yint64_t*)pData);
				break;
			case YXC_DB_DATA_TYPE_WSTRING:
			case YXC_DB_DATA_TYPE_VWSTRING:
			case YXC_DB_DATA_TYPE_LVWSTRING:
				ret = sqlite::_sqlite3_bind_text16(this->_stmt, uColIndex, pData, *pstDataLen, NULL);
				break;
			case YXC_DB_DATA_TYPE_ASTRING:
			case YXC_DB_DATA_TYPE_VASTRING:
			case YXC_DB_DATA_TYPE_LVASTRING:
				ret = sqlite::_sqlite3_bind_text(this->_stmt, uColIndex, (const char*)pData, *pstDataLen, NULL);
				break;
			case YXC_DB_DATA_TYPE_F32:
				ret = sqlite::_sqlite3_bind_double(this->_stmt, uColIndex, *(float*)pData);
				break;
			case YXC_DB_DATA_TYPE_F64:
				ret = sqlite::_sqlite3_bind_double(this->_stmt, uColIndex, *(double*)pData);
				break;
			case YXC_DB_DATA_TYPE_BINARY:
			case YXC_DB_DATA_TYPE_VBINARY:
			case YXC_DB_DATA_TYPE_LVBINARY:
				ret = sqlite::_sqlite3_bind_blob(this->_stmt, uColIndex, pData, *pstDataLen, NULL);
				break;
			default:
				_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, YC("sqlite don't support type(%d)"), byDataType);
				break;
			}
		}

		_YXC_CHECK_SQLITE_RET(ret, this->_db, YC("Failed to bind sqlite data(type:%d, index:%d)"), byDataType, uColIndex);
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::BindOutputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData)
	{
		_YXC_REPORT_NEW_RET(YXC_ERC_NOT_SUPPORTED, L"sqlite don't support out parameter.");
	}

	YXC_Status _SQLStatSQLite::QueryResultInfo(yuint32_t* puNumHeaders, YXC_DBRTColHeader* pHeaders)
	{
		yuint32_t data_count = (yuint32_t)sqlite::_sqlite3_data_count(this->_stmt);
		yuint32_t uConverted;

		if (*puNumHeaders >= data_count)
		{
			*puNumHeaders = data_count;
			for (int i = 0; i < data_count; ++i)
			{
				const yuint8_t* name = (const yuint8_t*)sqlite::_sqlite3_column_name(this->_stmt, i);

				YXC_TEUTF8ToWChar(name, YXC_STR_NTS, pHeaders[i].szColName, YXC_STRING_ARR_LEN(pHeaders[i].szColName), &uConverted, NULL);
				pHeaders[i].bCanBeNull = TRUE;
				pHeaders[i].uDataMaxLen = 0;

				int nType = sqlite::_sqlite3_column_type(this->_stmt, i);
				switch (nType)
				{
				case SQLITE_INTEGER:
					pHeaders[i].byDataType = YXC_DB_DATA_TYPE_I64;
					break;
				case SQLITE_FLOAT:
					pHeaders[i].byDataType = YXC_DB_DATA_TYPE_F64;
					break;
				case SQLITE_BLOB:
					pHeaders[i].byDataType = YXC_DB_DATA_TYPE_VBINARY;
					break;
				case SQLITE3_TEXT:
					pHeaders[i].byDataType = YXC_DB_DATA_TYPE_VASTRING;
					break;
				case SQLITE_NULL:
					pHeaders[i].byDataType = YXC_DB_DATA_TYPE_I32;
					break;
				default:
					pHeaders[i].byDataType = -1;
					break;
				}
			}

			return YXC_ERC_SUCCESS;
		}
		else
		{
			*puNumHeaders = data_count;
			_YXC_REPORT_NEW_RET(YXC_ERC_BUFFER_NOT_ENOUGH, L"Buffer not enough");
		}
	}

	YXC_Status _SQLStatSQLite::BindResultCols(yuint32_t uNumCols, const YXC_DBRTColBindInfo* pBindInfos,
		const YXC_DBRTCell** ppCellsPtr)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		yuint32_t uIndex = 0;
		this->_FreeColBindInfos(this->_uNumBindCols - 1);
		if (this->_uNumBindColsBuf < uNumCols)
		{
			free(this->_pBindInfos);
			this->_pBindInfos = (_SQLiteColBindInfo*)::calloc(uNumCols, sizeof(_SQLiteColBindInfo) + sizeof(YXC_DBRTCell));
			this->_pCellsPtr = (YXC_DBRTCell*)(this->_pBindInfos + uNumCols);
			_YXC_CHECK_REPORT_NEW_GOTO(this->_pCellsPtr != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc new column bind infos failed");

			this->_uNumBindColsBuf = uNumCols;
		}
		this->_uNumBindCols = uNumCols;

		for (; uIndex < uNumCols; ++uIndex)
		{
			_SQLiteColBindInfo& rDstInfo = this->_pBindInfos[uIndex];
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

	YXC_Status _SQLStatSQLite::RebindResultCol(yuint32_t uColIndex, const YXC_DBRTColBindInfo* pBindInfo)
	{
		_YXC_CHECK_REPORT_NEW_RET(uColIndex < this->_uNumBindCols, YXC_ERC_INDEX_OUT_OF_RANGE,
			L"Rebind col : invalid col index %d", uColIndex);

		_SQLiteColBindInfo& rBindInfo = this->_pBindInfos[uColIndex];
		return this->_AssignColBindInfo(rBindInfo, *pBindInfo, uColIndex);
	}

	void _SQLStatSQLite::_FreeColBindInfos(yuint32_t uLastBindIndex)
	{
		for (yuint32_t i = 0; i <= uLastBindIndex && i < this->_uNumBindCols; ++i)
		{
			_SQLiteColBindInfo& rDstInfo = this->_pBindInfos[i];
			if (rDstInfo.pMemory && rDstInfo.bAutoManage)
			{
				free(rDstInfo.pMemory);
			}
		}

		this->_uNumBindCols = 0;
	}

	YXC_Status _SQLStatSQLite::_CopyColBindInfo(_SQLiteColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo,
		yuint32_t uIndex)
	{
		rDstInfo.byDataType = rSrcInfo.byDataType;
		if (rSrcInfo.pMemory == NULL)
		{
			yuint32_t uBufSize = rSrcInfo.uBufferSize;
			if (uBufSize == 0)
			{
				YXC_Status rc = _GetSqlDefBufSize_sqlite(rDstInfo.byDataType, uBufSize);
				_YXC_CHECK_RC_RET(rc);
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

		return this->_BindCol(uIndex, rDstInfo);
	}

	YXC_Status _SQLStatSQLite::_AssignColBindInfo(_SQLiteColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo,
		yuint32_t uIndex)
	{
		_SQLiteColBindInfo newInfo;

		newInfo.byDataType = rSrcInfo.byDataType;
		if (rSrcInfo.pMemory == NULL)
		{
			yuint32_t uBufSize = rSrcInfo.uBufferSize;
			if (uBufSize == 0)
			{
				YXC_Status rc = _GetSqlDefBufSize_sqlite(rDstInfo.byDataType, uBufSize);
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
				_YXC_CHECK_REPORT_NEW_RET(rDstInfo.pMemory != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc memory for cell bind data failed, index = %u", uIndex);
			}
		}
		else
		{
			newInfo.uBufferSize = rSrcInfo.uBufferSize;
			newInfo.bAutoManage = FALSE;
			newInfo.pMemory = rSrcInfo.pMemory;
		}

		YXC_Status rc = this->_BindCol(uIndex, newInfo);
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

	YXC_Status _SQLStatSQLite::_BindCol(yuint32_t uIndex, _SQLiteColBindInfo& rDstInfo)
	{
		this->_pCellsPtr[uIndex].pCellData = rDstInfo.pMemory;
		this->_pCellsPtr[uIndex].uBufferLen = rDstInfo.uBufferSize;

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::FetchNextRow()
	{
		int sql_ret = this->_uLastExecution;
		if (sql_ret == 0)
		{
			sql_ret = sqlite::_sqlite3_step(this->_stmt);
		}
		else
		{
			this->_uLastExecution = 0;
		}

		if (sql_ret == SQLITE_ROW) /* Fetched a row. */
		{
			return this->_CopyBindingData();
		}
		else if (sql_ret == SQLITE_DONE)
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_NO_DATA, YC("No result to fetch"));
		}
		else
		{
			_YXC_CHECK_SQLITE_RET(sql_ret, this->_db, YC("Failed to fetch next row"));
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::RefetchCurrentRow()
	{
		YXC_Status rc = this->_CopyBindingData();
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::_CopyBindingData()
	{
		for (yuint32_t i = 0; i < this->_uNumBindCols; ++i)
		{
			YXC_Status rc = this->_CopyCol(this->_pCellsPtr + i, i);
			_YXC_CHECK_RC_RET(rc);
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::_CopyCol(YXC_DBRTCell* pCell, yuint32_t n)
	{
		int type = sqlite::_sqlite3_column_type(this->_stmt, n);
		sqlite::sqlite3_value* pVal = sqlite::_sqlite3_column_value(this->_stmt, n);
		int nLen = sqlite::_sqlite3_value_bytes(pVal);

		YXC_Status rc;
		switch (type)
		{
		case SQLITE_INTEGER:
			rc = _CopyInteger(this->_pBindInfos[n].byDataType, pCell, sqlite::_sqlite3_value_int64(pVal));
			break;
		case SQLITE3_TEXT:
			rc = _CopyText(this->_pBindInfos[n].byDataType, pCell, nLen, sqlite::_sqlite3_value_text(pVal));
			break;
		case SQLITE_BLOB:
			rc = _CopyBlob(this->_pBindInfos[n].byDataType, pCell, nLen, sqlite::_sqlite3_value_blob(pVal));
			break;
		case SQLITE_NULL:
			pCell->bIsNullVal = TRUE;
			pCell->uDataLen = 0;
			rc = YXC_ERC_SUCCESS;
			break;
		default:
			break;
		}

		return rc;
	}

	YXC_Status _SQLStatSQLite::_CopyInteger(ybyte_t byDataType, YXC_DBRTCell* pCell, yint64_t iInteger)
	{
		void* pData = pCell->pCellData;
		yuint32_t uCbBuffer = pCell->uBufferLen;
		switch (byDataType)
		{
		case YXC_DB_DATA_TYPE_I8:
			*(yint8_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yint8_t);
			break;
		case YXC_DB_DATA_TYPE_BIT:
		case YXC_DB_DATA_TYPE_U8:
			*(yuint8_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yuint8_t);
			break;
		case YXC_DB_DATA_TYPE_I16:
			*(yint16_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yint16_t);
			break;
		case YXC_DB_DATA_TYPE_U16:
			*(yuint16_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yuint16_t);
			break;
		case YXC_DB_DATA_TYPE_I32:
			*(yint32_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yint32_t);
			break;
		case YXC_DB_DATA_TYPE_U32:
			*(yuint32_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yint32_t);
			break;
		case YXC_DB_DATA_TYPE_U64:
		case YXC_DB_DATA_TYPE_I64:
			*(yint64_t*)pData = iInteger;
			pCell->uDataLen = sizeof(yint64_t);
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_TYPE, YC("Invalid integer type %d"), byDataType);
			break;
		}

		pCell->bIsNullVal = FALSE;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::_CopyText(ybyte_t byDataType, YXC_DBRTCell* pCell, yuint32_t uDataLen, const yuint8_t* utf8)
	{
		void* pData = pCell->pCellData;
		yuint32_t uCbBuffer = pCell->uBufferLen, uConverted;

		YXC_Status rc;
		switch (byDataType)
		{
		case YXC_DB_DATA_TYPE_ASTRING:
		case YXC_DB_DATA_TYPE_VASTRING:
		case YXC_DB_DATA_TYPE_LVASTRING:
			rc = YXC_TEUTF8ToChar(utf8, uDataLen, (char*)pCell->pCellData, pCell->uBufferLen / sizeof(char) - 1, &uConverted, NULL);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));
			break;
		case YXC_DB_DATA_TYPE_WSTRING:
		case YXC_DB_DATA_TYPE_VWSTRING:
		case YXC_DB_DATA_TYPE_LVWSTRING:
			rc = YXC_TEUTF8ToWChar(utf8, uDataLen, (wchar_t*)pCell->pCellData, pCell->uBufferLen / sizeof(wchar_t) - 1, &uConverted, NULL);
			_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, YXC_ERC_DATA_TRUNCATED, YC("String truncated"));
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_TYPE, YC("Invalid string type %d"), byDataType);
			break;
		}

		pCell->bIsNullVal = FALSE;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::_CopyBlob(ybyte_t byDataType, YXC_DBRTCell* pCell, yuint32_t uDataLen, const void* blob)
	{
		void* pData = pCell->pCellData;
		yuint32_t uCbBuffer = pCell->uBufferLen;
		yuint32_t nCpy = min(uCbBuffer, uDataLen);
		pCell->uDataLen = nCpy;
		switch (byDataType)
		{
		case YXC_DB_DATA_TYPE_BINARY:
		case YXC_DB_DATA_TYPE_VBINARY:
		case YXC_DB_DATA_TYPE_LVBINARY:
			memcpy(pData, blob, nCpy);
			_YXC_CHECK_REPORT_NEW_RET(uDataLen <= pCell->uBufferLen, YXC_ERC_DATA_TRUNCATED, YC("Data truncated(%d-%d)"),
				uCbBuffer, uDataLen);
			break;
		default:
			_YXC_REPORT_NEW_RET(YXC_ERC_INVALID_TYPE, YC("Invalid blob type %d"), byDataType);
			break;
		}

		pCell->bIsNullVal = FALSE;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatSQLite::EndTrans(ybool_t bCommit)
	{
		if (bCommit)
		{
			return this->_ExecuteSql("COMMIT TRANSACTION");
		}
		else
		{
			return this->_ExecuteSql("ROLLBACK TRANSACTION");
		}
	}

	YXC_Status _SQLStatSQLite::_ExecuteSql(const void* utf8)
	{
		int sql_ret = sqlite::_sqlite3_exec(this->_db, (const char*)utf8, NULL, NULL, NULL);
		_YXC_CHECK_SQLITE_RET(sql_ret, this->_db, YC("Failed to execute sql"));

		return YXC_ERC_SUCCESS;
	}
}

#define __MODULE__ "EK.DB.SQLite"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/DB/_YXC_SQLConnMySQL.hpp>
#include <YXC_Sys/DB/_YXC_SQLStatMySQL.hpp>
#include <YXC_Sys/DB/_YXC_MySQLCmn.hpp>
#include <YXC_Sys/YXC_TextEncoding.h>
#include <YXC_Sys/YXC_MMInterface.h>

#include <new>

namespace YXC_Inner
{
	_SQLStatMySQL::_SQLStatMySQL()
	{

	}

	_SQLStatMySQL::~_SQLStatMySQL()
	{

	}

	static void __LocalFreeStatement(_SQLStatMySQL* pStat)
	{
		pStat->~_SQLStatMySQL();
		free(pStat);
	}

	YXC_Status _SQLStatMySQL::CreateStatement(MYSQL* hdl, MYSQL_STMT* stmt, _SQLStatMySQL** ppStat)
	{
		_YCHK_MAL_R1(pStat, _SQLStatMySQL);
		new (pStat) _SQLStatMySQL();

		YXCLib::HandleRef<_SQLStatMySQL*> pStat_res(pStat, __LocalFreeStatement);

		pStat->_stmt = stmt;
		pStat->_db = hdl;
		pStat->_uNumBindCols = 0;
		pStat->_uNumBindColsBuf = 0;
		pStat->_pCellsPtr = NULL;
		pStat->_pBindInfos = NULL;
		pStat->_tempBuf = NULL;
		pStat->_stTempBuf = 0;
		pStat->_pBindingParams = NULL;
		pStat->_pBindingsEx = NULL;
		pStat->_stCbParam = 0;
		pStat->_stCbEx = 0;
		pStat->_uNumParams = 0;
		pStat->_bRealBound = FALSE;
		pStat->_offset0 = NULL;

		*ppStat = pStat_res.Detach();
		return YXC_ERC_SUCCESS;
	}

	void _SQLStatMySQL::FreeStatement(_SQLStatMySQL* pStat)
	{
		pStat->_FreeColBindInfos(pStat->_uNumBindCols - 1);
		if (pStat->_pBindInfos)
		{
			free(pStat->_pBindInfos);
		}

		if (pStat->_stmt)
		{
			mysql::_mysql_stmt_close(pStat->_stmt);
		}

		if (pStat->_tempBuf)
		{
			free(pStat->_tempBuf);
		}

		if (pStat->_pBindingParams)
		{
			free(pStat->_pBindingParams);
		}

		if (pStat->_pBindingsEx)
		{
			free(pStat->_pBindingsEx);
		}

		pStat->~_SQLStatMySQL();
		free(pStat);
	}

	YXC_Status _SQLStatMySQL::Prepare(const wchar_t* pszStmt)
	{
		size_t stCh = wcslen(pszStmt);
		_YCHK_MAL_STRA_R1(sqlStrA, stCh * 4); /* Enough memory prepared. */

		YXCLib::HandleRef<void*> sqlstr_res(sqlStrA, free);

		yuint32_t uConverted;
		YXC_Status rc = YXC_TEWCharToUTF8(pszStmt, stCh, (yuint8_t*)sqlStrA, stCh * 4, &uConverted, NULL);
		_YXC_CHECK_RC_RET(rc);

		int ret = mysql::_mysql_stmt_prepare(this->_stmt, sqlStrA, uConverted);
		_YXC_CHECK_MYSQL_RET(ret, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_prepare"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::ExecuteDirect(const wchar_t* pszStmt, ysize_t* pstAffectedRows)
	{
		YXC_Status rc = this->Prepare(pszStmt);
		_YXC_CHECK_RC_RET(rc);

		rc = this->Execute(pstAffectedRows);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::Reset()
	{
		ybool_t ret = mysql::_mysql_stmt_reset(this->_stmt);
		_YXC_CHECK_MYSQL_RET(ret, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_reset"));

		this->_ClearBindingParameters();
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::CloseCursor()
	{
		ybool_t ret = mysql::_mysql_stmt_reset(this->_stmt);
		_YXC_CHECK_MYSQL_RET(ret, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_reset"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::Execute(ysize_t* pstAffectedRows)
	{
		if (!this->_bRealBound)
		{
			YXC_Status rc = this->_RealBindingParameters();
			_YXC_CHECK_RC_RET(rc);

			this->_bRealBound = TRUE;
		}

		int n = mysql::_mysql_stmt_execute(this->_stmt);
		_YXC_CHECK_MYSQL_RET(n, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_execute"));

		n = mysql::_mysql_stmt_store_result(this->_stmt);
		_YXC_CHECK_MYSQL_RET(n, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_store_result"));

		YXC_Status rc = this->_GetAffectedRowNum(pstAffectedRows);
		_YXC_CHECK_RC_RET(rc);

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::_RealBindingParameters()
	{
		ybool_t bRet = mysql::_mysql_stmt_bind_param(this->_stmt, this->_pBindingsEx);
		_YXC_CHECK_MYSQL_RET(bRet, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_bind_param"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::MoreResults(ysize_t* pstAffectedRows)
	{
		ybool_t bMore = mysql::_mysql_more_results(this->_db);

		if (bMore)
		{
			int ret = mysql::_mysql_stmt_next_result(this->_stmt);
			_YXC_CHECK_MYSQL_RET(ret, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_next_result"));

			YXC_Status rc = this->_GetAffectedRowNum(pstAffectedRows);
			_YXC_CHECK_RC_RET(rc);

			return YXC_ERC_SUCCESS;
		}
		else
		{
			_YXC_REPORT_NEW_RET(YXC_ERC_NO_DATA, YC("No more results"));
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::_GetAffectedRowNum(ysize_t* pstAffectedRows)
	{
		YXC_Status rc = this->_ConvertOutputBindings();
		_YXC_CHECK_RC_RET(rc);

		if (pstAffectedRows != NULL)
		{
			yint64_t ret = mysql::_mysql_stmt_affected_rows(this->_stmt);
			*pstAffectedRows = ret;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::BindInputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, const void* pData)
	{
		void* pNewBuf = NULL;
		ysize_t newSize = 0;
		enum_field_types eft;
		YXC_Status rc = _ConvertInputBindings(byDataType, pData, *pstDataLen, &eft, &pNewBuf, &newSize);
		_YXC_CHECK_RC_RET(rc);

		YXCLib::HandleRef<void*> pNewBuf_res(free);
		if (pNewBuf != pData && pNewBuf != NULL) /* A convert occured, attach new buffer. */
		{
			pNewBuf_res.Attach(pNewBuf);
		}

		yuint32_t uNumParams = this->_uNumParams + 1;
		rc = YXC_MMCExpandBuffer((void**)&this->_pBindingParams, &this->_stCbParam, uNumParams * sizeof(_MySQLParamInfo));
		_YXC_CHECK_RC_RET(rc);

		rc =  YXC_MMCExpandBuffer((void**)&this->_pBindingsEx, &this->_stCbEx, uNumParams * sizeof(MYSQL_BIND));
		_YXC_CHECK_RC_RET(rc);

		_YXC_CHECK_REPORT_NEW_RET(this->_pBindingParams != NULL && this->_pBindingsEx != NULL, YXC_ERC_OUT_OF_MEMORY,
			YC("Alloc for bindings failed"));

		MYSQL_BIND& mBind = this->_pBindingsEx[this->_uNumParams];
		_MySQLParamInfo& paramInfo = this->_pBindingParams[this->_uNumParams];
		yuint32_t uColDef;

		memset(&paramInfo, 0, sizeof(_MySQLParamInfo));
		memset(&mBind, 0, sizeof(MYSQL_BIND));

		mBind.buffer = pNewBuf;
		_GetMySqlType(byDataType, mBind.buffer_type, uColDef);
		mBind.buffer_length = newSize;

		paramInfo.newData = pNewBuf == pData ? NULL : pNewBuf;
		paramInfo.bIsOutput = FALSE;

		this->_uNumParams = uNumParams;
		this->_bRealBound = FALSE;
		pNewBuf_res.Detach();
		return YXC_ERC_SUCCESS;
	}
	//YXC_DBStatBindOutputParam(this->_dbs, YXC_DB_DATA_TYPE_I64, 0, &stIntSize, &uUserCount);
	//YXC_Status YXC_DBStatBindOutputParam(YXC_DBStat dbs, ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData);
	//pStat->BindOutputParameter(byDataType, uColIndex, pstDataLen, pData);
	YXC_Status _SQLStatMySQL::BindOutputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData)
	{
		yuint32_t uNumParams = this->_uNumParams + 1;
		YXC_Status rc = YXC_MMCExpandBuffer((void**)&this->_pBindingParams, &this->_stCbParam, uNumParams * sizeof(_MySQLParamInfo));
		_YXC_CHECK_RC_RET(rc);
		rc =  YXC_MMCExpandBuffer((void**)&this->_pBindingsEx, &this->_stCbEx, uNumParams * sizeof(MYSQL_BIND));
		_YXC_CHECK_RC_RET(rc);

		_YXC_CHECK_REPORT_NEW_RET(this->_pBindingParams != NULL && this->_pBindingsEx != NULL, YXC_ERC_OUT_OF_MEMORY,
			YC("Alloc for bindings failed"));

		MYSQL_BIND& mBind = this->_pBindingsEx[this->_uNumParams];
		_MySQLParamInfo& paramInfo = this->_pBindingParams[this->_uNumParams];
		yuint32_t uColDef;

		memset(&paramInfo, 0, sizeof(_MySQLParamInfo));
		memset(&mBind, 0, sizeof(MYSQL_BIND));

		mBind.buffer = pData;
		_GetMySqlType(byDataType, mBind.buffer_type, uColDef);
		mBind.buffer_length = *pstDataLen;

		paramInfo.newData = NULL;
		paramInfo.bIsOutput = TRUE;

		this->_uNumParams = uNumParams;
		this->_bRealBound = FALSE;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::QueryResultInfo(yuint32_t* puNumHeaders, YXC_DBRTColHeader* pHeaders)
	{
		int data_count = mysql::_mysql_stmt_field_count(this->_stmt);
		yuint32_t uConverted;

		if (*puNumHeaders >= data_count)
		{
			*puNumHeaders = data_count;
			MYSQL_RES* res = mysql::_mysql_stmt_result_metadata(this->_stmt);
			MYSQL_FIELD* fields = (MYSQL_FIELD*)mysql::_mysql_fetch_field(res);
			for (int i = 0; i < data_count; ++i)
			{
				YXC_TEUTF8ToWChar((const yuint8_t*)fields[i].name, YXC_STR_NTS, pHeaders[i].szColName, YXC_STRING_ARR_LEN(pHeaders[i].szColName), &uConverted, NULL);
				pHeaders[i].bCanBeNull = !(fields[i].flags & NOT_NULL_FLAG);
				pHeaders[i].uDataMaxLen = fields[i].max_length;
				_GetTypeFromMySQLType(fields[i].type, pHeaders[i].byDataType);
			}
		}
		else
		{
			*puNumHeaders = data_count;
			_YXC_REPORT_NEW_RET(YXC_ERC_BUFFER_NOT_ENOUGH, L"Buffer not enough");
		}
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::BindResultCols(yuint32_t uNumCols, const YXC_DBRTColBindInfo* pBindInfos,
		const YXC_DBRTCell** ppCellsPtr)
	{
		YXC_Status rcRet = YXC_ERC_UNKNOWN;
		yuint32_t uIndex = 0;
		this->_FreeColBindInfos(this->_uNumBindCols - 1);
		if (this->_uNumBindColsBuf < uNumCols)
		{
			free(this->_pBindInfos);
			this->_pBindInfos = (_MySQLColBindInfo*)::calloc(uNumCols, sizeof(_MySQLColBindInfo) + sizeof(YXC_DBRTCell));
			this->_pCellsPtr = (YXC_DBRTCell*)(this->_pBindInfos + uNumCols);
			_YXC_CHECK_REPORT_NEW_GOTO(this->_pCellsPtr != NULL, YXC_ERC_OUT_OF_MEMORY, L"Alloc new column bind infos failed");

			this->_uNumBindColsBuf = uNumCols;
		}
		this->_uNumBindCols = uNumCols;

		for (; uIndex < uNumCols; ++uIndex)
		{
			_MySQLColBindInfo& rDstInfo = this->_pBindInfos[uIndex];
			const YXC_DBRTColBindInfo& rSrcInfo = pBindInfos[uIndex];

			YXC_Status rc = this->_CopyColBindInfo(rDstInfo, rSrcInfo, uIndex);
			_YXC_CHECK_REPORT_GOTO(rc == YXC_ERC_SUCCESS, rc, L"Copy column bind info failed, index = %u", uIndex);
		}

		YXC_Status rcAll = this->_BindAllCols();
		_YXC_CHECK_RC_GOTO(rcAll);

		*ppCellsPtr = this->_pCellsPtr;
		return YXC_ERC_SUCCESS;
err_ret:
		this->_FreeColBindInfos(uIndex);
		this->_uNumBindCols = 0;
		return rcRet;
	}

	YXC_Status _SQLStatMySQL::RebindResultCol(yuint32_t uColIndex, const YXC_DBRTColBindInfo* pBindInfo)
	{
		_YXC_CHECK_REPORT_NEW_RET(uColIndex < this->_uNumBindCols, YXC_ERC_INDEX_OUT_OF_RANGE,
			YC("Rebind col : invalid col index %d"), uColIndex);

		_MySQLColBindInfo& rBindInfo = this->_pBindInfos[uColIndex];
		return this->_AssignColBindInfo(rBindInfo, *pBindInfo, uColIndex);
	}

	void _SQLStatMySQL::_FreeColBindInfos(yuint32_t uLastBindIndex)
	{
		for (yuint32_t i = 0; i <= uLastBindIndex && i < this->_uNumBindCols; ++i)
		{
			_MySQLColBindInfo& rDstInfo = this->_pBindInfos[i];
			if (rDstInfo.pMemory && rDstInfo.bAutoManage)
			{
				free(rDstInfo.pMemory);
			}
		}

		this->_uNumBindCols = 0;
	}

	YXC_Status _SQLStatMySQL::_CopyColBindInfo(_MySQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo,
		yuint32_t uIndex)
	{
		rDstInfo.byDataType = rSrcInfo.byDataType;
		if (rSrcInfo.pMemory == NULL)
		{
			yuint32_t uBufSize = rSrcInfo.uBufferSize;
			if (uBufSize == 0)
			{
				YXC_Status rc = _GetSqlDefBufSize_mysql(rDstInfo.byDataType, uBufSize);
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

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::_AssignColBindInfo(_MySQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo,
		yuint32_t uIndex)
	{
		_MySQLColBindInfo oldInfo = rDstInfo;

		rDstInfo.byDataType = rSrcInfo.byDataType;
		if (rSrcInfo.pMemory == NULL)
		{
			yuint32_t uColDef;
			YXC_Status rc = _GetMySqlType(rDstInfo.byDataType, rDstInfo.buffer_type, uColDef);
			_YXC_CHECK_RC_RET(rc);

			rDstInfo.uBufferSize = rSrcInfo.uBufferSize;
			if (rDstInfo.uBufferSize == 0)
			{
				rDstInfo.uBufferSize = uColDef;
			}

			if (oldInfo.bAutoManage && oldInfo.uBufferSize > rDstInfo.uBufferSize)
			{
				rDstInfo.uBufferSize = oldInfo.uBufferSize;
				rDstInfo.pMemory = oldInfo.pMemory;
			}
			else
			{
				rDstInfo.pMemory = ::malloc(rDstInfo.uBufferSize);
				_YXC_CHECK_REPORT_NEW_RET(rDstInfo.pMemory != NULL, YXC_ERC_OUT_OF_MEMORY, YC("Alloc memory for cell bind data failed, index = %u"), uIndex);
			}
		}
		else
		{
			rDstInfo.uBufferSize = rSrcInfo.uBufferSize;
			rDstInfo.bAutoManage = FALSE;
			rDstInfo.pMemory = rSrcInfo.pMemory;
		}

		YXC_Status rc = this->_BindAllCols();
		if (rc == YXC_ERC_SUCCESS)
		{
			if (oldInfo.bAutoManage && rDstInfo.pMemory != oldInfo.pMemory)
			{
				free(oldInfo.pMemory);
			}
		}
		else
		{
			if (rDstInfo.bAutoManage && rDstInfo.pMemory != NULL && rDstInfo.pMemory != oldInfo.pMemory)
			{
				free(rDstInfo.pMemory);
			}
			rDstInfo = oldInfo;
		}

		return rc;
	}

	YXC_Status _SQLStatMySQL::_BindAllCols()
	{
		_YCHK_MAL_ARR_R1(bindings, MYSQL_BIND, this->_uNumBindCols);
		YXCLib::HandleRef<void*> bindings_res(bindings, free);

		memset(bindings, 0, sizeof(MYSQL_BIND) * this->_uNumBindCols);

		for (yuint32_t i = 0; i < this->_uNumBindCols; ++i)
		{
			_MySQLColBindInfo& rDstInfo = this->_pBindInfos[i];

			yuint32_t uColDef;
			YXC_Status rc = _GetMySqlType(rDstInfo.byDataType, rDstInfo.buffer_type, uColDef);
			_YXC_CHECK_RC_RET(rc);

			bindings[i].buffer = rDstInfo.pMemory;
			bindings[i].buffer_type = rDstInfo.buffer_type;
			bindings[i].buffer_length = rDstInfo.uBufferSize;
			bindings[i].is_null = (my_bool*)&rDstInfo.is_null;
			bindings[i].length = &rDstInfo.length;

			this->_pCellsPtr[i].pCellData = rDstInfo.pMemory;
			this->_pCellsPtr[i].uBufferLen = rDstInfo.uBufferSize;
		}

		ybool_t bRet = mysql::_mysql_stmt_bind_result(this->_stmt, bindings);
		_YXC_CHECK_MYSQL_RET(bRet, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("_mysql_stmt_bind_result"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::FetchNextRow()
	{
		this->_offset0 = mysql::_mysql_stmt_row_tell(this->_stmt);

		int sql_ret = mysql::_mysql_stmt_fetch(this->_stmt);
		YXC_Status rc = this->_CheckFetchResult(sql_ret);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Fetch offset row failed"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::RefetchCurrentRow()
	{
		MYSQL_ROW_OFFSET new_offset = mysql::_mysql_stmt_row_seek(this->_stmt, this->_offset0);

		int sql_ret = mysql::_mysql_stmt_fetch(this->_stmt);
		YXC_Status rc = this->_CheckFetchResult(sql_ret);
		_YXC_CHECK_REPORT_RET(rc == YXC_ERC_SUCCESS, rc, YC("Fetch offset row failed"));

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::EndTrans(ybool_t bCommit)
	{
		if (bCommit)
		{
			ysize_t stAffected;
			return this->ExecuteDirect(L"COMMIT", &stAffected);
		}
		else
		{
			ysize_t stAffected;
			return this->ExecuteDirect(L"ROLLBACK", &stAffected);
		}
	}

	YXC_Status _SQLStatMySQL::_CheckFetchResult(int ret)
	{
		if (ret == MYSQL_NO_DATA)
		{
			_YXC_REPORT_NEW_ERR(YXC_ERC_NO_DATA, L"No more result to fetch");
			return YXC_ERC_NO_DATA;
		}

		int real_ret = ret;
		if (ret == MYSQL_DATA_TRUNCATED)
		{
			ret = 0;
		}

		_YXC_CHECK_MYSQL_RET(ret, _YXC_MYSQL_HDLTYPE_STMT, this->_stmt, YC("Fetch result is not successful"));
		YXC_Status rc;

		for (yuint32_t i = 0; i < this->_uNumBindCols; ++i)
		{
			yuint32_t uConverted, uLen;
			this->_pCellsPtr[i].bIsNullVal = this->_pBindInfos[i].is_null;
			if (!this->_pBindInfos[i].is_null)
			{
				switch (this->_pBindInfos[i].byDataType)
				{
				case YXC_DB_DATA_TYPE_ASTRING:
				case YXC_DB_DATA_TYPE_VASTRING:
				case YXC_DB_DATA_TYPE_LVASTRING:
				case YXC_DB_DATA_TYPE_WSTRING:
				case YXC_DB_DATA_TYPE_VWSTRING:
				case YXC_DB_DATA_TYPE_LVWSTRING:
					uLen = YXCLib::TMin<yuint32_t>(this->_pBindInfos[i].length, this->_pBindInfos[i].uBufferSize);
					rc = YXC_MMCMakeSureBufferOrFree(&this->_tempBuf, &this->_stTempBuf, uLen);
					_YXC_CHECK_RC_RET(rc);
					memcpy(this->_tempBuf, this->_pBindInfos[i].pMemory, uLen);

					if (this->_pBindInfos[i].byDataType == YXC_DB_DATA_TYPE_ASTRING || this->_pBindInfos[i].byDataType == YXC_DB_DATA_TYPE_VASTRING
						|| this->_pBindInfos[i].byDataType == YXC_DB_DATA_TYPE_LVASTRING)
					{
						rc = YXC_TEUTF8ToChar((yuint8_t*)this->_tempBuf, uLen, (char*)this->_pBindInfos[i].pMemory,
							this->_pBindInfos[i].uBufferSize / sizeof(char) - 1, &uConverted, NULL);
						_YXC_CHECK_RC_RET(rc);
						this->_pCellsPtr[i].uDataLen = uConverted * sizeof(char);
					}
					else
					{
						rc = YXC_TEUTF8ToWChar((yuint8_t*)this->_tempBuf, uLen, (wchar_t*)this->_pBindInfos[i].pMemory,
							this->_pBindInfos[i].uBufferSize / sizeof(wchar_t) - 1, &uConverted, NULL);
						_YXC_CHECK_RC_RET(rc);
						this->_pCellsPtr[i].uDataLen = uConverted * sizeof(wchar_t);
					}
					break;
				default:
					this->_pCellsPtr[i].uDataLen = this->_pBindInfos[i].length;
					break;
				}
			}
		}

		if (real_ret == MYSQL_DATA_TRUNCATED)
		{
			_YXC_REPORT_NEW_ERR(YXC_ERC_DATA_TRUNCATED, L"MYSQL_DATA_TRUNCATED");
			return YXC_ERC_DATA_TRUNCATED;
		}

		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::_ConvertInputBindings(ybyte_t byDataType, const void* pData, yssize_t stDataLen, enum_field_types* pType,
		void** ppNewBuf, ysize_t* pNewSize)
	{
		if (stDataLen == YXC_DB_NULL_DATA)
		{
			*pType = MYSQL_TYPE_NULL;
			*ppNewBuf = NULL;
			*pNewSize = 0;
			return YXC_ERC_SUCCESS;
		}

		enum_field_types eft;
		yuint32_t u;
		_GetMySqlType(byDataType, eft, u);
		yuint8_t* pBufX;
		switch (byDataType)
		{
		case YXC_DB_DATA_TYPE_ASTRING:
		case YXC_DB_DATA_TYPE_VASTRING:
		case YXC_DB_DATA_TYPE_LVASTRING:
			_YCHK_MAL_ARR_R2(pBufX, yuint8_t, stDataLen * 2 + 1);
			YXC_TECharToUTF8((char*)pData, stDataLen, pBufX, stDataLen * 2, &u, NULL);
			*ppNewBuf = pBufX;
			*pNewSize = u;
			break;
		case YXC_DB_DATA_TYPE_WSTRING:
		case YXC_DB_DATA_TYPE_VWSTRING:
		case YXC_DB_DATA_TYPE_LVWSTRING:
			_YCHK_MAL_ARR_R2(pBufX, yuint8_t, stDataLen * 2 + 1);
			YXC_TEWCharToUTF8((wchar_t*)pData, stDataLen / sizeof(wchar_t), pBufX, stDataLen * 2, &u, NULL);
			*ppNewBuf = pBufX;
			*pNewSize = u;
			break;
		default:
			*ppNewBuf = (void*)pData;
			*pNewSize = stDataLen;
			break;
		}

		*pType = eft;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _SQLStatMySQL::_ConvertOutputBindings()
	{
		yuint32_t uConverted;
		for (yuint32_t i = 0; i < this->_uNumParams; ++i)
		{
			if (this->_pBindingParams[i].bIsOutput)
			{
				YXC_Status rc = YXC_MMCMakeSureBufferOrFree(&this->_tempBuf, &this->_stTempBuf, this->_pBindingsEx[i].buffer_length);
				_YXC_CHECK_RC_RET(rc);

				memcpy(this->_tempBuf, this->_pBindingsEx[i].buffer, this->_pBindingsEx[i].length_value);
				rc = YXC_TEUTF8ToChar((yuint8_t*)this->_tempBuf, this->_pBindingsEx[i].length_value,
					(char*)this->_pBindingsEx[i].buffer, this->_pBindingsEx[i].buffer_length, &uConverted, NULL);
				_YXC_CHECK_RC_RET(rc);
			}
		}

		return YXC_ERC_SUCCESS;
	}

	void _SQLStatMySQL::_ClearBindingParameters()
	{
		for (yuint32_t i = 0; i < this->_uNumParams; ++i)
		{
			if (this->_pBindingParams[i].newData)
			{
				free(this->_pBindingParams[i].newData);
			}
		}
		this->_uNumParams = 0;
		this->_bRealBound = FALSE;
	}
}

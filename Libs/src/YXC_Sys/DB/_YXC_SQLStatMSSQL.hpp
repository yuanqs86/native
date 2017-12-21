#ifndef __INNER_INC_YXC_SYS_BASE_SQL_STAT_MSSQL_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_STAT_MSSQL_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_MSSQLCmn.hpp>

namespace YXC_Inner
{
	struct _MSSQLColBindInfo : public YXC_DBRTColBindInfo
	{
		SQLLEN strlenOrInd;
		ybool_t bAutoManage;
	};

	class _SQLStatMSSQL : public _SQLStatBase
	{
	public:
		_SQLStatMSSQL();

		~_SQLStatMSSQL();

	public:
		virtual YXC_Status Prepare(const wchar_t* pszStmt);

		virtual YXC_Status BindInputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, const void* pData);

		virtual YXC_Status BindOutputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData);

		virtual YXC_Status Execute(ysize_t* pstAffectedRows);

		virtual YXC_Status MoreResults(ysize_t* pstAffectedRows);

		virtual YXC_Status Reset();

		virtual YXC_Status CloseCursor();

		virtual YXC_Status ExecuteDirect(const wchar_t* pszStmt, ysize_t* pstAffectedRows);

		virtual YXC_Status QueryResultInfo(yuint32_t* puNumHeaders, YXC_DBRTColHeader* pHeaders);

		YXC_Status BindResultCols(yuint32_t uNumCols, const YXC_DBRTColBindInfo* pBindInfos,
			const YXC_DBRTCell** ppCellsPtr);

		virtual YXC_Status RebindResultCol(yuint32_t uColIndex, const YXC_DBRTColBindInfo* pRebindInfo);

		virtual YXC_Status FetchNextRow();

		virtual YXC_Status RefetchCurrentRow();

		virtual YXC_Status EndTrans(ybool_t bCommit);

	public:
		virtual YXC_Status FetchRow(yuint32_t uRowOffset);

	public:
		static YXC_Status CreateStatement(SQLHDBC dbc, SQLHSTMT hStmt, _SQLStatMSSQL** ppStat);

		static void FreeStatement(_SQLStatMSSQL* pStat);

	private:
		_SQLStatMSSQL(const _SQLStatMSSQL& rhs);

		_SQLStatMSSQL& operator =(const _SQLStatMSSQL& rhs);

	private:
		YXC_Status _CheckFetchResult(SQLRETURN ret);

		void _FreeColBindInfos(yuint32_t uLastBindIndex);

		YXC_Status _BindParameter(SQLSMALLINT uParamType, ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData);

		YXC_Status _CopyColBindInfo(_MSSQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo, yuint32_t uIndex);

		YXC_Status _AssignColBindInfo(_MSSQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo, yuint32_t uIndex);

		YXC_Status _BindCol(yuint32_t uIndex, _MSSQLColBindInfo& rDstInfo, SQLLEN* pStrlenOrInd);

		void _GetAffectedRowNum(ysize_t* pstAffectedRows);

	private:
		SQLHSTMT _stmt;

		SQLHDBC _dbc;

		_MSSQLColBindInfo* _pBindInfos;

		YXC_DBRTCell* _pCellsPtr;

		yuint32_t _uNumBindCols;

		yuint32_t _uNumBindColsBuf;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_STAT_MSSQL_HPP__ */

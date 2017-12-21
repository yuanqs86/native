#ifndef __INNER_INC_YXC_SYS_BASE_SQL_STAT_SQLITE_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_STAT_SQLITE_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_SQLiteCmn.hpp>

namespace YXC_Inner
{
	struct _SQLiteColBindInfo : public YXC_DBRTColBindInfo
	{
		ybool_t bAutoManage;
	};

	class _SQLStatSQLite : public _SQLStatBase
	{
	public:
		_SQLStatSQLite();

		~_SQLStatSQLite();

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
		static YXC_Status CreateStatement(const wchar_t* pszTarget, ybool_t bAutoCommit, _SQLStatSQLite** ppStat);

		static void FreeStatement(_SQLStatSQLite* pStat);

	private:
		_SQLStatSQLite(const _SQLStatSQLite& rhs);

		_SQLStatSQLite& operator =(const _SQLStatSQLite& rhs);

	private:
		void _FreeColBindInfos(yuint32_t uLastBindIndex);

		YXC_Status _CopyColBindInfo(_SQLiteColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo, yuint32_t uIndex);

		YXC_Status _AssignColBindInfo(_SQLiteColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo, yuint32_t uIndex);

		YXC_Status _BindCol(yuint32_t uIndex, _SQLiteColBindInfo& rDstInfo);

		void _GetAffectedRowNum(ysize_t* pstAffectedRows);

		YXC_Status _CopyBindingData();

		YXC_Status _CopyCol(YXC_DBRTCell* pCell, yuint32_t n);

		YXC_Status _CopyInteger(ybyte_t byDataType, YXC_DBRTCell* pCell, yint64_t iInteger);

		YXC_Status _CopyText(ybyte_t byDataType, YXC_DBRTCell* pCell, yuint32_t uDataLen, const yuint8_t* utf8);

		YXC_Status _CopyBlob(ybyte_t byDataType, YXC_DBRTCell* pCell, yuint32_t uDataLen, const void* blob);

		YXC_Status _ExecuteSql(const void* utf8);

	private:
		sqlite::sqlite3_stmt* _stmt;

		sqlite::sqlite3* _db;

		YXC_DBRTCell* _pCellsPtr;

		_SQLiteColBindInfo* _pBindInfos;

		yuint32_t _uNumBindCols;

		yuint32_t _uNumBindColsBuf;

		ybool_t _bAutoCommit;

		yuint32_t _uLastExecution;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_STAT_MSSQL_HPP__ */

#ifndef __INNER_INC_YXC_SYS_BASE_SQL_STAT_SQLITE_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_STAT_SQLITE_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_MySQLCmn.hpp>

namespace YXC_Inner
{
	struct _MySQLColBindInfo : public YXC_DBRTColBindInfo
	{
		ybool_t bAutoManage;
		ybool_t is_null;
		ybool_t error;
		unsigned long length;
		enum_field_types buffer_type;
	};

	struct _MySQLParamInfo
	{
		yssize_t* exPtr;
		ybool_t bIsOutput;
		void* newData;
	};

	class _SQLStatMySQL : public _SQLStatBase
	{
	public:
		_SQLStatMySQL();

		~_SQLStatMySQL();

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
		static YXC_Status CreateStatement(MYSQL* hdl, MYSQL_STMT* stmt, _SQLStatMySQL** ppStat);

		static void FreeStatement(_SQLStatMySQL* pStat);

	private:
		_SQLStatMySQL(const _SQLStatMySQL& rhs);

		_SQLStatMySQL& operator =(const _SQLStatMySQL& rhs);

	private:
		void _FreeColBindInfos(yuint32_t uLastBindIndex);

		YXC_Status _CopyColBindInfo(_MySQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo, yuint32_t uIndex);

		YXC_Status _AssignColBindInfo(_MySQLColBindInfo& rDstInfo, const YXC_DBRTColBindInfo& rSrcInfo, yuint32_t uIndex);

		YXC_Status _BindAllCols();

		YXC_Status _RealBindingParameters();

		void _ClearBindingParameters();

		YXC_Status _GetAffectedRowNum(ysize_t* pstAffectedRows);

		YXC_Status _CheckFetchResult(int sql_ret);

		YXC_Status _ConvertInputBindings(ybyte_t byDataType, const void* pData, yssize_t stDataLen, enum_field_types* pType,
			void** ppNewBuf, ysize_t* pNewSize);

		YXC_Status _ConvertOutputBindings();

	private:
		MYSQL_STMT* _stmt;

		MYSQL* _db;

		YXC_DBRTCell* _pCellsPtr;

		_MySQLColBindInfo* _pBindInfos;

		yuint32_t _uNumBindCols;

		yuint32_t _uNumBindColsBuf;

		ysize_t _stTempBuf;
		void* _tempBuf;

		_MySQLParamInfo* _pBindingParams;
		MYSQL_BIND* _pBindingsEx;
		yuint32_t _uNumParams;
		ysize_t _stCbParam;
		ysize_t _stCbEx;

		MYSQL_ROW_OFFSET _offset0;

		ybool_t _bRealBound;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_STAT_MSSQL_HPP__ */

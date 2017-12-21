#ifndef __INNER_INC_YXC_SYS_BASE_SQL_STAT_BASE_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_STAT_BASE_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXC_Inner
{
	class _SQLStatBase
	{
	public:
		_SQLStatBase();

		~_SQLStatBase();

	public:
		virtual YXC_Status Prepare(const wchar_t* pszStmt) = 0;

		virtual YXC_Status Execute(ysize_t* pstAffectedRows) = 0;

		virtual YXC_Status MoreResults(ysize_t* pstAffectedRows) = 0;

		virtual YXC_Status ExecuteDirect(const wchar_t* pszStmt, ysize_t* pstAffectedRows) = 0;

		virtual YXC_Status BindInputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, const void* pData) = 0;

		virtual YXC_Status BindOutputParameter(ybyte_t byDataType, yuint32_t uColIndex, yssize_t* pstDataLen, void* pData) = 0;

		virtual YXC_Status Reset() = 0;

		virtual YXC_Status CloseCursor() = 0;

		virtual YXC_Status QueryResultInfo(yuint32_t* puNumHeaders, YXC_DBRTColHeader* pHeaders) = 0;

		virtual YXC_Status BindResultCols(yuint32_t uNumCols, const YXC_DBRTColBindInfo* pBindInfos,
			const YXC_DBRTCell** ppCellsPtr) = 0;

		virtual YXC_Status RebindResultCol(yuint32_t uColIndex, const YXC_DBRTColBindInfo* pRebindInfo) = 0;

		virtual YXC_Status FetchNextRow() = 0;

		virtual YXC_Status RefetchCurrentRow() = 0;

		virtual YXC_Status EndTrans(ybool_t bCommit) = 0;

	private:
		_SQLStatBase(const _SQLStatBase& rhs);

		_SQLStatBase& operator =(const _SQLStatBase& rhs);
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_DBStat, _SQLStatBase, _SQLStatPtr, _SQLStatHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_STAT_BASE_HPP__ */

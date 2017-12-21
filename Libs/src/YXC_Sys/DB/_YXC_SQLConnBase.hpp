#ifndef __INNER_INC_YXC_SYS_BASE_SQL_CONN_BASE_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_CONN_BASE_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_SQLStatBase.hpp>
#include <YXC_Sys/YXC_UtilMacros.hpp>

namespace YXC_Inner
{
	class _SQLConnBase
	{
	public:
		_SQLConnBase();

		virtual ~_SQLConnBase();

	private:
		_SQLConnBase(const _SQLConnBase& rhs);

		_SQLConnBase& operator =(const _SQLConnBase& rhs);

	public:
		virtual YXC_Status Init(ybool_t bAutoCommit) = 0;

		virtual YXC_Status ConnectToDBServer(
			const wchar_t* cpszTarget,
			yuint32_t uTargetPort,
			const wchar_t* cpszDriver,
			const wchar_t* cpszInitDB,
			const wchar_t* cpszUser,
			const wchar_t* cpszPwd
		) = 0;

		virtual YXC_Status AllocStatement(_SQLStatBase** ppStat) = 0;

		virtual void FreeStatement(_SQLStatBase* pStat) = 0;

		virtual void Free() = 0;
	};

	YXC_DECLARE_HANDLE_HP_TRANSFER(YXC_DBConn, _SQLConnBase, _SQLConnPtr, _SQLConnHdl);
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_CONN_BASE_HPP__ */

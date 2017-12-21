#ifndef __INNER_INC_YXC_SYS_BASE_SQL_CONN_SQLITE_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_CONN_SQLITE_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_SQLConnBase.hpp>
#include <YXC_Sys/DB/_YXC_SQLiteCmn.hpp>

namespace YXC_Inner
{
	class _SQLConnSQLite : public _SQLConnBase
	{
	public:
		_SQLConnSQLite();

		virtual ~_SQLConnSQLite();

	private:
		_SQLConnSQLite(const _SQLConnSQLite& rhs);

		_SQLConnSQLite& operator =(const _SQLConnSQLite& rhs);

	public:
		virtual YXC_Status Init(ybool_t bAutoCommit);

		virtual YXC_Status ConnectToDBServer(
			const wchar_t* cpszTarget,
			yuint32_t uTargetPort,
			const wchar_t* cpszDriver,
			const wchar_t* cpszInitDB,
			const wchar_t* cpszUser,
			const wchar_t* cpszPwd
		);

		virtual YXC_Status AllocStatement(_SQLStatBase** ppStat);

		virtual void FreeStatement(_SQLStatBase* pStat);

		virtual void Free();

	private:
		wchar_t* _dbName;

		ybool_t _bAutoCommit;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_CONN_SQLITE_HPP__ */

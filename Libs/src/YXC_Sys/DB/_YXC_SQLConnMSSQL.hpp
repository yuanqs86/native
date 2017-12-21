#ifndef __INNER_INC_YXC_SYS_BASE_SQL_CONN_MSSQL_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_CONN_MSSQL_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_SQLConnBase.hpp>

#include <sql.h>
#include <sqlext.h>

namespace YXC_Inner
{
	class _SQLConnMSSQL : public _SQLConnBase
	{
	public:
		_SQLConnMSSQL();

		virtual ~_SQLConnMSSQL();

	private:
		_SQLConnMSSQL(const _SQLConnMSSQL& rhs);

		_SQLConnMSSQL& operator =(const _SQLConnMSSQL& rhs);

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
		SQLHDBC _hDbc;

		SQLHENV _hEnv;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_CONN_MSSQL_HPP__ */

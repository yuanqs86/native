#ifndef __INNER_INC_YXC_SYS_BASE_SQL_CONN_MYSQL_HPP__
#define __INNER_INC_YXC_SYS_BASE_SQL_CONN_MYSQL_HPP__

#include <YXC_Sys/YXC_Database.h>
#include <YXC_Sys/DB/_YXC_SQLConnBase.hpp>
#include <YXC_Sys/DB/_YXC_MySQLCmn.hpp>

namespace YXC_Inner
{
	class _SQLConnMySQL : public _SQLConnBase
	{
	public:
		_SQLConnMySQL();

		virtual ~_SQLConnMySQL();

	private:
		_SQLConnMySQL(const _SQLConnMySQL& rhs);

		_SQLConnMySQL& operator =(const _SQLConnMySQL& rhs);

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
		MYSQL* _mysql;

		ybool_t _bAutoCommit;
	};
}

#endif /* __INNER_INC_YXC_SYS_BASE_SQL_CONN_MYSQL_HPP__ */

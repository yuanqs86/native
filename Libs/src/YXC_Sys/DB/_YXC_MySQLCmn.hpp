#ifndef __INNER_INC_YXC_SYS_BASE_MYSQL_COMMON_HPP
#define __INNER_INC_YXC_SYS_BASE_MYSQL_COMMON_HPP

#include <YXC_Sys/YXC_Database.h>
#include <mysql.h>

namespace mysql
{
	typedef MYSQL* (__stdcall *mysql_init_func)(MYSQL*);
	typedef MYSQL* (__stdcall *mysql_real_connect_func)(MYSQL* mysql, const char* host, const char* usr, const char* passwd, const char* db,
		unsigned int port, const char* unix_socket, unsigned long clientflag);
	typedef int (__stdcall *mysql_close_func)(MYSQL*);

	typedef int (__stdcall *mysql_query_func)(MYSQL*, const char *sql);
	typedef yuint64_t (__stdcall *mysql_insert_id_func)(MYSQL*);
	typedef int (__stdcall *mysql_errno_func)(MYSQL *db);
	typedef const char* (__stdcall *mysql_error_func)(MYSQL *db);
	typedef MYSQL_RES* (__stdcall *mysql_store_result_func)(MYSQL *mysql);
	typedef	void (__stdcall *mysql_free_result)(MYSQL_RES *result);
	typedef int	(__stdcall *mysql_fetch_row_func)(MYSQL_RES *result);
	typedef int	(__stdcall *mysql_fetch_field_func)(MYSQL_RES *result);

	typedef MYSQL_STMT* (__stdcall* mysql_stmt_init_func)(MYSQL *mysql);
	typedef int (__stdcall* mysql_stmt_prepare_func)(MYSQL_STMT *stmt, const char *query, unsigned long length);
	typedef int (__stdcall* mysql_stmt_execute_func)(MYSQL_STMT *stmt);
	typedef int (__stdcall* mysql_stmt_fetch_func)(MYSQL_STMT* stmt);
	typedef yint64_t (__stdcall* mysql_stmt_affected_rows_func)(MYSQL_STMT *stmt);
	typedef int (__stdcall* mysql_stmt_store_result_func)(MYSQL_STMT *stmt);
	typedef MYSQL_RES* (__stdcall* mysql_stmt_result_metadata_func)(MYSQL_STMT *stmt);
	typedef ybool_t (__stdcall* mysql_stmt_bind_param_func)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
	typedef ybool_t (__stdcall* mysql_stmt_bind_result_func)(MYSQL_STMT * stmt, MYSQL_BIND * bnd);
	typedef ybool_t (__stdcall* mysql_stmt_close_func)(MYSQL_STMT * stmt);
	typedef ybool_t (__stdcall* mysql_stmt_reset_func)(MYSQL_STMT * stmt);
	typedef MYSQL_ROW_OFFSET (__stdcall* mysql_stmt_row_seek_func)(MYSQL_STMT *stmt, MYSQL_ROW_OFFSET offset);
	typedef MYSQL_ROW_OFFSET (__stdcall* mysql_stmt_row_tell_func)(MYSQL_STMT *stmt);
	typedef yint64_t (__stdcall* mysql_stmt_insert_id_func)(MYSQL_STMT* stmt);
	typedef unsigned int (__stdcall* mysql_stmt_field_count_func)(MYSQL_STMT* stmt);
	typedef unsigned int (__stdcall* mysql_stmt_errno_func)(MYSQL_STMT* stmt);
	typedef const char* (__stdcall* mysql_stmt_error_func)(MYSQL_STMT* stmt);
	//void STDCALL mysql_stmt_data_seek(MYSQL_STMT *stmt, my_ulonglong offset);
	//my_ulonglong STDCALL mysql_stmt_num_rows(MYSQL_STMT *stmt);

	typedef ybool_t (__stdcall* mysql_commit_func)(MYSQL* mysql);
	typedef ybool_t (__stdcall* mysql_rollback_func)(MYSQL* mysql);
	typedef int (__stdcall* mysql_autocommit_func)(MYSQL* mysql, ybool_t auto_mode);
	typedef int (__stdcall* mysql_more_results_func)(MYSQL* mysql);
	typedef int (__stdcall* mysql_next_result_func)(MYSQL* mysql);
	typedef int (__stdcall* mysql_stmt_next_result_func)(MYSQL_STMT* stmt);

	extern mysql_init_func _mysql_init;
	extern mysql_real_connect_func _mysql_real_connect;
	extern mysql_close_func _mysql_close;

	extern mysql_query_func _mysql_query;
	extern mysql_insert_id_func _mysql_insert_id;
	extern mysql_errno_func _mysql_errno;
	extern mysql_error_func _mysql_error;
	extern mysql_fetch_row_func _mysql_fetch_row;
	extern mysql_fetch_field_func _mysql_fetch_field;
	extern mysql_store_result_func _mysql_store_result;
	extern mysql_free_result _mysql_free_result;

	extern mysql_stmt_init_func _mysql_stmt_init;
	extern mysql_stmt_prepare_func _mysql_stmt_prepare;
	extern mysql_stmt_execute_func _mysql_stmt_execute;
	extern mysql_stmt_fetch_func _mysql_stmt_fetch;
	extern mysql_stmt_affected_rows_func _mysql_stmt_affected_rows;
	extern mysql_stmt_store_result_func _mysql_stmt_store_result;
	extern mysql_stmt_result_metadata_func _mysql_stmt_result_metadata;
	extern mysql_stmt_bind_param_func _mysql_stmt_bind_param;
	extern mysql_stmt_bind_result_func _mysql_stmt_bind_result;
	extern mysql_stmt_close_func _mysql_stmt_close;
	extern mysql_stmt_reset_func _mysql_stmt_reset;
	extern mysql_stmt_row_seek_func _mysql_stmt_row_seek;
	extern mysql_stmt_row_tell_func _mysql_stmt_row_tell;
	extern mysql_stmt_insert_id_func _mysql_stmt_insert_id;
	extern mysql_stmt_field_count_func _mysql_stmt_field_count;
	extern mysql_stmt_errno_func _mysql_stmt_errno;
	extern mysql_stmt_error_func _mysql_stmt_error;

	extern mysql_commit_func _mysql_commit;
	extern mysql_rollback_func _mysql_rollback;
	extern mysql_autocommit_func _mysql_autocommit;
	extern mysql_more_results_func _mysql_more_results;
	extern mysql_next_result_func _mysql_next_result;
	extern mysql_stmt_next_result_func _mysql_stmt_next_result;
}

#define _YXC_MYSQL_HDLTYPE_HDBC 0
#define _YXC_MYSQL_HDLTYPE_STMT 1

namespace YXC_Inner
{
	YXC_Status _GetSqlDefBufSize_mysql(ybyte_t byDataType, yuint32_t& uBufSize);

	void _GetTypeFromMySQLType(enum_field_types sqlType, ybyte_t& myType);

	YXC_Status _GetMySqlType(ybyte_t byDataType, enum_field_types& rSQLType, yuint32_t& uColDef);

	YXC_Status _ReportMySQLError(int sql_ret, int hdl_type, void* hdl, const wchar_t* cpszModule, const wchar_t* cpszFile,
		int iLine, const wchar_t* cpszFunc, const wchar_t* cpszMsg, ...);
}

#define _YXC_MYSQL_REPORT(sql_ret, hdl_type, hdl, wszMsg, ...) YXC_Inner::_ReportMySQLError(sql_ret, hdl_type, hdl, __EMODULE__, __EFILE__, __LINE__, \
	__EFUNCTION__, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_MYSQL_RET(sql_proc, hdl_type, hdl, wszMsg, ...)						\
	do {																				\
		int x_sql_ret = (sql_proc);														\
		if (x_sql_ret != 0) {															\
			return _YXC_MYSQL_REPORT(x_sql_ret, hdl_type, hdl, wszMsg, ##__VA_ARGS__);		\
		}																				\
	} while (0)

#define _YXC_CHECK_MYSQL_GOTO(sql_proc, hdl_type, hdl, wszMsg, ...)								\
	do {																						\
		int x_sql_ret = (sql_proc);																\
		if (x_sql_ret != 0) {																	\
			__VARIABLE_RC__ = _YXC_MYSQL_REPORT(x_sql_ret, hdl_type, hdl, wszMsg, ##__VA_ARGS__);	\
			goto __LABEL__;																		\
		}																						\
	} while (0)

#define _YXC_CHECK_MYSQL_RETB(sql_proc, hdl_type, hdl, wszMsg, ...)						\
	do {																				\
		int x_sql_ret = (sql_proc);														\
		if (!x_sql_ret) {																\
			return _YXC_MYSQL_REPORT(x_sql_ret, hdl_type, hdl, wszMsg, ##__VA_ARGS__);		\
		}																				\
	} while (0)

#endif /* __INNER_INC_YXC_SYS_BASE_MYSQL_COMMON_HPP */

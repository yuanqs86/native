#define __MODULE__ "EK.DB.MYSQL"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/DB/_YXC_MySQLCmn.hpp>
#include <stdlib.h>

#define _SQL_LOAD_FUNC(_funcPtr, _module, _funcName)		\
	do {													\
		FARPROC p = GetProcAddress(_module, _funcName);		\
		_YXC_FATAL_ASSERT(p != NULL);						\
		memcpy(&_funcPtr, &p, sizeof(FARPROC));				\
	} while (0)

namespace mysql
{
	static HMODULE gs_mysql_module = NULL;
	void _InitMySQL()
	{
		gs_mysql_module = LoadLibraryA("libmysql.dll");
		_YXC_FATAL_ASSERT(gs_mysql_module != NULL);

		_SQL_LOAD_FUNC(_mysql_init, gs_mysql_module, "mysql_init");
		_SQL_LOAD_FUNC(_mysql_real_connect, gs_mysql_module, "mysql_real_connect");
		_SQL_LOAD_FUNC(_mysql_close, gs_mysql_module, "mysql_close");
		_SQL_LOAD_FUNC(_mysql_query, gs_mysql_module, "mysql_query");
		_SQL_LOAD_FUNC(_mysql_insert_id, gs_mysql_module, "mysql_insert_id");
		_SQL_LOAD_FUNC(_mysql_errno, gs_mysql_module, "mysql_errno");
		_SQL_LOAD_FUNC(_mysql_error, gs_mysql_module, "mysql_error");
		_SQL_LOAD_FUNC(_mysql_fetch_row, gs_mysql_module, "mysql_fetch_row");
		_SQL_LOAD_FUNC(_mysql_fetch_field, gs_mysql_module, "mysql_fetch_field");
		_SQL_LOAD_FUNC(_mysql_store_result, gs_mysql_module, "mysql_store_result");
		_SQL_LOAD_FUNC(_mysql_free_result, gs_mysql_module, "mysql_free_result");
		_SQL_LOAD_FUNC(_mysql_stmt_init, gs_mysql_module, "mysql_stmt_init");
		_SQL_LOAD_FUNC(_mysql_stmt_prepare, gs_mysql_module, "mysql_stmt_prepare");
		_SQL_LOAD_FUNC(_mysql_stmt_execute, gs_mysql_module, "mysql_stmt_execute");
		_SQL_LOAD_FUNC(_mysql_stmt_fetch, gs_mysql_module, "mysql_stmt_fetch");
		_SQL_LOAD_FUNC(_mysql_stmt_affected_rows, gs_mysql_module, "mysql_stmt_affected_rows");
		_SQL_LOAD_FUNC(_mysql_stmt_store_result, gs_mysql_module, "mysql_stmt_store_result");
		_SQL_LOAD_FUNC(_mysql_stmt_result_metadata, gs_mysql_module, "mysql_stmt_result_metadata");
		_SQL_LOAD_FUNC(_mysql_stmt_bind_param, gs_mysql_module, "mysql_stmt_bind_param");
		_SQL_LOAD_FUNC(_mysql_stmt_bind_result, gs_mysql_module, "mysql_stmt_bind_result");
		_SQL_LOAD_FUNC(_mysql_stmt_close, gs_mysql_module, "mysql_stmt_close");
		_SQL_LOAD_FUNC(_mysql_stmt_reset, gs_mysql_module, "mysql_stmt_reset");
		_SQL_LOAD_FUNC(_mysql_stmt_row_tell, gs_mysql_module, "mysql_stmt_row_tell");
		_SQL_LOAD_FUNC(_mysql_stmt_row_seek, gs_mysql_module, "mysql_stmt_row_seek");
		_SQL_LOAD_FUNC(_mysql_stmt_insert_id, gs_mysql_module, "mysql_stmt_insert_id");
		_SQL_LOAD_FUNC(_mysql_stmt_field_count, gs_mysql_module, "mysql_stmt_field_count");
		_SQL_LOAD_FUNC(_mysql_stmt_errno, gs_mysql_module, "mysql_stmt_errno");
		_SQL_LOAD_FUNC(_mysql_stmt_error, gs_mysql_module, "mysql_stmt_error");
		_SQL_LOAD_FUNC(_mysql_commit, gs_mysql_module, "mysql_commit");
		_SQL_LOAD_FUNC(_mysql_rollback, gs_mysql_module, "mysql_rollback");
		_SQL_LOAD_FUNC(_mysql_autocommit, gs_mysql_module, "mysql_autocommit");
		_SQL_LOAD_FUNC(_mysql_more_results, gs_mysql_module, "mysql_more_results");
		_SQL_LOAD_FUNC(_mysql_next_result, gs_mysql_module, "mysql_next_result");
		_SQL_LOAD_FUNC(_mysql_stmt_next_result, gs_mysql_module, "mysql_stmt_next_result");
	}

	mysql_init_func _mysql_init;
	mysql_real_connect_func _mysql_real_connect;
	mysql_close_func _mysql_close;

	mysql_query_func _mysql_query;
	mysql_insert_id_func _mysql_insert_id;
	mysql_errno_func _mysql_errno;
	mysql_error_func _mysql_error;
	mysql_fetch_row_func _mysql_fetch_row;
	mysql_fetch_field_func _mysql_fetch_field;
	mysql_store_result_func _mysql_store_result;
	mysql_free_result _mysql_free_result;

	mysql_stmt_init_func _mysql_stmt_init;
	mysql_stmt_prepare_func _mysql_stmt_prepare;
	mysql_stmt_execute_func _mysql_stmt_execute;
	mysql_stmt_fetch_func _mysql_stmt_fetch;
	mysql_stmt_affected_rows_func _mysql_stmt_affected_rows;
	mysql_stmt_store_result_func _mysql_stmt_store_result;
	mysql_stmt_result_metadata_func _mysql_stmt_result_metadata;
	mysql_stmt_bind_param_func _mysql_stmt_bind_param;
	mysql_stmt_bind_result_func _mysql_stmt_bind_result;
	mysql_stmt_close_func _mysql_stmt_close;
	mysql_stmt_reset_func _mysql_stmt_reset;
	mysql_stmt_row_seek_func _mysql_stmt_row_seek;
	mysql_stmt_row_tell_func _mysql_stmt_row_tell;
	mysql_stmt_insert_id_func _mysql_stmt_insert_id;
	mysql_stmt_field_count_func _mysql_stmt_field_count;
	mysql_stmt_errno_func _mysql_stmt_errno;
	mysql_stmt_error_func _mysql_stmt_error;

	mysql_commit_func _mysql_commit;
	mysql_rollback_func _mysql_rollback;
	mysql_autocommit_func _mysql_autocommit;
	mysql_more_results_func _mysql_more_results;
	mysql_next_result_func _mysql_next_result;
	mysql_stmt_next_result_func _mysql_stmt_next_result;
}

namespace
{
	struct MySqlColTypeDesc
	{
		enum_field_types mySqlType;
		ybool_t bFixedSizeType;
		yuint32_t uDefBufSize;
	};

	static MySqlColTypeDesc gs_colHeaderDescs[] = {
		{ MYSQL_TYPE_TINY, TRUE, sizeof(yint8_t) },
		{ MYSQL_TYPE_SHORT, TRUE, sizeof(yint16_t) },
		{ MYSQL_TYPE_LONG, TRUE, sizeof(yint32_t) },
		{ MYSQL_TYPE_LONGLONG, TRUE, sizeof(yint64_t) },
		{ MYSQL_TYPE_TINY, TRUE, sizeof(yuint8_t) },
		{ MYSQL_TYPE_SHORT, TRUE, sizeof(yuint16_t) },
		{ MYSQL_TYPE_LONG, TRUE, sizeof(yuint32_t) },
		{ MYSQL_TYPE_LONGLONG, TRUE, sizeof(yuint64_t) },
		{ MYSQL_TYPE_BIT, TRUE, sizeof(ybool_t) },
		{ MYSQL_TYPE_FLOAT, TRUE, sizeof(float) },
		{ MYSQL_TYPE_DOUBLE, TRUE, sizeof(double) },
		{ MYSQL_TYPE_STRING, TRUE, 256 * sizeof(char) },
		{ MYSQL_TYPE_VAR_STRING, FALSE, 256 * sizeof(char) },
		{ MYSQL_TYPE_VAR_STRING, FALSE, 4096 * sizeof(char),  },
		{ MYSQL_TYPE_STRING, TRUE, 256 * sizeof(wchar_t) },
		{ MYSQL_TYPE_VAR_STRING, FALSE, 256 * sizeof(wchar_t) },
		{ MYSQL_TYPE_VAR_STRING, FALSE, 4096 * sizeof(wchar_t) },
		{ MYSQL_TYPE_BLOB, TRUE, 1024 },
		{ MYSQL_TYPE_BLOB, FALSE, 1024, },
		{ MYSQL_TYPE_BLOB, FALSE, 32768 },
		{ MYSQL_TYPE_DATE, TRUE, sizeof(MYSQL_TIME) },
		{ MYSQL_TYPE_TIME, TRUE, sizeof(MYSQL_TIME) },
		{ MYSQL_TYPE_TIMESTAMP, TRUE, sizeof(MYSQL_TIME) },
		{ MYSQL_TYPE_DATETIME, TRUE, sizeof(MYSQL_TIME) },
	};
}

namespace YXC_Inner
{
	YXC_Status _GetSqlDefBufSize_mysql(ybyte_t byDataType, yuint32_t& uBufSize)
	{
		_YXC_CHECK_REPORT_NEW_RET(byDataType <= YXC_DB_DATA_TYPE_MAX, YXC_ERC_INVALID_PARAMETER, YC("Invalid data type to switch"));

		uBufSize = gs_colHeaderDescs[byDataType].uDefBufSize;
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _GetMySqlType(ybyte_t byDataType, enum_field_types& rSQLType, yuint32_t& uColDef)
	{
		_YXC_CHECK_REPORT_NEW_RET(byDataType <= YXC_DB_DATA_TYPE_MAX, YXC_ERC_INVALID_PARAMETER, YC("Invalid data type to switch"));

		MySqlColTypeDesc& rDesc = gs_colHeaderDescs[byDataType];

		rSQLType = rDesc.mySqlType;
		uColDef = rDesc.uDefBufSize;

		return YXC_ERC_SUCCESS;
	}

	void _GetTypeFromMySQLType(enum_field_types sqlType, ybyte_t& myType)
	{
		switch (sqlType)
		{
		case MYSQL_TYPE_BIT:
		case MYSQL_TYPE_TINY:
			myType = YXC_DB_DATA_TYPE_I8;
			break;
		case MYSQL_TYPE_SHORT:
			myType = YXC_DB_DATA_TYPE_I16;
			break;
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
			myType = YXC_DB_DATA_TYPE_I32;
			break;
		case MYSQL_TYPE_LONGLONG:
			myType = YXC_DB_DATA_TYPE_I64;
			break;
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
			myType = YXC_DB_DATA_TYPE_VBINARY;
		case MYSQL_TYPE_LONG_BLOB:
			myType = YXC_DB_DATA_TYPE_LVBINARY;
			break;
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_VAR_STRING:
			myType = YXC_DB_DATA_TYPE_VASTRING;
			break;
		case MYSQL_TYPE_STRING:
			myType = YXC_DB_DATA_TYPE_ASTRING;
			break;
		case MYSQL_TYPE_FLOAT:
			myType = YXC_DB_DATA_TYPE_F32;
			break;
		case MYSQL_TYPE_DOUBLE:
			myType = YXC_DB_DATA_TYPE_F64;
			break;
		default:
			myType = -1;
			break;
		}
	}

	YXC_Status _ReportMySQLError(int sql_ret, int hdl_type, void* hdl, const wchar_t* cpszModule, const wchar_t* cpszFile,
		int iLine, const wchar_t* cpszFunc, const wchar_t* cpszMsg, ...)
	{
		wchar_t szMessage[YXC_BASE_ERROR_BUFFER] = {0};
		wchar_t szMessage2[2 * YXC_BASE_ERROR_BUFFER];

		va_list vl;
		va_start(vl, cpszMsg);
		swprintf(szMessage, cpszMsg, vl);
		va_end(vl);

		int err_code = sql_ret;
		if (hdl_type == _YXC_MYSQL_HDLTYPE_HDBC)
		{
			const char* mysql_err = mysql::_mysql_error((MYSQL*)hdl);
			err_code = mysql::_mysql_errno((MYSQL*)hdl);
			swprintf(szMessage2, L"MYSQL error %S", mysql_err);
		}
		else
		{
			const char* mysql_err = mysql::_mysql_stmt_error((MYSQL_STMT*)hdl);
			err_code = mysql::_mysql_stmt_errno((MYSQL_STMT*)hdl);
			swprintf(szMessage2, L"MYSQL statement error %S", mysql_err);
		}

		YXC_Status rc = YXC_ReportErrorExFormat(TRUE, YXC_ERROR_SRC_SQL, YXC_ERR_CAT_SQL_MYSQL, err_code, cpszModule, cpszFile, iLine,
			cpszFunc, NULL, 0, L"%s [%s]", szMessage, szMessage2);
		return YXC_ERC_SQL;
	}
}

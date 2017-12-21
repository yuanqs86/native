#define __MODULE__ "EK.DB.SQLITE"

#include <YXC_Sys/YXC_ErrMacros.hpp>
#include <YXC_Sys/YXC_StrUtil.hpp>
#include <YXC_Sys/DB/_YXC_SQLiteCmn.hpp>
#include <stdlib.h>

#define _SQL_LOAD_FUNC(_funcPtr, _module, _funcName)		\
	do {													\
		FARPROC p = GetProcAddress(_module, _funcName);		\
		_YXC_FATAL_ASSERT(p != NULL);						\
		memcpy(&_funcPtr, &p, sizeof(FARPROC));				\
	} while (0)

namespace sqlite
{
	static HMODULE gs_sqlite_module = NULL;
	void _InitSQLite()
	{
		gs_sqlite_module = LoadLibraryA("sqlite3.dll");
		if (gs_sqlite_module != NULL)
		{
			_SQL_LOAD_FUNC(_sqlite3_open, gs_sqlite_module, "sqlite3_open");
			_SQL_LOAD_FUNC(_sqlite3_close, gs_sqlite_module, "sqlite3_close");
			_SQL_LOAD_FUNC(_sqlite3_init, gs_sqlite_module, "sqlite3_initialize");
			_SQL_LOAD_FUNC(_sqlite3_last_insert_rowid, gs_sqlite_module, "sqlite3_last_insert_rowid");
			_SQL_LOAD_FUNC(_sqlite3_changes, gs_sqlite_module, "sqlite3_changes");
			_SQL_LOAD_FUNC(_sqlite3_errcode, gs_sqlite_module, "sqlite3_errcode");
			_SQL_LOAD_FUNC(_sqlite3_extended_errcode, gs_sqlite_module, "sqlite3_extended_errcode");
			_SQL_LOAD_FUNC(_sqlite3_errmsg, gs_sqlite_module, "sqlite3_errmsg");
			_SQL_LOAD_FUNC(_sqlite3_errstr, gs_sqlite_module, "sqlite3_errstr");
			_SQL_LOAD_FUNC(_sqlite3_exec, gs_sqlite_module, "sqlite3_exec");
			_SQL_LOAD_FUNC(_sqlite3_prepare, gs_sqlite_module, "sqlite3_prepare");
			_SQL_LOAD_FUNC(_sqlite3_step, gs_sqlite_module, "sqlite3_step");
			_SQL_LOAD_FUNC(_sqlite3_data_count, gs_sqlite_module, "sqlite3_data_count");
			_SQL_LOAD_FUNC(_sqlite3_reset, gs_sqlite_module, "sqlite3_reset");
			_SQL_LOAD_FUNC(_sqlite3_finalize, gs_sqlite_module, "sqlite3_finalize");
			_SQL_LOAD_FUNC(_sqlite3_clear_bindings, gs_sqlite_module, "sqlite3_clear_bindings");
			_SQL_LOAD_FUNC(_sqlite3_bind_blob, gs_sqlite_module, "sqlite3_bind_blob");
			_SQL_LOAD_FUNC(_sqlite3_bind_double, gs_sqlite_module, "sqlite3_bind_double");
			_SQL_LOAD_FUNC(_sqlite3_bind_int, gs_sqlite_module, "sqlite3_bind_int");
			_SQL_LOAD_FUNC(_sqlite3_bind_int64, gs_sqlite_module, "sqlite3_bind_int64");
			_SQL_LOAD_FUNC(_sqlite3_bind_null, gs_sqlite_module, "sqlite3_bind_null");
			_SQL_LOAD_FUNC(_sqlite3_bind_text, gs_sqlite_module, "sqlite3_bind_text");
			_SQL_LOAD_FUNC(_sqlite3_bind_text16, gs_sqlite_module, "sqlite3_bind_text16");
			_SQL_LOAD_FUNC(_sqlite3_bind_value, gs_sqlite_module, "sqlite3_bind_value");
			_SQL_LOAD_FUNC(_sqlite3_bind_zeroblob, gs_sqlite_module, "sqlite3_bind_zeroblob");
			_SQL_LOAD_FUNC(_sqlite3_column_count, gs_sqlite_module, "sqlite3_column_count");
			_SQL_LOAD_FUNC(_sqlite3_column_name, gs_sqlite_module, "sqlite3_column_name");
			_SQL_LOAD_FUNC(_sqlite3_column_type, gs_sqlite_module, "sqlite3_column_type");
			_SQL_LOAD_FUNC(_sqlite3_column_decltype, gs_sqlite_module, "sqlite3_column_decltype");
			_SQL_LOAD_FUNC(_sqlite3_column_value, gs_sqlite_module, "sqlite3_column_value");
			_SQL_LOAD_FUNC(_sqlite3_value_type, gs_sqlite_module, "sqlite3_value_type");
			_SQL_LOAD_FUNC(_sqlite3_value_numeric_type, gs_sqlite_module, "sqlite3_value_numeric_type");
			_SQL_LOAD_FUNC(_sqlite3_value_bytes, gs_sqlite_module, "sqlite3_value_bytes");
			_SQL_LOAD_FUNC(_sqlite3_value_bytes16, gs_sqlite_module, "sqlite3_value_bytes16");
			_SQL_LOAD_FUNC(_sqlite3_value_double, gs_sqlite_module, "sqlite3_value_double");
			_SQL_LOAD_FUNC(_sqlite3_value_int, gs_sqlite_module, "sqlite3_value_int");
			_SQL_LOAD_FUNC(_sqlite3_value_int64, gs_sqlite_module, "sqlite3_value_int64");
			_SQL_LOAD_FUNC(_sqlite3_value_text, gs_sqlite_module, "sqlite3_value_text");
			_SQL_LOAD_FUNC(_sqlite3_value_text16, gs_sqlite_module, "sqlite3_value_text16");
			_SQL_LOAD_FUNC(_sqlite3_value_blob, gs_sqlite_module, "sqlite3_value_blob");
		}
	}

	sqlite3_open_func _sqlite3_open;
	sqlite3_close_func _sqlite3_close;
	sqlite3_initialize_func _sqlite3_init;
	sqlite3_last_insert_rowid_func _sqlite3_last_insert_rowid;
	sqlite3_changes_func _sqlite3_changes;
	sqlite3_errcode_func _sqlite3_errcode;
	sqlite3_extended_errcode_func _sqlite3_extended_errcode;
	sqlite3_errmsg_func _sqlite3_errmsg;
	sqlite3_errstr_func _sqlite3_errstr;
	sqlite3_prepare_func _sqlite3_prepare;
	sqlite3_exec_func _sqlite3_exec;

	sqlite3_step_func _sqlite3_step;
	sqlite3_data_count_func _sqlite3_data_count;
	sqlite3_reset_func _sqlite3_reset;
	sqlite3_finalize_func _sqlite3_finalize;
	sqlite3_clear_bindings_func _sqlite3_clear_bindings;

	sqlite3_bind_blob_func _sqlite3_bind_blob;
	sqlite3_bind_double_func _sqlite3_bind_double;
	sqlite3_bind_int_func _sqlite3_bind_int;
	sqlite3_bind_int64_func _sqlite3_bind_int64;
	sqlite3_bind_null_func _sqlite3_bind_null;
	sqlite3_bind_text_func _sqlite3_bind_text;
	sqlite3_bind_text16_func _sqlite3_bind_text16;
	sqlite3_bind_value_func _sqlite3_bind_value;
	sqlite3_bind_zeroblob_func _sqlite3_bind_zeroblob;

	sqlite3_column_count_func _sqlite3_column_count;
	sqlite3_column_name_func _sqlite3_column_name;
	sqlite3_column_type_func _sqlite3_column_type;
	sqlite3_column_decltype_func _sqlite3_column_decltype;
	sqlite3_column_value_func _sqlite3_column_value;

	sqlite3_value_type_func _sqlite3_value_type;
	sqlite3_value_numeric_type_func _sqlite3_value_numeric_type;
	sqlite3_value_blob_func _sqlite3_value_blob;
	sqlite3_value_bytes_func _sqlite3_value_bytes;
	sqlite3_value_bytes16_func _sqlite3_value_bytes16;
	sqlite3_value_double_func _sqlite3_value_double;
	sqlite3_value_int_func _sqlite3_value_int;
	sqlite3_value_int64_func _sqlite3_value_int64;
	sqlite3_value_text_func _sqlite3_value_text;
	sqlite3_value_text16_func _sqlite3_value_text16;
}

namespace
{
	static yuint32_t gs_colBufferSizes[] = {
		sizeof(yint8_t),
		sizeof(yint16_t),
		sizeof(yint32_t),
		sizeof(yint64_t),
		sizeof(yuint8_t),
		sizeof(yuint16_t),
		sizeof(yuint32_t),
		sizeof(yuint64_t),
		sizeof(ybool_t),
		sizeof(float),
		sizeof(double),
		256 * sizeof(char),
		256 * sizeof(char),
		4096 * sizeof(char),
		256 * sizeof(wchar_t),
		256 * sizeof(wchar_t),
		4096 * sizeof(wchar_t),
		1024,
		1024,
		32768,
		sizeof(YXC_Date),
		sizeof(YXC_Time),
		sizeof(YXC_DateTime),
		sizeof(YXC_DateTime)
	};
}

namespace YXC_Inner
{
	YXC_Status _GetSqlDefBufSize_sqlite(ybyte_t byDataType, yuint32_t& uBufSize)
	{
		_YXC_CHECK_REPORT_NEW_RET(byDataType <= YXC_DB_DATA_TYPE_MAX, YXC_ERC_INVALID_PARAMETER, YC("Invalid data type to switch"));

		uBufSize = gs_colBufferSizes[byDataType];
		return YXC_ERC_SUCCESS;
	}

	YXC_Status _ReportSQLiteError(int sql_ret, sqlite::sqlite3* db, const wchar_t* cpszModule, const wchar_t* cpszFile, int iLine,
		const wchar_t* cpszFunc, const wchar_t* cpszMsg, ...)
	{
		wchar_t szMessage[YXC_BASE_ERROR_BUFFER] = {0};
		wchar_t szMessage2[2 * YXC_BASE_ERROR_BUFFER];

		va_list vl;
		va_start(vl, cpszMsg);
		swprintf(szMessage, cpszMsg, vl);
		va_end(vl);

		const char* sqlite_err = sqlite::_sqlite3_errmsg(db);
		swprintf(szMessage2, L"sqlite error %S", sqlite_err);

		YXC_Status rc = YXC_ReportErrorExFormat(TRUE, YXC_ERROR_SRC_SQL, YXC_ERR_CAT_SQL_SQLITE, sql_ret, cpszModule, cpszFile, iLine,
			cpszFunc, NULL, 0, L"%s [%s]", szMessage, szMessage2);
		return YXC_ERC_SQL;
	}
}

#ifndef __INNER_INC_YXC_SYS_BASE_SQLITE_COMMON_HPP
#define __INNER_INC_YXC_SYS_BASE_SQLITE_COMMON_HPP

#include <YXC_Sys/YXC_Database.h>

#define SQLITE_INTEGER  1
#define SQLITE_FLOAT    2
#define SQLITE_BLOB     4
#define SQLITE_NULL     5
#ifdef SQLITE_TEXT
# undef SQLITE_TEXT
#else
# define SQLITE_TEXT     3
#endif
#define SQLITE3_TEXT     3

#define SQLITE_OK           0   /* Successful result */

/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_NOTICE      27   /* Notifications from sqlite3_log() */
#define SQLITE_WARNING     28   /* Warnings from sqlite3_log() */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */
/* end-of-error-codes */

namespace sqlite
{
	typedef struct sqlite3_t sqlite3;
	typedef struct sqlite3_stmt_t sqlite3_stmt;
	typedef struct Mem sqlite3_value;

	typedef int (*sqlite3_initialize_func)(void);
	typedef int (*sqlite3_open_func)(
		const char *filename,   /* Database filename (UTF-8) */
		sqlite3 **ppDb          /* OUT: SQLite db handle */
		);
	typedef int (*sqlite3_exec_func)(
		sqlite3*,                                  /* An open database */
		const char *sql,                           /* SQL to be evaluated */
		int (*callback)(void*,int,char**,char**),  /* Callback function */
		void *,                                    /* 1st argument to callback */
		char **errmsg                              /* Error msg written here */
	);
	typedef int (*sqlite3_close_func)(sqlite3*);
	typedef int (*sqlite3_last_insert_rowid_func)(sqlite3*);
	typedef int (*sqlite3_changes_func)(sqlite3*);

	typedef int (*sqlite3_errcode_func)(sqlite3 *db);
	typedef int (*sqlite3_extended_errcode_func)(sqlite3 *db);
	typedef const char* (*sqlite3_errmsg_func)(sqlite3*);
	typedef const char* (*sqlite3_errstr_func)(int);
	typedef int (*sqlite3_prepare_func)(
		sqlite3 *db,            /* Database handle */
		const char *zSql,       /* SQL statement, UTF-8 encoded */
		int nByte,              /* Maximum length of zSql in bytes. */
		sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
		const char **pzTail     /* OUT: Pointer to unused portion of zSql */
	);
	typedef int (*sqlite3_step_func)(sqlite3_stmt*);
	typedef int (*sqlite3_data_count_func)(sqlite3_stmt *pStmt);
	typedef int (*sqlite3_reset_func)(sqlite3_stmt *pStmt);
	typedef int (*sqlite3_finalize_func)(sqlite3_stmt *pStmt);
	typedef int (*sqlite3_clear_bindings_func)(sqlite3_stmt*);

	typedef int (*sqlite3_bind_blob_func)(sqlite3_stmt*, int, const void*, int n, void(*)(void*));
	typedef int (*sqlite3_bind_double_func)(sqlite3_stmt*, int, double);
	typedef int (*sqlite3_bind_int_func)(sqlite3_stmt*, int, int);
	typedef int (*sqlite3_bind_int64_func)(sqlite3_stmt*, int, yint64_t);
	typedef int (*sqlite3_bind_null_func)(sqlite3_stmt*, int);
	typedef int (*sqlite3_bind_text_func)(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
	typedef int (*sqlite3_bind_text16_func)(sqlite3_stmt*, int, const void*, int, void(*)(void*));
	typedef int (*sqlite3_bind_value_func)(sqlite3_stmt*, int, const sqlite3_value*);
	typedef int (*sqlite3_bind_zeroblob_func)(sqlite3_stmt*, int, int n);

	typedef int (*sqlite3_column_count_func)(sqlite3_stmt *pStmt);
	typedef const char* (*sqlite3_column_name_func)(sqlite3_stmt*, int N);
	typedef const char* (*sqlite3_column_decltype_func)(sqlite3_stmt*, int N);
	typedef int (*sqlite3_column_type_func)(sqlite3_stmt*, int N);
	typedef sqlite3_value* (*sqlite3_column_value_func)(sqlite3_stmt*, int iCol);

	typedef const void* (*sqlite3_value_blob_func)(sqlite3_value*);
	typedef int (*sqlite3_value_bytes_func)(sqlite3_value*);
	typedef int (*sqlite3_value_bytes16_func)(sqlite3_value*);
	typedef double (*sqlite3_value_double_func)(sqlite3_value*);
	typedef int (*sqlite3_value_int_func)(sqlite3_value*);
	typedef yint64_t (*sqlite3_value_int64_func)(sqlite3_value*);
	typedef const unsigned char* (*sqlite3_value_text_func)(sqlite3_value*);
	typedef const void* (*sqlite3_value_text16_func)(sqlite3_value*);
	typedef int (*sqlite3_value_type_func)(sqlite3_value*);
	typedef int (*sqlite3_value_numeric_type_func)(sqlite3_value*);

	//typedef const void* (*sqlite3_column_blob_func)(sqlite3_stmt*, int iCol);
	//typedef int (*sqlite3_column_bytes_func)(sqlite3_stmt*, int iCol);
	//typedef int (*sqlite3_column_bytes16_func)(sqlite3_stmt*, int iCol);
	//typedef double (*sqlite3_column_double_func)(sqlite3_stmt*, int iCol);
	//typedef int (*sqlite3_column_int_func)(sqlite3_stmt*, int iCol);
	//typedef yint64_t (*sqlite3_column_int64_func)(sqlite3_stmt*, int iCol);
	//typedef const unsigned char* (*sqlite3_column_text_func)(sqlite3_stmt*, int iCol);
	//typedef const void* (*sqlite3_column_text16_func)(sqlite3_stmt*, int iCol);
	//typedef int (*sqlite3_column_type_func)(sqlite3_stmt*, int iCol);

	//int sqlite3_table_column_metadata(
	//	sqlite3 *db,                /* Connection handle */
	//	const char *zDbName,        /* Database name or NULL */
	//	const char *zTableName,     /* Table name */
	//	const char *zColumnName,    /* Column name */
	//	char const **pzDataType,    /* OUTPUT: Declared data type */
	//	char const **pzCollSeq,     /* OUTPUT: Collation sequence name */
	//	int *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
	//	int *pPrimaryKey,           /* OUTPUT: True if column part of PK */
	//	int *pAutoinc               /* OUTPUT: True if column is auto-increment */
	//);

	extern sqlite3_open_func _sqlite3_open;
	extern sqlite3_close_func _sqlite3_close;
	extern sqlite3_initialize_func _sqlite3_init;
	extern sqlite3_last_insert_rowid_func _sqlite3_last_insert_rowid;
	extern sqlite3_changes_func _sqlite3_changes;
	extern sqlite3_errcode_func _sqlite3_errcode;
	extern sqlite3_extended_errcode_func _sqlite3_extended_errcode;
	extern sqlite3_errmsg_func _sqlite3_errmsg;
	extern sqlite3_errstr_func _sqlite3_errstr;
	extern sqlite3_exec_func _sqlite3_exec;
	extern sqlite3_prepare_func _sqlite3_prepare;

	extern sqlite3_step_func _sqlite3_step;
	extern sqlite3_data_count_func _sqlite3_data_count;
	extern sqlite3_reset_func _sqlite3_reset;
	extern sqlite3_finalize_func _sqlite3_finalize;
	extern sqlite3_clear_bindings_func _sqlite3_clear_bindings;

	extern sqlite3_bind_blob_func _sqlite3_bind_blob;
	extern sqlite3_bind_double_func _sqlite3_bind_double;
	extern sqlite3_bind_int_func _sqlite3_bind_int;
	extern sqlite3_bind_int64_func _sqlite3_bind_int64;
	extern sqlite3_bind_null_func _sqlite3_bind_null;
	extern sqlite3_bind_text_func _sqlite3_bind_text;
	extern sqlite3_bind_text16_func _sqlite3_bind_text16;
	extern sqlite3_bind_value_func _sqlite3_bind_value;
	extern sqlite3_bind_zeroblob_func _sqlite3_bind_zeroblob;

	extern sqlite3_column_count_func _sqlite3_column_count;
	extern sqlite3_column_name_func _sqlite3_column_name;
	extern sqlite3_column_decltype_func _sqlite3_column_decltype;
	extern sqlite3_column_type_func _sqlite3_column_type;
	extern sqlite3_column_value_func _sqlite3_column_value;

	extern sqlite3_value_type_func _sqlite3_value_type;
	extern sqlite3_value_numeric_type_func _sqlite3_value_numeric_type;
	extern sqlite3_value_blob_func _sqlite3_value_blob;
	extern sqlite3_value_bytes_func _sqlite3_value_bytes;
	extern sqlite3_value_bytes16_func _sqlite3_value_bytes16;
	extern sqlite3_value_double_func _sqlite3_value_double;
	extern sqlite3_value_int_func _sqlite3_value_int;
	extern sqlite3_value_int64_func _sqlite3_value_int64;
	extern sqlite3_value_text_func _sqlite3_value_text;
	extern sqlite3_value_text16_func _sqlite3_value_text16;
}

namespace YXC_Inner
{
	YXC_Status _GetSqlDefBufSize_sqlite(ybyte_t byDataType, yuint32_t& uBufSize);

	YXC_Status _ReportSQLiteError(int sql_ret, sqlite::sqlite3* db, const wchar_t* cpszModule, const wchar_t* cpszFile,
		int iLine, const wchar_t* cpszFunc, const wchar_t* cpszMsg, ...);
}

#define _YXC_SQLITE_REPORT(sql_ret, db, wszMsg, ...) YXC_Inner::_ReportSQLiteError(sql_ret, db, __EMODULE__, __EFILE__, __LINE__, \
	__EFUNCTION__, wszMsg, ##__VA_ARGS__)

#define _YXC_CHECK_SQLITE_RET(sql_proc, _db, wszMsg, ...)						\
	do {																		\
		int x_sql_ret = (sql_proc);												\
		if (x_sql_ret != 0) {													\
			return _YXC_SQLITE_REPORT(x_sql_ret, _db, wszMsg, ##__VA_ARGS__);		\
		}																		\
	} while (0)

#endif /* __INNER_INC_YXC_SYS_BASE_MSSQL_COMMON_HPP */

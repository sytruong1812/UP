#include "statement.h"

namespace SQLiteHelper {

	Statement::Statement(sqlite3* db, std::string query) {
		stmt = NULL;
		ret_code = SQLITE_OK;
		int ret = sqlite3_prepare_v2(db, query.c_str(), (int)query.length(), &stmt, 0);
		if (SQLITE_OK != ret) {
			ret_code = sqlite3_errcode(db);
		}
	}
	Statement::~Statement() 
	{
		sqlite3_finalize(stmt);
	}

	sqlite3* Statement::getDbHandle()
	{
		return sqlite3_db_handle(stmt);
	}

	sqlite3_stmt* Statement::getStmtObject()
	{
		return stmt;
	}

	int Statement::execute_step()
	{
		return sqlite3_step(stmt);
	}

	// Execute a one-step query with no expected result
	int Statement::execute()
	{
		const int ret = sqlite3_step(stmt);
		if (SQLITE_DONE != ret) {
			if (SQLITE_ROW == ret) 
			{
				//exec() does not expect results. Use executeStep.
				ret_code = sqlite3_errcode(getDbHandle());
			}
		}
		// Return the number of rows modified by those SQL statements (INSERT, UPDATE or DELETE)
		return sqlite3_changes(getDbHandle());
	}

	int Statement::result()
	{
		return ret_code;
	}

	int Statement::reset()
	{
		return sqlite3_reset(stmt);
	}

	/*===================[BINDING]===================*/

	int Statement::bindNull(int index)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_null(stmt, index)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindInt(int index, int value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_int(stmt, index, value)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindInt64(int index, long long value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_int64(stmt, index, value)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindDouble(int index, double value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_double(stmt, index, value)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindText(int index, const char* text, int size)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_text(stmt, index, text, size, NULL)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindText16(int index, const char* text, int size)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_text16(stmt, index, text, size, NULL)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindBlob(int index, unsigned char* data, int size)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_blob(stmt, index, data, size, NULL)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::bindValue(int index, sqlite3_value* value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_value(stmt, index, value)) != SQLITE_OK)
		{
			ret_code = lastResult;
		}
		return lastResult;
	}

	int Statement::clearBindings()
	{
		return sqlite3_clear_bindings(stmt);
	}

	/*===================[COLUMN]====================*/

	std::string Statement::columnName(int index)
	{
		return sqlite3_column_name(stmt, index);
	}

	int Statement::columnType(int index)
	{
		return sqlite3_column_type(stmt, index);
	}

	int Statement::columnCount()
	{
		return sqlite3_column_count(stmt);
	}

	int Statement::getInt(int index)
	{
		return sqlite3_column_int(stmt, index);
	}

	int Statement::getBytes(int index)
	{
		return sqlite3_column_bytes(stmt, index);
	}

	long long Statement::getInt64(int index)
	{
		return static_cast<long long>(sqlite3_column_int64(stmt, index));
	}

	unsigned Statement::getUInt(int index)
	{
		return static_cast<unsigned>(getInt64(index));
	}

	double Statement::getDouble(int index)
	{
		return sqlite3_column_double(stmt, index);
	}

	const char* Statement::getText(int index)
	{
		const char* pText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
		return pText ? pText : NULL;
	}

	std::string Statement::getTextString(int index)
	{
		const char* pText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
		return std::string(pText, sqlite3_column_bytes(stmt, index));
	}

	const void* Statement::getBlob(int index)
	{
		return sqlite3_column_blob(stmt, index);
	}

	std::string Statement::getBlobString(int index)
	{
		const char* data = static_cast<const char*>(sqlite3_column_blob(stmt, index));
		return std::string(data, sqlite3_column_bytes(stmt, index));
	}
}
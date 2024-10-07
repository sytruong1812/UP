#include "statement.h"

namespace SQLiteHelper {

	Statement::Statement(sqlite3* db, std::string query) {
		mbHasRow = false;
		mbDone = false;
		stmt = NULL;
		int ret = sqlite3_prepare_v2(db, query.c_str(), (int)query.length(), &stmt, 0);
		if (SQLITE_OK != ret) {
			throw SQLiteException(db);
		}
	}
	Statement::~Statement() {

	}

	sqlite3* Statement::getDatabaseHandle()
	{
		return sqlite3_db_handle(stmt);
	}

	sqlite3_stmt* Statement::getStatementObject()
	{
		return stmt;
	}

	int Statement::try_execute_step()
	{
		if (false == mbDone)
		{
			const int ret = sqlite3_step(stmt);
			if (SQLITE_ROW == ret) // one row is ready : call COLUMN method(index) to access it
			{
				mbHasRow = true;
			}
			else if (SQLITE_DONE == ret) // no (more) row ready : the query has finished executing
			{
				mbHasRow = false;
				mbDone = true;
			}
			else
			{
				mbHasRow = false;
				mbDone = false;
			}
			return ret;
		}
		else
		{
			return SQLITE_MISUSE;	// Statement needs to be reseted!
		}
	}

	// Execute a step of the query to fetch one row of results
	int Statement::execute_step()
	{
		const int ret = try_execute_step();

		if ((SQLITE_ROW != ret) && (SQLITE_DONE != ret)) // on row or no (more) row ready, else it's a problem
		{
			throw SQLiteException(getDatabaseHandle());
		}
		return mbHasRow; // true only if one row is accessible by getColumn(N)
	}

	// Execute a one-step query with no expected result
	int Statement::execute()
	{
		const int ret = try_execute_step();

		if (SQLITE_DONE != ret) {
			if (SQLITE_ROW == ret) {
				throw SQLiteException("exec() does not expect results. Use executeStep.");
			}
			else {
				throw SQLiteException(getDatabaseHandle());
			}
		}
		// Return the number of rows modified by those SQL statements (INSERT, UPDATE or DELETE)
		return sqlite3_changes(getDatabaseHandle());
	}

	void Statement::reset()
	{
		sqlite3_reset(stmt);
	}

	void Statement::close()
	{
		sqlite3_finalize(stmt);
	}

	/*===================[BINDING]===================*/

	int Statement::bindNull(int index)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_null(stmt, index)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding Null");
		}
		return lastResult;
	}

	int Statement::bindInt(int index, int value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_int(stmt, index, value)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding Integer [" + std::to_string(value) + "]");
		}
		return lastResult;
	}

	int Statement::bindInt64(int index, long long value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_int64(stmt, index, value)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding Long [" + std::to_string(value) + "]");
		}
		return lastResult;
	}

	int Statement::bindDouble(int index, double value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_double(stmt, index, value)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding double [" + std::to_string(value) + "]");
		}
		return lastResult;
	}

	int Statement::bindText(int index, std::string value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_text(stmt, index, value.c_str(), -1, NULL)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding Text [" + value + "]");
		}
		return lastResult;
	}

	int Statement::bindBlob(int index, unsigned char* data, size_t size)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_blob(stmt, index, data, (int)size, NULL)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding Blob [...010...]");
		}
		return lastResult;
	}

	int Statement::bindValue(int index, sqlite3_value* value)
	{
		int lastResult = 0;
		if ((lastResult = sqlite3_bind_value(stmt, index, value)) != SQLITE_OK)
		{
			throw SQLiteException("Error " + std::to_string(lastResult) + " binding Null");
		}
		return lastResult;
	}

	void Statement::clearBindings()
	{
		const int ret = sqlite3_clear_bindings(stmt);
		if (SQLITE_OK != ret) {
			throw SQLiteException(getDatabaseHandle());
		}
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
		return sqlite3_column_int64(stmt, index);
	}

	unsigned Statement::getUInt(int index)
	{
		return static_cast<unsigned>(getInt64(index));
	}

	double Statement::getDouble(int index)
	{
		return sqlite3_column_double(stmt, index);
	}

	const void* Statement::getBlob(int index)
	{
		return sqlite3_column_blob(stmt, index);
	}

	std::string Statement::getText(int index)
	{
		const char* pText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, index));
		return pText ? pText : "";
	}

	std::string Statement::getString(int index)
	{
		// Note: using sqlite3_column_blob and not sqlite3_column_text
		// - no need for sqlite3_column_text to add a \0 on the end, as we're getting the bytes length directly
		const char* data = static_cast<const char*>(sqlite3_column_blob(stmt, index));

		// SQLite docs: "The safest policy is to invoke… sqlite3_column_blob() followed by sqlite3_column_bytes()"
		// Note: std::string is ok to pass nullptr as first arg, if length is 0
		return std::string(data, sqlite3_column_bytes(stmt, index));
	}

}
#include "exception.h"

namespace SQLiteHelper {

	SQLiteException::SQLiteException(std::string const& message) :
		runtime_error{ message },
		m_code{ SQLITE_ERROR },
		m_extendedCode{ SQLITE_ERROR }
	{
	}

	SQLiteException::SQLiteException(sqlite3* sqlite) :
		runtime_error{ sqlite3_errmsg(sqlite) },
		m_code{ sqlite3_errcode(sqlite) },
		m_extendedCode{ sqlite3_extended_errcode(sqlite) }
	{
	}
}

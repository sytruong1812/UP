#pragma once
#include <string>
#include <stdexcept>
#include "sqlite3.h"

namespace SQLiteHelper {

	class SQLiteException : public std::runtime_error 
	{
	public:
		explicit SQLiteException(std::string const& message);
		/**
		 * @brief Encapsulation of the error message from SQLite3, based on std::runtime_error.
		 *
		 * @param[in] sqlite The SQLite object, to obtain detailed error messages from.
		 */
		explicit SQLiteException(sqlite3* sqlite);

		inline int code() const noexcept {
			return m_code;
		}

		inline int extendedCode() const noexcept {
			return m_extendedCode;
		}

	private:
		int m_code;
		int m_extendedCode;
	};
}
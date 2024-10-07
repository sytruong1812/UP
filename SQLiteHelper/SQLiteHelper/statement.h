#pragma once
#include <string>
#include "sqlite3.h"
#include "database.h"
#include "exception.h"

namespace SQLiteHelper {

	class Statement 
	{
	private:
		bool mbDone;         //true when the last executeStep() had no more row to fetch
		bool mbHasRow;       //true when a row has been fetched with executeStep()
		sqlite3_stmt* stmt;
	public:
		Statement(sqlite3* db, std::string query);
		~Statement();
		sqlite3* getDatabaseHandle();
		sqlite3_stmt* getStatementObject();
		int try_execute_step();
		int execute_step();
		int execute();
		void reset();
		void close();
	public:
		/*===================[BINDING]===================*/
		int bindNull(int index);
		int bindInt(int index, int value);
		int bindInt64(int index, long long value);
		int bindDouble(int index, double value);
		int bindText(int index, std::string value);
		int bindBlob(int index, unsigned char* data, size_t size);
		int bindValue(int index, sqlite3_value* value);
		void clearBindings();
	public:
		/*===================[COLUMN]====================*/
		int columnCount();
		int columnType(int index);
		std::string columnName(int index);
		int getInt(int index);
		long long getInt64(int index);
		unsigned getUInt(int index);
		double getDouble(int index);
		int getBytes(int index);
		const void* getBlob(int index);
		std::string getText(int index);
		std::string getString(int index);
	};

}
#pragma once
#include <string>
#include "sqlite3.h"
#include "database.h"

namespace SQLiteHelper {

	class Statement 
	{
	private:
		int ret_code;
		sqlite3_stmt* stmt;
	public:
		Statement(sqlite3* db, std::string query);
		~Statement();
		sqlite3* getDbHandle();
		sqlite3_stmt* getStmtObject();
		int execute_step();
		int execute();
		int result();
		int reset();
	public:
		/*===================[BINDING]===================*/
		int clearBindings();
		int bindNull(int index);
		int bindInt(int index, int value);
		int bindInt64(int index, long long value);
		int bindDouble(int index, double value);
		int bindText(int index, const char* text, int size);
		int bindText16(int index, const char* text, int size);
		int bindBlob(int index, unsigned char* data, int size);
		int bindValue(int index, sqlite3_value* value);
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
		const char* getText(int index);
		const void* getBlob(int index);
		std::string getTextString(int index);
		std::string getBlobString(int index);
	};

}
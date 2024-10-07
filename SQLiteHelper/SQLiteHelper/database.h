#pragma once
#include <string>
#include "sqlite3.h"
#include "statement.h"
#include "DataTable/table.h"

using namespace DATA_TABLE;

namespace SQLiteHelper {

	class Database
	{
	private:
		sqlite3* db;
	public:
		Database(const std::string& path_db);
		~Database();
		sqlite3* connection();
		void open(const std::string& path_db);
		void open_v2(const std::string& fileName, int flags, int busyTimeoutMs, const std::string& vfs);
		void close();
		void close_v2();
		void changes();
		void setBusyTimeout(const int aBusyTimeoutMs);
		void executeNonQuery(std::string query);
	private:
		void getListColumn(void* stmt, DataColumn* column);
		int readNextRow(void* stmt, DataTable* table);
		void bindParameter(void* stmt, int index, int type, Object value);
		int writeNextRow(void* stmt, DataColumn type_root, DataRow row);
		std::string createInsertQuery(const std::string& table_name, DataTable* table);
	public:
		void deleteTable(const std::string& table_name);
		void selectTableNames(std::vector<std::string>& tableNames);
		int importTable(const std::string& table_name, DataTable* table);
		int exportTable(const std::string& table_name, DataTable* table);
	};
}


#pragma once
#include "error.h"
#include "database.h"
#include "date_time.h"
#include "transaction.h"

namespace SQLiteHelper
{
	class BackupImport {
	private:
		int total = 0;
		int failed = 0;
		int success = 0;
		int limit_row = 100;	//Default: 100 rows export
		bool del_table = false;	//Delete all old row table 
		std::string key_see;
		std::string col_utc_update;
	private:
		bool updateColumnUtc(DataTable* table);
		bool hasHeaderSQLite(const std::wstring& path_db);
		std::string buildCreateTableQuery(const std::string& table_name, DataColumn* column);
		std::string buildSelectFilterQuery(const std::string& table_name, int limit, int offset);
		std::string buildInsertTableQuery(const std::string& table_name, DataColumn* column);
		bool backupOneTable(const std::string& table_name, Database* db, Database* db_backup);
		bool importOneTable(const std::string& table_name, Database* db, Database* db_import);
	public:
		BackupImport() = default;
		BackupImport(const BackupImport* other) : 
			key_see(other->key_see), 
			limit_row(other->limit_row), 
			del_table(other->del_table),
			col_utc_update(other->col_utc_update) {}

		std::wstring showResult();
		std::wstring showError(int error_code);

		/*
		- Purpose: This function sets the maximum number of rows for each export operation.
		- Parameters:
			1. [IN] limit: The maximum row count for export.
		- Return value: None.
		*/
		void setLimitRow(int limit);

		/*
		- Purpose: This function enables or disables table deletion before importing new rows.
		- Parameters:
			1. [IN] option: A boolean value to enable (true) or disable (false) table deletion.
		- Return value: None.
		*/
		void setDeleteTable(bool option);

		/*
		- Purpose: This function sets the key for encrypting or decrypting the database.
		- Parameters:
			1. [IN] keySee: A pointer to the encryption key.
			2. [IN] keySize: The size of the encryption key.
		- Return value: None.
		*/
		void setKeySee(const BYTE* keySee, const size_t keySize);

		/*
		- Purpose: This function sets the column name used for UTC updates.
		- Parameters:
			1. [IN] column_name: The name of the column for UTC updates.
		- Return value: None.
		*/
		void setColumnUtcForUpdate(const std::string& column_name);

		/*
		- Purpose: This function is used to back up the database from a source path to a target path.
		- Parameters:
			1. [IN] path_from: The file path of the source database.
			2. [OUT] path_to: The file path for the backup database.
		- Return value: An integer representing the result code of the operation.
		*/
		int backupDatabase(const std::wstring& path_from, const std::wstring& path_to);

		/*
		- Purpose: This function imports a database from a source path to a target path.
		- Parameters:
			1. [IN] path_from: The file path of the source database.
			2. [OUT] path_to: The file path for the import database.
		- Return value: An integer representing the result code of the operation.
		*/
		int importDatabase(const std::wstring& path_from, const std::wstring& path_to);

		/*
		- Purpose: This function merges a source database into a target database.
		- Parameters:
			1. [IN] path_from: The file path of the source database.
			2. [OUT] path_to: The file path for the target database.
		- Return value: An integer representing the result code of the operation.
		*/
		int mergeDatabase(const std::wstring& path_from, const std::wstring& path_to);
	};
}
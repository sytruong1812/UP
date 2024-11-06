#include <fstream>
#include <sstream>
#include <iostream>
#include "backup_import.h"

namespace SQLiteHelper
{
	std::wstring BackupImport::showResult()
	{
		std::wstringstream result;
		result << L"--> Total: " << total << L" row" << std::endl;
		result << L"--> Success: " << success << L" row" << std::endl;
		result << L"--> Failed: " << failed << L" row";
		return result.str();
	}

	std::wstring BackupImport::showError(int error_code)
	{
		return ERROR_STRING[error_code];
	}

	void BackupImport::setLimitRow(int limit)
	{
		limit_row = limit;
	}

	void BackupImport::setDeleteTable(bool option)
	{
		del_table = option;
	}

	void BackupImport::setKeySee(const BYTE* keySee, const size_t keySize)
	{
		if (keySee != NULL && keySize != 0) {
			key_see = std::string((char*)keySee, keySize);
		}
	}

	void BackupImport::setColumnUtcForUpdate(const std::string& column_name)
	{
		col_utc_update = column_name;
	}

	bool BackupImport::updateColumnUtc(DataTable* table)
	{
		if (table->getDataColumnSize() != table->getDataRowSize())
		{
			return false;
		}
		if (table->getDataColumn()->contain(col_utc_update) == false)
		{
			return false;
		}
		size_t index = table->getDataColumn()->getColumnIndex(col_utc_update);
		std::unique_ptr<Object> obj = std::make_unique<Object>();
		obj->object_type = table->getDataColumn()->getTypeAt(index);
		obj->object_value.llong_ = DateTime::instance()->getExpiresUtc();
		table->updateRowValueAt(index, obj.get());
		return true;
	}

	bool BackupImport::hasHeaderSQLite(const std::wstring& path_db)
	{
		char header[16] = "\0";
		if (path_db.empty() == false) {
			std::ifstream fileBuffer(path_db.c_str(), std::ios::in | std::ios::binary);
			if (fileBuffer.is_open()) {
				fileBuffer.seekg(0, std::ios::beg);
				fileBuffer.getline(header, 16);
				fileBuffer.close();
			}
		}
		return strncmp(header, "SQLite format 3\000", 16) == 0 ? true : false;
	}

	std::string BackupImport::buildCreateTableQuery(const std::string& table_name, DataColumn* column)
	{
		size_t numberOfColumn = column->size();
		std::stringstream query;
		query << "CREATE TABLE \"" << table_name << "\" (";
		for (size_t i = 0; i < numberOfColumn; ++i) {
			query << "\"" << column->getNameAt(i) << "\" ";
			switch (column->getTypeAt(i)) {
			case ObjectTypes::TYPE_INT:
			case ObjectTypes::TYPE_LONG:
				query << "INTEGER NOT NULL";
				break;
			case ObjectTypes::TYPE_FLOAT:
			case ObjectTypes::TYPE_DOUBLE:
				query << "REAL NOT NULL";
				break;
			case ObjectTypes::TYPE_STRING:
				query << "TEXT NOT NULL";
				break;
			case ObjectTypes::TYPE_BLOB:
				query << "BLOB NOT NULL";
				break;
			case ObjectTypes::TYPE_NULL:
				break;
			}
			if (i < numberOfColumn - 1) {
				query << ", ";
			}
		}
		query << ");";
		return query.str();
	}

	std::string BackupImport::buildSelectFilterQuery(const std::string& table_name, int limit, int offset)
	{
		//TODO: Handle from - to/ from - end / begin - to / Get all
		std::stringstream query;
		query << "SELECT * FROM \"" << table_name << "\"";
		if (!DateTime::instance()->getColumn_utc().empty()) {
			query << " WHERE " << DateTime::instance()->getColumn_utc();
			if (!DateTime::instance()->getWildCard().empty())
			{
				query << " LIKE " << "'" << DateTime::instance()->getWildCard() << "'";
			}
			if(DateTime::instance()->getDateTimeFrom() != 0 && DateTime::instance()->getDateTimeTo() != 0) {
				query << " BETWEEN '" << DateTime::instance()->getDateTimeFrom() << "'";
				query << " AND '" << DateTime::instance()->getDateTimeTo() << "'";
			}
			else {
				if (DateTime::instance()->getDateTimeFrom() != 0) {
					//From
					query << " > '" << DateTime::instance()->getDateTimeFrom() << "'";
				}
				if(DateTime::instance()->getDateTimeTo() != 0) {
					//To
					query << " < '" << DateTime::instance()->getDateTimeTo() << "'";
				}
			}
		}
		if (limit > 0) {
			query << " LIMIT " << limit << " OFFSET " << offset;
		}
		query << ";";
		return query.str();
	}

	std::string BackupImport::buildInsertTableQuery(const std::string& table_name, DataColumn* column)
	{
		size_t numberOfColumn = column->size();
		std::stringstream query;
		query << "INSERT INTO \"" << table_name << "\" (";
		for (size_t i = 0; i < numberOfColumn; ++i) {
			query << "\"" << column->getNameAt(i) << "\"";
			if (i < numberOfColumn - 1) {
				query << ", ";
			}
		}
		query << ") VALUES (";
		for (size_t i = 0; i < numberOfColumn; ++i) {
			query << "?";
			if (i < numberOfColumn - 1) {
				query << ", ";
			}
		}
		query << ")";

		return query.str();
	}

	bool BackupImport::backupOneTable(const std::string& table_name, Database* db, Database* db_backup)
	{
		/*==================[DB1: Export one table]=====================*/
		int offset = 0;
		while (true) 
		{
			std::unique_ptr<DataTable> table = std::make_unique<DataTable>(table_name);
			std::string query1 = buildSelectFilterQuery(table_name, limit_row, offset);
			total += db->exportTable(query1, table.get());
			if (total == -1) {
				return false;
			}
			if (table->getNumberOfRow() == 0) {
				break;
			}
			/*==================[DB2: Import one table]=====================*/
			std::string query3 = buildInsertTableQuery(table->getName(), table->getDataColumn());
			success += db_backup->importTable(query3, table.get());
			if (success == -1) {
				return false;
			}
			offset += limit_row;
		}
		return true;
	}

	int BackupImport::backupDatabase(const std::wstring& path_from, const std::wstring& path_to)
	{
		/*======================[DB1: Open database]==========================*/
		std::unique_ptr<Database> db = std::make_unique<Database>();
		if (db->open_16(path_from.c_str()) == false) {
			return OPEN_DB_ERROR;
		}
		/*===================[DB1: Get list table name]=======================*/
		std::vector<std::string> lsTableName;
		if (db->getTableNames(&lsTableName) == false) {
			return GET_TABLE_NAME_ERROR;
		}
		if (lsTableName.empty()) {
			return DATABASE_EMPTY;
		}
		/*===================[DB2: Create file backup]=======================*/
		std::unique_ptr<Database> db_backup = std::make_unique<Database>();
		std::unique_ptr<Transaction> transaction = std::make_unique<Transaction>(db_backup.get());
		if (db_backup->open_16(path_to.c_str()) == false) {
			return OPEN_DB_ERROR;
		}
		//TODO: Backup -> Set key SEE
		if (key_see.empty() == false) {
			if (db_backup->setKeySee(key_see.c_str(), (int)key_see.size()) == false) {
				return SET_KEY_SEE_ERROR;
			}
		}
		/*======================[Backup DB1 -> DB2]==========================*/
		for (size_t i = 0; i < lsTableName.size(); i++) {
			std::string table_name = lsTableName[i];
			/*==================[DB1: Get Schema table]=====================*/
			std::string query2 = db->getSchemaTable(table_name);
			/*==================[DB2: Create one table]====================*/
			if (db_backup->execute(query2) == false) 
			{
				return EXECUTE_ERROR;
			}
			/*==================[Backup One Table]=======================*/
			if (backupOneTable(table_name, db.get(), db_backup.get()) == false) {
				transaction->rollback();
				return FUNCTION_ERROR;
			}
		}
		transaction->commit();
		failed = total - success;
		return FUNCTION_DONE;
	}

	bool BackupImport::importOneTable(const std::string& table_name, Database* db, Database* db_import)
	{
		/*==================[DB1: Export one table]=====================*/
		int offset = 0;
		while (true) 
		{
			std::unique_ptr<DataTable> table = std::make_unique<DataTable>(table_name);
			std::string query2 = buildSelectFilterQuery(table_name, limit_row, offset);
			total += db->exportTable(query2, table.get());
			if (total == -1) {
				return false;
			}
			if (table->getNumberOfRow() == 0) {
				break;
			}
			/*==================[DB1: Update Expires_Utc]===================*/
			if (col_utc_update.empty() == false) {
				if (updateColumnUtc(table.get()) == false) {
					return false;
				}
			}
			/*==================[DB2: Import one table]=====================*/
			if (del_table == true) {
				if (db_import->deleteTable(table_name) == false) {
					return false;
				}
			}
			std::string query3 = buildInsertTableQuery(table->getName(), table->getDataColumn());
			success += db_import->importTable(query3, table.get());
			if (success == -1) {
				return false;
			}
			offset += limit_row;
		}
		return true;
	}

	int BackupImport::importDatabase(const std::wstring& path_from, const std::wstring& path_to)
	{
		/*========================[DB1: Open database]==========================*/
		std::unique_ptr<Database> db = std::make_unique<Database>();
		if (db->open_16(path_from.c_str()) == false) {
			return OPEN_DB_ERROR;
		}
		//TODO: Import -> Set key SEE
		if (hasHeaderSQLite(path_from))
		{
			if (key_see.empty() == false)
			{
				return DB_NOT_ENCRYPT_SET_SEE;
			}
		}
		else {		//Encrypted SEE
			if (key_see.empty() == true)
			{
				return DB_ENCRYPTED_SEE_NOTSET;
			}
			if (db->setKeySee(key_see.c_str(), (int)key_see.size()) == false)
			{
				return SET_KEY_SEE_ERROR;
			}
		}
		/*===================[DB1: Get list table name]=======================*/
		std::vector<std::string> lsTableName;
		if (db->getTableNames(&lsTableName) == false) {
			return GET_TABLE_NAME_ERROR;
		}
		if (lsTableName.empty()) {
			return DATABASE_EMPTY;
		}
		/*====================[DB2: Open/Create database import]=======================*/
		std::unique_ptr<Database> db_import = std::make_unique<Database>();
		std::unique_ptr<Transaction> transaction = std::make_unique<Transaction>(db_import.get());
		if (db_import->open_16(path_to.c_str()) == false)
		{
			return OPEN_DB_ERROR;
		}
		/*======================[Import DB1 -> DB2]==========================*/
		for (size_t i = 0; i < lsTableName.size(); i++) {
			std::string table_name = lsTableName[i];
			if (Utils::isFileEmpty(path_to))
			{
				/*==================[DB1: Get Schema table]=====================*/
				std::string query1 = db->getSchemaTable(table_name);
				/*==================[DB2: Create one table]===================*/
				if (db_import->execute(query1) == false) 
				{
					return EXECUTE_ERROR;
				}
			}
			/*==================[Import One Table]=======================*/
			if (importOneTable(table_name, db.get(), db_import.get()) == false) {
				transaction->rollback();
				return FUNCTION_ERROR;
			}
		}
		transaction->commit();
		failed = total - success;
		return FUNCTION_DONE;
	}
	
	int BackupImport::mergeDatabase(const std::wstring& path_from, const std::wstring& path_to)
	{
		/*======================[DB1: Open database]==========================*/
		std::unique_ptr<Database> db_1 = std::make_unique<Database>();
		if (db_1->open_16(path_from.c_str()) == false) {
			return OPEN_DB_ERROR;
		}
		if (hasHeaderSQLite(path_from) == false) {	//Encrypted SEE
			return DB_ENCRYPTED_SEE_NOTSET;
		}
		/*====================[DB1: Get list table name]=======================*/
		std::vector<std::string> lsTableName1;
		if (db_1->getTableNames(&lsTableName1) == false) {
			return GET_TABLE_NAME_ERROR;
		}
		if (lsTableName1.empty()) {
			return DATABASE_EMPTY;
		}
		/*===================[DB2: Create file backup]=======================*/
		std::unique_ptr<Database> db_2 = std::make_unique<Database>();
		std::unique_ptr<Transaction> transaction = std::make_unique<Transaction>(db_2.get());
		if (db_2->open_16(path_to.c_str()) == false) {
			return OPEN_DB_ERROR;
		}
		if (hasHeaderSQLite(path_from) == false) {	//Encrypted SEE
			return DB_ENCRYPTED_SEE_NOTSET;
		}
		/*===================[DB2: Get list table name]=======================*/
		std::vector<std::string> lsTableName2;
		if (db_2->getTableNames(&lsTableName2) == false) {
			return GET_TABLE_NAME_ERROR;
		}
		if (lsTableName2.empty()) {
			return DATABASE_EMPTY;
		}
		/*======================[Merge DB1 -> DB2]==========================*/
		if (lsTableName1.size() != lsTableName2.size()) {
			return DB_NOT_SAME;
		}
		for (size_t i = 0; i < lsTableName1.size(); i++) {
			std::string table_name = lsTableName1[i];
			if (std::find(lsTableName2.begin(), lsTableName2.end(), table_name) != lsTableName2.end()) {
				/*==================[Merge One Table]=======================*/
				if (backupOneTable(table_name, db_1.get(), db_2.get()) == false) {
					transaction->rollback();
					return FUNCTION_ERROR;
				}
			}
		}
		transaction->commit();
		failed = total - success;
		return FUNCTION_DONE;
	}
}

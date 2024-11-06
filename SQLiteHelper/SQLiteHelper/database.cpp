#include <sstream>
#include <iostream>
#include "database.h"
#include "TraceLogger.h"

namespace SQLiteHelper {

	Database::Database()
	{
		db = NULL;
		err_code = SQLITE_OK;
	}

	Database::Database(const std::string& path, const int flags)
	{
		db = NULL;
		err_code = SQLITE_OK;
		const int result = sqlite3_open_v2(path.c_str(), &db, flags, 0);
		if (SQLITE_OK != result) {
			err_code = result;
		}
	}

	Database::~Database()
	{
		close();
	}

	sqlite3* Database::handle()
	{
		return db ? db : nullptr;
	}

	bool Database::connected()
	{
		return (bool)db;
	}

	bool Database::open_16(const void* path)
	{
		const int result = sqlite3_open16(path, &db);
		if (SQLITE_OK != result) 
		{
			err_code = result;
			return false;
		}
		return true;
	}

	bool Database::close()
	{
		const int result = sqlite3_close(handle());
		if (SQLITE_OK != result)
		{
			err_code = result;
			return false;
		}
		return true;
	}

	bool Database::open_v2(const std::string& path, const int flags, const int busyTimeoutMs, const std::string& vfs)
	{
		const int result = sqlite3_open_v2(path.c_str(), &db, flags, vfs.empty() ? NULL : vfs.c_str());
		if (SQLITE_OK == result) {
			if (busyTimeoutMs > 0) {
				if (!setBusyTimeout(busyTimeoutMs)) {
					sqlite3_close_v2(db);
					return false;
				}
			}
		}
		else {
			err_code = result;
			return false;
		}
		return true;
	
	}

	bool Database::is_readonly()
	{
		if (db) {
			return (sqlite3_db_readonly(handle(), "main") == 1);
		}
		else {
			return true;
		}
	}

	int Database::changes()
	{
		return sqlite3_changes(handle());
	}

	int Database::totalChanges()
	{
		return sqlite3_total_changes(handle());
	}

	int Database::getErrorCode()
	{
		return err_code;
	}

	//TODO: An N value of 0 means the main database file. An N of 1 is the "temp" schema. Larger values of N correspond to various ATTACH-ed databases.
	std::string Database::schema(int n)
	{
		const char* psch = NULL;
		if (db) 
		{
			psch = sqlite3_db_name(handle(), n);
		}
		return psch ? psch : std::string();
	}

	std::string Database::getErrorString()
	{
		return sqlite3_errstr(err_code);
	}

	bool Database::setKeySee(const void* key, const int size)
	{
		const int result = sqlite3_key(handle(), key, size);
		if (SQLITE_OK != result)
		{
			err_code = result;
			return false;
		}
		return true;
	}

	bool Database::setBusyTimeout(const int aBusyTimeoutMs) 
	{
		const int result = sqlite3_busy_timeout(handle(), aBusyTimeoutMs);
		if (SQLITE_OK != result)
		{
			err_code = result;
			return false;
		}
		return true;
	}

	bool Database::execute(const std::string& query)
	{
		int result = sqlite3_exec(handle(), query.c_str(), 0, 0, 0);
		if (SQLITE_OK != result) {
			err_code = result;
			return false;
		}
		return true;
	}

	bool Database::executeQuery(const std::string& query, DataTable* table)
	{
		if (table == NULL) {
			return false;
		}
		std::unique_ptr<Statement> stmt = std::make_unique<Statement>(this->db, query);
		if (stmt->result() != SQLITE_OK) {
			return false;
		}
		while (true) {
			DataRow* row = new DataRow();
			if (readNextRow(stmt.get(), row) != SQLITE_ROW) {
				delete row;
				break;
			}
			table->addDataRow(row);
		}
		return true;
	}

	bool Database::getColumnTable(void* stmt, DataColumn* column)
	{
		Statement* istmt = static_cast<Statement*>(stmt);
		int columnCount = istmt->columnCount();
		if (istmt->execute_step() == false)
		{
			err_code = istmt->result();
			return false;
		}
		for (int i = 0; i < columnCount; i++)
		{
			std::string columnName = istmt->columnName(i);
			if (column->contain(columnName)) {
				return false;
			}
			int columnType = istmt->columnType(i);
			switch (columnType)
			{
				case SQLITE_INTEGER:
				{
					column->add(columnName, ObjectTypes::TYPE_LLONG);
					break;
				}
				case SQLITE_FLOAT:
				{
					column->add(columnName, ObjectTypes::TYPE_FLOAT);
					break;
				}
				case SQLITE_TEXT:
				{
					column->add(columnName, ObjectTypes::TYPE_STRING);
					break;
				}
				case SQLITE_BLOB:
				{
					column->add(columnName, ObjectTypes::TYPE_BLOB);
					break;
				}
				default:
				{
					column->add(columnName, ObjectTypes::TYPE_NULL);
					break;
				}
			}
		}
		return true;
	}

	void Database::getParameter(void* stmt, const int index, const int type, DataRow* row)
	{
		Statement* istmt = static_cast<Statement*>(stmt);
		switch (type)
		{
			case SQLITE_INTEGER:
			{
				row->add<long long>(istmt->getInt64(index));
				break;
			}
			case SQLITE_FLOAT:
			{
				row->add<double>(istmt->getDouble(index));
				break;
			}
			case SQLITE_TEXT:
			{
				//TODO: Get string with getText() and getBytes()
				// SQLITE_TEXT -> length + 1 
				// -> error table_name -> string end with NULL 
				// -> error create query with stringstream
				char* buffer = NULL;
				size_t length = (size_t)istmt->getBytes(index);
				if (length > 0) {
					buffer = new char[length + 1];
					memcpy(buffer, istmt->getText(index), length + 1);
					row->add<char*>(buffer, length);
				}
				else {
					row->add<char*>(NULL, 0);
				}
				break;
			}
			case SQLITE_BLOB:
			{

				if (AesManager::instance()->getEncryptFlag())
				{
					//TODO: Encrypt AES-GCM
					size_t length_in = (size_t)istmt->getBytes(index);
					size_t length_out = AesManager::instance()->getSizeCipherText(length_in);
					uint8_t* buffer = new uint8_t[length_out];
					if (AesManager::instance()->encryptData((uint8_t*)istmt->getBlob(index), length_in, buffer))
					{
						row->add<uint8_t*>(buffer, length_out);
					}
					else {
						row->add<uint8_t*>(NULL, 0);
					}
				}
				else 
				{
					size_t length = (size_t)istmt->getBytes(index);
					if(length > 0)
					{
						uint8_t* buffer = new uint8_t[length];
						memcpy(buffer, (uint8_t*)istmt->getBlob(index), length);
						row->add<uint8_t*>(buffer, length);
					}
					else {
						row->add<uint8_t*>(NULL, 0);
					}	
				}
				break;
			}
			default:
			{
				row->add(NULL);
				break;
			}
		}
	}

	int Database::readNextRow(void* stmt, DataRow* row)
	{
		Statement* istmt = static_cast<Statement*>(stmt);
		int result = istmt->execute_step();
		if (result == SQLITE_DONE) {
			return result;
		}
		int columnCount = istmt->columnCount();
		LOG_INFO_A("--> Number of column = %d", columnCount);
		for (int i = 0; i < columnCount; i++)
		{
			int columnType = istmt->columnType(i);
			std::string columnName = istmt->columnName(i);
			LOG_DEBUG_A("--> Column[%d] -> %s -> Type value: %d", i, columnName.c_str(), columnType);
			getParameter(istmt, i, columnType, row);
		}
		return result;
	}

	bool Database::bindParameter(void* stmt, const int index, const int type, Object* value)
	{
		int result;
		Statement* istmt = static_cast<Statement*>(stmt);
		switch (type)
		{
			case ObjectTypes::TYPE_INT:
			case ObjectTypes::TYPE_LONG:
			case ObjectTypes::TYPE_LLONG:
			{
				result = istmt->bindInt64(index, value->object_value.llong_);
				break;
			}
			case ObjectTypes::TYPE_FLOAT:
			case ObjectTypes::TYPE_DOUBLE:
			{
				result = istmt->bindDouble(index, (double)value->object_value.double_);
				break;
			}
			case ObjectTypes::TYPE_STRING:
			{
				//TODO: Add string with binText() or bindText16() 
				size_t length = value->object_value.text_.length;
				if (length > 0) {
					result = istmt->bindText(index, value->object_value.text_.str, (int)length);
				}
				else {
					result = istmt->bindNull(index);
				}
				break;
			}
			case ObjectTypes::TYPE_BLOB:
			{
				if (AesManager::instance()->getDecryptFlag())
				{
					//TODO: Decrypt AES-GCM
					uint8_t* buffer = value->object_value.blob_.data;
					size_t length_in = value->object_value.blob_.length;
					size_t length_out = AesManager::instance()->getSizePlainText(length_in);
					if (AesManager::instance()->decryptData(buffer, length_in, buffer)) {
						result = istmt->bindBlob(index, buffer, (int)length_out);
					}
					else {
						result = istmt->bindNull(index);
						std::cout << "1 bindBlob -> NULL!" << std::endl;
					}
				}
				else 
				{
					if (value->object_value.blob_.length > 0) {
						result = istmt->bindBlob(index, value->object_value.blob_.data, (int)value->object_value.blob_.length);
					}
					else {
						result = istmt->bindNull(index);
						std::cout << "2 bindBlob -> NULL!" << std::endl;
					}
				}
				break;
			}
			default:
			{
				result = istmt->bindNull(index);
				break;
			}
		}
		if (result != SQLITE_OK) {
			err_code = istmt->result();
			return false;
		}
		return true;
	}

	int Database::writeNextRow(void* stmt, DataColumn* column, DataRow* row)
	{
		Statement* istmt = static_cast<Statement*>(stmt);
		if (column->size() != row->size())
		{
			//Import database will fail if the number of columns of the imported table is different from the original table of the database.
			return SQLITE_ERROR;
		}
		int columnCount = (int)column->size();
		auto vec_object = row->to_vector();		//Add row value to vector
		LOG_INFO_A("--> Number of column = %d", columnCount);
		for (int c = 0; c < columnCount; c++) {
			LOG_DEBUG_A("--> Column[%d] -> %s -> Type value: %d", c, column->getNameAt(c).c_str(), (int)column->getTypeAt(c));
			if (!bindParameter(istmt, c + 1, column->getTypeAt(c), vec_object[c])) {
				return SQLITE_ERROR;
			}
		}
		int result = istmt->execute_step();
		if (result == SQLITE_DONE) {
			changes();	//One row modified
		}
		//TODO: Memory leak here --> Fix: Delete all object in vector
		row->clear_vector(vec_object);
		istmt->reset();
		return result;
	}

	int Database::exportTable(const std::string& query, DataTable* table)
	{
		int success = 0;
		if (table == NULL)
		{
			return -1;
		}
		std::unique_ptr<Statement> stmt = std::make_unique<Statement>(this->db, query);
		if (stmt->result() != SQLITE_OK) {
			return -1;
		}
		while (true) {
			DataRow* row = new DataRow();
			if (readNextRow(stmt.get(), row) != SQLITE_ROW) {
				delete row;
				break;
			}
			table->addDataRow(row);
			LOG_INFO_A("--> Export --> Row[%d]", success);
			success++;
		}
		if (getColumnTable(stmt.get(), table->getDataColumn()) == false) {
			return false;
		}
		return success;
	}

	int Database::importTable(const std::string& query, DataTable* table)
	{
		int success = 0;
		if (table == NULL)
		{
			return -1;
		}
		std::unique_ptr<Statement> stmt = std::make_unique<Statement>(this->db, query);
		if (stmt->result() != SQLITE_OK) {
			return -1;
		}
		for (int r = 0; r < (int)table->getNumberOfRow(); r++)
		{
			LOG_INFO_A("--> Import --> Row[%d]", r);
			int result = writeNextRow(stmt.get(), table->getDataColumn(), table->getDataRowAt(r));
			if (result != SQLITE_CONSTRAINT) {
				//Duplicate
				success++;
			}
		}
		return success;
	}

	bool Database::deleteTable(const std::string& table_name)
	{
		std::string query = "DELETE FROM " + table_name;
		return execute(query);
	}

	bool Database::getTableNames(std::vector<std::string>* list_name)
	{
		std::string query = "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY 1";
		std::unique_ptr<DataTable> record = std::make_unique<DataTable>("record");
		if (!executeQuery(query, record.get())) {
			return false;
		}
		for (size_t i = 0; i < record->getNumberOfRow(); i++) {
			DataRow* name = record->getDataRowAt(i);
			if (name->size() == 0) {
				return false;
			}
			Object* obj = name->getValueAt(0);
			std::string table_name(obj->object_value.text_.str, obj->object_value.text_.length);
			if (table_name.find("sqlite_", 0) == std::string::npos) {
				LOG_DEBUG_A("table_name: %s", table_name.c_str());
				list_name->emplace_back(table_name);
			}
			if (obj != NULL) {
				delete obj;
			}
		}
		return true;
	}

	std::string Database::getSchemaTable(const std::string& table_name)
	{
		std::string query = "SELECT sql FROM sqlite_schema WHERE name = \'" + table_name + "\'";
		std::unique_ptr <Statement> stmt = std::make_unique<Statement>(this->db, query);
		if (stmt->result() != SQLITE_OK) {
			return std::string();
		}
		int result = stmt->execute_step();
		std::string schema = stmt->getTextString(0);
		stmt->reset();
		return schema;
	}

}

#include "common.h"
#include "database.h"
#include "TraceLogger.h"

namespace SQLiteHelper {

	Database::Database(const string& path_db)
	{
		open(path_db);
	}

	Database::~Database() {
		close();
	}

	sqlite3* Database::connection()
	{
		return db;
	}

	void Database::open(const std::string& path_db)
	{
		if (sqlite3_open(path_db.c_str(), &db) != SQLITE_OK)
		{
			db = NULL;
			string lastError = "Error with opening database " + path_db + "!";
			throw SQLiteException(lastError);
		}
	}

	void Database::open_v2(const std::string& fileName, int flags, int busyTimeoutMs, const std::string& vfs)
	{
		const int result = sqlite3_open_v2(fileName.c_str(), &db, flags, vfs.empty() ? nullptr : vfs.c_str());
		if (SQLITE_OK == result) {
			if (busyTimeoutMs > 0) {
				setBusyTimeout(busyTimeoutMs);
			}
		}
		else {
			SQLiteException SQLiteException(db);
			sqlite3_close_v2(db);
			throw SQLiteException;
		}
	}

	void Database::close()
	{
		const int result = sqlite3_close(db);
		if (SQLITE_OK != result)
		{
			throw SQLiteException(db);
		}
	}

	void Database::close_v2()
	{
		const int result = sqlite3_close_v2(db);
		if (SQLITE_OK != result)
		{
			throw SQLiteException(db);
		}
	}

	void Database::changes()
	{
		const int result = sqlite3_changes(db);
		if (SQLITE_OK != result)
		{
			throw SQLiteException(db);
		}
	}

	void Database::setBusyTimeout(const int aBusyTimeoutMs) {
		const int result = sqlite3_busy_timeout(db, aBusyTimeoutMs);
		if (SQLITE_OK != result)
		{
			throw SQLiteException(db);
		}
	}

	void Database::executeNonQuery(string query)
	{
		if (db != NULL) {
			int ret = sqlite3_exec(db, query.c_str(), 0, 0, 0);
			if (SQLITE_OK != ret) {
				throw SQLiteException(db);
			}
		}
	}

	void Database::deleteTable(const std::string& table_name)
	{
		string query = "DELETE FROM " + table_name;
		executeNonQuery(query);
	}

	void Database::selectTableNames(std::vector<std::string>& tableNames)
	{
		std::string query = "SELECT name FROM sqlite_master WHERE type = 'table' ORDER BY 1";
		Statement* stmt = new Statement(this->db, query);
		while (stmt->execute_step()) {
			std::string tableName = stmt->getText(0); // Lấy giá trị của cột đầu tiên (tên bảng)
			tableNames.push_back(tableName);          // Thêm tên bảng vào vector
		}

		stmt->reset();
		delete stmt;
	}

	std::string Database::createInsertQuery(const std::string& table_name, DataTable* table)
	{
		if (table->getDataColumn()->size() == 0)
		{
			throw SQLiteException("The DataTable must have at least one column.");
		}

		std::string query = "INSERT INTO \"" + table_name + "\" (";

		// Lấy tên các cột từ DataColumn
		DataColumn* columns = table->getDataColumn();
		for (size_t i = 0; i < columns->size(); ++i) {
			query += "\"" + columns->getNameAt(i) + "\"";
			if (i < columns->size() - 1) {
				query += ", ";
			}
		}
		query += ") VALUES (";

		// Tạo danh sách các placeholder ?
		for (size_t i = 0; i < columns->size(); ++i) {
			query += "?";
			if (i < columns->size() - 1) {
				query += ", ";
			}
		}
		query += ")";

		return query;
	}

	int Database::importTable(const std::string& table_name, DataTable* table)
	{
		int success = 0;
		std::string query = createInsertQuery(table_name, table);
		Statement* stmt = new Statement(this->db, query);

		// Lặp qua các hàng dữ liệu trong DataTable và chèn từng hàng
		const auto& rows = table->getDataRow();
		for (const auto& row : rows)
		{
			if (writeNextRow(stmt, *table->getDataColumn(), *row) != SQLITE_DONE)
			{
				break;
			}
			success++;
		}

		stmt->close();
		return success;
	}

	int Database::exportTable(const std::string& table_name, DataTable* table)
	{
		int success = 0;
		string query = "SELECT * FROM " + table_name;
		Statement* stmt = new Statement(this->db, query);
		do {
			success++;
		} while (readNextRow(stmt, table));
		stmt->close();
		return success;
	}

	void Database::getListColumn(void* stmt, DataColumn* column)
	{
		Statement* istmt = static_cast<Statement*>(stmt);
		int columnCount = istmt->columnCount();
		if (istmt->execute_step())
		{
			for (int i = 0; i < columnCount; i++)
			{
				int columnType = istmt->columnType(i);
				string columnName = istmt->columnName(i);
				switch (columnType)
				{
				case SQLITE_INTEGER:
				{
					column->add(columnName, ObjectTypes::TYPE_INT);
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
		}
	}

	int Database::readNextRow(void* stmt, DataTable* table)
	{
		int result;
		Statement* istmt = static_cast<Statement*>(stmt);
		int columnCount = istmt->columnCount();

		if ((result = istmt->execute_step()))
		{
			auto row = std::make_unique<DataRow>(); // Sử dụng std::make_unique
			for (int i = 0; i < columnCount; i++)
			{
				switch (istmt->columnType(i))
				{
				case SQLITE_INTEGER:
					row->add(static_cast<long>(istmt->getInt64(i)), 0);
					break;
				case SQLITE_FLOAT:
					row->add(istmt->getDouble(i), 0);
					break;
				case SQLITE_TEXT:
					row->add(istmt->getText(i), istmt->getText(i).size());
					break;
				case SQLITE_BLOB:
				{
					const void* blobData = istmt->getBlob(i);
					int blobSize = istmt->getBytes(i);
					if (blobData && blobSize > 0)
					{
						uint8_t* blobCopy = new uint8_t[blobSize];
						memcpy(blobCopy, blobData, blobSize);
						row->add(blobCopy, blobSize); // Kích thước cho blob
					}
					else
					{
						row->add(nullptr, 0); // Kích thước cho blob null
					}
					break;
				}
				case SQLITE_NULL:
					row->add(static_cast<void*>(nullptr), 0); // Kích thước không cần thiết với null
					break;
				default:
					throw SQLiteException("Unsupported column type");
				}
			}
			table->addRow(std::move(row)); // Thêm hàng vào bảng
		}
		return result;
	}



	void Database::bindParameter(void* stmt, int index, int type, Object value)
	{
		Statement* istmt = static_cast<Statement*>(stmt);
		switch (type)
		{
		case SQLITE_INTEGER:
		{
			istmt->bindInt64(index, (long long)value.object_value.long_);
			break;
		}
		case SQLITE_FLOAT:
		{
			istmt->bindDouble(index, (double)value.object_value.double_);
			break;
		}
		case SQLITE_TEXT:
		{
			std::string text = value.object_value.str_;
			istmt->bindText(index, text);
			break;
		}
		case SQLITE_BLOB:
		{
			std::string data((char*)value.object_value.blob_);
			istmt->bindBlob(index, (BYTE*)data.c_str(), data.size());
			break;
		}
		default:
		{
			istmt->bindNull(index);
			break;
		}
		}
	}

	int Database::writeNextRow(void* stmt, DataColumn column_root, DataRow row)
	{
		int result;
		Statement* istmt = static_cast<Statement*>(stmt);
		if (column_root.size() != row.size())
		{
			string lastError = "Import database will fail if the number of columns of the imported table is different from the original table of the database.";
			throw SQLiteException(lastError);
		}
		int index = 1;
		// Lặp qua từng phần tử trong hàng để gán giá trị
		for (size_t i = 0; i < row.size(); ++i) {
			// Lấy giá trị từ hàng
			Object value = row.getValueAt(i); // Giả định có hàm getValueAt trong DataRow
			bindParameter(istmt, index, column_root.getTypeAt(i), value); // Giả định có hàm getTypeAt trong DataColumn
			index++;
		}
		result = istmt->execute_step();
		if (result == SQLITE_DONE)
		{
			changes();
			istmt->reset();
		}
		else if (result == SQLITE_ERROR)
		{
			istmt->reset();
			throw SQLiteException(db);
		}
		else if (result == SQLITE_CONSTRAINT)
		{
			istmt->reset();
			throw SQLiteException(db);
		}
		else if (result == SQLITE_NOTADB)
		{
			istmt->reset();
			throw SQLiteException(db);
		}
		else
		{
			istmt->reset();
			throw SQLiteException(db);
		}
		return result;
	}
}

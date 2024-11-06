#pragma once
#include <string>
#include "sqlite3.h"
#include "statement.h"
#include "aes_manager.h"
#include "DataTable/table.h"

using namespace GenericTable;

namespace SQLiteHelper 
{
    class Database
    {
    private:
        sqlite3* db;
        int err_code;
    public:
        Database();
        Database(const std::string& path, const int flags);
        ~Database();
        sqlite3* handle();

        /*
        - Purpose: Checks if the database is currently connected.
        - Return value: A boolean indicating the connection status (true if connected, false otherwise).
        */
        bool connected();
        /*
        - Purpose: Opens a database using UTF-16 encoding.
        - Parameters:
            1. [IN] path: The file path to the database in UTF-16 encoding.
        - Return value: A boolean indicating success (true) or failure (false) of the open operation.
        */
        bool open_16(const void* path);

        /*
        - Purpose: Opens a database connection with additional flags, timeout, and VFS settings.
        - Parameters:
            1. [IN] path: The file path of the SQLite database.
            2. [IN] flags: Flags that define the database open mode.
            3. [IN] busyTimeoutMs: The busy timeout in milliseconds.
            4. [IN] vfs: The VFS name to use for SQLite.
        - Return value: A boolean indicating success (true) or failure (false) of the open operation.
        */
        bool open_v2(const std::string& path, const int flags, const int busyTimeoutMs, const std::string& vfs);

        /*
        - Purpose: Closes the database connection.
        - Return value: A boolean indicating success (true) or failure (false) of the close operation.
        */
        bool close();

        /*
        - Purpose: Checks if the database is in read-only mode.
        - Return value: A boolean indicating if the database is read-only (true) or not (false).
        */
        bool is_readonly();

        /*
        - Purpose: Gets the number of rows affected by the last operation.
        - Return value: An integer representing the number of rows changed.
        */
        int changes();

        /*
        - Purpose: Gets the total number of changes since the database connection was opened.
        - Return value: An integer representing the total number of changes.
        */
        int totalChanges();

        /*
        - Purpose: Retrieves the last error code from the database.
        - Return value: An integer representing the last error code.
        */
        int getErrorCode();

        /*
        - Purpose: Gets the database schema based on the specified schema index.
        - Parameters:
            1. [IN] n: The index of the schema to retrieve.
        - Return value: A string representing the schema.
        */
        std::string schema(int n);

        /*
        - Purpose: Retrieves a string description of the last error encountered.
        - Return value: A string describing the last error.
        */
        std::string getErrorString();

        /*
        - Purpose: Sets an encryption key for the database.
        - Parameters:
            1. [IN] key: A pointer to the encryption key.
            2. [IN] size: The size of the encryption key.
        - Return value: A boolean indicating success (true) or failure (false) of the key setting operation.
        */
        bool setKeySee(const void* key, const int size);

        /*
        - Purpose: Sets the busy timeout for the database connection.
        - Parameters:
            1. [IN] aBusyTimeoutMs: The busy timeout in milliseconds.
        - Return value: A boolean indicating success (true) or failure (false) of the timeout setting.
        */
        bool setBusyTimeout(const int aBusyTimeoutMs);

        /*
        - Purpose: Executes a given SQL query on the database.
        - Parameters:
            1. [IN] query: The SQL query to execute.
        - Return value: A boolean indicating success (true) or failure (false) of the execution.
        */
        bool execute(const std::string& query);

        /*
        - Purpose: Executes a given SQL query and stores the result in a DataTable object.
        - Parameters:
            1. [IN] query: The SQL query to execute.
            2. [OUT] table: A pointer to a DataTable object to store the result.
        - Return value: A boolean indicating success (true) or failure (false) of the query execution.
        */
        bool executeQuery(const std::string& query, DataTable* table);
    private:
        void getParameter(void* stmt, const int index, const int type, DataRow* row);
        int readNextRow(void* stmt, DataRow* row);
        bool bindParameter(void* stmt, const int index, const int type, Object* value);
        int writeNextRow(void* stmt, DataColumn* column, DataRow* row);
    public:
        /*
        - Purpose: Deletes a specified table from the database.
        - Parameters:
            1. [IN] table_name: The name of the table to delete.
        - Return value: A boolean indicating success (true) or failure (false) of the delete operation.
        */
        bool deleteTable(const std::string& table_name);

        /*
        - Purpose: Exports data from the database based on a given query and stores it in a DataTable.
        - Parameters:
            1. [IN] query: The SQL query to retrieve data for export.
            2. [OUT] table: A pointer to a DataTable object to store the exported data.
        - Return value: An integer representing the result of the export operation.
        */
        int exportTable(const std::string& query, DataTable* table);

        /*
        - Purpose: Imports data into the database from a DataTable based on a given query.
        - Parameters:
            1. [IN] query: The SQL query specifying how to import the data.
            2. [IN] table: A pointer to a DataTable object containing the data to import.
        - Return value: An integer representing the result of the import operation.
        */
        int importTable(const std::string& query, DataTable* table);

        /*
        - Purpose: Retrieves the column information from a prepared SQL statement.
        - Parameters:
            1. [IN] stmt: A pointer to the prepared SQL statement.
            2. [OUT] column: A pointer to a DataColumn object to store column information.
        - Return value: A boolean indicating success (true) or failure (false) of the operation.
        */
        bool getColumnTable(void* stmt, DataColumn* column);

        /*
        - Purpose: Retrieves the names of all tables in the database.
        - Parameters:
            1. [OUT] list_name: A pointer to a vector that will store the names of the tables.
        - Return value: A boolean indicating success (true) or failure (false) of the operation.
        */
        bool getTableNames(std::vector<std::string>* list_name);

        /*
        - Purpose: Gets the schema of a specified table in the database.
        - Parameters:
            1. [IN] table_name: The name of the table to retrieve the schema for.
        - Return value: A string representing the table schema.
        */
        std::string getSchemaTable(const std::string& table_name);
    };
}


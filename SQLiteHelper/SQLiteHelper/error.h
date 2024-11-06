#pragma once

#define FUNCTION_DONE             0
#define FUNCTION_ERROR            1
#define OPEN_DB_ERROR             2
#define DATABASE_EMPTY            3
#define GET_TABLE_NAME_ERROR      4
#define DB_ENCRYPTED_SEE_NOTSET   5
#define DB_NOT_ENCRYPT_SET_SEE    6
#define SET_KEY_SEE_ERROR         7
#define FILE_NOT_FOUND            8
#define DELETE_FILE_ERROR         9
#define CREATE_DIR_ERROR          10
#define EXECUTE_ERROR             11
#define DB_NOT_SAME               12

const std::wstring ERROR_STRING[] = {
    L"Function executed successfully!", // Function_DONE
    L"An error occurred during function execution!",  // FUNCTION_ERROR
    L"Failed to open the database!",  // OPEN_DB_ERROR
    L"Database is empty!",  // DATABASE_EMPTY
    L"Failed to retrieve the table name from the database!",  // GET_TABLE_NAME_ERROR
    L"The database is encrypted, please set the key SEE!",  // DB_ENCRYPTED_SEE_NOTSET
    L"Unable to set the key SEE, the database was not previously encrypted!",  // DB_NOT_ENCRYPT_SET_SEE
    L"Failed to set the key SEE!",  // SET_KEY_SEE_ERROR
    L"File not found. Please ensure the path does not exceed 260 characters!",  // FILE_NOT_FOUND
    L"Failed to delete the file. The file is currently in use!",  // DELETE_FILE_ERROR
    L"Failed to create the nested directories!",  // CREATE_DIR_ERROR
    L"Failed to execute the query string!",  // EXECUTE_ERROR
    L"Failed to merge databases. The two databases are not the same!"  // DB_NOT_SAME
};

 
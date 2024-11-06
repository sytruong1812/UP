#pragma once
#include <map>
#include <io.h>
#include <vector>
#include <Shlwapi.h>
#include <Windows.h>

class Utils
{
public:
    /*
    - Purpose: Checks if the provided string represents a valid date and time.
    - Parameters:
        1. [IN] value: The string to check.
    - Return value: A boolean indicating if the string is a valid date and time.
    */
    static bool isDateTime(const std::string& value);

    /*
    - Purpose: Retrieves the current system time as a formatted string.
    - Return value: A string representing the current system time.
    */
    static std::string getCurrentTime();

    /*
    - Purpose: Returns the current date and time as a formatted string, with the year adjusted by the given `year` value.
    - Syntax:
        1. [IN] WORD year
        2. [OUT] std::string
    - Parameters:
        1. year: The number of years to adjust the current year by.
    - Return value: A formatted date-time string in the format "YYYY-MM-DD HH:MM:SS" with the adjusted year.
    */
    static std::string getCurrentDateTime(WORD year);

    /*
    - Purpose: Determines the format of the provided date and time string.
    - Parameters:
        1. [IN] value: The date and time string to evaluate.
    - Return value: A string representing the format of the provided date and time.
    */
    static std::string getFormat(const std::string& value);
    /*
    - Purpose: Converts a SYSTEMTIME structure to a formatted string.
    - Parameters:
        1. [IN] systime: The SYSTEMTIME structure to convert.
    - Return value: A string representing the SYSTEMTIME in a formatted date-time format.
    */
    static std::string systemTimeToString(const SYSTEMTIME& systime);

    /*
    - Purpose: Converts a formatted date-time string to a SYSTEMTIME structure.
    - Parameters:
        1. [IN] systime: The date-time string to convert.
    - Return value: A SYSTEMTIME structure representing the date and time.
    */
    static SYSTEMTIME stringToSystemTime(const std::string& systime);

    /*
    - Purpose: Converts a date-time string to a time_t object.
    - Parameters:
        1. [IN] timestamp: The date-time string to convert.
    - Return value: A time_t object representing the date and time.
    */
    static std::time_t getTimeFromString(const std::string& timestamp);

    /*
    - Purpose: Converts a time_t object to a formatted date-time string.
    - Parameters:
        1. [IN] timestamp: The time_t object to convert.
    - Return value: A string representing the date and time.
    */
    static std::string getStringFromTime(const std::time_t timestamp);

    /*
    - Purpose: Converts a standard string (std::string) to a wide string (std::wstring) using the current locale.
    - Syntax:
        1. [IN] std::string str
        2. [OUT] std::wstring
    - Parameters:
        1. str: The standard string to be converted.
    - Return value: The corresponding wide string (std::wstring).
    */
    static std::wstring str2wstr(const std::string& str);

    /*
    - Purpose: Converts a wide string (std::wstring) to a standard string (std::string) using the current locale.
    - Syntax:
        1. [IN] std::wstring wstr
        2. [OUT] std::string
    - Parameters:
        1. wstr: The wide string to be converted.
    - Return value: The corresponding standard string (std::string).
    */
    static std::string wstr2str(const std::wstring& wstr);

    /*
    - Purpose: Converts a multibyte character string (const char*) to a wide character string (wchar_t*) using UTF-8 encoding.
    - Syntax:
        1. [IN] const char* st_wd
        2. [OUT] wchar_t*
    - Parameters:
        1. st_wd: The multibyte character string to be converted.
    - Return value: The corresponding wide character string (wchar_t*). The caller is responsible for freeing the memory.
    */
    static char* wstr2str_v2(const wchar_t* ws_wd);

    /*
    - Purpose: Converts a wide character string (const wchar_t*) to a multibyte character string (char*) using UTF-8 encoding.
    - Syntax:
        1. [IN] const wchar_t* ws_wd
        2. [OUT] char*
    - Parameters:
        1. ws_wd: The wide character string to be converted.
    - Return value: The corresponding multibyte character string (char*). The caller is responsible for freeing the memory.
    */
    static wchar_t* str2wstr_v2(const char* st_wd);

    /*
    - Purpose: This function checks whether the given string is a number.
    - Syntax:
        [IN]  s: The string to be checked.
    - Parameters:
        s: The string that needs to be checked.
    - Return value:
        Returns true if the string is a number, false otherwise.
    */
    static bool is_number(const std::string& s);

    /*
    - Purpose: This function converts the entire string to lowercase.
    - Syntax:
        [IN/OUT]  s: The string to be converted to lowercase.
    - Parameters:
        s: The string that needs to be converted to lowercase.
    - Return value:
        None.
    */
    static void to_lower(std::wstring& s);

    /*
    - Purpose: This function converts the entire string to uppercase.
    - Syntax:
        [IN/OUT]  s: The string to be converted to uppercase.
    - Parameters:
        s: The string that needs to be converted to uppercase.
    - Return value:
        None.
    */
    static void to_upper(std::wstring& s);

    /*
    - Purpose: This function removes a substring from the original string.
    - Syntax:
        [IN/OUT]  str: The original string from which a substring will be removed.
        [IN]       str_to_rm: The substring to remove.
    - Parameters:
        str: The string that will have a substring removed.
        str_to_rm: The substring to be removed from the string.
    - Return value:
        None.
    */
    static void remove_sstr(std::string& str, const std::string& str_to_rm);

    /*
    - Purpose: This function retrieves the executable file's name.
    - Syntax:
        [OUT]  Returns the name of the executable file.
    - Parameters:
        None.
    - Return value:
        The name of the executable file.
    */
    static std::wstring get_exe_name();

    /*
    - Purpose: This function extracts the file name from a full path.
    - Syntax:
        [IN]   path: The full path from which to extract the file name.
    - Parameters:
        path: The full path of the file.
    - Return value:
        The file name extracted from the given path.
    */
    static std::wstring getFileFromPath(const std::wstring& path);

    /*
    - Purpose: This function extracts the directory name from a full path.
    - Syntax:
        [IN]   path: The full path from which to extract the directory name.
    - Parameters:
        path: The full path from which to extract the directory.
    - Return value:
        The directory name extracted from the given path.
    */
    static std::wstring getDirFromPath(const std::wstring& path);

    /*
    - Purpose: This function retrieves the value of an environment variable.
    - Syntax:
        [IN]   env: The environment variable name.
    - Parameters:
        env: The name of the environment variable.
    - Return value:
        The value of the environment variable.
    */
    static std::wstring getPathFromEnv(const std::wstring& env);

    /*
    - Purpose: This function checks if a given path exists.
    - Syntax:
        [IN]   path: The path to check for existence.
    - Parameters:
        path: The path to check for existence.
    - Return value:
        Returns true if the path exists, false otherwise.
    */
    static bool path_exists(const std::wstring& path);

    /*
    - Purpose: This function deletes a file at the given path.
    - Syntax:
        [IN]   path: The file path to be deleted.
    - Parameters:
        path: The path of the file to be deleted.
    - Return value:
        Returns true if the file was deleted successfully, false otherwise.
    */
    static bool delete_file(const std::wstring& path);

    /*
    - Purpose: This function deletes a folder at the given path.
    - Syntax:
        [IN]   path: The folder path to be deleted.
    - Parameters:
        path: The path of the folder to be deleted.
    - Return value:
        Returns true if the folder was deleted successfully, false otherwise.
    */
    static bool delete_folder(const std::wstring& path);

    /*
    - Purpose: This function checks if a file is empty.
    - Syntax:
        [IN]   filename: The file name to check.
    - Parameters:
        filename: The file to check for emptiness.
    - Return value:
        Returns true if the file is empty, false if it contains data.
    */
    static bool isFileEmpty(const std::wstring& filename);

    /*
    - Purpose: Creates a nested directory structure at the specified path.
    - Parameters:
        1. [IN] dir: The path of the directory structure to create.
    - Return value: A boolean indicating success (true) or failure (false) of the directory creation.
    */
    static bool createNestedDir(const std::wstring& dir);

    /*
    - Purpose: Retrieves a list of files in the specified folder.
    - Parameters:
        1. [IN] path: The folder path to retrieve files from.
    - Return value: A vector of WIN32_FIND_DATA structures representing the files.
    */
    static std::vector<WIN32_FIND_DATA> getFilesInFolder(const std::wstring& path);

    /*
    - Purpose: Recursively retrieves files from the specified folder and its subfolders.
    - Parameters:
        1. [IN] path: The root folder path to start the search.
        2. [OUT] files: A pointer to a vector to store the retrieved files.
    - Return value: A boolean indicating success (true) or failure (false) of the file retrieval.
    */
    static bool getFilesInNestedFolder(const std::wstring& path, std::vector<WIN32_FIND_DATA>* files);

    /*
    - Purpose: Retrieves files from nested folders and associates input and output paths for each file.
    - Parameters:
        1. [IN] path_in: The input folder path.
        2. [IN] path_out: The output folder path.
        3. [OUT] contents: A pointer to a vector to store pairs of input and output file paths.
    - Return value: A boolean indicating success (true) or failure (false) of the file retrieval.
    */
    static bool getFilesInNestedFolder_v2(const std::wstring& path_in, const std::wstring& path_out, std::vector<std::pair<std::wstring, std::wstring>>* contents);
};
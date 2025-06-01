#pragma once
#include <io.h>
#include <ctime>
#include <chrono>
#include <vector>
#include <string>
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace Helper 
{
    std::string createUUIDString();
    std::wstring getExecutableName();
    BOOL generateRandomBytes(uint8_t* data, size_t length);

    class TimeHelper 
    {
    public:
        static SYSTEMTIME getCurrentTime();
        static int compareSystemTime(SYSTEMTIME st1, SYSTEMTIME st2);
        static BOOL isValidDateTime(const std::string& value);
        static std::string getDateTimeFormat(const std::string& value);
        static FILETIME convertStringToFileTime(const std::string& filetime);
        static std::string convertFileTimeToString(const FILETIME& filetime);
        static SYSTEMTIME convertStringToSystemTime(const std::string& systime);
        static std::string convertSystemTimeToString(const SYSTEMTIME& systime);
        static std::time_t convertStringToTime(const std::string& timestamp);
        static std::string convertTimeToString(const std::time_t timestamp);
        static LARGE_INTEGER convertFileTimeLargeInteger(const FILETIME& filetime);
        static FILETIME convertLargeIntegerToFileTime(const LARGE_INTEGER& integer);
    };

    class PathHelper 
    {
    public:
        static BOOL isDirectory(const std::wstring& path);
        static BOOL isValidFilePath(const std::wstring& path);
        static std::wstring extractFolderFromFilePath(const std::wstring& path);
        static std::wstring extractFileNameFromFilePath(const std::wstring& path);
        static std::wstring extractFolderNameFromFilePath(const std::wstring& path);
        static std::wstring extractFileExtensionFromFilePath(const std::wstring& path);
        static std::wstring extractLastComponentFromPath(const std::wstring& path);
        static std::wstring removeLastComponentFromPath(const std::wstring& path);
        static std::wstring getPathFromEnvironmentVariable(const std::wstring& env);
        static std::wstring combinePathComponent(const std::wstring& path, const std::wstring& component);
    };

    class StringHelper
    {
    public:
        static BOOL isStringHex(const std::wstring& text);
        static BOOL isStringNumeric(const std::wstring& text);
        static std::string convertBytesHexString(const BYTE* byteArray, size_t length);
        static std::wstring convertToLowerCase(const std::wstring& text);
        static std::wstring convertToUpperCase(const std::wstring& text);

        static wchar_t* convertStringToWideString(const char* c);
        static char* convertWideStringToString(const wchar_t* wc);
        static std::wstring convertStringToWideString(const std::string& str);
        static std::string convertWideStringToString(const std::wstring& wstr);

        static void removeSubstring(std::string& str, const std::string& str_to_rm);
        static void removeSubwstring(std::wstring& str, const std::wstring& str_to_rm);
        static void replaceSubstring(std::string& str, const std::string& from, const std::string& to);
        static void replaceSubwstring(std::wstring& str, const std::wstring& from, const std::wstring& to);
    };
};

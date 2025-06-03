#include <map>
#include <regex>
#include <random>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iostream>
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "utils.h"

namespace Helper 
{
    std::string createUUIDString()
    {
        GUID guid;
        char guid_string[37]; // 32 hex chars + 4 hyphens + null terminator
        if (CoCreateGuid(&guid) != S_OK)
        {
            throw std::runtime_error("Failed to create GUID");
        }
        snprintf(guid_string, sizeof(guid_string),
            "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2],
            guid.Data4[3], guid.Data4[4], guid.Data4[5],
            guid.Data4[6], guid.Data4[7]);
        return guid_string;
    }

    std::wstring getExecutableName()
    {
        WCHAR lpwzExePath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, lpwzExePath, MAX_PATH);
        LPWSTR wzEnd = wcsrchr(lpwzExePath, L'\\');
        if (wzEnd != NULL)
        {
            return std::wstring(wzEnd + 1);
        }
        return std::wstring(lpwzExePath);
    }

    BOOL generateRandomBytes(uint8_t* data, size_t length)
    {
        if (data == NULL)
        {
            return FALSE;
        }
        std::random_device rd;  // Seed for random number generation
        std::mt19937 generator(rd());   // Mersenne Twister RNG
        std::uniform_int_distribution<int> distribution(0, 255);    // Range for bytes
        for (size_t i = 0; i < length; ++i)
        {
            data[i] = static_cast<uint8_t>(distribution(generator));
        }
        return TRUE;
    }

    static const std::map<std::string, std::regex> _regular_expressions
    {
        // yyyy/mm/dd
        std::pair<std::string, std::regex>("%4d/%2d/%2d", std::regex("^\\d{4}/(0[1-9]|1[012])/(0[1-9]|[12][0-9]|3[01])$")),
        // yyyy-mm-dd                     
        std::pair<std::string, std::regex>("%4d-%2d-%2d", std::regex("^\\d{4}\\-(0[1-9]|1[012])\\-(0[1-9]|[12][0-9]|3[01])$")),
        // mm/dd/yyyy                     
        std::pair<std::string, std::regex>("%2d/%2d/%4d", std::regex("^(1[0-2]|0[1-9])/(3[01]|[12][0-9]|0[1-9])/[0-9]{4}$")),
        // mm-dd-yyyy                     
        std::pair<std::string, std::regex>("%2d-%2d-%4d", std::regex("^(1[0-2]|0[1-9])\\-(3[01]|[12][0-9]|0[1-9])\\-[0-9]{4}$")),
        // yyyy/mm/dd hh:mm:ss            
        std::pair<std::string, std::regex>("%4d/%2d/%2d %2d:%2d:%2d", std::regex("\\d{4}/\\d{2}/\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}")),
        // yyyy-mm-dd hh:mm:ss            
        std::pair<std::string, std::regex>("%4d-%2d-%2d %2d:%2d:%2d", std::regex("\\d{4}\\-\\d{2}\\-\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}")),
    };

    SYSTEMTIME TimeHelper::getCurrentTime()
    {
        SYSTEMTIME st, lt;
        GetSystemTime(&st);
        GetLocalTime(&lt);
        return lt;
    }

    int TimeHelper::compareSystemTime(SYSTEMTIME st1, SYSTEMTIME st2)
    {
        // Compare year
        if (st1.wYear < st2.wYear) return -1;
        if (st1.wYear > st2.wYear) return 1;

        // Compare month
        if (st1.wMonth < st2.wMonth) return -1;
        if (st1.wMonth > st2.wMonth) return 1;

        // Compare day
        if (st1.wDay < st2.wDay) return -1;
        if (st1.wDay > st2.wDay) return 1;

        // Compare hour
        if (st1.wHour < st2.wHour) return -1;
        if (st1.wHour > st2.wHour) return 1;

        // Compare minute
        if (st1.wMinute < st2.wMinute) return -1;
        if (st1.wMinute > st2.wMinute) return 1;

        // Compare second
        if (st1.wSecond < st2.wSecond) return -1;
        if (st1.wSecond > st2.wSecond) return 1;

        // Compare milliseconds
        if (st1.wMilliseconds < st2.wMilliseconds) return -1;
        if (st1.wMilliseconds > st2.wMilliseconds) return 1;

        // If all components are equal
        return 0;
    }

    BOOL TimeHelper::isValidDateTime(const std::string& value)
    {
        for (auto it = _regular_expressions.begin(); it != _regular_expressions.end(); it++)
        {
            if (std::regex_match(value, it->second))
            {
                return TRUE;
            }
        }
        return FALSE;
    }

    std::string TimeHelper::getDateTimeFormat(const std::string& value)
    {
        for (auto it = _regular_expressions.begin(); it != _regular_expressions.end(); it++)
        {
            if (std::regex_match(value, it->second))
            {
                return it->first;
            }
        }
        return std::string();
    }

    FILETIME TimeHelper::convertStringToFileTime(const std::string& sTime)
    {
        std::istringstream istr(sTime);
        SYSTEMTIME st = { 0 };
        FILETIME ft = { 0 };
        //Ex: 2023/09/2 02:30
        istr >> st.wYear;
        istr.ignore(1, '/');
        istr >> st.wMonth;
        istr.ignore(1, '/');
        istr >> st.wDay;
        istr.ignore(1, ' ');
        istr >> st.wHour;
        istr.ignore(1, ':');
        istr >> st.wMinute;
        SystemTimeToFileTime(&st, &ft);
        return ft;
    }

    std::string TimeHelper::convertFileTimeToString(const FILETIME& filetime)
    {
        SYSTEMTIME st;
        FileTimeToSystemTime(&filetime, &st);
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << st.wYear << "-"
            << std::setw(2) << st.wMonth << "-"
            << std::setw(2) << st.wDay << " "
            << std::setw(2) << st.wHour << ":"
            << std::setw(2) << st.wMinute << ":"
            << std::setw(2) << st.wSecond;
        return oss.str();
    }

    SYSTEMTIME TimeHelper::convertStringToSystemTime(const std::string& systime)
    {
        SYSTEMTIME st = { 0 };
        std::string format = getDateTimeFormat(systime);

        int t_wYear, t_wMonth, t_wDay;
        int t_wHour, t_wMinute, t_wSecond;

        sscanf_s(systime.c_str(), format.c_str(),
            &t_wYear, &t_wMonth, &t_wDay,
            &t_wHour, &t_wMinute, &t_wSecond);

        st.wYear = t_wYear; st.wMonth = t_wMonth; st.wDay = t_wDay;
        st.wHour = t_wHour; st.wMinute = t_wMinute; st.wSecond = t_wSecond;
        return st;
    }

    std::string TimeHelper::convertSystemTimeToString(const SYSTEMTIME& systime) {
        std::ostringstream oss;
        oss << std::setfill('0')
            << std::setw(4) << systime.wYear << "-"
            << std::setw(2) << systime.wMonth << "-"
            << std::setw(2) << systime.wDay << " "
            << std::setw(2) << systime.wHour << ":"
            << std::setw(2) << systime.wMinute << ":"
            << std::setw(2) << systime.wSecond;
        return oss.str();
    }

    std::time_t TimeHelper::convertStringToTime(const std::string& timestamp)
    {
        FILETIME ft;
        SYSTEMTIME st = convertStringToSystemTime(timestamp);
        SystemTimeToFileTime(&st, &ft);
        //Converts a FILETIME structure into a 64 bit integer
        std::time_t t64 = static_cast<__int64>(ft.dwHighDateTime) << 32 | ft.dwLowDateTime;
        return (t64 / 10);    // 1 microsecond = 10 * 100 - nanoseconds.
    }

    std::string TimeHelper::convertTimeToString(const std::time_t timestamp)
    {
        SYSTEMTIME st;
        std::time_t t64 = timestamp * 10;       // 1 microsecond = 10 * 100 - nanoseconds
        //Converts a 64 bit integer into a FILETIME structure
        FILETIME ft = { static_cast<DWORD>(t64 & 0xFFFFFFFF), static_cast<DWORD>(t64 >> 32) };
        FileTimeToSystemTime(&ft, &st);
        return convertSystemTimeToString(st);
    }

    LARGE_INTEGER TimeHelper::convertFileTimeLargeInteger(const FILETIME& filetime) {
        LARGE_INTEGER li;
        li.LowPart = filetime.dwLowDateTime;
        li.HighPart = filetime.dwHighDateTime;
        return li;
    }

    FILETIME TimeHelper::convertLargeIntegerToFileTime(const LARGE_INTEGER& integer) {
        FILETIME ft;
        ft.dwLowDateTime = integer.LowPart;
        ft.dwHighDateTime = integer.HighPart;
        return ft;
    }

    BOOL PathHelper::isFile(const std::wstring& path)
    {
        DWORD attrs = GetFileAttributesW(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    BOOL PathHelper::isFolder(const std::wstring& path)
    {
        DWORD attrs = GetFileAttributesW(path.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    BOOL PathHelper::isValidFilePath(const std::wstring& path)
    {
        if (path.empty())
        {
            return FALSE;
        }
        if (path.length() > 260)
        {
            return FALSE;
        }
        for (wchar_t c : path)
        {
            if (c == L'<' || c == L'>' || c == L'"' || c == L'/' ||
                c == L'|' || c == L'?' || c == L'*' || c == L'`' )
            {
                return FALSE;
            }
        }
        size_t dotPosition = path.rfind(L'.');
        size_t slashPosition = path.find_last_of(L"/\\");
        return (dotPosition != std::wstring::npos) && (dotPosition > slashPosition);
    }

    BOOL PathHelper::isSubPath(const std::wstring& parent, const std::wstring& child)
    {
        return child.length() > parent.length() && child.compare(0, parent.length(), parent) == 0;
    }

    // Check if childPath is a subfolder of parentPath
    BOOL PathHelper::isSubFolder(const std::wstring& parentPath, const std::wstring& childPath)
    {
        return childPath.find(parentPath) == 0 && childPath != parentPath;
    }

    // Check if parentPath is the parent of childPath
    BOOL PathHelper::isParentFolder(const std::wstring& parentPath, const std::wstring& childPath)
    {
        return isSubFolder(parentPath, childPath);
    }

    DWORD PathHelper::getPathLevel(const std::wstring& path)
    {
        DWORD n = 0;
        size_t pos = 0;
        do {
            pos = path.find(L'\\', pos);
            if (pos++ > 0) {
                n++;
            }
        } while (pos > 0);
        return n;
    }

    std::wstring PathHelper::extractParentPathFromPath(const std::wstring& path)
    {
        size_t lastSlashPos = path.find_last_of(L"/\\");
        if (lastSlashPos != std::wstring::npos && lastSlashPos > 1)
        {
            return path.substr(0, lastSlashPos);
        }
        return path;
    }

    std::wstring PathHelper::extractFileNameFromFilePath(const std::wstring& path)
    {
        std::wstring file_name;
        size_t lastSlashPos = path.find_last_of(L"/\\");
        if (lastSlashPos != std::wstring::npos && lastSlashPos > 1)
        {
            file_name = path.substr(lastSlashPos + 1);
        }
        return file_name;
    }

    std::wstring PathHelper::extractParentNameFromPath(const std::wstring& path)
    {
        std::wstring folder_name;
        size_t lastSlashPos = path.find_last_of(L"/\\");
        if (lastSlashPos != std::wstring::npos && lastSlashPos > 1)
        {
            std::wstring folder_path = path.substr(0, lastSlashPos);
            size_t lastSlashPos2 = folder_path.find_last_of(L"/\\");
            if (lastSlashPos2 != std::wstring::npos && lastSlashPos2 > 1)
            {
                folder_name = folder_path.substr(lastSlashPos2 + 1);
            }
        }
        return folder_name;
    }

    std::wstring PathHelper::extractFileExtensionFromFilePath(const std::wstring& path)
    {
        std::wstring file_extension = L"";
        size_t lastSlashPos = path.find_last_of(L"/\\");
        if (lastSlashPos != std::wstring::npos && lastSlashPos > 1)
        {
            std::wstring file_name = path.substr(lastSlashPos + 1);
            size_t lastDotPos = file_name.find_last_of(L".");
            if (lastDotPos != std::wstring::npos && lastDotPos > 1)
            {
                file_extension = file_name.substr(lastDotPos);
            }
        }
        return file_extension;
    }

    std::wstring PathHelper::extractLastComponentFromPath(const std::wstring& path)
    {
        std::wstring lastComponent;
        size_t lastSlashPos = path.find_last_of(L"/\\");
        if (lastSlashPos != std::wstring::npos && lastSlashPos > 1)
        {
            lastComponent = path.substr(lastSlashPos + 1);
        }
        return lastComponent;
    }

    std::wstring PathHelper::removeLastComponentFromPath(const std::wstring& path)
    {
        size_t lastSlashPos = path.find_last_of(L"/\\");
        if (lastSlashPos != std::wstring::npos && lastSlashPos > 1)
        {
            return path.substr(0, lastSlashPos);
        }
        return path;
    }

    std::wstring PathHelper::getPathFromEnvironmentVariable(const std::wstring& env)
    {
        wchar_t envPath[MAX_PATH];
        int pathLength = ExpandEnvironmentStringsW(env.c_str(), envPath, MAX_PATH);
        return (pathLength != 0 && pathLength < MAX_PATH) ? envPath : env;
    }

    std::wstring PathHelper::combinePathComponent(const std::wstring& path, const std::wstring& component) {
    
        if (!path.empty() && path.back() != L'\\')
        {
            return path + L'\\' + component;
        }
        return path + component;
    }


    BOOL StringHelper::isStringNumeric(const std::wstring& text)
    {
        auto it = text.begin();
        while (it != text.end() && std::isdigit(*it)) ++it;
        return !text.empty() && it == text.end();
    }

    BOOL StringHelper::isStringHex(const std::wstring& text)
    {
        return 0;
    }

    std::string StringHelper::convertBytesHexString(const BYTE* byteArray, size_t length)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < length; ++i)
        {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)byteArray[i];
        }
        return oss.str();
    }

    std::wstring StringHelper::convertToLowerCase(const std::wstring& text)
    {
        std::wstring s = text;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    std::wstring StringHelper::convertToUpperCase(const std::wstring& text)
    {
        std::wstring s = text;
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return s;
    }

    wchar_t* StringHelper::convertStringToWideString(const char* c)
    {
        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, c, -1, NULL, 0);
        wchar_t* wstr = new wchar_t[wchars_num];
        MultiByteToWideChar(CP_UTF8, 0, c, -1, wstr, wchars_num);
        return wstr;
    }

    char* StringHelper::convertWideStringToString(const wchar_t* wc)
    {
        int char_num = WideCharToMultiByte(CP_UTF8, 0, wc, -1, NULL, 0, NULL, NULL);
        char* str = new char[char_num];
        WideCharToMultiByte(CP_UTF8, 0, wc, -1, str, char_num, NULL, NULL);
        return str;
    }

    std::wstring StringHelper::convertStringToWideString(const std::string& str)
    {
        return convertStringToWideString(str.c_str());
    }

    std::string StringHelper::convertWideStringToString(const std::wstring& wstr)
    {
        return convertWideStringToString(wstr.c_str());
    }

    void StringHelper::removeSubstring(std::string& str, const std::string& str_to_rm)
    {
        for (auto i = str.find(str_to_rm); i != std::string::npos; i = str.find(str_to_rm))
        {
            str.erase(i, str_to_rm.size());
        }
    }

    void StringHelper::removeSubwstring(std::wstring& str, const std::wstring& str_to_rm)
    {
        for (auto i = str.find(str_to_rm); i != std::wstring::npos; i = str.find(str_to_rm))
        {
            str.erase(i, str_to_rm.size());
        }
    }

    void StringHelper::replaceSubstring(std::string& str, const std::string& from, const std::string& to)
    {
        size_t pos = 0;
        while ((pos = str.find(from)) != std::string::npos)
        {
            str.replace(pos, from.length(), to);
        };
    }
    void StringHelper::replaceSubwstring(std::wstring& str, const std::wstring& from, const std::wstring& to)
    {
        size_t pos = 0;
        while ((pos = str.find(from)) != std::wstring::npos)
        {
            str.replace(pos, from.length(), to);
        };
    }

}
#include <map>
#include <regex>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <strsafe.h>
#include <windows.h>
#pragma comment(lib, "User32.lib")
#include "utils.h"

static const std::map<std::string, std::regex> _regular_expressions{
    // yyyy/mm/dd
    std::pair<std::string, std::regex>("%d/%d/%d", std::regex("^\\d{4}/(0[1-9]|1[012])/(0[1-9]|[12][0-9]|3[01])$")),
    // yyyy-mm-dd
    std::pair<std::string, std::regex>("%d-%d-%d", std::regex("^\\d{4}\\-(0[1-9]|1[012])\\-(0[1-9]|[12][0-9]|3[01])$")),
    // mm/dd/yyyy
    std::pair<std::string, std::regex>("%d/%d/%d", std::regex("^(1[0-2]|0[1-9])/(3[01]|[12][0-9]|0[1-9])/[0-9]{4}$")),
    // mm-dd-yyyy
    std::pair<std::string, std::regex>("%d-%d-%d", std::regex("^(1[0-2]|0[1-9])\\-(3[01]|[12][0-9]|0[1-9])\\-[0-9]{4}$")),
    // yyyy/mm/dd hh:mm:ss
    std::pair<std::string, std::regex>("%d/%d/%d %d:%d:%d", std::regex("\\d{4}/\\d{2}/\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}")),
    // yyyy-mm-dd hh:mm:ss
    std::pair<std::string, std::regex>("%d-%d-%d %d:%d:%d", std::regex("\\d{4}\\-\\d{2}\\-\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}"))
};

std::string Utils::getCurrentTime()
{
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    char tmp[64] = { '\0' };
    sprintf_s(tmp, "%02d:%02d:%02d:%02d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
    return tmp;
}

std::string Utils::getCurrentDateTime(WORD year)
{
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    char tmp[64] = { '\0' };
    sprintf_s(tmp, "%04d-%02d-%02d %02d:%02d:%02d", sys.wYear + year, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond);
    return tmp;
}

bool Utils::isDateTime(const std::string& value)
{
    for (auto it = _regular_expressions.begin(); it != _regular_expressions.end(); it++)
    {
        if (std::regex_match(value, it->second)) {
            return true;
        }
    }
    return false;
}

std::string Utils::getFormat(const std::string& value)
{
    for (auto it = _regular_expressions.begin(); it != _regular_expressions.end(); it++)
    {
        if (std::regex_match(value, it->second))
            return it->first;
    }
    return std::string();
}

std::string Utils::systemTimeToString(const SYSTEMTIME& systime) {
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

SYSTEMTIME Utils::stringToSystemTime(const std::string& systime)
{
    SYSTEMTIME st;
    memset(&st, 0, sizeof(st));
    sscanf_s(systime.c_str(), getFormat(systime).c_str(),
        &st.wYear,
        &st.wMonth,
        &st.wDay,
        &st.wHour,
        &st.wMinute,
        &st.wSecond);
    return st;
}

std::time_t Utils::getTimeFromString(const std::string& timestamp)
{
    SYSTEMTIME st = stringToSystemTime(timestamp);
    FILETIME ft;
    SystemTimeToFileTime(&st, &ft);
    //Converts a FILETIME structure into a 64 bit integer
    std::time_t t64 = static_cast<__int64>(ft.dwHighDateTime) << 32 | ft.dwLowDateTime;
    return (t64 / 10);    // 1 microsecond = 10 * 100 - nanoseconds.
}

std::string Utils::getStringFromTime(const std::time_t timestamp)
{
    SYSTEMTIME st;
    std::time_t t64 = timestamp * 10;       // 1 microsecond = 10 * 100 - nanoseconds
    //Converts a 64 bit integer into a FILETIME structure
    FILETIME ft = { static_cast<DWORD>(t64 & 0xFFFFFFFF), static_cast<DWORD>(t64 >> 32) };
    FileTimeToSystemTime(&ft, &st);
    return Utils::systemTimeToString(st);
}

std::wstring Utils::str2wstr(const std::string& str)
{
    const std::locale& loc = std::locale{};
    std::vector<wchar_t> buf(str.size());
    std::use_facet<std::ctype<wchar_t>>(loc).widen(str.data(), str.data() + str.size(), buf.data());
    return std::wstring(buf.data(), buf.size());
}

std::string Utils::wstr2str(const std::wstring& wstr)
{
    const std::locale& loc = std::locale{};
    std::vector<char> buf(wstr.size());
    std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
    return std::string(buf.data(), buf.size());
}

wchar_t* Utils::str2wstr_v2(const char* st_wd)
{
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, st_wd, -1, NULL, 0);
    wchar_t* wideCharString = new wchar_t[wideCharLength];
    MultiByteToWideChar(CP_UTF8, 0, st_wd, -1, wideCharString, wideCharLength);
    return wideCharString;
}

char* Utils::wstr2str_v2(const wchar_t* ws_wd)
{
    int multibyteLength = WideCharToMultiByte(CP_UTF8, 0, ws_wd, -1, NULL, 0, NULL, NULL);
    char* multibyteString = new char[multibyteLength];
    WideCharToMultiByte(CP_UTF8, 0, ws_wd, -1, multibyteString, multibyteLength, NULL, NULL);
    return multibyteString;
}

bool Utils::is_number(const std::string& s)
{
    auto it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

void Utils::remove_sstr(std::string& str, const std::string& str_to_rm)
{
    for (auto i = str.find(str_to_rm); i != std::string::npos; i = str.find(str_to_rm))
    {
        str.erase(i, str_to_rm.size());
    }
}

void Utils::to_lower(std::wstring& s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}

void Utils::to_upper(std::wstring& s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}

std::wstring Utils::get_exe_name()
{
    WCHAR lpwzExePath[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, lpwzExePath, MAX_PATH);
    LPWSTR wzEnd = wcsrchr(lpwzExePath, L'\\');
    if (wzEnd != nullptr)
    {
        return std::wstring(wzEnd + 1);
    }
    return std::wstring(lpwzExePath);
}

std::wstring Utils::getFileFromPath(const std::wstring& path)
{
    std::wstring file_name = L"";
    size_t lastSeparatorPos = path.find_last_of(L"\\");
    if (lastSeparatorPos != std::wstring::npos && lastSeparatorPos > 1) {
        file_name = path.substr(lastSeparatorPos + 1);
    }
    return file_name;
}

std::wstring Utils::getDirFromPath(const std::wstring& path)
{
    size_t lastSeparatorPos = path.find_last_of(L"\\");
    if (lastSeparatorPos != std::wstring::npos && lastSeparatorPos > 1) {
        return path.substr(0, lastSeparatorPos);
    }
    return path;
}

std::wstring Utils::getPathFromEnv(const std::wstring& env)
{
    wchar_t envPath[MAX_PATH];
    int pathLength = ExpandEnvironmentStringsW(env.c_str(), envPath, MAX_PATH);
    return (pathLength != 0 && pathLength < MAX_PATH) ? envPath : env;
}

bool Utils::path_exists(const std::wstring& path)
{
    return _waccess_s(path.c_str(), 0) == 0 ? true : false;
}

bool Utils::delete_file(const std::wstring& path)
{
    return DeleteFileW(path.c_str());
}

bool Utils::delete_folder(const std::wstring& path)
{
    return RemoveDirectoryW(path.c_str());
}

bool Utils::isFileEmpty(const std::wstring& filename) {
    std::ifstream file(filename);
    return file.peek() == std::ifstream::traits_type::eof();
}

bool Utils::createNestedDir(const std::wstring& path)
{
    if (path.size() > MAX_PATH)
    {
        return false;
    }
    if (PathFileExistsW(path.c_str())) {
        return true;
    }
    else {
        size_t lastSeparatorPos = path.find_last_of(L"\\");
        if (lastSeparatorPos != std::wstring::npos && lastSeparatorPos > 1) {
            std::wstring parentPath = path.substr(0, lastSeparatorPos);
            if (createNestedDir(parentPath)) {
                return CreateDirectoryW(path.c_str(), NULL);
            }
        }
        return false;
    }
}

std::vector<WIN32_FIND_DATA> Utils::getFilesInFolder(const std::wstring& path)
{
    std::vector<WIN32_FIND_DATA> list_file;
    if (wcslen(path.c_str()) > (MAX_PATH - 3)) {
        return {};
    }

    WCHAR szDir[MAX_PATH];
    StringCchCopyW(szDir, MAX_PATH, path.c_str());
    StringCchCatW(szDir, MAX_PATH, L"\\*");

    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFileW(szDir, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        return {};
    }
    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            list_file.push_back(ffd);
        }
    } while (FindNextFileW(hFind, &ffd) != 0);
    if (hFind != NULL) {
        FindClose(hFind);
    }
    return list_file;
}


bool Utils::getFilesInNestedFolder(const std::wstring& path, std::vector<WIN32_FIND_DATA>* files) 
{
    WIN32_FIND_DATA ffd;
    if (wcslen(path.c_str()) > (MAX_PATH - 3)) {
        return false;
    }
    HANDLE hFind = FindFirstFileW((path + L"\\*").c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }
    do
    {
        if (!wcscmp(L".", ffd.cFileName) || !wcscmp(L"..", ffd.cFileName)) {
            continue;
        }
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::wstring folder_name = ffd.cFileName;
            std::wstring folder = path + L"\\" + folder_name;
            if (getFilesInNestedFolder(folder, files) == false) {
                return false;
            }
        }
        else {
            files->push_back(ffd);
        }
    } while (FindNextFileW(hFind, &ffd) != 0);
    if (hFind != NULL) {
        FindClose(hFind);
    }
    return true;
}

bool Utils::getFilesInNestedFolder_v2(const std::wstring& path_in, const std::wstring& path_out, std::vector<std::pair<std::wstring, std::wstring>>* contents)
{
    WIN32_FIND_DATA ffd;
    if (wcslen(path_in.c_str()) > (MAX_PATH - 3)) {
        return false;
    }
    HANDLE hFind = FindFirstFileW((path_in + L"\\*").c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }
    do
    {
        if (!wcscmp(L".", ffd.cFileName) || !wcscmp(L"..", ffd.cFileName)) {
            continue;
        }
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            std::wstring folder_name = ffd.cFileName;
            std::wstring folder_in = path_in + L"\\" + folder_name;
            std::wstring folder_out = path_out + L"\\" + folder_name;
            if (getFilesInNestedFolder_v2(folder_in, folder_out, contents) == false) {
                return false;
            }
        }
        else {
            std::wstring file_name = ffd.cFileName;
            std::wstring file_in = path_in + L"\\" + file_name;
            std::wstring file_out = path_out + L"\\" + file_name;
            contents->push_back(std::pair<std::wstring, std::wstring>(file_in, file_out));
        }
    } while (FindNextFileW(hFind, &ffd) != 0);
    if (hFind != NULL) {
        FindClose(hFind);
    }
    return true;
}


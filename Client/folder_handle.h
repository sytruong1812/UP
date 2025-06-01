#pragma once
#include <windows.h>
#include "folder_info.h"
#include "file_handle.h"

namespace ResourceOperations
{
    enum FOLDER_ERROR : DWORD
    {
        FOLDER_ACTION_SUCCESS,
        ERROR_MAX_PATH,
        ERROR_GET_FOLDER_ATTRIBUTE,
        ERROR_FILE_TIME_TO_SYSTIME,
        ERROR_SYSTIME_TO_LOCAL_TIME,
        ERROR_FIND_FIRST_FILE,
        ERROR_BUILD_JSON_STRING,
    };

    const std::string ErrorMessageFolder[] =
    {
        "",
        "Failed to handle directory path, it is too long!",
        "Failed to get folder attributes!",
        "Failed to convert file time to system time format!",
        "Failed to convert time in UTC to local time!",
        "Failed to get file information; path cannot be a directory!",
        "Failed to build JSON string!",
    };

    class FolderHandle
    {
    private:
        static DWORD last_error;
    public:
        static DWORD GetLastError();
        static std::string GetLastErrorString();
        static BOOL IsFileEmpty(const std::wstring& path);
        static BOOL IsPathExists(const std::wstring& path);
        static BOOL IsFolderExists(const std::wstring& path);
        static BOOL CreateFolder(const std::wstring& path, const std::wstring& name);
        static BOOL CreateNestedFolders(const std::wstring& path);
        static BOOL RenameFolder(const std::wstring& path, const std::wstring& rename);
        static BOOL DeleteFolder(const std::wstring& path);
        static BOOL GetFolderInfo(const std::wstring& path, FolderInfo& folder);
        static BOOL GetFolderFilter(const std::wstring& path, const std::wstring& filter, FolderInfo& folder);
    };
}
#pragma once
#include <thread> 
#include <chrono> 
#include "logger.h"
#include "file_cache.h"
#include "http_client.h"
#include "json_utility.h"
#include "folder_handle.h"

using namespace NetworkOperations;
using namespace ResourceOperations;

namespace UserOperations 
{
    class UserHandle 
    {
    private:
        std::wstring token_id;
        std::wstring user_name;
        BOOL logged_in = FALSE;
        HttpClient* net_api;
        FileCache* cache_api;
    public:
        UserHandle() : net_api(NULL), cache_api(NULL) {}
        void SetupNetwork(HttpClient* net) { net_api = net; }
        void SetupFileCache(FileCache* cache) { cache_api = cache; }

        BOOL RegisterAccount(const UserInfo& info);
        BOOL LoginAccount(const std::wstring& user_name, const std::wstring& password);
        BOOL LogoutAccount();
        BOOL DeleteAccount();
        BOOL GetUserProfile(UserInfo& info);
        BOOL UpdateUserProfile(const UserInfo& info);
        BOOL ChangePassword(const std::wstring& old_password, const std::wstring& new_password);

        BOOL RemoveFile(const std::wstring& file_path);
        BOOL RenameFile(const std::wstring& file_path, std::wstring& new_file_name);

        BOOL UploadFile(const FileInfo& file);
        BOOL UpdateFile(const FileInfo& file);


        BOOL RemoveFolder(const std::wstring& folder_path);
        BOOL RenameFolder(const std::wstring& folder_path, std::wstring& new_folder_name);

        BOOL UploadFolder(const FolderInfo& folder);
        BOOL UpdateFolder(const FolderInfo& folder);

        BOOL WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter = L"*.*", DWORD sleep = INFINITE);

    private:
        HttpResponse UploadFileMultipart(const FileInfo& file, const std::string& upload_id);
        HttpResponse UpdateFileMultipart(const FileInfo& file, const std::string& upload_id);

        BOOL ProcessSync(const std::wstring& folder_path, PFILE_NOTIFY_INFORMATION notify);

        BOOL ProcessFileAdd(std::wstring& file_path);
        BOOL ProcessFileModified(std::wstring& file_path);
        BOOL ProcessFileRemove(const std::wstring& file_path);
        BOOL ProcessFileRename(const std::wstring& file_path, std::wstring& new_file_name);

        BOOL ProcessFolderAdd(std::wstring& folder_path);
        BOOL ProcessFolderModified(std::wstring& folder_path);
        BOOL ProcessFolderRemove(const std::wstring& folder_path);
        BOOL ProcessFolderRename(const std::wstring& folder_path, std::wstring& new_folder_name);

    };
}

#pragma once
#include <thread> 
#include "logger.h"
#include "file_cache.h"
#include "http_client.h"
#include "json_utility.h"
#include "folder_handle.h"
#include <mutex>

using namespace NetworkOperations;
using namespace ResourceOperations;

namespace UserOperations 
{
    enum SyncActionType
    {
        ACTION_UPLOAD,
        ACTION_DELETE,
        ACTION_UPDATE,
        ACTION_RENAME
    };
    struct SyncAction
    {
        SyncActionType type;
        BOOL is_file_path;
        std::wstring path;
        std::wstring new_path; // optional for rename
    };
    typedef std::vector<SyncAction> ActionList;

    class UserHandle 
    {
    private:
        std::wstring token_id;
        std::wstring user_name;
        BOOL logged_in = FALSE;
        HttpClient* net_api;
        FileCache* cache_api;

        FolderInfo current_snapshot_;
        std::mutex snapshot_mutex_;

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
        BOOL UploadFolderWithFilter(const std::wstring& folder_path, const std::wstring& filter);
        BOOL UpdateFolderWithFilter(const std::wstring& folder_path, const std::wstring& filter);

        BOOL WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter = L"*.*", DWORD wait = INFINITE);

    private:
        HttpResponse UploadFileMultipart(const FileInfo& file, const std::string& upload_id);
        HttpResponse UpdateFileMultipart(const FileInfo& file, const std::string& upload_id);
        
        //---- NEW ------
        BOOL PrepareWatch(FolderInfo& folder);
		void DetectAndProcessChanges(const FolderInfo& old_snapshot, const FolderInfo& new_snapshot);
        BOOL ProcessSync(ActionList actions, FolderInfo& folder);
        //---- NEW ------

        BOOL ProcessFileAdd(FolderInfo& folder, std::wstring wSubName);
        BOOL ProcessFileModified(FolderInfo& folder, std::wstring wSubName);
        BOOL ProcessFileRemove(FolderInfo& folder, std::wstring wSubName);
        BOOL ProcessFileRename(FolderInfo& folder, std::wstring wSubName, std::wstring& wLastSubName);

        BOOL ProcessFolderAdd(FolderInfo& folder, std::wstring wSubName);
        BOOL ProcessFolderModified(FolderInfo& folder, std::wstring wSubName);
        BOOL ProcessFolderRemove(FolderInfo& folder, std::wstring wSubName);
        BOOL ProcessFolderRename(FolderInfo& folder, std::wstring wSubName, std::wstring& wLastSubName);

    };
}

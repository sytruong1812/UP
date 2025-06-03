#pragma once
#include <mutex>
#include <string>
#include "logger.h"
#include "file_cache.h"
#include "http_client.h"
#include "json_utility.h"
#include "folder_handle.h"


using namespace NetworkOperations;
using namespace ResourceOperations;

namespace UserOperations 
{
    enum SyncActionType
    {
        ACTION_ADD,
        ACTION_RENAME,
        ACTION_REMOVE,
        ACTION_MODIFIED
    };

    struct SyncAction
    {
        SyncActionType type_;
        BOOL is_folder_;
        union 
        {
            FileInfo* file_old_;
            FolderInfo* folder_old_;
        } object_old_;
        union 
        {
            FileInfo* file_new_;
            FolderInfo* folder_new_;
        } object_new_;

        SyncAction(SyncActionType type, FileInfo* file_old, FileInfo* file_new ) 
        {
            type_ = type;
            is_folder_ = FALSE;
            object_old_.file_old_ = file_old;
            object_new_.file_new_ = file_new;
        }

        SyncAction(SyncActionType type, FolderInfo* folder_old, FolderInfo* folder_new) 
        {
            type_ = type;
            is_folder_ = TRUE;
            object_old_.folder_old_ = folder_old;
            object_new_.folder_new_ = folder_new;
        }
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

        std::mutex snapshot_mutex;
        FolderInfo current_snapshot;

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
        BOOL RenameFile(const std::wstring& file_path, const std::wstring& new_name);
        BOOL UploadFile(const FileInfo& file);
        BOOL UpdateFile(const FileInfo& file);

        BOOL RemoveFolder(const std::wstring& folder_path);
        BOOL RenameFolder(const std::wstring& folder_path, const std::wstring& new_name);
        BOOL UploadFolder(const FolderInfo& folder);
        BOOL UpdateFolder(const FolderInfo& folder);
        BOOL UploadFolderWithFilter(const std::wstring& folder_path, const std::wstring& filter);
        BOOL UpdateFolderWithFilter(const std::wstring& folder_path, const std::wstring& filter);

        BOOL WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter = L"*.*", DWORD waitMilliseconds = 5000);
 
    private:
        HttpResponse UploadFileMultipart(const FileInfo& file, const std::string& upload_id);
        HttpResponse UpdateFileMultipart(const FileInfo& file, const std::string& upload_id);
        
        //---- NEW ------
        BOOL PrepareWatch(FolderInfo& folder);
		void DetectChangeForFile(const FolderInfo& old_snapshot, const FolderInfo& new_snapshot, ActionList& actions);
        void DetectChangeForFolder(const FolderInfo& old_snapshot, const FolderInfo& new_snapshot, ActionList& actions);
        BOOL ProcessSync(const ActionList& actions);
        //---- NEW ------

        BOOL ProcessFileAdd(const FileInfo& file);
        BOOL ProcessFileModified(const FileInfo& file);
        BOOL ProcessFileRemove(const FileInfo& file);
        BOOL ProcessFileRename(const FileInfo& old_file, const FileInfo& new_file);

        BOOL ProcessFolderAdd(const FolderInfo& folder);
        BOOL ProcessFolderModified(const FolderInfo& folder);
        BOOL ProcessFolderRemove(const FolderInfo& folder);
        BOOL ProcessFolderRename(const FolderInfo& old_folder, const FolderInfo& new_folder);

    };
}

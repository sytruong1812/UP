#pragma once
#include <windows.h>
#include <algorithm>
#include "file_info.h"

namespace ResourceOperations
{
    class FolderInfo
    {
    public:
        // Default constructor
        FolderInfo() : is_root_(TRUE), user_id_(0), group_id_(0), permissions_(0), folder_size_(0), access_time_({}), change_time_({}), parent_folder_(NULL) {}

        // Constructor with parameters
        FolderInfo(BOOL is_root,
            std::wstring path,
            DWORD size,
            DWORD user_id,
            DWORD group_id,
            DWORD permissions,
            FILETIME access_time,
            FILETIME change_time)
            :
            is_root_(is_root),
            folder_path_(path),
            folder_size_(size),
            user_id_(user_id),
            group_id_(group_id),
            permissions_(permissions),
            access_time_(access_time),
            change_time_(change_time)
        {
            folder_name_ = Helper::PathHelper::extractFolderNameFromFilePath(path);
            parent_folder_ = this->parent_folder_;
        }

        ~FolderInfo() = default;

        // Copy constructor
        FolderInfo(const FolderInfo& other)
            : is_root_(other.is_root_)
            , folder_path_(other.folder_path_)
            , folder_name_(other.folder_name_)
            , folder_size_(other.folder_size_)
            , parent_folder_(other.parent_folder_)
            , files_(other.files_)
            , children_(other.children_)
        {
            for (auto& child : children_) {
                child.SetParentFolder(this);
                for (auto& file : child.GetFiles()) {
                    file.SetParentFolder(&child);
                }
            }
            for (auto& file : files_) {
                file.SetParentFolder(this);
            }
        }

        // Copy assignment
        FolderInfo& operator=(const FolderInfo& other) {
            if (this != &other) {
                is_root_ = other.is_root_;
                folder_path_ = other.folder_path_;
                folder_name_ = other.folder_name_;
                folder_size_ = other.folder_size_;
                parent_folder_ = other.parent_folder_;
                files_ = other.files_;
                children_ = other.children_;

                for (auto& child : children_) {
                    child.SetParentFolder(this);
                    for (auto& file : child.GetFiles()) {
                        file.SetParentFolder(&child);
                    }
                }
                for (auto& file : files_) {
                    file.SetParentFolder(this);
                }
            }
            return *this;
        }

        // Move constructor
        FolderInfo(FolderInfo&& other) noexcept
            : 
            is_root_(other.is_root_), 
            folder_path_(std::move(other.folder_path_)), 
            folder_name_(std::move(other.folder_name_)),
            folder_size_(other.folder_size_), 
            user_id_(other.user_id_), 
            group_id_(other.group_id_),
            permissions_(other.permissions_), 
            access_time_(other.access_time_),
            change_time_(other.change_time_),
            files_(std::move(other.files_)),
            children_(std::move(other.children_)),
            parent_folder_(other.parent_folder_) {}

        // Move assignment operator
        FolderInfo& operator=(FolderInfo&& other) noexcept 
        {
            if (this != &other) {
                is_root_ = other.is_root_;
                folder_path_ = std::move(other.folder_path_);
                folder_name_ = std::move(other.folder_name_);
                folder_size_ = other.folder_size_;
                user_id_ = other.user_id_;
                group_id_ = other.group_id_;
                permissions_ = other.permissions_;
                access_time_ = other.access_time_;
                change_time_ = other.change_time_;
                files_ = std::move(other.files_);
                children_ = std::move(other.children_);
                parent_folder_ = other.parent_folder_;
            }
            return *this;
        }

        // Compare two FolderInfo objects
        bool operator==(const FolderInfo& other) const 
        {
            return 
                is_root_ == other.is_root_ &&
                folder_path_ == other.folder_path_ &&
                folder_name_ == other.folder_name_ &&
                folder_size_ == other.folder_size_ &&
                user_id_ == other.user_id_ &&
                group_id_ == other.group_id_ &&
                permissions_ == other.permissions_ &&
                CompareFileTime(&access_time_, &other.access_time_) == 0 &&
                CompareFileTime(&change_time_, &other.change_time_) == 0 &&
                files_.size() == other.files_.size() &&
                children_.size() == other.children_.size();
        }

        bool operator!=(const FolderInfo& other) const 
        {
            return !(*this == other);
        }

        /*=====================[ Getter Methods ]========================*/
        BOOL GetRoot() const { return is_root_; }
        std::wstring GetFolderPath() const { return folder_path_; }
        std::wstring GetFolderName() const { return folder_name_; }
        DWORD GetFolderSize() const { return folder_size_; }
        DWORD GetUserId() const { return user_id_; }
        DWORD GetGroupId() const { return group_id_; }
        DWORD GetPermissions() const { return permissions_; }
        FILETIME GetAccessTime() const { return access_time_; }
        FILETIME GetChangeTime() const { return change_time_; }
        FolderInfo* GetParentFolder() const { return parent_folder_; }

        /*=====================[ Setter Methods ]========================*/
        void SetRoot(BOOL root) { is_root_ = root; }
        void SetFolderPath(const std::wstring& path) { folder_path_ = path; }
        void SetFolderName(const std::wstring& name) { folder_name_ = name; }
        void SetFolderSize(DWORD size) { folder_size_ = size; }
        void SetUserId(DWORD id) { user_id_ = id; }
        void SetGroupId(DWORD id) { group_id_ = id; }
        void SetPermissions(DWORD perms) { permissions_ = perms; }
        void SetAccessTime(FILETIME time) { access_time_ = time; }
        void SetChangeTime(FILETIME time) { change_time_ = time; }
        void SetParentFolder(FolderInfo* parent) { parent_folder_ = parent; }

        /*=====================[ Folder Content Handling ]========================*/
        void ClearFile() { files_.clear(); }
        void ClearChildren() { children_.clear(); }

        void AddFile(FileInfo& file) 
        {
            file.SetParentFolder(this);  
            files_.push_back(file);
        }
        void AddChildren(FolderInfo& folder) 
        {
            folder.SetParentFolder(this); 
            children_.push_back(folder);
        }

        BOOL IsRoot() const { return is_root_; }
        BOOL IsChildren() const { return !is_root_; }

        size_t CountFile() const { return files_.size(); }
        size_t CountChildren() const { return children_.size(); }

        LIST_FILE& GetFiles() { return files_; }
        const LIST_FILE& GetFiles() const { return files_; }

        BOOL HasFile(const std::wstring& file_name) const {
            return std::any_of(files_.begin(), files_.end(),
                [&file_name](const FileInfo& file) {
                    return file.GetFileName() == file_name;
                }) ? TRUE : FALSE;
        }

        BOOL HasChildren(const std::wstring& folder_name) const {
            return std::any_of(children_.begin(), children_.end(),
                [&folder_name](const FolderInfo& folder) {
                    return folder.GetFolderName() == folder_name;
                }) ? TRUE : FALSE;
        }

        FileInfo GetFile(const std::wstring& file_name) const {
            auto it = std::find_if(files_.begin(), files_.end(),
                [&file_name](const FileInfo& file) {
                    return file.GetFileName() == file_name;
                });
            return (it != files_.end()) ? *it : FileInfo();
        }

        FolderInfo GetChildren(const std::wstring& folder_name) const {
            auto it = std::find_if(children_.begin(), children_.end(),
                [&folder_name](const FolderInfo& folder) {
                    return folder.GetFolderName() == folder_name;
                });
            return (it != children_.end()) ? *it : FolderInfo();
        }

        void RemoveFile(const std::wstring& file_name) {
            files_.erase(std::remove_if(files_.begin(), files_.end(),
                [&file_name](const FileInfo& file) {
                    return file.GetFileName() == file_name;
                }), files_.end());
        }

        void RemoveChildren(const std::wstring& folder_name) {
            children_.erase(std::remove_if(children_.begin(), children_.end(),
                [&folder_name](const FolderInfo& folder) {
                    return folder.GetFolderPath() == folder_name;
                }), children_.end());
        }

 
        LIST_FILE GetFilesRecursive() const 
        {
            LIST_FILE all_files;
            all_files.insert(all_files.end(), files_.begin(), files_.end());
            for (const auto& child : children_) {
                LIST_FILE child_files = child.GetFilesRecursive();
                all_files.insert(all_files.end(), child_files.begin(), child_files.end());
            }
            return all_files;
        }

        std::wstring GetPathToRoot() const
        {
            const FolderInfo* current = this;
            std::vector<std::wstring> pathComponents;

            while (current != nullptr) {
                if (!current->is_root_) {
                    pathComponents.push_back(current->folder_name_);
                }
                else {
                    pathComponents.push_back(current->folder_name_);
                    break;
                }
                current = current->parent_folder_;
            }

            std::wstring path;
            for (auto it = pathComponents.rbegin(); it != pathComponents.rend(); ++it) {
                if (it != pathComponents.rbegin()) {
                    path += L"\\";
                }
                path += *it;
            }

            return path;
        }

    private:
        BOOL is_root_;                          // TRUE if this is the root folder
        DWORD folder_size_;                     // Size of folder
        std::wstring folder_path_;              // Full path of folder
        std::wstring folder_name_;              // Folder name extracted from path
        DWORD user_id_;                         // Owner user ID
        DWORD group_id_;                        // Group ID
        DWORD permissions_;                     // Permission bits
        FILETIME access_time_;                  // Last access time
        FILETIME change_time_;                  // Last modification time
        std::vector<FileInfo> files_;           // List of files in this folder
        std::vector<FolderInfo> children_;      // List of subfolders in this folder
        FolderInfo* parent_folder_;             // Pointer to parent folder, nullptr if this is root folder
    };

} // namespace ResourceOperations

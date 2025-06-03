#pragma once
#include <windows.h>
#include <algorithm>
#include "file_info.h"

namespace ResourceOperations {
    class FolderInfo {
    public:
        // Default constructor
        FolderInfo() :
            is_root_(TRUE), folder_size_(0),
            user_id_(0), group_id_(0), permissions_(0),
            access_time_({}), change_time_({}),
            parent_folder_(nullptr)
        {}

        // Constructor with parameters
        FolderInfo(BOOL is_root, std::wstring path, DWORD size,
            DWORD user_id, DWORD group_id, DWORD permissions,
            FILETIME access_time, FILETIME change_time)
            :
            is_root_(is_root), folder_path_(path), folder_size_(size),
            user_id_(user_id), group_id_(group_id), permissions_(permissions),
            access_time_(access_time), change_time_(change_time)
        {
            folder_name_ = Helper::PathHelper::extractParentNameFromPath(path);
            parent_folder_ = nullptr;
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
            for (auto& child : children_)
            {
                child.SetParentFolder(this);
                for (auto& file : child.GetFiles())
                {
                    file.SetParentFolder(&child);
                }
            }
            for (auto& file : files_)
            {
                file.SetParentFolder(this);
            }
        }

        // Copy assignment
        FolderInfo& operator=(const FolderInfo& other)
        {
            if (this != &other)
            {
                is_root_ = other.is_root_;
                folder_path_ = other.folder_path_;
                folder_name_ = other.folder_name_;
                folder_size_ = other.folder_size_;
                files_ = other.files_;
                children_ = other.children_;
                parent_folder_ = other.parent_folder_;

                for (auto& child : children_)
                {
                    child.SetParentFolder(this);
                    for (auto& file : child.GetFiles())
                    {
                        file.SetParentFolder(&child);
                    }
                }
                for (auto& file : files_)
                {
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
            parent_folder_(std::move(other.parent_folder_)) {}

        // Move assignment operator
        FolderInfo& operator=(FolderInfo&& other) noexcept
        {
            if (this != &other)
            {
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
                parent_folder_ = std::move(other.parent_folder_);
            }
            return *this;
        }

        // Equality operator
        bool operator==(const FolderInfo& other) const
        {
            return is_root_ == other.is_root_ &&
                folder_path_ == other.folder_path_ &&
                folder_name_ == other.folder_name_ &&
                folder_size_ == other.folder_size_ &&
                user_id_ == other.user_id_ &&
                group_id_ == other.group_id_ &&
                permissions_ == other.permissions_ &&
                CompareFileTime(&access_time_, &other.access_time_) == 0 &&
                CompareFileTime(&change_time_, &other.change_time_) == 0 &&
                files_.size() == other.files_.size() &&
                children_.size() == other.children_.size() &&
                parent_folder_ == other.parent_folder_;
        }

        // Inequality operator
        bool operator!=(const FolderInfo& other) const
        {
            return !(*this == other);
        }

        /*=====================[ Getter Methods ]========================*/
        BOOL GetRoot() const { return is_root_; }
        std::wstring GetFolderPath() const { return folder_path_; }
        std::wstring GetFolderName() const { return folder_name_; }
        ULONGLONG GetFolderSize() const { return folder_size_; }
        DWORD GetUserId() const { return user_id_; }
        DWORD GetGroupId() const { return group_id_; }
        DWORD GetPermissions() const { return permissions_; }
        FILETIME GetAccessTime() const { return access_time_; }
        FILETIME GetChangeTime() const { return change_time_; }
        FolderInfo* GetParentFolder() const { return parent_folder_; }

        /*=====================[ Setter Methods ]========================*/
        void SetRoot(BOOL root) { is_root_ = root; }
        void SetFolderPath(const std::wstring& path)
        {
            folder_path_ = path;
            folder_name_ = Helper::PathHelper::extractLastComponentFromPath(path);
        }
        void SetFolderName(const std::wstring& name)
        {
            if (folder_name_.compare(name) != 0)
            {
                Helper::StringHelper::replaceSubwstring(folder_path_, folder_name_, name);
                folder_name_ = name;
            }
        }
        void SetFolderSize(ULONGLONG  size) {
            if (folder_size_ != size)
            {
                folder_size_ = size;
            }
        }
        void SetUserId(DWORD id) {
            if (user_id_ != id)
            {
                user_id_ = id;
            }
        }
        void SetGroupId(DWORD id) {
            if (group_id_ != id)
            {
                group_id_ = id;
            }
        }
        void SetPermissions(DWORD perms) {
            if (permissions_ != perms)
            {
                permissions_ = perms;
            }
        }
        void SetAccessTime(FILETIME time) {
            if (CompareFileTime(&access_time_, &time) != 0)
            {
                access_time_ = time;
            }
        }
        void SetChangeTime(FILETIME time) {
            if (CompareFileTime(&change_time_, &time) != 0)
            {
                change_time_ = time;
            }
        }
        void SetParentFolder(FolderInfo* parent) {
            if (parent_folder_ != parent)
            {
                parent_folder_ = parent;
            }
        }

        /*=====================[ Folder Content Handling ]========================*/
        BOOL IsRoot() const { return is_root_; }
        BOOL IsChildren() const { return !is_root_; }

        size_t CountFile() const { return files_.size(); }
        size_t CountChildren() const { return children_.size(); }

        void ClearFile() { files_.clear(); }
        void ClearChildren() { children_.clear(); }

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

        void UpdateFile(const FileInfo& file)
        {
            auto it = std::find_if(files_.begin(), files_.end(),
                [&file](const FileInfo& existingFile) {
                return existingFile.GetFileName() == file.GetFileName();
            });

            if (it != files_.end())
            {
                *it = file; // Replace the object
            }
        }
        void UpdateChildren(const FolderInfo& folder)
        {
            auto it = std::find_if(children_.begin(), children_.end(),
                [&folder](const FolderInfo& existingFolder) {
                return existingFolder.GetFolderName() == folder.GetFolderName();
            });

            if (it != children_.end())
            {
                *it = folder; // Replace the object
            }
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

        FileInfo GetFile(int index) const { return files_[index]; }
        FolderInfo GetChildren(int index) const { return children_[index]; }

        LIST_FILE GetFiles() const { return files_; }
        LIST_FOLDER GetChildrens() const { return children_; }

        LIST_FILE GetFilesRecursive() const
        {
            LIST_FILE all_files;
            all_files.reserve(files_.size());   // Reserve space for folders

            // Add child folders
            all_files.insert(all_files.end(), files_.begin(), files_.end());
            // Recursively add files from child folders
            for (const auto& child : children_)
            {
                LIST_FILE child_files = child.GetFilesRecursive();
                all_files.insert(all_files.end(), child_files.begin(), child_files.end());
            }

            return all_files;
        }
        LIST_FOLDER GetFolderRecursive() const
        {
            LIST_FOLDER all_folders;
            all_folders.reserve(children_.size()); // Reserve space for folders

            // Add child folders
            all_folders.insert(all_folders.end(), children_.begin(), children_.end());
            // Recursively add folders from child folders
            for (const auto& child : children_)
            {
                LIST_FOLDER sub_folders = child.GetFolderRecursive();
                all_folders.insert(all_folders.end(), sub_folders.begin(), sub_folders.end());
            }

            return all_folders;
        }

        FileInfo FindFileRecursive(const std::wstring& folder_path, const std::wstring& file_name) const {
            // Compare current path with folder path
            std::wstring currentPath = this->GetRelativePath();
            if (currentPath.compare(folder_path) == 0)
            {
                // Find file in current directory
                auto it = std::find_if(files_.begin(), files_.end(),
                    [&file_name](const FileInfo& file) {
                    return file.GetFileName() == file_name;
                });
                if (it != files_.end())
                {
                    return *it;
                }
            }

            // Recursively search in subdirectories
            for (const auto& child : children_)
            {
                FileInfo result = child.FindFileRecursive(folder_path, file_name);
                if (!result.GetFileName().empty())
                {  // If file is found
                    return result;
                }
            }
            return FileInfo();  // Return empty FileInfo if not found
        }
        FileInfo FindFileRecursive(const std::wstring& file_path) const {
            // Compare current path with folder path
            if (folder_name_.compare(Helper::PathHelper::extractParentNameFromPath(file_path)) == 0)
            {
                // Find file in current directory
                auto it = std::find_if(files_.begin(), files_.end(),
                    [&file_path](const FileInfo& file) {
                    return file.GetFilePath() == file_path;
                });
                if (it != files_.end())
                {
                    return *it;
                }
            }

            // Recursively search in subdirectories
            for (const auto& child : children_)
            {
                FileInfo result = child.FindFileRecursive(file_path);
                if (!result.GetFileName().empty())
                {
                    return result;  // If file is found
                }
            }
            return FileInfo();  // Return empty FileInfo if not found
        }
        FolderInfo FindChildrenRecursive(const std::wstring& folder_path) const
        {
            if (folder_path_.compare(folder_path) == 0)
            {
                return *this;
            }
            else
            {
                auto it = std::find_if(children_.begin(), children_.end(),
                    [&folder_path](const FolderInfo& folder) {
                    return folder.GetFolderPath() == folder_path;
                });
                if (it != children_.end())
                {
                    return *it;
                }
            }
            for (int i = 0; i < children_.size(); i++)
            {
                return children_[i].FindChildrenRecursive(folder_path);
            }
            return FolderInfo();
        }

        void UpdateFileRecursive(const FileInfo& file) {
            // Update the file in the current folder
            for (auto& existingFile : files_)
            {
                if (existingFile.GetFilePath() == file.GetFilePath())
                {
                    existingFile = file;
                    return; // Exit after updating
                }
            }

            // Recursively update in child folders
            for (auto& child : children_)
            {
                child.UpdateFileRecursive(file);
            }
        }
        void UpdateChildrenRecursive(const FolderInfo& folder) {
            // Update the folder in the current folder
            for (auto& existingFolder : children_)
            {
                if (existingFolder.GetFolderPath() == folder.GetFolderPath())
                {
                    existingFolder = folder;
                    return; // Exit after updating
                }
            }

            // Recursively update in child folders
            for (auto& child : children_)
            {
                child.UpdateChildrenRecursive(folder);
            }
        }

        void AddFileRecursive(const std::wstring& folder_name, FileInfo& file)
        {
            if (folder_name_.compare(folder_name) == 0)
            {
                AddFile(file);
            }
            else
            {
                auto it = std::find_if(children_.begin(), children_.end(),
                    [&folder_name](const FolderInfo& folder) {
                    return folder.GetFolderName() == folder_name;
                });
                if (it != children_.end())
                {
                    it->AddFile(file);
                }
            }
            for (int i = 0; i < children_.size(); i++)
            {
                return children_[i].AddFileRecursive(folder_name, file);
            }
        }
        void AddChildrenRecursive(const std::wstring& folder_name, FolderInfo& folder)
        {
            if (folder_name_.compare(folder_name) == 0)
            {
                AddChildren(folder);
            }
            else
            {
                auto it = std::find_if(children_.begin(), children_.end(),
                    [&folder_name](const FolderInfo& folder) {
                    return folder.GetFolderName() == folder_name;
                });
                if (it != children_.end())
                {
                    it->AddChildren(folder);
                }
            }
        }

        std::wstring GetRelativePath() const
        {
            const FolderInfo* current = this;
            std::vector<std::wstring> pathComponents;
            while (current != nullptr)
            {
                if (!current->is_root_)
                {
                    pathComponents.push_back(current->folder_name_);
                }
                else
                {
                    pathComponents.push_back(current->folder_name_);
                    break;
                }
                current = current->parent_folder_;
            }
            std::wstring pathCombined;
            for (auto it = pathComponents.rbegin(); it != pathComponents.rend(); ++it)
            {
                if (it != pathComponents.rbegin())
                {
                    pathCombined += L"\\";
                }
                pathCombined += *it;
            }
            return pathCombined;
        }

    private:
        BOOL is_root_;                          // TRUE if this is the root folder
        std::wstring folder_name_;              // Folder name extracted from path
        LIST_FILE files_;                       // List of files in this folder
        LIST_FOLDER children_;                  // List of subfolders in this folder
        FolderInfo* parent_folder_;             // Pointer to parent folder, nullptr if this is root folder
        ULONGLONG folder_size_;                 // Size of folder
        DWORD user_id_;                         // Owner user ID
        DWORD group_id_;                        // Group ID
        DWORD permissions_;                     // Permission bits
        FILETIME access_time_;                  // Last access time
        FILETIME change_time_;                  // Last modification time
        std::wstring folder_path_;              // Full path of folder
    };

} // namespace ResourceOperations

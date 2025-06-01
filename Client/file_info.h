#pragma once
#include "utils.h"

namespace ResourceOperations
{
    class FolderInfo;
    class FileInfo
    {
    public:
        // Default constructor
        FileInfo() : file_size_(0), file_attribute_(0), create_time_({}), last_write_time_({}), last_access_time_({})
        {
            parent_folder_ = nullptr;
        }

        // Constructor with parameters
        FileInfo(const std::wstring& file_path, DWORD file_size, const std::string& hash, DWORD file_attribute,
            FILETIME create_time, FILETIME last_write_time, FILETIME last_access_time)
            : 
            file_path_(file_path), file_size_(file_size), hash_file_(hash), file_attribute_(file_attribute),
            create_time_(create_time), last_write_time_(last_write_time), last_access_time_(last_access_time)
        {
            if (Helper::PathHelper::isValidFilePath(file_path_))
            {
                file_name_ = Helper::PathHelper::extractFileNameFromFilePath(file_path_);
                extension_ = Helper::PathHelper::extractFileExtensionFromFilePath(file_path_);
            }
            parent_folder_ = nullptr;
        }

        // Destructor
        ~FileInfo() = default;

        // Copy constructor
        FileInfo(const FileInfo& other)
            : 
            file_path_(other.file_path_),
            file_name_(other.file_name_),
            file_size_(other.file_size_),
            extension_(other.extension_),
            hash_file_(other.hash_file_),
            file_attribute_(other.file_attribute_),
            create_time_(other.create_time_),
            last_write_time_(other.last_write_time_),
            last_access_time_(other.last_access_time_),
            parent_folder_(other.parent_folder_)
        {}

        // Copy assignment operator
        FileInfo& operator=(const FileInfo& other)
        {
            if (this != &other)
            {
                file_path_ = other.file_path_;
                file_name_ = other.file_name_;
                file_size_ = other.file_size_;
                extension_ = other.extension_;
                hash_file_ = other.hash_file_;
                file_attribute_ = other.file_attribute_;
                create_time_ = other.create_time_;
                last_write_time_ = other.last_write_time_;
                last_access_time_ = other.last_access_time_;
                parent_folder_ = other.parent_folder_;
            }
            return *this;
        }

        // Move constructor
        FileInfo(FileInfo&& other) noexcept
            : 
            file_path_(std::move(other.file_path_)),
            file_name_(std::move(other.file_name_)),
            file_size_(other.file_size_),
            extension_(std::move(other.extension_)),
            hash_file_(std::move(other.hash_file_)),
            file_attribute_(other.file_attribute_),
            create_time_(other.create_time_),
            last_write_time_(other.last_write_time_),
            last_access_time_(other.last_access_time_),
            parent_folder_(std::move(other.parent_folder_))
        { }

        // Move assignment operator
        FileInfo& operator=(FileInfo&& other) noexcept
        {
            if (this != &other)
            {
                file_path_ = std::move(other.file_path_);
                file_name_ = std::move(other.file_name_);
                file_size_ = other.file_size_;
                extension_ = std::move(other.extension_);
                file_attribute_ = other.file_attribute_;
                hash_file_ = std::move(other.hash_file_);
                create_time_ = other.create_time_;
                last_write_time_ = other.last_write_time_;
                last_access_time_ = other.last_access_time_;
                parent_folder_ = std::move(other.parent_folder_);
            }
            return *this;
        }

        // Equality operator
        bool operator==(const FileInfo& other) const
        {
            return file_path_ == other.file_path_
                && file_name_ == other.file_name_
                && file_size_ == other.file_size_
                && extension_ == other.extension_
                && file_attribute_ == other.file_attribute_
                && hash_file_ == other.hash_file_
                && CompareFileTime(&create_time_, &other.create_time_) == 0
                && CompareFileTime(&last_write_time_, &other.last_write_time_) == 0
                && CompareFileTime(&last_access_time_, &other.last_access_time_) == 0
                && parent_folder_ == other.parent_folder_;
        }

        // Inequality operator
        bool operator!=(const FileInfo& other) const
        {
            return !(*this == other);
        }

        /*=====================[ Getter Methods ]========================*/
        std::wstring GetFilePath() const { return file_path_; }
        std::wstring GetFileName() const { return file_name_; }
        DWORD GetFileSize() const { return file_size_; }
        std::wstring GetFileExtension() const { return extension_; }
        DWORD GetFileAttribute() const { return file_attribute_; }
        std::string GetHashFile() const { return hash_file_; }
        FILETIME GetCreateTime() const { return create_time_; }
        FILETIME GetLastWriteTime() const { return last_write_time_; }
        FILETIME GetLastAccessTime() const { return last_access_time_; }
        FolderInfo* GetParentFolder() const { return parent_folder_; }

        /*=====================[ Setter Methods ]========================*/
        void SetFilePath(const std::wstring& path) { file_path_ = path; }
        void SetFileName(const std::wstring& name) { file_name_ = name; }
        void SetFileSize(DWORD size) { file_size_ = size; }
        void SetFileExtension(const std::wstring& ext) { extension_ = ext; }
        void SetFileAttribute(DWORD attr) { file_attribute_ = attr; }
        void SetHashFile(const std::string& hash) { hash_file_ = hash; }
        void SetCreateTime(FILETIME time) { create_time_ = time; }
        void SetLastWriteTime(FILETIME time) { last_write_time_ = time; }
        void SetLastAccessTime(FILETIME time) { last_access_time_ = time; }
        void SetParentFolder(FolderInfo* folder) { parent_folder_ = folder; }

    private:
        std::wstring file_name_;        // File name (e.g. example.txt)
        DWORD file_size_;               // Size of the file in bytes
        std::wstring extension_;        // File extension (e.g. .txt, .exe)
        FolderInfo* parent_folder_;     // Pointer to parent folder contain file
        DWORD file_attribute_;          // File attributes (e.g. readonly, hidden)
        std::string hash_file_;        // File hash
        FILETIME create_time_;          // File creation time
        FILETIME last_write_time_;      // Last write time
        FILETIME last_access_time_;     // Last access time
        std::wstring file_path_;        // Full path to the file
    };

    // Define type aliases for file list
    typedef std::vector<FileInfo> LIST_FILE;
    typedef std::vector<FileInfo>::iterator PLIST_FILE;

} // namespace ResourceOperations

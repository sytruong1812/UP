#pragma once
#include "folder_info.h"

using namespace ResourceOperations;

namespace UserOperations 
{
    struct UserInfo 
    {
        std::wstring first_name;
        std::wstring last_name;
        std::wstring user_name;
        std::wstring password;
        std::wstring email;
        std::wstring birthday;
    };

    typedef std::pair<std::wstring, std::wstring> FileMissing;

    class JsonUtility 
    {
    public:
        static std::string CreateJsonRegister(const UserInfo& info);
        static std::string CreateJsonUpdateProfile(const UserInfo& info);
        static std::string CreateJsonFileUpload(const FileInfo& file);
        static std::string CreateJsonFileUpdate(const FileInfo& file, DWORD file_id);
        static std::string CreateJsonFolderTree(const FolderInfo& folder);
        static std::string CreateJsonLogin(const std::wstring& user_name, const std::wstring& password);
        static std::string CreateJsonChangePassword(const std::wstring& old_password, const std::wstring& new_password);
        static std::string CreateJsonFileRename(const std::wstring& old_file_name, const std::wstring& new_file_name);
        static std::string CreateJsonFolderRename(const std::wstring& old_folder_name, const std::wstring& new_folder_name);
    public:
        static void ParserJsonRegisterResponse(const std::string& message, DWORD& user_id);
        static void ParseJsonGetProfileResponse(const std::string& message, UserInfo& user_info);
        static void ParserJsonLoginResponse(const std::string& message, std::wstring& token_id);
        static void ParserJsonUploadFileResponse(const std::string& message, std::string& upload_id, DWORD& file_id);
        static void ParserJsonUpdateFileResponse(const std::string& message, std::string& update_id, DWORD& file_id);
        static void ParserJsonFileMissResponse(const std::string& message, std::vector<FileMissing>& files);
    };
}
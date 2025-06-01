#include "json_utility.h"
#include "json/json_value.h"
#include "json/json_writer.h"

namespace UserOperations 
{
	std::string JsonUtility::CreateJsonRegister(const UserInfo& info)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("first_name", Helper::StringHelper::convertWideStringToString(info.first_name));
			jw->KeyValue("last_name", Helper::StringHelper::convertWideStringToString(info.last_name));
			jw->KeyValue("user_name", Helper::StringHelper::convertWideStringToString(info.user_name));
			jw->KeyValue("password", Helper::StringHelper::convertWideStringToString(info.password));
			jw->KeyValue("email", Helper::StringHelper::convertWideStringToString(info.email));
			jw->KeyValue("birthday", Helper::StringHelper::convertWideStringToString(info.birthday));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonUpdateProfile(const UserInfo& info)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			if (!info.first_name.empty())
			{
				jw->KeyValue("first_name", Helper::StringHelper::convertWideStringToString(info.first_name));
			}
			if (!info.last_name.empty())
			{
				jw->KeyValue("last_name", Helper::StringHelper::convertWideStringToString(info.last_name));
			}
			if (!info.user_name.empty())
			{
				jw->KeyValue("user_name", Helper::StringHelper::convertWideStringToString(info.user_name));
			}
			if (!info.password.empty())
			{
				jw->KeyValue("password", Helper::StringHelper::convertWideStringToString(info.password));
			}
			if (!info.email.empty())
			{
				jw->KeyValue("email", Helper::StringHelper::convertWideStringToString(info.email));
			}
			if (!info.birthday.empty())
			{
				jw->KeyValue("birthday", Helper::StringHelper::convertWideStringToString(info.birthday));
			}	
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonLogin(const std::wstring& user_name, const std::wstring& password)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("user_name", Helper::StringHelper::convertWideStringToString(user_name));
			jw->KeyValue("password", Helper::StringHelper::convertWideStringToString(password));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonChangePassword(const std::wstring& old_password, const std::wstring& new_password)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("old_password", Helper::StringHelper::convertWideStringToString(old_password));
			jw->KeyValue("new_password", Helper::StringHelper::convertWideStringToString(new_password));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonFileRename(const std::wstring& old_file_name, const std::wstring& new_file_name)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("old_file_name", Helper::StringHelper::convertWideStringToString(old_file_name));
			jw->KeyValue("new_file_name", Helper::StringHelper::convertWideStringToString(new_file_name));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonFolderRename(const std::wstring& old_folder_name, const std::wstring& new_folder_name)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("old_folder_name", Helper::StringHelper::convertWideStringToString(old_folder_name));
			jw->KeyValue("new_folder_name", Helper::StringHelper::convertWideStringToString(new_folder_name));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonFileUpload(const FileInfo& file)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("file_name", Helper::StringHelper::convertWideStringToString(file.GetFileName()));
			jw->KeyValue("file_size", (uint64_t)file.GetFileSize());
			jw->KeyValue("folder", Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetPathToRoot()));
			jw->KeyValue("attribute", (uint64_t)file.GetFileAttribute());
			jw->KeyValue("create_time", Helper::TimeHelper::convertFileTimeToString(file.GetCreateTime()));
			jw->KeyValue("last_write_time", Helper::TimeHelper::convertFileTimeToString(file.GetLastWriteTime()));
			jw->KeyValue("last_access_time", Helper::TimeHelper::convertFileTimeToString(file.GetLastAccessTime()));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonFileUpdate(const FileInfo& file, DWORD file_id)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		jw->SetWriter(&os);
		jw->StartObject();
			jw->KeyValue("file_id", (int32_t)file_id);
			jw->KeyValue("file_name", Helper::StringHelper::convertWideStringToString(file.GetFileName()));
			jw->KeyValue("file_size", (uint64_t)file.GetFileSize());
			jw->KeyValue("folder", Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetPathToRoot()));
			jw->KeyValue("attribute", (uint64_t)file.GetFileAttribute());
			jw->KeyValue("create_time", Helper::TimeHelper::convertFileTimeToString(file.GetCreateTime()));
			jw->KeyValue("last_write_time", Helper::TimeHelper::convertFileTimeToString(file.GetLastWriteTime()));
			jw->KeyValue("last_access_time", Helper::TimeHelper::convertFileTimeToString(file.GetLastAccessTime()));
		jw->EndObject();
		return os.str();
	}

	std::string JsonUtility::CreateJsonFolderTree(const FolderInfo& folder)
	{
		std::ostringstream os;
		JsonWriter* jw = new JsonWriter();
		LIST_FILE files = folder.GetFilesRecursive();

		jw->SetWriter(&os);
		jw->StartArray();
		for (int i = 0; i < files.size(); i++)
		{
			jw->StartObject();
			jw->KeyValue("file_name", Helper::StringHelper::convertWideStringToString(files[i].GetFileName()));
			jw->KeyValue("file_size", (uint64_t)files[i].GetFileSize());
			jw->KeyValue("folder", Helper::StringHelper::convertWideStringToString(files[i].GetParentFolder()->GetPathToRoot()));
			jw->KeyValue("attribute", (uint64_t)files[i].GetFileAttribute());
			jw->KeyValue("create_time", Helper::TimeHelper::convertFileTimeToString(files[i].GetCreateTime()));
			jw->KeyValue("last_write_time", Helper::TimeHelper::convertFileTimeToString(files[i].GetLastWriteTime()));
			jw->KeyValue("last_access_time", Helper::TimeHelper::convertFileTimeToString(files[i].GetLastAccessTime()));
			jw->EndObject();
		}
		jw->EndArray();
		return os.str();
	}

	void JsonUtility::ParserJsonRegisterResponse(const std::string& message, DWORD& user_id)
	{
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsObject() && jr->CountChildren() > 0)
		{
			user_id = (DWORD)jr->Child(L"user_id")->AsNumber();
		}
		if (jr) { delete jr; }
	}

	void JsonUtility::ParseJsonGetProfileResponse(const std::string& message, UserInfo& user_info)
	{
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsObject() && jr->CountChildren() > 0)
		{
			user_info.first_name = jr->Child(L"first_name")->AsString();
			user_info.last_name = jr->Child(L"last_name")->AsString();
			user_info.user_name = jr->Child(L"user_name")->AsString();
			user_info.password = jr->Child(L"password")->AsString();
			user_info.birthday = jr->Child(L"birthday")->AsString();
			user_info.email = jr->Child(L"email")->AsString();
		}
		if (jr) { delete jr; }
	}

	void JsonUtility::ParserJsonLoginResponse(const std::string& message, std::wstring& token_id)
	{
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsObject())
		{
			token_id = jr->Child(L"token")->AsString();
		}
		if (jr) { delete jr; }
	}

	void JsonUtility::ParserJsonUploadFileResponse(const std::string& message, std::string& upload_id, DWORD& file_id)
	{
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsObject())
		{
			file_id = (DWORD)jr->Child(L"file_id")->AsNumber();
			upload_id = Helper::StringHelper::convertWideStringToString(jr->Child(L"upload_id")->AsString());
		}
		if (jr) { delete jr; }
	}

	void JsonUtility::ParserJsonUpdateFileResponse(const std::string& message, std::string& update_id, DWORD& file_id)
	{
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsObject())
		{
			file_id = (DWORD)jr->Child(L"file_id")->AsNumber();
			update_id = Helper::StringHelper::convertWideStringToString(jr->Child(L"update_id")->AsString());
		}
		if (jr) { delete jr; }
	}
	
	void JsonUtility::ParserJsonFileMissResponse(const std::string& message, std::vector<FileMissing>& files)
	{
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsArray())
		{
			JsonArray array = jr->AsArray();
			for (int i = 0; i < array.size(); i++)
			{
				JsonObject obj = array[i]->AsObject();
				files.push_back({obj[L"folder_path"]->AsString(),obj[L"file_name"]->AsString()});
			}
		}
		if (jr) { delete jr; }
	}
}
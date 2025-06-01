#include <queue>
#include <atomic>
#include <conio.h>
#include <sstream>

#include "utils.h"
#include "logger.h"
#include "base64.h"
#include "user_handle.h"
#include "zlib/zstream/izstream.h"
#include "zlib/zstream/ozstream.h"

std::atomic<BOOL> exitMonitorFlag(FALSE); // Shared variable to signal exit

namespace UserOperations 
{
	BOOL UserHandle::RegisterAccount(const UserInfo& info)
	{
		HttpHeaders headers;
		HttpResponse response;
		std::string json_register = JsonUtility::CreateJsonRegister(info);
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Content-Type", L"application/json");

		response = net_api->Post(L"register", headers, json_register);
		if (response.GetStatusCode() != 201)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());;
			return FALSE;
		}

		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		return TRUE;
	}

	BOOL UserHandle::LoginAccount(const std::wstring& user_name, const std::wstring& password)
	{
		BOOL result = FALSE;
		HttpHeaders headers;
		HttpResponse response;
		std::string json_login = JsonUtility::CreateJsonLogin(user_name, password);
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Content-Type", L"application/json");
		response = net_api->Post(L"login", headers, json_login);

		this->logged_in = FALSE;
		this->user_name = L"";
		this->token_id = L"";

		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			result = FALSE;
		}
		else
		{
			if (response.CheckContentIsJson())
			{
				JsonUtility::ParserJsonLoginResponse(response.GetContentString(), this->token_id);
			}
			LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
			this->user_name = user_name;
			this->logged_in = TRUE;
			result = TRUE;
		}
		return result;
	}

	BOOL UserHandle::LogoutAccount()
	{
		BOOL result = FALSE;
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		response = net_api->Post(this->user_name + L"/logout", headers, "");
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			result = FALSE;
		}
		else
		{
			LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
			result = TRUE;
		}
		this->logged_in = FALSE;
		this->user_name = L"";
		this->token_id = L"";
		return result;
	}

	BOOL UserHandle::GetUserProfile(UserInfo& info)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);

		response = net_api->Get(this->user_name + L"/profile", headers);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		JsonUtility::ParseJsonGetProfileResponse(response.GetContentString(), info);
		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		return TRUE;
	}

	BOOL UserHandle::DeleteAccount()
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);

		response = net_api->Delete(this->user_name, headers);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		this->logged_in = FALSE;
		this->user_name = L"";
		this->token_id = L"";
		return TRUE;
	}

	BOOL UserHandle::UpdateUserProfile(const UserInfo& info)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");

		std::string json_profile = JsonUtility::CreateJsonUpdateProfile(info);

		response = net_api->Put(this->user_name + L"/profile", headers, json_profile);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}

		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		this->user_name = info.user_name;
		return TRUE;
	}

	BOOL UserHandle::ChangePassword(const std::wstring& old_password, const std::wstring& new_password)
	{
		HttpHeaders headers;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");

		std::string json_change_pw = JsonUtility::CreateJsonChangePassword(old_password, new_password);

		HttpResponse response = net_api->Put(this->user_name + L"/password", headers, json_change_pw);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		return TRUE;
	}

	BOOL UserHandle::RemoveFile(const std::wstring& file_path)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);

		DWORD file_id = cache_api->getFileID(file_path);
		response = net_api->Delete(this->user_name + L"\\files\\" + std::to_wstring(file_id), headers);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		cache_api->removeFile(file_path);

		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		return TRUE;
	}

	BOOL UserHandle::RenameFile(const std::wstring& file_path, std::wstring& new_file_name)
	{
		FileInfo file;
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");

		DWORD file_id = cache_api->getFileID(file_path);
		std::wstring old_file_name = Helper::PathHelper::extractFileNameFromFilePath(file_path);
		std::string json_request = JsonUtility::CreateJsonFileRename(old_file_name, new_file_name);
		response = net_api->Put(this->user_name + L"\\files\\rename\\" + std::to_wstring(file_id), headers, json_request);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}

		std::wstring old_folder_path = Helper::PathHelper::extractFolderFromFilePath(file_path);
		std::wstring new_file_path = Helper::PathHelper::combinePathComponent(old_folder_path, new_file_name);
		cache_api->removeFile(file_path);
		cache_api->insertFile(new_file_path, file_id);

		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		return TRUE;
	}

	//---- Private method
	HttpResponse UserHandle::UploadFileMultipart(const FileInfo& file, const std::string& upload_id)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return HttpResponse();
		}
		DWORD fileSize = file.GetFileSize();
		DWORD bufferSize = 10 * MB, bytesRead = 0, totalBytesUploaded = 0;
		std::string boundary = Helper::createUUIDString();
		std::wstring id = Helper::StringHelper::convertStringToWideString(upload_id);

		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader("Content-Type", "multipart/form-data; boundary=" + boundary + "\r\n");
		if (fileSize < bufferSize)
		{
			bufferSize = fileSize;
		}
		BYTE* buffer = new BYTE[bufferSize];
		if (!buffer)
		{
			LOG_ERROR_W(L"Failed to allocate memory!");
			return HttpResponse();
		}
		// Open file handle
		HANDLE hFile = CreateFileW(file.GetFilePath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR_W(L"Failed to opening file handle!");
			return HttpResponse();
		}
		// Loop request POST file data
		while (ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0)
		{
			std::string parent_folder = Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetPathToRoot());
			std::string file_name = Helper::StringHelper::convertWideStringToString(file.GetFileName());
			std::stringstream body;
			/*---------[Folder]-----------------*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"folder\"" << "\r\n";
			body << "Content-Type: text/plain\r\n\r\n";
			body << parent_folder << "\r\n";
			/*---------[File Data]--------------*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"filedata\"; filename=\"" << file_name << "\"\r\n";
			body << "Content-Type: application/octet-stream\r\n\r\n";
			/*---------[Compress Data]----------*/
			std::ostringstream compress;
			{
				//TODO: Using zlib compress buffer
				zstream::ozstream zipper(compress);
				zipper << buffer;
			}
			body << compress.str() << "\r\n";
			/*-----------------------------------*/
			body << "--" << boundary << "--\r\n";
			response = net_api->Post(this->user_name + L"/files/upload/" + id, headers, body.str());
			if (response.GetStatusCode() != 200)
			{
				return response;
			}
			totalBytesUploaded += bytesRead;
			if (totalBytesUploaded > fileSize)
			{
				totalBytesUploaded = fileSize;
			}
			memset(buffer, 0, bufferSize);

			int percentUploaded = (int)(((double)(totalBytesUploaded) / fileSize) * 100);
			printf("\r[Uploading %s: %d%%]", file_name.c_str(), percentUploaded);
			fflush(stdout);
		}
		printf("\n");

		if (hFile != NULL)
		{
			CloseHandle(hFile);
		}
		delete[] buffer;
		return response;
	}
	//---- Private method
	HttpResponse UserHandle::UpdateFileMultipart(const FileInfo& file, const std::string& update_id)
	{
		HttpHeaders headers;
		HttpResponse response;

		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return HttpResponse();
		}
		DWORD fileSize = file.GetFileSize();
		DWORD bufferSize = 10 * MB, bytesRead = 0, totalBytesUpdated = 0;
		std::string boundary = Helper::createUUIDString();
		std::wstring id = Helper::StringHelper::convertStringToWideString(update_id);

		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader("Content-Type", "multipart/form-data; boundary=" + boundary + "\r\n");
		if (fileSize < bufferSize)
		{
			bufferSize = fileSize;
		}
		BYTE* buffer = new BYTE[bufferSize];
		if (!buffer)
		{
			LOG_ERROR_W(L"Failed to allocate memory!");
			return HttpResponse();
		}
		// Open file handle
		HANDLE hFile = CreateFileW(file.GetFilePath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR_W(L"Failed to opening file handle!");
			return HttpResponse();
		}
		// Loop request POST file data
		while (ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0)
		{
			std::string parent_folder = Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetPathToRoot());
			std::string file_name = Helper::StringHelper::convertWideStringToString(file.GetFileName());
			std::stringstream body;
			/*---------[Folder]-----------------*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"folder\"" << "\r\n";
			body << "Content-Type: text/plain\r\n\r\n";
			body << parent_folder << "\r\n";
			/*---------[File Data]--------------*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"filedata\"; filename=\"" << file_name << "\"\r\n";
			body << "Content-Type: application/octet-stream\r\n\r\n";
			/*---------[Compress Data]----------*/
			std::ostringstream compress;
			{
				//TODO: Using zlib compress buffer
				zstream::ozstream zipper(compress);
				zipper << buffer;
			}
			body << compress.str() << "\r\n";
			/*-----------------------------------*/
			body << "--" << boundary << "--\r\n";
			response = net_api->Put(this->user_name + L"/files/update/" + id, headers, body.str());
			if (response.GetStatusCode() != 200)
			{
				return response;
			}
			totalBytesUpdated += bytesRead;
			if (totalBytesUpdated > fileSize)
			{
				totalBytesUpdated = fileSize;
			}
			memset(buffer, 0, bufferSize);

			int percentUpdated = (int)(((double)(totalBytesUpdated) / fileSize) * 100);
			printf("\r[Updating %s: %d%%]", file_name.c_str(), percentUpdated);
			fflush(stdout);
		}
		printf("\n");

		if (hFile != NULL)
		{
			CloseHandle(hFile);
		}
		delete[] buffer;
		return response;
	}


	BOOL UserHandle::UploadFile(const FileInfo& file)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		if (file.GetFileSize() == 0)
		{
			LOG_INFO_W(L"[Client]: File %s is empty. Continue.....", file.GetFileName().c_str());
			return TRUE;
		}

		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");
		/*=====================[Step 1: Initialize Session]======================*/
		std::string json_init = JsonUtility::CreateJsonFileUpload(file);
		response = net_api->Post(this->user_name + L"/files/upload/init", headers, json_init);
		if (response.GetStatusCode() != 201)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		DWORD file_id;
		std::string upload_id;
		if (response.CheckContentIsJson())
		{
			JsonUtility::ParserJsonUploadFileResponse(response.GetContentString(), upload_id, file_id);
			cache_api->insertFile(file.GetFilePath(), file_id);
		}
		/*=======================[Step 2: Upload File Part]=========================*/
		if (upload_id.empty())
		{
			LOG_ERROR_W(L"[Client]: Upload session id cannot be set. \"upload_id\" is empty.");
			return FALSE;
		}
		LOG_INFO_W(L"[Client][POST] Uploading file: %s\n", file.GetFileName().c_str());
		response = UploadFileMultipart(file, upload_id);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		/*==========================[Step 3: Complete Upload]========================*/
		std::string json_complete = "{\n\t upload_id: " + upload_id + " \n}";
		response = net_api->Post(this->user_name + L"/files/upload/complete", headers, json_complete);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		LOG_SUCCESS_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
		return TRUE;
	}

	BOOL UserHandle::UpdateFile(const FileInfo& file)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		if (file.GetFileSize() == 0)
		{
			LOG_INFO_W(L"[Client]: File %s is empty. Continue.....", file.GetFileName().c_str());
			return TRUE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");

		DWORD file_id;
		std::string update_id;
		std::wstring file_path = file.GetFilePath();
		if (!cache_api->isFileExist(file_path))
		{
			LOG_ERROR_W(L"[Client]: Cache file %s not found!", file.GetFileName().c_str());
			return FALSE;
		}
		file_id = cache_api->getFileID(file_path);
		/*=====================[Step 1: Initialize Session]======================*/
		std::string json_init = JsonUtility::CreateJsonFileUpdate(file, file_id);
		response = net_api->Put(this->user_name + L"/files/update/init", headers, json_init);
		if (response.GetStatusCode() != 201)
		{
			return FALSE;
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
		}

		if (response.CheckContentIsJson())
		{
			JsonUtility::ParserJsonUpdateFileResponse(response.GetContentString(), update_id, file_id);
		}
		if (update_id.empty())
		{
			LOG_ERROR_W(L"[Client]: Update session id cannot be set. \"update_id\" is empty.");
			return FALSE;
		}
		/*=======================[Step 2: Update File Part]=========================*/
		LOG_INFO_W(L"[Client][PUT] Updating file: %s", file.GetFileName().c_str());
		response = UpdateFileMultipart(file, update_id);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		/*==========================[Step 3: Complete Update]========================*/
		std::string json_complete = "{\n\t update_id: " + update_id + " \n}";
		response = net_api->Put(this->user_name + L"/files/update/complete", headers, json_complete);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		LOG_SUCCESS_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
		return TRUE;
	}


	BOOL UserHandle::RemoveFolder(const std::wstring& folder_name)
	{
		return FALSE;
	}

	BOOL UserHandle::RenameFolder(const std::wstring& folder_path, std::wstring& new_folder_name)
	{
		return FALSE;
	}


	BOOL UserHandle::UploadFolder(const FolderInfo& folder)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader("Content-Type", "application/json");

		LOG_INFO_W(L"[Client][POST] Uploading folder: %s", folder.GetFolderPath().c_str());

		for (int f = 0; f < folder.CountFile(); f++)
		{
			UploadFile(folder.GetFile(f));
		}
		for (int c = 0; c < folder.CountChildren(); c++)
		{
			return UploadFolder(folder.GetChildren(c));
		}
		cache_api->saveFileCache();
		return TRUE;
	}

	BOOL UserHandle::UpdateFolder(const FolderInfo& folder)
	{
		HttpHeaders headers;
		HttpResponse response;
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader("Content-Type", "application/json");

		LOG_INFO_W(L"[Client][PUT] Updating folder: %s", folder.GetFolderPath().c_str());

		for (int f = 0; f < folder.CountFile(); f++)
		{
			UpdateFile(folder.GetFile(f));
		}
		for (int c = 0; c < folder.CountChildren(); c++)
		{
			return UpdateFolder(folder.GetChildren(c));
		}
		return TRUE;
	}


	BOOL UserHandle::UploadFolderWithFilter(const std::wstring& folder_path, const std::wstring& filter)
	{
		FolderInfo folder;
		if (!FolderHandle::GetFolderFilter(folder_path, filter, folder))
		{
			return FALSE;
		}
		return UploadFolder(folder);
	}

	BOOL UserHandle::UpdateFolderWithFilter(const std::wstring& folder_path, const std::wstring& filter)
	{
		FolderInfo folder;
		if (!FolderHandle::GetFolderFilter(folder_path, filter, folder))
		{
			return FALSE;
		}
		return UpdateFolder(folder);
	}




	//---- Monitor user input to allow exiting
	DWORD WINAPI MonitorKeyboardInput(LPVOID lpParam)
	{
		while (!exitMonitorFlag)
		{
			// Check if 'Q' is pressed
			if (_kbhit())
			{
				// Get the character from keyboard
				char ch = _getch();
				if (ch == 'q' || ch == 'Q')
				{
					exitMonitorFlag = TRUE;
				}
			}
			Sleep(100);
		}
		return 0;
	}

	BOOL UserHandle::PrepareWatch(FolderInfo& folder)
	{
		HttpHeaders headers;
		HttpResponse response;
		cache_api->loadFileCache();

		// Check authentication
		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Auth] Authentication required - User not logged in");
			return FALSE;
		}

		// Setup HTTP headers
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");

		// Step 1: Send folder tree to server for comparison
		LOG_INFO_W(L"[Sync] Sending folder structure to server for comparison");
		std::string json_folder_tree = JsonUtility::CreateJsonFolderTree(folder);
		response = net_api->Post(this->user_name + L"/files/compare", headers, json_folder_tree);

		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server] Request failed with status %ld: %s",
				response.GetStatusCode(),
				response.GetContentWString().c_str());
			return FALSE;
		}

		// Step 2: Parse server response for missing files
		LOG_INFO_W(L"[Sync] Analyzing server response for missing files");
		std::vector<FileMissing> files_missing;
		if (response.CheckContentIsJson())
		{
			std::string json_response = response.GetContentString();
			JsonUtility::ParserJsonFileMissResponse(json_response, files_missing);
		}

		// Step 3: Upload missing files to server
		if (!files_missing.empty())
		{
			LOG_INFO_W(L"[Sync] Found %d files missing on server", files_missing.size());

			for (size_t i = 0; i < files_missing.size(); i++)
			{
				FileMissing file_miss = files_missing[i];
				FileInfo file = folder.FindFileRecursive(file_miss.first, file_miss.second);

				LOG_INFO_W(L"[Upload] Processing file %d/%d: %s",
					i + 1,
					files_missing.size(),
					file.GetFilePath().c_str());

				if (!UploadFile(file))
				{
					LOG_ERROR_W(L"[Upload] Failed to upload file: %s", file.GetFilePath().c_str());
					return FALSE;
				}
			}
			LOG_INFO_W(L"[Sync] Successfully uploaded all missing files");
		}
		else
		{
			LOG_INFO_W(L"[Sync] No missing files found on server");
		}

		LOG_INFO_W(L"[Watch] Watch preparation completed successfully");
		return TRUE;
	}

	BOOL UserHandle::WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter, DWORD wait)
	{
		// Get initial snapshot
		if (!FolderHandle::GetFolderFilter(folder_path, filter, current_snapshot_)) {
			LOG_ERROR_W(L"[Snapshot] Failed to get initial folder tree for: %s", folder_path.c_str());
			return FALSE;
		}
		LOG_INFO_W(L"[Snapshot] Successfully created folder tree for: %s", folder_path.c_str());

		// Prepare watch operation
		if (!PrepareWatch(current_snapshot_)) {
			LOG_ERROR_W(L"[Watch] Failed to prepare watch operation for folder: %s", folder_path.c_str());
			return FALSE;
		}

		// Create directory change notification handle
		HANDLE hDir = CreateFileW(folder_path.c_str(), FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL
		);

		if (hDir == INVALID_HANDLE_VALUE) {
			LOG_ERROR_W(L"[System] Failed to create directory handle. Error code: %d", GetLastError());
			return FALSE;
		}

		// Set up overlapped structure
		OVERLAPPED overlapped = { 0 };
		overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlapped.hEvent) {
			LOG_ERROR_W(L"[System] Failed to create event handle. Error code: %d", GetLastError());
			CloseHandle(hDir);
			return FALSE;
		}

		// Buffer for changes
		BYTE buffer[BUFFER_SIZE] = { 0 };
		DWORD bytesReturned = 0;

		// Create keyboard monitor thread
		HANDLE hKeyboard = CreateThread(NULL, 0, MonitorKeyboardInput, this, 0, NULL);
		if (!hKeyboard) {
			LOG_ERROR_W(L"[System] Failed to create keyboard monitor thread. Error code: %d", GetLastError());
			CloseHandle(overlapped.hEvent);
			CloseHandle(hDir);
			return FALSE;
		}

		LOG_INFO_W(L"[Watch] Starting file system watch on: %s", folder_path.c_str());
		LOG_INFO_W(L"[Watch] Press 'Q' to exit monitoring");

		// Array of handles to wait on
		HANDLE handles[2] = { overlapped.hEvent, hKeyboard };

		while (!exitMonitorFlag) 
		{
			// Start async watch
			BOOL success = ReadDirectoryChangesW(hDir, 
					buffer, BUFFER_SIZE, TRUE,  // Watch subtree
					FILE_NOTIFY_CHANGE_FILE_NAME
					| FILE_NOTIFY_CHANGE_DIR_NAME
					| FILE_NOTIFY_CHANGE_LAST_WRITE,
					&bytesReturned, &overlapped, NULL);

			if (!success) {
				LOG_ERROR_W(L"[System] ReadDirectoryChangesW failed. Error code: %d", GetLastError());
				break;
			}

			// Wait for either directory change or keyboard input
			DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, wait);
			switch (waitResult)
			{
			case WAIT_OBJECT_0:     // Directory change event
			{
				// Changes detected, get a new snapshot and compare
				FolderInfo new_snapshot;
				if (FolderHandle::GetFolderFilter(folder_path, filter, new_snapshot))
				{
					std::lock_guard<std::mutex> lock(snapshot_mutex_);
					DetectAndProcessChanges(current_snapshot_, new_snapshot);
					current_snapshot_ = std::move(new_snapshot);
				}
				else {
					LOG_ERROR_W(L"[Snapshot] Failed to get updated folder tree");
				}
				// Reset event for next notification
				ResetEvent(overlapped.hEvent);
			}
			break;
			case WAIT_OBJECT_0 + 1: // Keyboard thread completed
			{
				LOG_INFO_W(L"[Watch] Received exit signal from keyboard");
				exitMonitorFlag = true;
			}
			break;
			case WAIT_TIMEOUT:
				// Timeout occurred, continue monitoring
				break;
			default:
			{
				LOG_ERROR_W(L"Wait failed with error %d", GetLastError());
				exitMonitorFlag = true;
			}
			break;
			}
		}

		// Cleanup
		if (hKeyboard) {
			CloseHandle(hKeyboard);
		}
		if (overlapped.hEvent) {
			CloseHandle(overlapped.hEvent);
		}
		if (hDir) {
			CloseHandle(hDir);
		}
		return TRUE;
	}

	void UserHandle::DetectAndProcessChanges(const FolderInfo& old_snapshot, const FolderInfo& new_snapshot)
	{
		ActionList actions;

		// 1. Check for deleted and renamed files - Compare old snapshot with new snapshot
		for (const auto& old_file : old_snapshot.GetFilesRecursive())
		{
			FileInfo new_file = new_snapshot.FindFileRecursive
			(
				old_file.GetParentFolder()->GetPathToRoot(),
				old_file.GetFileName()
			);

			// If file not found in new snapshot, check if it was renamed
			if (new_file == FileInfo()) {
				bool isRenamed = false;
				// Look for a file with same size and timestamp but different name
				for (const auto& potential_rename : new_snapshot.GetFilesRecursive())
				{
					FILETIME potentialTime = potential_rename.GetLastWriteTime();
					FILETIME oldFileTime = old_file.GetLastWriteTime();

					if (potential_rename.GetFileSize() == old_file.GetFileSize() &&
						CompareFileTime(&potentialTime, &oldFileTime) == 0 &&
						potential_rename.GetFileName() != old_file.GetFileName()) {
						// Found renamed file
						actions.push_back({
							ACTION_RENAME,
							TRUE,
							potential_rename.GetFilePath(),  // new path
							old_file.GetFilePath()          // old path
							});
						LOG_INFO_W(L"[RENAME] %s -> %s",
							old_file.GetFilePath().c_str(),
							potential_rename.GetFilePath().c_str());
						isRenamed = true;
						break;
					}
				}

				// If not renamed, then it was deleted
				if (!isRenamed) {
					actions.push_back({
						ACTION_DELETE,
						TRUE,
						old_file.GetFilePath(),
						L""
						});
					LOG_INFO_W(L"[DELETE] %s", old_file.GetFilePath().c_str());
				}
			}
		}

		// 2. Check for new and modified files - Compare new snapshot with old snapshot
		for (const auto& new_file : new_snapshot.GetFilesRecursive()) {
			// Skip if this is a renamed file (already handled)
			bool isRenamed = false;
			for (const auto& action : actions) {
				if (action.type == ACTION_RENAME && action.path == new_file.GetFilePath()) {
					isRenamed = true;
					break;
				}
			}
			if (isRenamed) continue;

			FileInfo old_file = old_snapshot.FindFileRecursive
			(
				new_file.GetParentFolder()->GetPathToRoot(),
				new_file.GetFileName()
			);

			if (old_file == FileInfo()) {
				// File not found in old snapshot - it's a new file
				actions.push_back({
					ACTION_UPLOAD,
					TRUE,
					new_file.GetFilePath(),
					L""
					});
				LOG_INFO_W(L"[UPLOAD] %s", new_file.GetFilePath().c_str());
			}
			else {
				// Check for file modifications by comparing size and last write time
				FILETIME newTime = new_file.GetLastWriteTime();
				FILETIME oldTime = old_file.GetLastWriteTime();

				if (new_file.GetFileSize() != old_file.GetFileSize() ||
					CompareFileTime(&newTime, &oldTime) != 0) {
					actions.push_back({
						ACTION_UPDATE,
						TRUE,
						new_file.GetFilePath(),
						L""
						});
					LOG_INFO_W(L"[UPDATE] %s", new_file.GetFilePath().c_str());
				}
			}
		}

		// Process all detected changes using the action list
		if (!actions.empty())
		{
			ProcessSync(actions, const_cast<FolderInfo&>(new_snapshot));
		}
	}

	BOOL UserHandle::ProcessSync(ActionList actions, FolderInfo& folder)
	{
		if (actions.empty()) {
			return TRUE;
		}

		for (const auto& action : actions)
		{
			switch (action.type)
			{
			case SyncActionType::ACTION_UPLOAD:
				LOG_INFO_W(L" ---> Upload File To Server: %s", action.path.c_str());
				// TODO: Add upload logic here
				break;

			case SyncActionType::ACTION_UPDATE:
				LOG_INFO_W(L" ---> Update File To Server: %s", action.path.c_str());
				// TODO: Add update logic here
				break;

			case SyncActionType::ACTION_DELETE:
				LOG_INFO_W(L" ---> Delete File On Server: %s", action.path.c_str());
				// TODO: Add delete logic here
				break;

			case SyncActionType::ACTION_RENAME:
				LOG_INFO_W(L" ---> Rename File On Server: %s -> %s", action.path.c_str(), action.new_path.c_str());
				// TODO: Add rename logic here
				break;
			}
		}
		return TRUE;
	}

	//---- Private method
	BOOL UserHandle::ProcessFileAdd(FolderInfo& folder, std::wstring wSubName)
	{
		FileInfo file_add;
		std::wstring wPath = Helper::PathHelper::combinePathComponent(folder.GetFolderPath(), wSubName);
		std::wstring parent_folder = Helper::PathHelper::extractFolderNameFromFilePath(wPath);
		if (!FileHandle::GetFileInfo(wPath, file_add))
		{
			LOG_ERROR_W(L"Error retrieving file information for: %s", wPath.c_str());
			return FALSE;
		}
		if (!UploadFile(file_add))
		{
			LOG_ERROR_W(L"Failed to upload file: %s", file_add.GetFilePath().c_str());
			return FALSE;
		}
		folder.AddFileRecursive(parent_folder, file_add);
		return TRUE;
	}

	BOOL UserHandle::ProcessFileModified(FolderInfo& folder, std::wstring wSubName)
	{
		if (!folder.HasFile(wSubName))
		{
			LOG_ERROR_W(L"Folder not contain file %s", wSubName.c_str());
			return FALSE;
		}
		FileInfo file = folder.GetFile(wSubName);
		if (!UpdateFile(file))
		{
			LOG_ERROR_W(L"Failed to update file: %s", file.GetFilePath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFileRemove(FolderInfo& folder, std::wstring wSubName)
	{
		if (!folder.HasFile(wSubName))
		{
			LOG_ERROR_W(L"Folder not contain file %s", wSubName.c_str());
			return FALSE;
		}
		FileInfo file = folder.GetFile(wSubName);
		if (!RemoveFile(file.GetFilePath()))
		{
			return FALSE;
		}
		folder.RemoveFile(wSubName);
		return TRUE;
	}

	BOOL UserHandle::ProcessFileRename(FolderInfo& folder, std::wstring wSubName, std::wstring& wLastSubName)
	{
		if (!folder.HasFile(wSubName))
		{
			LOG_ERROR_W(L"Folder not contain file %s", wSubName.c_str());
			return FALSE;
		}
		FileInfo file = folder.GetFile(wLastSubName);
		if (!RenameFile(file.GetFilePath(), wSubName))
		{
			return FALSE;
		}
		file.SetFileName(wSubName);
		return TRUE;
	}

	//---- Private method
	BOOL UserHandle::ProcessFolderAdd(FolderInfo& folder, std::wstring wSubName)
	{
		FolderInfo folder_add;
		std::wstring wPath = Helper::PathHelper::combinePathComponent(folder.GetFolderPath(), wSubName);
		std::wstring parent_folder = Helper::PathHelper::extractFolderNameFromFilePath(wPath);
		if (!FolderHandle::GetFolderInfo(wPath, folder_add))
		{
			LOG_ERROR_W(L"Error retrieving folder information for: %s", wPath.c_str());
			return FALSE;
		}
		if (!UploadFolder(folder_add))
		{
			LOG_ERROR_W(L"Failed to upload folder: %s", folder.GetFolderPath().c_str());
			return FALSE;
		}
		folder.AddChildrenRecursive(parent_folder, folder_add);
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderModified(FolderInfo& folder, std::wstring wSubName)
	{
		if (!folder.HasChildren(wSubName))
		{
			LOG_ERROR_W(L"Folder not contain children %s", wSubName.c_str());
			return FALSE;
		}
		FolderInfo children = folder.GetChildren(wSubName);
		if (!UpdateFolder(folder))
		{
			LOG_ERROR_W(L"Failed to update folder: %s", folder.GetFolderPath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderRemove(FolderInfo& folder, std::wstring wSubName)
	{
		if (!folder.HasChildren(wSubName))
		{
			LOG_ERROR_W(L"Folder not contain children %s", wSubName.c_str());
			return FALSE;
		}
		FolderInfo children = folder.GetChildren(wSubName);
		if (!RemoveFolder(folder.GetFolderPath()))
		{
			return FALSE;
		}
		folder.RemoveChildren(wSubName);
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderRename(FolderInfo& folder, std::wstring wSubName, std::wstring& wLastSubName)
	{
		if (!folder.HasChildren(wSubName))
		{
			LOG_ERROR_W(L"Folder not contain children %s", wSubName.c_str());
			return FALSE;
		}
		FolderInfo children = folder.GetChildren(wLastSubName);
		if (!RenameFolder(folder.GetFolderPath(), wSubName))
		{
			return FALSE;
		}
		children.SetFolderName(wSubName);
		return TRUE;
	}

}
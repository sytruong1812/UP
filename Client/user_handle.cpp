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
#include <unordered_map>

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

	BOOL UserHandle::RenameFile(const std::wstring& file_path, const std::wstring& new_name)
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
		std::string json_request = JsonUtility::CreateJsonFileRename(old_file_name, new_name);
		response = net_api->Put(this->user_name + L"\\files\\rename\\" + std::to_wstring(file_id), headers, json_request);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}

		std::wstring old_folder_path = Helper::PathHelper::extractParentPathFromPath(file_path);
		std::wstring new_file_path = Helper::PathHelper::combinePathComponent(old_folder_path, new_name);
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
			std::string parent_folder = Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetRelativePath());
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
			std::string parent_folder = Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetRelativePath());
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

	BOOL UserHandle::RenameFolder(const std::wstring& folder_path, const std::wstring& new_name)
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
		LOG_INFO_W(L"[Prepare Watch] Sending folder structure to server for comparison");
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
		LOG_INFO_W(L"[Prepare Watch] Analyzing server response for missing files");
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

		LOG_INFO_W(L"[Prepare Watch] Watch preparation completed successfully");
		return TRUE;
	}

	BOOL UserHandle::WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter, DWORD waitMilliseconds)
	{
		//---- Step 1: Get initial snapshot
		if (!FolderHandle::GetFolderFilter(folder_path, filter, current_snapshot))
		{
			LOG_ERROR_W(L"[Snapshot] Failed to get initial folder tree for: %s", folder_path.c_str());
			return FALSE;
		}
		LOG_INFO_W(L"[Snapshot] Successfully created folder tree for: %s", folder_path.c_str());

		//---- Step 2: Prepare watch operation
		//if (!PrepareWatch(current_snapshot_)) 
		//{
		//    LOG_ERROR_W(L"[Watch] Failed to prepare watch operation for folder: %s", folder_path.c_str());
		//    return FALSE;
		//}

		//---- Step 3: Create directory change notification handle
		HANDLE hDirectory = CreateFileW(folder_path.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);
		if (hDirectory == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR_W(L"[System] Failed to create directory handle. Error code: %d", GetLastError());
			return FALSE;
		}

		// Set up overlapped structure
		OVERLAPPED overlapped = { 0 };
		overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!overlapped.hEvent)
		{
			LOG_ERROR_W(L"[System] Failed to create event handle. Error code: %d", GetLastError());
			CloseHandle(hDirectory);
			return FALSE;
		}

		// Buffer for changes
		BYTE buffer[10000] = { 0 };
		DWORD bytesReturned = 0;

		// Create keyboard monitor thread
		exitMonitorFlag = FALSE; // Ensure flag is reset
		HANDLE hKeyboard = CreateThread(NULL, 0, MonitorKeyboardInput, NULL, 0, NULL);
		if (!hKeyboard)
		{
			LOG_ERROR_W(L"[System] Failed to create keyboard monitor thread. Error code: %d", GetLastError());
			CloseHandle(overlapped.hEvent);
			CloseHandle(hDirectory);
			return FALSE;
		}

		LOG_INFO_W(L"[Watch] Starting file system watch on: %s", folder_path.c_str());
		LOG_INFO_W(L"[Watch] Press 'Q' to exit monitoring");

		ActionList actions;	// List action
		HANDLE handles[2] = { hKeyboard, overlapped.hEvent }; // Array of handles to wait on

		while (!exitMonitorFlag)
		{
			// Always reset event before issuing next overlapped I/O
			ResetEvent(overlapped.hEvent);

			BOOL success = ReadDirectoryChangesW(hDirectory,
				buffer, sizeof(buffer), TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
				&bytesReturned, &overlapped, NULL);
			if (!success) {
				LOG_ERROR_W(L"[System] ReadDirectoryChangesW failed. Error code: %d", GetLastError());
				break;
			}

			// Wait for either directory change or keyboard input
			DWORD dwWaitStatus = WaitForMultipleObjects(_countof(handles), handles, FALSE, waitMilliseconds);
			switch (dwWaitStatus)
			{
			case WAIT_OBJECT_0: // Keyboard thread completed
			{
				LOG_INFO_W(L"[Watch] Received exit signal from keyboard");
				exitMonitorFlag = true;
				break;
			}
			case WAIT_OBJECT_0 + 1:     // Directory change event
			{
				BOOL isFileChange = TRUE;
				DWORD dwByteTransferred = 0;
				if (GetOverlappedResult(hDirectory, &overlapped, &dwByteTransferred, FALSE) && dwByteTransferred > 0)
				{
					FILE_NOTIFY_INFORMATION* pNotify = (FILE_NOTIFY_INFORMATION*)buffer;
					std::wstring subject(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
					std::wstring full_path = folder_path + L"\\" + subject;
					// Determine if this is a file or folder change
					isFileChange = Helper::PathHelper::isValidFilePath(full_path);
				}
				// Changes detected, get a new snapshot and compare
				FolderInfo new_snapshot;
				if (!FolderHandle::GetFolderFilter(folder_path, filter, new_snapshot))
				{
					LOG_ERROR_W(L"[Snapshot] Failed to get updated folder tree!");
				}
				else
				{
					std::lock_guard<std::mutex> lock(snapshot_mutex);
					//Detected changes saving to the action list
					if (isFileChange)
					{
						LOG_INFO_W(L"[Detect] Change for file... ");
						DetectChangeForFile(current_snapshot, new_snapshot, actions);
					}
					else
					{
						LOG_INFO_W(L"[Detect] Change for folder... ");
						DetectChangeForFolder(current_snapshot, new_snapshot, actions);
					}
					current_snapshot = std::move(new_snapshot);
				}
				break;
			}
			case WAIT_TIMEOUT:	// Timeout occurred, continue monitoring
			{
				LOG_INFO_W(L"[Watch] Timeout - processing accumulated changes");
				if (!ProcessSync(actions))
				{
					LOG_ERROR_W(L"[Syncing] Failed to sync folder!");
				}
				actions.clear();
				break;
			}
			default:
			{
				LOG_ERROR_W(L"Wait failed with error %d", GetLastError());
				exitMonitorFlag = true;
				break;
			}
			}
		}

		// Cleanup
		if (hKeyboard) {
			CloseHandle(hKeyboard);
		}
		if (overlapped.hEvent) {
			CloseHandle(overlapped.hEvent);
		}
		if (hDirectory) {
			CloseHandle(hDirectory);
		}
		return TRUE;
	}

	void UserHandle::DetectChangeForFile(const FolderInfo& old_snapshot, const FolderInfo& new_snapshot, ActionList& actions)
	{
		// Step 1: Check for REMOVED or RENAMED files - Compare old snapshot with new snapshot
		auto old_files = old_snapshot.GetFilesRecursive();
		auto new_files = new_snapshot.GetFilesRecursive();

		for (const auto& old_file : old_files)
		{
			// 1.1: If file not found in new snapshot, check if it was renamed
			if (new_snapshot.FindFileRecursive(old_file.GetFilePath()) == FileInfo())
			{
				BOOL isRenamed = FALSE;
				if (old_files.size() >= new_files.size())
				{
					// Look for a file with same timestamp and size but different path 
					for (const auto& new_file : new_files)
					{
						FILETIME newFileTime = new_file.GetLastWriteTime();
						FILETIME oldFileTime = old_file.GetLastWriteTime();

						if (CompareFileTime(&newFileTime, &oldFileTime) == 0
							&& new_file.GetFileSize() == old_file.GetFileSize()
							&& new_file.GetHashFile() == old_file.GetHashFile()
							&& new_file.GetFileName() != old_file.GetFileName()
							&& new_file.GetFilePath() != old_file.GetFilePath())
						{
							// Found renamed file
							actions.push_back({ ACTION_RENAME, new FileInfo(old_file), new FileInfo(new_file) });
							LOG_INFO_W(L"[FILE][RENAME] %s -> %s", old_file.GetFilePath().c_str(), new_file.GetFilePath().c_str());
							isRenamed = TRUE;
							break;
						}
					}
					// 1.2: If not renamed, then it was deleted
					if (!isRenamed)
					{
						actions.push_back({ ACTION_REMOVE,  new FileInfo(old_file), NULL });
						LOG_INFO_W(L"[FILE][REMOVE] %s", old_file.GetFilePath().c_str());
					}
				}
			}
		}

		// Step 2: Check for ADD NEW or MODIFIED files - Compare new snapshot with old snapshot
		for (const auto& new_file : new_files)
		{
			// 2.1: Skip if this is a renamed file (already handled)
			BOOL skip = FALSE;
			for (const auto& action : actions)
			{
				if (action.is_folder_ == FALSE
					&& action.type_ == ACTION_RENAME
					&& action.object_new_.file_new_->GetFilePath() == new_file.GetFilePath())
				{
					skip = TRUE;
					break;
				}
			}
			if (skip)
			{
				continue;
			}
			// 2.2: If file not found in old snapshot - it's a new file
			FileInfo old_file = old_snapshot.FindFileRecursive(new_file.GetFilePath());
			if (old_file == FileInfo())
			{
				actions.push_back({ ACTION_ADD, new FileInfo(new_file),  NULL });
				LOG_INFO_W(L"[FILE][ADD] %s", new_file.GetFilePath().c_str());
			}
			else
			{
				FILETIME newFileTime = new_file.GetLastWriteTime();
				FILETIME oldFileTime = old_file.GetLastWriteTime();
				// Check for file modifications by comparing time, size, or hash (any difference = MODIFIED)
				if (
					(CompareFileTime(&newFileTime, &oldFileTime) != 0 ||
						new_file.GetFileSize() != old_file.GetFileSize() ||
						new_file.GetHashFile() != old_file.GetHashFile())
					&& new_file.GetFilePath() == old_file.GetFilePath()
					)
				{
					actions.push_back({ ACTION_MODIFIED, new FileInfo(new_file), NULL });
					LOG_INFO_W(L"[FILE][MODIFIED] %s", new_file.GetFilePath().c_str());
				}
			}
		}
	}

	void UserHandle::DetectChangeForFolder(const FolderInfo& old_snapshot, const FolderInfo& new_snapshot, ActionList& actions)
	{
		auto old_folders = old_snapshot.GetFolderRecursive();
		auto new_folders = new_snapshot.GetFolderRecursive();

		// Step 1: Check for REMOVED or RENAMED folder - Compare old snapshot with new snapshot
		for (const auto& old_folder : old_folders)
		{
			// 1.1: If folder not found in new snapshot, check if it was renamed
			if (new_snapshot.FindChildrenRecursive(old_folder.GetFolderPath()) == FolderInfo())
			{
				BOOL skip = FALSE;
				for (const auto& new_folder : new_folders)
				{
					FILETIME newFolderTime = new_folder.GetChangeTime();
					FILETIME oldFolderTime = old_folder.GetChangeTime();

					if (CompareFileTime(&newFolderTime, &oldFolderTime) == 0
						&& old_folder.GetFolderSize() == new_folder.GetFolderSize()
						&& old_folder.GetFolderName() != new_folder.GetFolderName()
						&& old_folder.GetFolderPath() != new_folder.GetFolderPath())
					{
						// Found renamed folder
						actions.push_back({ ACTION_RENAME, new FolderInfo(old_folder), new FolderInfo(new_folder) });
						LOG_INFO_W(L"[FOLDER][RENAME] %s -> %s", old_folder.GetFolderPath().c_str(), new_folder.GetFolderPath().c_str());
						skip = TRUE;
						break;
					}
				}
				
				if (old_folders.size() > new_folders.size())
				{
					// 1.2: If not renamed, then it was deleted
					if (!skip)
					{
						actions.push_back({ ACTION_REMOVE, new FolderInfo(old_folder), NULL });
						LOG_INFO_W(L"[FOLDER][REMOVE] %s", old_folder.GetFolderPath().c_str());
					}
				}				
			}
		}

		// Step 2: Check for ADD NEW or MODIFIED folders - Compare new snapshot with old snapshot
		for (const auto& new_folder : new_folders)
		{
			// 2.1: Skip if this is a renamed folder (already handled)
			BOOL skip = FALSE;
			for (const auto& action : actions)
			{
				if (action.is_folder_
					&& action.type_ == ACTION_RENAME
					&& action.object_new_.folder_new_->GetFolderPath() == new_folder.GetFolderPath())
				{
					skip = TRUE;
					break;
				}
				if (action.is_folder_ 
					&& action.type_ == ACTION_ADD
					&& action.object_old_.folder_old_->HasChildren(new_folder.GetFolderName()))
				{
					skip = TRUE;
					break;
				}
			}
			if (skip)
			{
				continue;
			}
			// 2.2: If folder not found in old snapshot - it's a new folder
			FolderInfo old_folder = old_snapshot.FindChildrenRecursive(new_folder.GetFolderPath());
			if (old_folder == FolderInfo())
			{ 
				actions.push_back({ ACTION_ADD , new FolderInfo(new_folder), NULL });
				LOG_INFO_W(L"[FOLDER][ADD] %s", new_folder.GetFolderPath().c_str());
			}
		}
	}


	BOOL UserHandle::ProcessSync(const ActionList& actions)
	{
		if (actions.empty()) 
		{
			//LOG_INFO_W(L" ---> [Actions] empty!");
			return TRUE;
		}

		//LOG_INFO_W(L" ---> [Actions] number of action: %d", actions.size());
		for (const auto& action : actions)
		{
			switch (action.type_)
			{
			case SyncActionType::ACTION_ADD:
				if (action.is_folder_)
				{
					LOG_INFO_W(L" ---> [Upload] folder on server: %s", action.object_old_.folder_old_->GetFolderPath().c_str());
					FolderInfo folder_add = *action.object_old_.folder_old_;
					//ProcessFolderAdd(folder_add);
				}
				else
				{
					LOG_INFO_W(L" ---> [Upload] file to server: %s", action.object_old_.file_old_->GetFilePath().c_str());
					FileInfo file_add = *action.object_old_.file_old_;
					//ProcessFileAdd(file_add);
				}
				break;

			case SyncActionType::ACTION_MODIFIED:
				if (action.is_folder_)
				{
					LOG_INFO_W(L" ---> [Update] folder on server: %s", action.object_old_.folder_old_->GetFolderPath().c_str());
					FolderInfo folder_update = *action.object_old_.folder_old_;
					//ProcessFolderModified(folder_update);
				}
				else
				{
					LOG_INFO_W(L" ---> [Update] file to server: %s", action.object_old_.file_old_->GetFilePath().c_str());
					FileInfo file_update = *action.object_old_.file_old_;
					//ProcessFileModified(file_update);
				}
				break;

			case SyncActionType::ACTION_REMOVE:
				if (action.is_folder_)
				{
					LOG_INFO_W(L" ---> [Delete] folder on server: %s", action.object_old_.folder_old_->GetFolderPath().c_str());
					FolderInfo folder_delete = *action.object_old_.folder_old_;
					//ProcessFolderRemove(folder_delete);
				}
				else
				{
					LOG_INFO_W(L" ---> [Delete] file on server: %s", action.object_old_.file_old_->GetFilePath().c_str());
					FileInfo file_delete = *action.object_old_.file_old_;
					//ProcessFileRemove(file_delete);
				}
				break;

			case SyncActionType::ACTION_RENAME:
				if (action.is_folder_)
				{
					LOG_INFO_W(L" ---> [Rename] folder on server: %s -> %s", action.object_old_.folder_old_->GetFolderPath().c_str(), action.object_new_.folder_new_->GetFolderName().c_str());
					FolderInfo old_folder = *action.object_old_.folder_old_;
					FolderInfo new_folder = *action.object_new_.folder_new_;
					//ProcessFolderRename(old_folder, new_folder);
				}
				else
				{
					LOG_INFO_W(L" ---> [Rename] file on server: %s -> %s", action.object_old_.file_old_->GetFilePath().c_str(), action.object_new_.file_new_->GetFilePath().c_str());
					FileInfo old_file = *action.object_old_.file_old_;
					FileInfo new_file = *action.object_new_.file_new_;
					//ProcessFileRename(old_file, new_file);
				}
				break;
			}
		}
		return TRUE;
	}

	//---- Private method
	BOOL UserHandle::ProcessFileAdd(const FileInfo& file_add)
	{
		if (!UploadFile(file_add))
		{
			LOG_ERROR_W(L"Failed to upload file: %s", file_add.GetFilePath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFileModified(const FileInfo& file)
	{
		if (!UpdateFile(file))
		{
			LOG_ERROR_W(L"Failed to update file: %s", file.GetFilePath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFileRemove(const FileInfo& file)
	{
		if (!RemoveFile(file.GetFilePath()))
		{
			LOG_ERROR_W(L"Failed to remove file: %s", file.GetFilePath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFileRename(const FileInfo& old_file, const FileInfo& new_file)
	{
		if (!RenameFile(old_file.GetFilePath(), new_file.GetFileName()))
		{
			LOG_ERROR_W(L"Failed to rename file %s to %s", old_file.GetFilePath().c_str(), new_file.GetFileName().c_str());
			return FALSE;
		}
		return TRUE;
	}

	//---- Private method
	BOOL UserHandle::ProcessFolderAdd(const FolderInfo& folder)
	{
		if (!UploadFolder(folder))
		{
			LOG_ERROR_W(L"Failed to upload folder: %s", folder.GetFolderPath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderModified(const FolderInfo& folder)
	{
		if (!UpdateFolder(folder))
		{
			LOG_ERROR_W(L"Failed to update folder: %s", folder.GetFolderPath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderRemove(const FolderInfo& folder)
	{
		if (!RemoveFolder(folder.GetFolderPath()))
		{
			LOG_ERROR_W(L"Failed to remove folder: %s", folder.GetFolderPath().c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderRename(const FolderInfo& old_folder, const FolderInfo& new_folder)
	{
		if (!RenameFolder(old_folder.GetFolderPath(), new_folder.GetFolderName()))
		{
			LOG_ERROR_W(L"Failed to rename folder %s to %s", old_folder.GetFolderPath().c_str(), new_folder.GetFolderName().c_str());
			return FALSE;
		}
		return TRUE;
	}

}
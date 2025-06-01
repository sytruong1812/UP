#include <queue>
#include <atomic>
#include <conio.h>
#include <sstream>

#include <chrono> 
#include <unordered_map>
#include <unordered_set>

#include "utils.h"
#include "logger.h"
#include "base64.h"
#include "user_handle.h"
#include "zlib/zstream/izstream.h"
#include "zlib/zstream/ozstream.h"

std::atomic<BOOL> exitMonitorFlag(FALSE); // Shared variable to signal exit
std::unordered_map<std::wstring, std::chrono::steady_clock::time_point> debounceMap;
const std::chrono::milliseconds debounceInterval(500);


namespace UserOperations {
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

	void UserHandle::MonitorFolder(const std::wstring& folder_path, const std::wstring& filter, DWORD reload_time)
	{
		if (!FolderHandle::GetFolderInfo(folder_path, snapshot_))
		{
			LOG_ERROR_A("Get snapshot error!");
			return;
		}
		LOG_INFO_A("-> Starting folder monitoring...");
		do
		{
			FolderInfo new_snapshot;
			if (!FolderHandle::GetFolderFilter(folder_path, filter, new_snapshot))
			{
				LOG_ERROR_A("Get new snapshot error!");
				break;
			}

			LOG_INFO_A("-> Checking for changes...");
			if (new_snapshot != snapshot_)
			{
				LOG_INFO_A("-> Changes detected in the folder.");


				snapshot_ = new_snapshot;
			}
			std::this_thread::sleep_for(std::chrono::seconds(reload_time)); // Default 30s 
		} while (true);
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
	//---- Debounce method
	BOOL ProcessDebounce(const std::wstring& filePath)
	{
		auto now = std::chrono::steady_clock::now();      // Step1: Get the current steady clock time
		auto it = debounceMap.find(filePath);             // Step2: Look up the file path in the debounce map

		if (it != debounceMap.end())                      // Step3: If the file has been processed before
		{
			if (now - it->second < debounceInterval)      // Step4: If the elapsed time since last processing is less than debounce interval
			{
				return FALSE;                             // Skip processing to avoid duplicate handling
			}
		}
		debounceMap[filePath] = now;                      // Step5: Update the last processed time to now
		return TRUE;                                      // Allow processing this event
	}
	//---- Remove action name
	void RemoveRedundantUpdates(ACTIONS& actions)
	{
		// Step 1: Collect all paths involved in rename operations
		std::unordered_set<std::wstring> renamedPaths;

		for (const auto& action : actions)
		{
			if (action.type == ACTION_RENAME)
			{
				renamedPaths.insert(action.path);       // old path
				renamedPaths.insert(action.new_path);   // new path
			}
		}

		// Step 2: Remove any UPDATE actions that refer to renamed paths
		actions.erase(std::remove_if(actions.begin(), actions.end(),
			[&renamedPaths](const SyncAction& action) 
			{
					return action.type == ACTION_UPDATE && renamedPaths.find(action.path) != renamedPaths.end();
			}),
			actions.end());
	}


	BOOL UserHandle::PrepareWatch(FolderInfo& folder)
	{
		HttpHeaders headers;
		HttpResponse response;
		cache_api->loadFileCache();

		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return FALSE;
		}
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader("Content-Type", "application/json");

		// Step1: Send folder tree to server
		std::string json_folder_tree = JsonUtility::CreateJsonFolderTree(folder);
		response = net_api->Post(this->user_name + L"/files/compare", headers, json_folder_tree);
		if (response.GetStatusCode() != 200)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		// Step2: Check files miss on server
		std::vector<FileMissing> files_missing;
		if (response.CheckContentIsJson())
		{
			std::string json_response = response.GetContentString();
			JsonUtility::ParserJsonFileMissResponse(json_response, files_missing);
		}
		// Step3: Upload files missing
		for (int i = 0; i < files_missing.size(); i++)
		{
			FileMissing file_miss = files_missing[i];
			FileInfo file = folder.FindFileRecursive(file_miss.first, file_miss.second);
			LOG_INFO_W(L"File missing: %s | %s", file.GetFileName().c_str(), file.GetParentFolder()->GetPathToRoot().c_str());
			if (!UploadFile(file))
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	void UserHandle::MonitorAction(const std::wstring& base_path, FILE_NOTIFY_INFORMATION* notify, ACTIONS& actions)
	{
		std::wstring pendingOldRename;
		FILE_NOTIFY_INFORMATION* pNotify = notify;
		do
		{
			std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
			std::wstring fullPath = base_path + L"\\" + fileName;

			switch (pNotify->Action)
			{
				case FILE_ACTION_ADDED:
					if (ProcessDebounce(fullPath))
					{
						actions.push_back({ SyncActionType::ACTION_UPLOAD, Helper::PathHelper::isValidFilePath(fullPath), fullPath, L"" });
						LOG_INFO_W(L"[UPLOAD] %s", fullPath.c_str());
					}
					break;

				case FILE_ACTION_MODIFIED:
					if (ProcessDebounce(fullPath))
					{
						actions.push_back({ SyncActionType::ACTION_UPDATE, Helper::PathHelper::isValidFilePath(fullPath), fullPath, L"" });
						LOG_INFO_W(L"[UPDATE] %s", fullPath.c_str());
					}
					break;

				case FILE_ACTION_REMOVED:
					actions.push_back({ SyncActionType::ACTION_DELETE, Helper::PathHelper::isValidFilePath(fullPath), fullPath, L"" });
					LOG_INFO_W(L"[DELETE] %s", fullPath.c_str());
					break;

				case FILE_ACTION_RENAMED_OLD_NAME:
					pendingOldRename = fullPath;
					break;

				case FILE_ACTION_RENAMED_NEW_NAME:
					if (!pendingOldRename.empty())
					{
						if (ProcessDebounce(pendingOldRename))
						{
							actions.push_back({ SyncActionType::ACTION_RENAME, Helper::PathHelper::isValidFilePath(fullPath), pendingOldRename, fullPath });
							LOG_INFO_W(L"[RENAME] %s -> %s", pendingOldRename.c_str(), fullPath.c_str());
						}
						pendingOldRename.clear();
					}
					break;
			}
			if (pNotify->NextEntryOffset == 0)
			{
				break;
			}
			pNotify = (FILE_NOTIFY_INFORMATION*)((LPBYTE)pNotify + pNotify->NextEntryOffset);

		} while (TRUE);
	}

	BOOL UserHandle::ProcessSync(ACTIONS actions, FolderInfo& folder)
	{
		if (actions.empty())
			return TRUE;

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


	//BOOL UserHandle::WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter, DWORD wait)
	//{
	//	HANDLE dwHandles[2];
	//	BYTE lpBuffer[10000];
	//	const DWORD dwNotificationFlags = FILE_NOTIFY_CHANGE_LAST_WRITE
	//									| FILE_NOTIFY_CHANGE_CREATION
	//									| FILE_NOTIFY_CHANGE_FILE_NAME
	//									| FILE_NOTIFY_CHANGE_DIR_NAME;

	//	/*=========================[Step 1: Get Folder Tree]==============================*/
	//	FolderInfo folder_sync;
	//	if (!FolderHandle::GetFolderFilter(folder_path, filter, folder_sync))
	//	{
	//		LOG_ERROR_W(L"Failed to get folder filter for: %s", folder_path.c_str());
	//		return FALSE;
	//	}
	//	LOG_INFO_W(L"Get folder tree for: %s --> OK", folder_sync.GetFolderName().c_str());

	//	/*=========================[Step 2: Prepare Watch]==============================*/
	//	//if (!PrepareWatch(folder_sync))
	//	//{
	//	//	LOG_ERROR_W(L"Failed to prepare watch with server metadata for folder: %s", folder_sync.GetFolderName().c_str());
	//	//	return FALSE;
	//	//}
	//	//LOG_INFO_W(L"Prepare watch with server metadata --> OK");
	//	/*=========================[Step 3: ]==============================*/
	//	HANDLE hDirectory = CreateFileW(folder_path.c_str(),
	//									GENERIC_READ,
	//									FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
	//									NULL, OPEN_EXISTING,
	//									FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
	//									NULL);
	//	if (hDirectory == INVALID_HANDLE_VALUE)
	//	{
	//		LOG_ERROR_W(L"CreateFile(folder to trace) function failed = %ld", GetLastError());
	//		return FALSE;
	//	}

	//	HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
	//	if (hEvent == NULL)
	//	{
	//		LOG_ERROR_W(L"Cannot create event! error code = %ld", GetLastError());
	//		return FALSE;
	//	}

	//	// Start the keyboard monitoring thread
	//	HANDLE keyboardMoniterThread = CreateThread(NULL, 0, MonitorKeyboardInput, NULL, 0, 0);

	//	std::wcout << L"Watching for: " << folder_path << L" | Press 'Q' to exit!" << std::endl;

	//	ACTIONS actions;
	//	auto lastActionTime = std::chrono::steady_clock::now();
	//	const std::chrono::milliseconds syncDelay(1000);  // wait 1s before processing batch

	//	while (!exitMonitorFlag)
	//	{
	//		OVERLAPPED overlapped = {};
	//		overlapped.hEvent = hEvent;
	//		LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL;

	//		BOOL result = ReadDirectoryChangesW(hDirectory,
	//											lpBuffer, sizeof(lpBuffer), TRUE,
	//											dwNotificationFlags, 0, &overlapped, lpCompletionRoutine);
	//		if (result)
	//		{
	//			dwHandles[0] = overlapped.hEvent;
	//			dwHandles[1] = keyboardMoniterThread;

	//			DWORD dwWaitStatus = WaitForMultipleObjects(_countof(dwHandles), dwHandles, FALSE, wait);
	//			switch (dwWaitStatus)
	//			{
	//				case WAIT_OBJECT_0: 
	//				{
	//					DWORD NumberOfBytesTransferred = 0;
	//					if (!GetOverlappedResult(hDirectory, &overlapped, &NumberOfBytesTransferred, FALSE))
	//					{
	//						LOG_ERROR_W(L"GetOverlappedResult() return error = %ld", GetLastError());
	//					}
	//					else if (NumberOfBytesTransferred > 0)
	//					{
	//						MonitorAction(folder_path, (FILE_NOTIFY_INFORMATION*)lpBuffer, actions);
	//						
	//						

	//						ResetEvent(overlapped.hEvent);
	//					}
	//					break;
	//				}
	//				case WAIT_OBJECT_0 + 1:
	//				{
	//					std::wcout << L"User has pressed 'Q'. Exiting the monitoring loop..." << std::endl;
	//					exitMonitorFlag = TRUE;
	//					break;
	//				}
	//				case WAIT_TIMEOUT:
	//				{
	//					RemoveRedundantUpdates(actions);

	//					ProcessSync(actions, folder_sync);

	//					actions.clear();
	//					break;
	//				}
	//				case WAIT_IO_COMPLETION:
	//					break;

	//				default:
	//				{
	//					LOG_ERROR_W(L"Unhandled status = %ld", dwWaitStatus);
	//					ExitProcess(GetLastError());
	//					break;
	//				}
	//			}
	//		}
	//		else
	//		{
	//			LOG_ERROR_W(L"ReadDirectoryChangesW error = %lld", GetLastError());
	//		}
	//	}

	//	// Cleanup
	//	if (hEvent) CloseHandle(hEvent);
	//	if (hDirectory) CloseHandle(hDirectory);
	//	if (keyboardMoniterThread) CloseHandle(keyboardMoniterThread);

	//	return TRUE;
	//}

	BOOL UserHandle::WatchFolderSync(const std::wstring& folder_path, const std::wstring& filter, DWORD wait)
	{
		HANDLE dwHandles[2];
		BYTE lpBuffer[10000];
		const DWORD dwNotificationFlags =
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_CREATION |
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME;

		// [Step 1: Get folder tree]
		FolderInfo folder_sync;
		if (!FolderHandle::GetFolderFilter(folder_path, filter, folder_sync))
		{
			LOG_ERROR_W(L"Failed to get folder filter for: %s", folder_path.c_str());
			return FALSE;
		}
		LOG_INFO_W(L"Get folder tree for: %s --> OK", folder_sync.GetFolderName().c_str());

		// [Step 2: Prepare watch]
		//if (!PrepareWatch(folder_sync))
		//{
		//	LOG_ERROR_W(L"Failed to prepare watch with server metadata for folder: %s", folder_sync.GetFolderName().c_str());
		//	return FALSE;
		//}
		//LOG_INFO_W(L"Prepare watch with server metadata --> OK");

		// [Step 3: Prepare watch]
		HANDLE hDirectory = CreateFileW(folder_path.c_str(),
			GENERIC_READ,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);
		if (hDirectory == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR_W(L"CreateFile(folder to trace) function failed = %ld", GetLastError());
			return FALSE;
		}

		HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
		if (hEvent == NULL)
		{
			LOG_ERROR_W(L"Cannot create event! error code = %ld", GetLastError());
			return FALSE;
		}

		HANDLE keyboardMoniterThread = CreateThread(NULL, 0, MonitorKeyboardInput, NULL, 0, 0);
		std::wcout << L"Watching for: " << folder_path << L" | Press 'Q' to exit!" << std::endl;

		ACTIONS actions;
		const size_t batchThreshold = 10;
		const std::chrono::milliseconds syncDelay(1000);
		auto lastActionTime = std::chrono::steady_clock::now();

		while (!exitMonitorFlag)
		{
			OVERLAPPED overlapped = {};
			overlapped.hEvent = hEvent;
			LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL;

			BOOL result = ReadDirectoryChangesW(hDirectory,
				lpBuffer, sizeof(lpBuffer), TRUE,
				dwNotificationFlags, 0, &overlapped, lpCompletionRoutine);

			if (result)
			{
				dwHandles[0] = overlapped.hEvent;
				dwHandles[1] = keyboardMoniterThread;

				DWORD dwWaitStatus = WaitForMultipleObjects(_countof(dwHandles), dwHandles, FALSE, wait);
				switch (dwWaitStatus)
				{
					case WAIT_OBJECT_0: // Directory changed
					{
						DWORD NumberOfBytesTransferred = 0;
						if (GetOverlappedResult(hDirectory, &overlapped, &NumberOfBytesTransferred, FALSE) && NumberOfBytesTransferred > 0)
						{
							MonitorAction(folder_path, (FILE_NOTIFY_INFORMATION*)lpBuffer, actions);
							lastActionTime = std::chrono::steady_clock::now();
							ResetEvent(overlapped.hEvent);
						}
						break;
					}
					case WAIT_OBJECT_0 + 1: // Q pressed
					{
						std::wcout << L"User has pressed 'Q'. Exiting the monitoring loop..." << std::endl;
						exitMonitorFlag = TRUE;
						break;
					}
					case WAIT_TIMEOUT: // Timer tick
					{
						auto now = std::chrono::steady_clock::now();
						if (!actions.empty() &&
							(actions.size() >= batchThreshold || now - lastActionTime >= syncDelay))
						{
							RemoveRedundantUpdates(actions);
							ProcessSync(actions, folder_sync);
							actions.clear();
						}
						break;
					}
					default:
						LOG_ERROR_W(L"Unhandled status = %ld", dwWaitStatus);
						ExitProcess(GetLastError());
						break;
				}
			}
			else
			{
				LOG_ERROR_W(L"ReadDirectoryChangesW error = %lld", GetLastError());
			}
		}

		if (hEvent) CloseHandle(hEvent);
		if (hDirectory) CloseHandle(hDirectory);
		if (keyboardMoniterThread) CloseHandle(keyboardMoniterThread);

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
#include <queue>
#include <atomic>
#include <conio.h> // For _kbhit() and _getch()

#include "utils.h"
#include "logger.h"
#include "base64.h"
#include "user_handle.h"

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
		if (this->logged_in)
		{
			LOG_ERROR_W(L"[Client]: You are already logged in with account \"%s\". Please log out before trying to log in again.", this->user_name.c_str());
			return FALSE;
		}
		response = net_api->Post(L"login", headers, json_login);
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
		std::wstring new_file_path = Helper::PathHelper::combinePath(old_folder_path, new_file_name);
		cache_api->removeFile(file_path);
		cache_api->insertFile(new_file_path, file_id);

		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
		return TRUE;
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
		headers.SetHeader(L"Accept-Encoding", L"gzip, deflate");
		headers.SetHeader(L"Authorization", L"Bearer " + this->token_id);
		headers.SetHeader(L"Content-Type", L"application/json");

		DWORD file_id;
		std::string upload_id;
		/*=====================[Step 1: Initialize Session]======================*/
		std::string json_init = JsonUtility::CreateJsonFileUpload(file);
		response = net_api->Post(this->user_name + L"/files/upload/init", headers, json_init);
		if (response.GetStatusCode() != 201)
		{
			LOG_ERROR_W(L"[Server][%ld]: %s", response.GetStatusCode(), response.GetContentWString().c_str());
			return FALSE;
		}
		if (response.CheckContentIsJson())
		{
			JsonUtility::ParserJsonUploadFileResponse(response.GetContentString(), upload_id, file_id);
			cache_api->insertFile(file.GetFilePath(), file_id);
		}
		/*=======================[Step 2: Upload File Part]=========================*/
		if (upload_id.empty())
		{
			LOG_ERROR_W(L"[Client][Upload-Step2]: Upload session id cannot be set. \"upload_id\" is empty.");
			return FALSE;
		}
		LOG_INFO_W(L"[Client][POST] Uploading file: %s", file.GetFileName().c_str());
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

		LOG_SUCCESS_W(L"[Server]: Response \n%s %s\n", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
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
			LOG_ERROR_W(L"[Client][Update-Step2]: Update session id cannot be set. \"update_id\" is empty.");
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
		LOG_SUCCESS_W(L"[Server]: Response \n%s \n%s", response.GetHeaderWString().c_str(), response.GetContentWString().c_str());
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

		auto files = folder.GetFilesRecursive();
		if (files.empty())
		{
			LOG_ERROR_W(L"The files collection is empty!");
			return FALSE;
		}

		LOG_INFO_W(L"[Client][POST] Uploading folder: %s", folder.GetFolderPath().c_str());
		for (const auto& file : files)
		{
			UploadFile(file);
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

		auto files = folder.GetFilesRecursive();
		if (files.empty())
		{
			LOG_ERROR_W(L"The files collection is empty!");
			return FALSE;
		}

		LOG_INFO_W(L"[Client][PUT] Updating folder: %s", folder.GetFolderPath().c_str());
		for (const auto& file : files)
		{
			UpdateFile(file);
		}
		return TRUE;
	}


	//---- Private method
	HttpResponse UserHandle::UploadFileMultipart(const FileInfo& file, const std::string& upload_id)
	{
		HttpHeaders headers;
		HttpResponse response;

		DWORD fileSize = file.GetFileSize();
		DWORD bufferSize = 10 * MB, bytesRead = 0, totalBytesUploaded = 0;
		std::string boundary = Helper::createUUIDString();
		std::wstring id = Helper::StringHelper::convertStringToWideString(upload_id);

		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return HttpResponse();
		}
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
		HANDLE hInputFile = CreateFileW(file.GetFilePath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFile == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR_W(L"Failed to opening file handle!");
			return HttpResponse();
		}
		// Loop request POST file data
		while (ReadFile(hInputFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0)
		{
			std::string parent_folder = Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetPathToRoot());
			std::string file_name = Helper::StringHelper::convertWideStringToString(file.GetFileName());
			std::stringstream body;
			/*-----[Folder]-----*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"folder\"" << "\r\n";
			body << "Content-Type: text/plain\r\n\r\n";
			body << parent_folder << "\r\n";
			/*-----[File Data]-----*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"filedata\"; filename=\"" << file_name << "\"\r\n";
			body << "Content-Type: application/octet-stream\r\n\r\n";
			body << std::string((char*)buffer, bytesRead) << "\r\n";
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
			wprintf(L"\r[Uploading %s: %d%%]", file.GetFileName().c_str(), percentUploaded);
			fflush(stdout);
		}
		wprintf(L"\n");

		if (hInputFile != NULL)
		{
			CloseHandle(hInputFile);
		}
		delete[] buffer;
		return response;
	}

	HttpResponse UserHandle::UpdateFileMultipart(const FileInfo& file, const std::string& update_id)
	{
		HttpHeaders headers;
		HttpResponse response;

		DWORD fileSize = file.GetFileSize();
		DWORD bufferSize = 10 * MB, bytesRead = 0, totalBytesUploaded = 0;
		std::string boundary = Helper::createUUIDString();
		std::wstring id = Helper::StringHelper::convertStringToWideString(update_id);

		if (!this->logged_in || this->token_id.empty() || this->user_name.empty())
		{
			LOG_ERROR_W(L"[Client]: You need to login to use this function!");
			return HttpResponse();
		}
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
		HANDLE hInputFile = CreateFileW(file.GetFilePath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFile == INVALID_HANDLE_VALUE)
		{
			LOG_ERROR_W(L"Failed to opening file handle!");
			return HttpResponse();
		}
		// Loop request POST file data
		while (ReadFile(hInputFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0)
		{
			std::string parent_folder = Helper::StringHelper::convertWideStringToString(file.GetParentFolder()->GetPathToRoot());
			std::string file_name = Helper::StringHelper::convertWideStringToString(file.GetFileName());
			std::stringstream body;
			/*-----[Folder]-----*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"folder\"" << "\r\n";
			body << "Content-Type: text/plain\r\n\r\n";
			body << parent_folder << "\r\n";
			/*-----[File Data]-----*/
			body << "--" << boundary << "\r\n";
			body << "Content-Disposition: form-data; name=\"filedata\"; filename=\"" << file_name << "\"\r\n";
			body << "Content-Type: application/octet-stream\r\n\r\n";
			body << std::string((char*)buffer, bytesRead) << "\r\n";
			body << "--" << boundary << "--\r\n";

			response = net_api->Put(this->user_name + L"/files/update/" + id, headers, body.str());
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
			wprintf(L"\r[Uploading %s: %d%%]", file.GetFileName().c_str(), percentUploaded);
			fflush(stdout);
		}
		wprintf(L"\n");

		if (hInputFile != NULL)
		{
			CloseHandle(hInputFile);
		}
		delete[] buffer;
		return response;
	}
	//---- Private method


	// Monitor user input to allow exiting
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

	BOOL UserHandle::WatchFolderSync(const std::wstring& folder_path, const std::wstring& extension, DWORD sleep)
	{
		DWORD nEvent = 2;
		BYTE lpBuffer[10000];
		DWORD dwWaitStatus = 0;
		HANDLE dwChangeHandles[2];

		if (cache_api->isEmptyCache())
		{
			cache_api->loadFileCache();
		}
		LOG_INFO_W(L"Watching for: %s", folder_path.c_str());

		HANDLE hDirectory = CreateFileW(folder_path.c_str(),
										GENERIC_READ,
										FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
										NULL,
										OPEN_EXISTING,
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

		// Start the keyboard monitoring thread
		HANDLE keyboardMoniterThread = CreateThread(NULL, 0, MonitorKeyboardInput, NULL, 0, 0);

		while (!exitMonitorFlag) // Check exit flag in the loop condition
		{
			OVERLAPPED overlapped;
			overlapped.hEvent = hEvent;
			LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine = NULL;

			BOOL result = ReadDirectoryChangesW(hDirectory, lpBuffer, 10000,
												TRUE,
												FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
												FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE |
												FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
												0, &overlapped, lpCompletionRoutine);

			if (result)
			{
				std::wcout << L"\n\tWaiting for notification... Press 'q' to exit!" << std::endl;

				dwChangeHandles[0] = overlapped.hEvent;
				dwChangeHandles[1] = keyboardMoniterThread;
				dwWaitStatus = WaitForMultipleObjects(nEvent, dwChangeHandles, FALSE, sleep);
				switch (dwWaitStatus)
				{
					case WAIT_OBJECT_0:
					{
						DWORD NumberOfBytesTransferred = 0;
						if (!GetOverlappedResult(hDirectory, &overlapped, &NumberOfBytesTransferred, FALSE))
						{
							LOG_ERROR_W(L"GetOverlappedResult() return error = %ld", GetLastError());
						}
						else
						{
							if (NumberOfBytesTransferred > 0)
							{
								if (!ProcessSync(folder_path, (FILE_NOTIFY_INFORMATION*)lpBuffer))
								{
									//exitMonitorFlag = TRUE; // Exit on failure
									//break;
									continue;
								}
								ResetEvent(overlapped.hEvent);
							}
						}
						break;
					}
					case WAIT_OBJECT_0 + 1:
					{
						std::wcout << L"User has pressed 'Q'. Exiting the monitoring loop..." << std::endl;
						exitMonitorFlag = TRUE;
						break;
					}
					case WAIT_TIMEOUT:
					{
						LOG_ERROR_W(L"No changes in the timeout period.");
						break;
					}
					default:
					{
						LOG_ERROR_W(L"Unhandled status = %ld", dwWaitStatus);
						ExitProcess(GetLastError());
						break;
					}
				}
			}
			else
			{
				LOG_ERROR_W(L"ReadDirectoryChangesW error = %lld", GetLastError());
			}
		}
		if (keyboardMoniterThread)
		{
			CloseHandle(keyboardMoniterThread);
		}
		if (hEvent)
		{
			CloseHandle(hEvent);
		}
		if(hDirectory)
		{
			CloseHandle(hDirectory);
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessSync(const std::wstring& folder_path, PFILE_NOTIFY_INFORMATION notify)
	{
		DWORD nRecord = 1;
		DWORD offsetNext = 0;
		std::wstring wPath;
		std::wstring wPathOld;
		std::wstring wSubName;
		std::wstring wActionName;

		do
		{
			notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>((char*)notify + offsetNext);  // Next record
			wSubName.assign(notify->FileName, notify->FileNameLength / sizeof(WCHAR));
			wPath = folder_path + L"\\" + wSubName;

			switch (notify->Action)
			{
				case FILE_ACTION_ADDED:
				{
					wActionName = L"Added";
					if (Helper::PathHelper::isDirectory(wPath))
					{
						if (!ProcessFolderAdd(wPath))
						{
							return FALSE;
						}
					}
					else
					{
						if (!ProcessFileAdd(wPath))
						{
							return FALSE;
						}
					}
					break;
				}
				case FILE_ACTION_REMOVED:
				{
					wActionName = L"Removed";
					if (Helper::PathHelper::isValidFilePath(wPath))
					{
						if (!ProcessFileRemove(wPath))
						{
							return FALSE;
						}
					}
					else
					{
						if (!ProcessFolderRemove(wPath))
						{
							return FALSE;
						}
					}
					break;
				}
				case FILE_ACTION_MODIFIED:
				{
					wActionName = L"Modified";
					if (Helper::PathHelper::isDirectory(wPath))
					{
						if (!ProcessFolderModified(wPath))
						{
							return FALSE;
						}
					}
					else
					{
						if (!ProcessFileModified(wPath))
						{
							return FALSE;
						}
					}
					break;
				}
				case FILE_ACTION_RENAMED_OLD_NAME:
				{
					wActionName = L"Renamed (old)";
					wPathOld = wPath;
					break;
				}
				case FILE_ACTION_RENAMED_NEW_NAME:
				{
					wActionName = L"Renamed (new)";
					if (Helper::PathHelper::isDirectory(wPath))
					{
						if (!ProcessFolderRename(wPathOld, wSubName))
						{
							return FALSE;
						}
					}
					else
					{
						if (!ProcessFileRename(wPathOld, wSubName))
						{
							return FALSE;
						}
					}
					break;
				}
				default:
				{
					wActionName = L"Unknown code";
					break;
				}
			}
			LOG_INFO_W(L"Record number %ld : Action = %s (%ld) | Folder/File = %s", nRecord, wActionName.c_str(), notify->Action, wSubName.c_str());
			offsetNext = notify->NextEntryOffset;
			nRecord++;
		} while (offsetNext > 0);

		return TRUE;
	}
	
	//---- Private method
	BOOL UserHandle::ProcessFileAdd(std::wstring& file_path)
	{
		FileInfo file;
		if (!FileHandle::GetFileInfo(file_path, file))
		{
			LOG_ERROR_W(L"Error retrieving file information for: %s", file_path.c_str());
			return FALSE;
		}
		if (!UploadFile(file))
		{
			LOG_ERROR_W(L"Failed to upload file: %s", file_path.c_str());
			return FALSE;
		}
		
		return TRUE;
	}

	BOOL UserHandle::ProcessFileModified(std::wstring& file_path)
	{
		FileInfo file;
		if (!FileHandle::GetFileInfo(file_path, file))
		{
			LOG_ERROR_W(L"Error retrieving file information for: %s", file_path.c_str());
			return FALSE;
		}
		if (!UpdateFile(file))
		{
			LOG_ERROR_W(L"Failed to update file: %s", file_path.c_str());
			return FALSE;
		}
		
		return TRUE;
	}

	BOOL UserHandle::ProcessFileRemove(const std::wstring& file_path)
	{
		return RemoveFile(file_path);
	}

	BOOL UserHandle::ProcessFileRename(const std::wstring& file_path, std::wstring& new_file_name)
	{
		return RenameFile(file_path, new_file_name);
	}
	
	//---- Private method
	BOOL UserHandle::ProcessFolderAdd(std::wstring& folder_path) 
	{
		FolderInfo folder;
		if (!FolderHandle::GetFolderInfo(folder_path, folder))
		{
			LOG_ERROR_W(L"Error retrieving folder information for: %s", folder_path.c_str());
			return FALSE;
		}
		if (!UploadFolder(folder))
		{
			LOG_ERROR_W(L"Failed to upload folder: %s", folder_path.c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderModified(std::wstring& folder_path) 
	{
		FolderInfo folder;
		if (!FolderHandle::GetFolderInfo(folder_path, folder))
		{
			LOG_ERROR_W(L"Error retrieving folder information for: %s", folder_path.c_str());
			return FALSE;
		}
		if (!UpdateFolder(folder))
		{
			LOG_ERROR_W(L"Failed to update folder: %s", folder_path.c_str());
			return FALSE;
		}
		return TRUE;
	}

	BOOL UserHandle::ProcessFolderRemove(const std::wstring& folder_path) 
	{
		return RemoveFolder(folder_path);
	}

	BOOL UserHandle::ProcessFolderRename(const std::wstring& folder_path, std::wstring& new_folder_name) 
	{
		return RenameFolder(folder_path, new_folder_name);
	}

}
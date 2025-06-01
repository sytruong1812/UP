#include "utils.h"
#include "logger.h"
#include "sha256.h"
#include "file_handle.h"
#include "json/json_writer.h"

namespace ResourceOperations
{
	DWORD FileHandle::last_error = FILE_ACTION_SUCCESS;

	DWORD FileHandle::GetLastError()
	{
		return last_error;
	}

	std::string FileHandle::GetLastErrorString()
	{
		return ErrorMessageFile[last_error];
	}

	BOOL FileHandle::GetFileInfo(const std::wstring& path, FileInfo& info)
	{
		WIN32_FILE_ATTRIBUTE_DATA attr_data;
		if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attr_data))
		{
			last_error = ERROR_GET_FILE_ATTRIBUTE;
			return FALSE;
		}
		ULARGE_INTEGER file_size;
		file_size.HighPart = attr_data.nFileSizeHigh;
		file_size.LowPart = attr_data.nFileSizeLow;

		//TODO: Update hash file	
		BYTE digest[SHA256_DIGEST_LENGTH];
		//BYTE* buffer = NULL;
		//DWORD buffer_size = 0;
		//if (!ReadFileData(path, buffer, buffer_size))
		//{
		//	return FALSE;
		//}
		//LOG_INFO_W(L"Hashing file %s", path.c_str());
		//if (!Crypto::SHA256_Hash(buffer, buffer_size, digest))
		//{
		//	return FALSE;
		//}
		//LOG_INFO_W(L"Hashing done!");
		//if (buffer)
		//{
		//	delete[] buffer;
		//}
		info = FileInfo(path, 
						(DWORD)file_size.QuadPart, 
						std::string(reinterpret_cast<char*>(digest), sizeof(digest)),
						attr_data.dwFileAttributes, attr_data.ftCreationTime, attr_data.ftLastWriteTime, attr_data.ftLastAccessTime);
		return TRUE;

	}

	BOOL FileHandle::SetFileInfo(const std::wstring& path, const FileInfo& info)
	{
		HANDLE hOutputFile = CreateFileW(path.c_str(), GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_WRITE_ATTRIBUTES, NULL);
		if (hOutputFile == INVALID_HANDLE_VALUE)
		{
			last_error = ERROR_OPEN_FILE;
			return FALSE;
		}
		FILETIME create_time = info.GetCreateTime();
		FILETIME last_write_time = info.GetLastWriteTime();
		FILETIME last_access_time = info.GetLastAccessTime();
		if (!SetFileTime(hOutputFile, &create_time, &last_access_time, &last_write_time))
		{
			last_error = ERROR_SET_FILE_TIME;
			return FALSE;
		}
		if (!SetFileAttributesW(info.GetFileName().c_str(), info.GetFileAttribute()))
		{
			last_error = ERROR_SET_FILE_ATTRIBUTE;
			return FALSE;
		}
		return 0;
	}

	BOOL FileHandle::RenameFile(const std::wstring& path, const std::wstring& rename)
	{
		return 0;
	}

	BOOL FileHandle::ReadFileData(const std::wstring& path, BYTE*& pData, DWORD& szData)
	{
		BYTE* nBuffer = NULL;
		DWORD nBufferSize = 100 * (1024 * 1024);	//10 MB
		DWORD fileSize, bytesRead, totalBytesRead = 0;
		std::wstring pathEnv = Helper::PathHelper::getPathFromEnvironmentVariable(path);
		HANDLE hInputFile = CreateFileW(pathEnv.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFile == INVALID_HANDLE_VALUE)
		{
			last_error = ERROR_OPEN_FILE;
			return FALSE;
		}
		fileSize = GetFileSize(hInputFile, NULL);
		pData = new BYTE[fileSize];
		if (pData == NULL)
		{
			last_error = ERROR_ALLOCATE_MEMORY;
			CloseHandle(hInputFile);
			return FALSE;
		}
		nBuffer = new BYTE[nBufferSize];
		if (nBuffer == NULL)
		{
			last_error = ERROR_ALLOCATE_MEMORY;
			CloseHandle(hInputFile);
			return FALSE;
		}
		while (ReadFile(hInputFile, nBuffer, nBufferSize, &bytesRead, NULL) && bytesRead > 0)
		{
			memcpy(pData + totalBytesRead, nBuffer, bytesRead);
			totalBytesRead += bytesRead;
		}
		szData = totalBytesRead;
		if (hInputFile != NULL)
		{
			CloseHandle(hInputFile);
		}
		if (nBuffer != NULL)
		{
			delete[] nBuffer;
			nBuffer = NULL;
		}
		return TRUE;
	}

	BOOL FileHandle::WriteFileData(const std::wstring& path, const BYTE* pData, const DWORD& szData)
	{
		DWORD bytesWrite = 0;
		std::wstring pathEnv = Helper::PathHelper::getPathFromEnvironmentVariable(path);
		HANDLE hOutputFile = CreateFileW(pathEnv.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hOutputFile == INVALID_HANDLE_VALUE)
		{
			last_error = ERROR_OPEN_FILE;
			return FALSE;
		}
		if (!WriteFile(hOutputFile, pData, szData, &bytesWrite, NULL) || bytesWrite != szData)
		{
			last_error = ERROR_WRITE_FILE;
			CloseHandle(hOutputFile);
			return FALSE;
		}
		if (hOutputFile != NULL)
		{
			CloseHandle(hOutputFile);
		}
		return TRUE;
	}

	BOOL FileHandle::ReadFilePointer(const std::wstring path, const LONG distance, const DWORD method, BYTE*& pData, DWORD& szData)
	{
		DWORD dwBytesRead = 0;
		std::wstring pathEnv = Helper::PathHelper::getPathFromEnvironmentVariable(path);
		HANDLE hInputFile = CreateFileW(pathEnv.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInputFile == INVALID_HANDLE_VALUE)
		{
			last_error = ERROR_OPEN_FILE;
			return FALSE;
		}
		pData = new BYTE[szData];
		if (pData == NULL)
		{
			last_error = ERROR_ALLOCATE_MEMORY;
			CloseHandle(hInputFile);
			return FALSE;
		}
		DWORD dwPos = SetFilePointer(hInputFile, distance, NULL, method);
		if (dwPos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		{
			last_error = ERROR_SET_FILE_POINTER;
			return FALSE;
		}
		if (ReadFile(hInputFile, pData, szData, &dwBytesRead, NULL))
		{
			if (dwBytesRead == 0)
			{
				last_error = ERROR_READ_EMPTY;
				CloseHandle(hInputFile);
				return FALSE;
			}
		}
		else
		{
			last_error = (GetLastError() == ERROR_HANDLE_EOF) ? ERROR_READ_EOF : ERROR_READ_FILE;
			CloseHandle(hInputFile);
			return FALSE;
		}
		if (hInputFile != NULL)
		{
			CloseHandle(hInputFile);
		}
		return TRUE;
	}

	BOOL FileHandle::WriteFilePointer(const std::wstring path, const LONG distance, const DWORD method, const BYTE* pData, const DWORD& szData)
	{
		DWORD dwPos, dwBytesWritten = 0;
		std::wstring pathEnv = Helper::PathHelper::getPathFromEnvironmentVariable(path);
		HANDLE hAppend = CreateFileW(pathEnv.c_str(), FILE_APPEND_DATA | FILE_GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hAppend == INVALID_HANDLE_VALUE)
		{
			last_error = ERROR_OPEN_FILE;
			return FALSE;
		}
		dwPos = SetFilePointer(hAppend, distance, NULL, method);
		if (!LockFile(hAppend, dwPos, 0, szData, 0))
		{
			last_error = ERROR_LOCK_FILE;
			CloseHandle(hAppend);
			return FALSE;
		}
		if (!WriteFile(hAppend, pData, szData, &dwBytesWritten, NULL))
		{
			last_error = ERROR_WRITE_FILE;
			CloseHandle(hAppend);
			return FALSE;
		}
		if (!UnlockFile(hAppend, dwPos, 0, szData, 0))
		{
			last_error = ERROR_UNLOCK_FILE;
			CloseHandle(hAppend);
			return FALSE;
		}
		if (hAppend != NULL)
		{
			CloseHandle(hAppend);
		}
		return TRUE;
	}
}
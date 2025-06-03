#include <fstream>
#include "file_handle.h"
#include "folder_handle.h"
#include "json/json_writer.h"

namespace ResourceOperations
{
	DWORD FolderHandle::last_error = FOLDER_ACTION_SUCCESS;

	DWORD FolderHandle::GetLastError()
	{
		return last_error;
	}

	std::string FolderHandle::GetLastErrorString()
	{
		return ErrorMessageFolder[last_error];
	}

	BOOL FolderHandle::CreateFolder(const std::wstring& path, const std::wstring& name)
	{
		std::wstring folder_path = path + L"\\" + name;
		return CreateDirectoryW(folder_path.c_str(), NULL);
	}

	BOOL FolderHandle::CreateNestedFolders(const std::wstring& path)
	{
		if (path.length() > MAX_PATH)
		{
			return FALSE;
		}
		if (PathFileExistsW(path.c_str()))
		{
			return TRUE;
		}
		else
		{
			size_t lastSeparatorPos = path.find_last_of(L"\\");
			if (lastSeparatorPos != std::wstring::npos && lastSeparatorPos > 1)
			{
				std::wstring parentPath = path.substr(0, lastSeparatorPos);
				if (CreateNestedFolders(parentPath))
				{
					return CreateDirectoryW(path.c_str(), NULL);
				}
			}
			return FALSE;
		}
	}

	BOOL FolderHandle::RenameFolder(const std::wstring& path, const std::wstring& rename)
	{
		return 0;
	}

	BOOL FolderHandle::DeleteFolder(const std::wstring& path)
	{
		return 0;
	}

	BOOL FolderHandle::GetFolderInfo(const std::wstring& path, FolderInfo& folder)
	{
		if (path.length() > MAX_PATH)
		{
			return FALSE;
		}
		HANDLE hFind = NULL;
		ULONGLONG totalSize = 0;
		folder.SetFolderPath(path);
		folder.SetFolderName(Helper::PathHelper::extractLastComponentFromPath(path));

		WIN32_FILE_ATTRIBUTE_DATA attr_data;
		if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attr_data))
		{
			last_error = ERROR_GET_FOLDER_ATTRIBUTE;
			return FALSE;
		}
		folder.SetChangeTime(attr_data.ftLastWriteTime);
		folder.SetAccessTime(attr_data.ftLastAccessTime);

		WIN32_FIND_DATAW ffd;
		hFind = FindFirstFileW((path + L"\\*").c_str(), &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		do
		{
			if (!wcscmp(ffd.cFileName, L".") || !wcscmp(ffd.cFileName, L".."))
			{
				continue;
			}
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FolderInfo children;
				children.SetRoot(FALSE);
				std::wstring folder_path = path + L"\\" + ffd.cFileName;
				WIN32_FILE_ATTRIBUTE_DATA attr_data;
				if (!GetFileAttributesExW(folder_path.c_str(), GetFileExInfoStandard, &attr_data))
				{
					last_error = ERROR_GET_FOLDER_ATTRIBUTE;
					FindClose(hFind);
					return FALSE;
				}
				children.SetChangeTime(attr_data.ftLastWriteTime);
				children.SetAccessTime(attr_data.ftLastAccessTime);

				if (GetFolderInfo(folder_path, children))
				{
					folder.AddChildren(children);
					totalSize += children.GetFolderSize();
				}
			}
			else
			{
				FileInfo file;
				std::wstring file_path = path + L"\\" + ffd.cFileName;
				if (!FileHandle::GetFileInfo(file_path, file))
				{
					FindClose(hFind);
					last_error = ERROR_GET_FILE_INFO;
					return FALSE;
				}
				totalSize += file.GetFileSize();
				file.SetParentFolder(&folder);
				folder.AddFile(file);
			}
		} while (FindNextFileW(hFind, &ffd) != 0);

		folder.SetFolderSize((DWORD)totalSize);
		if (hFind)
		{
			FindClose(hFind);
		}
		return TRUE;
	}

	BOOL FolderHandle::GetFolderFilter(const std::wstring& path, const std::wstring& filter, FolderInfo& folder)
	{
		if (path.length() > MAX_PATH)
		{
			return FALSE;
		}
		HANDLE hFind = NULL;
		ULONGLONG totalSize = 0;
		folder.SetFolderPath(path);
		folder.SetFolderName(Helper::PathHelper::extractLastComponentFromPath(path));

		WIN32_FILE_ATTRIBUTE_DATA attr_data;
		if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attr_data))
		{
			last_error = ERROR_GET_FOLDER_ATTRIBUTE;
			return FALSE;
		}
		folder.SetChangeTime(attr_data.ftLastWriteTime);
		folder.SetAccessTime(attr_data.ftLastAccessTime);

		WIN32_FIND_DATAW ffd;
		hFind = FindFirstFileExW((path + L"\\" + filter).c_str(), FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		do
		{
			if (!wcscmp(ffd.cFileName, L".") || !wcscmp(ffd.cFileName, L".."))
			{
				continue;
			}
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				FolderInfo children;
				children.SetRoot(FALSE);
				std::wstring folder_path = path + L"\\" + ffd.cFileName;
				WIN32_FILE_ATTRIBUTE_DATA attr_data;
				if (!GetFileAttributesExW(folder_path.c_str(), GetFileExInfoStandard, &attr_data))
				{
					last_error = ERROR_GET_FOLDER_ATTRIBUTE;
					FindClose(hFind);
					return FALSE;
				}
				children.SetChangeTime(attr_data.ftLastWriteTime);
				children.SetAccessTime(attr_data.ftLastAccessTime);
	
				if (GetFolderFilter(folder_path, filter, children))
				{
					folder.AddChildren(children);
					totalSize += children.GetFolderSize();
				}
			}
			else
			{
				FileInfo file;
				std::wstring file_path = path + L"\\" + ffd.cFileName;
				if (!FileHandle::GetFileInfo(file_path, file))
				{
					FindClose(hFind);
					last_error = ERROR_GET_FILE_INFO;
					return FALSE;
				}
				totalSize += file.GetFileSize();
				file.SetParentFolder(&folder);
				folder.AddFile(file);
			}

		} while (FindNextFileW(hFind, &ffd) != 0);

		folder.SetFolderSize((DWORD)totalSize);
		if (hFind)
		{
			FindClose(hFind);
		}
		return TRUE;
	}
}
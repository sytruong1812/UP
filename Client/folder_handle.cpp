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

	BOOL FolderHandle::IsFileEmpty(const std::wstring& filename)
	{
		std::ifstream file(filename);
		return file.peek() == std::ifstream::traits_type::eof();
	}

	BOOL FolderHandle::IsPathExists(const std::wstring& path)
	{
		return GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES;
	}

	BOOL FolderHandle::IsFolderExists(const std::wstring& path)
	{
		DWORD attributes = GetFileAttributesW(path.c_str());
		return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
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
		DWORD folder_size = 0;
		folder.SetFolderPath(path);
		folder.SetFolderName(Helper::PathHelper::extractLastComponentFromPath(path));

		WIN32_FIND_DATAW ffd;
		HANDLE hFind = FindFirstFileW((path + L"\\*").c_str(), &ffd);
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

				if (GetFolderInfo(folder_path, children))
				{
					folder.AddChildren(children);
				}
				folder_size += children.GetFolderSize();
			}
			else
			{
				FileInfo file;
				std::wstring file_path = path + L"\\" + ffd.cFileName;
				if (!FileHandle::GetFileInfo(file_path, file))
				{
					FindClose(hFind);
					return FALSE;
				}
				folder_size += file.GetFileSize();
				file.SetParentFolder(&folder);
				folder.AddFile(file);
			}
		} while (FindNextFileW(hFind, &ffd) != 0);

		folder.SetFolderSize(folder_size);
		FindClose(hFind);
		return TRUE;
	}

	BOOL FolderHandle::GetFolderFilter(const std::wstring& path, const std::wstring& filter, FolderInfo& folder)
	{
		if (path.length() > MAX_PATH)
		{
			return FALSE;
		}
		DWORD folder_size = 0;
		folder.SetFolderPath(path);
		folder.SetFolderName(Helper::PathHelper::extractLastComponentFromPath(path));

		WIN32_FIND_DATAW ffd;
		HANDLE hFind = FindFirstFileExW((path + L"\\" + filter).c_str(), FindExInfoStandard, &ffd, FindExSearchNameMatch, NULL, 0);
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

				if (GetFolderInfo(folder_path, children))
				{
					folder.AddChildren(children);
				}
				folder_size += children.GetFolderSize();
			}
			else
			{
				FileInfo file;
				std::wstring file_path = path + L"\\" + ffd.cFileName;
				if (!FileHandle::GetFileInfo(file_path, file))
				{
					FindClose(hFind);
					return FALSE;
				}
				folder_size += file.GetFileSize();
				file.SetParentFolder(&folder);
				folder.AddFile(file);
			}
			folder.SetFolderSize(folder_size);

		} while (FindNextFileW(hFind, &ffd) != 0);

		FindClose(hFind);
		return TRUE;
	}
}
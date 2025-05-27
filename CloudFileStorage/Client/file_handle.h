#pragma once
#include "Windows.h"
#include "file_info.h"

namespace ResourceOperations
{
	enum FILE_ERROR : DWORD
	{
		FILE_ACTION_SUCCESS,
		ERROR_OPEN_FILE,
		ERROR_GET_FILE_SIZE,
		ERROR_ALLOCATE_MEMORY,
		ERROR_READ_FILE,
		ERROR_WRITE_FILE,
		ERROR_READ_EMPTY,
		ERROR_SET_FILE_POINTER,
		ERROR_LOCK_FILE,
		ERROR_UNLOCK_FILE,
		ERROR_READ_EOF,
		ERROR_GET_FILE_TIME,
		ERROR_SET_FILE_TIME,
		ERROR_GET_FILE_ATTRIBUTE,
		ERROR_SET_FILE_ATTRIBUTE,
	};

	const std::string ErrorMessageFile[] =
	{
		"",
		"Failed to opening file handle!",
		"Failed to get file size!",
		"Failed to allocate memory!",
		"Failed to read data from file!",
		"Failed to write data to file!",
		"Failed to read data, data is empty!",
		"Failed to set file pointer!",
		"Failed to lock file pointer!",
		"Failed to unlock file pointer!",
		"Reached the end of the file!"
		"Failed to get file time!",
		"Failed to set file time!",
		"Failed to get file attribute!",
		"Failed to set file attribute!",
	};

	class FileHandle 
	{
	private:
		static DWORD last_error;
	public:
		static DWORD GetLastError();
		static std::string GetLastErrorString();
		static BOOL GetFileInfo(const std::wstring& path, FileInfo& info);
		static BOOL SetFileInfo(const std::wstring& path, const FileInfo& info);
		static BOOL RenameFile(const std::wstring& path, const std::wstring& rename);
		static BOOL ReadFileData(const std::wstring& path, BYTE*& pData, DWORD& szData);
		static BOOL WriteFileData(const std::wstring& path, const BYTE* pData, const DWORD& szData);
		static BOOL ReadFilePointer(const std::wstring path, const LONG distance, const DWORD method, BYTE*& pData, DWORD& szData);
		static BOOL WriteFilePointer(const std::wstring path, const LONG distance, const DWORD method, const BYTE* pData, const DWORD& szData);
	};
}

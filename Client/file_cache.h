#pragma once
#include <map>
#include <string>
#include <Windows.h>

namespace UserOperations 
{
	class FileCache 
	{
	private:
		std::wstring store_path;
		std::map<std::wstring /*file_path*/, DWORD /*file_id*/> file_cache;
	public:
		FileCache(const std::wstring& path) : store_path(path) {}
		bool isEmptyCache();
		bool isFileExist(const std::wstring& path);
		void insertFile(const std::wstring& path, DWORD id);
		void removeFile(const std::wstring& path);
		void removeFile(int index);
		DWORD getFileID(const std::wstring& path);
		DWORD getFileID(int index);
		std::wstring getFilePath(DWORD id);
		std::wstring getFilePath(int index);
		void saveFileCache();
		void loadFileCache();
		void deleteFileCache();
	};

}
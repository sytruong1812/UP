#include <fstream>
#include "file_cache.h"

namespace UserOperations 
{
	bool FileCache::isEmptyCache()
	{
		return file_cache.empty();
	}

	bool FileCache::isFileExist(const std::wstring& path)
	{
		return file_cache.find(path) != file_cache.end();
	}

	void FileCache::insertFile(const std::wstring& path, DWORD id)
	{
		if (!file_cache.empty() && (file_cache.find(path) != file_cache.end()))
		{
			file_cache[path] = id;
		}
		else
		{
			file_cache.insert({ path, id });
		}
	}

	void FileCache::removeFile(int index) 
	{
		if (index < 0 || index >= file_cache.size())
		{
			throw std::out_of_range("Index out of range");
		}
		auto it = file_cache.begin();
		std::advance(it, index);
		file_cache.erase(it);
	}

	void FileCache::removeFile(const std::wstring& path) 
	{
		file_cache.erase(path);
	}

	DWORD FileCache::getFileID(int index)
	{
		if (index < 0 || index >= file_cache.size())
		{
			throw std::out_of_range("Index out of range");
		}
		auto it = file_cache.begin();
		std::advance(it, index);
		return it->second;
	}

	DWORD FileCache::getFileID(const std::wstring& path)
	{
		auto it = file_cache.find(path);
		if (it != file_cache.end())
		{
			return it->second;
		}
		throw std::runtime_error("File not found");
	}

	std::wstring FileCache::getFilePath(int index) 
	{
		if (index < 0 || index >= file_cache.size())
		{
			throw std::out_of_range("Index out of range");
		}
		auto it = file_cache.begin();
		std::advance(it, index);
		return it->first;
	}

	std::wstring FileCache::getFilePath(DWORD id)
	{
		for (const auto& pair : file_cache)
		{
			if (pair.second == id)
			{
				return pair.first;
			}
		}
		throw std::runtime_error("File ID not found");
	}

	void FileCache::saveFileCache()
	{
		std::wofstream ofs(store_path);
		if (!ofs)
		{
			throw std::runtime_error("Could not open file for writing");
		}
		for (const auto& pair : file_cache)
		{
			ofs << pair.first << L" " << pair.second << L"\n";
		}
		ofs.close();
	}

	void FileCache::loadFileCache()
	{
		DWORD id;
		std::wstring path;
		std::wifstream ifs(store_path);
		if (!ifs)
		{
			throw std::runtime_error("Could not open file for reading");
		}
		file_cache.clear();
		while (ifs >> path >> id)
		{
			file_cache[path] = id;
		}
		ifs.close();
	}
}
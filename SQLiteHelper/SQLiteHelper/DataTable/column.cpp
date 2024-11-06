#include "column.h"
#include <stdexcept>

namespace GenericTable {

	DataColumn::~DataColumn()
	{
		column.clear();
	}

	void DataColumn::clear()
	{
		column.clear();
	}

	size_t DataColumn::size()
	{
		return column.size();
	}

	bool DataColumn::contain(const std::string& name)
	{
		for (const auto& it : column)
		{
			if (name.compare(it.first.c_str()) == 0)
			{
				return true;
			}
		}
		return false;
	}

	void DataColumn::removeColumn(const std::string& name)
	{
		auto it = column.begin();
		while (it != column.end())
		{
			if (name.compare(it->first.c_str()) == 0)
			{
				column.erase(it);
			}
			++it;
		}
	}

	ObjectTypes DataColumn::getTypeAt(size_t index)
	{
		if (index <= column.size()) {
			auto it = column.begin();
			std::advance(it, index);
			return it->second;
		}
		return ObjectTypes::TYPE_NULL;
	}

	size_t DataColumn::getColumnIndex(const std::string& name)
	{
		int index = 0;
		auto it = column.begin();
		while (it != column.end())
		{
			if (name.compare(it->first.c_str()) == 0)
			{
				break;
			}
			++index;
			++it;
		}
		return index;
	}

	void DataColumn::removeColumnAt(size_t index)
	{
		if (index <= column.size()) {
			column.erase(column.begin() + index);
		}
	}

	std::string DataColumn::getNameAt(size_t index)
	{
		if (index <= column.size()) {
			auto it = column.begin();
			std::advance(it, index);
			return it->first;
		}
		return std::string();
	}

	void DataColumn::add(const std::string& name, ObjectTypes type)
	{
		column.emplace_back(std::make_pair(name, type));
	}
}
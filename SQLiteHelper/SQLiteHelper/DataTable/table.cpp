#include "table.h"

namespace GenericTable
{
	DataTable::DataTable(const std::string& name) {
		name_ = name;
		columns_ = new DataColumn();
	}

	DataTable::~DataTable() {
		clearDataRow();
		clearDataColumn();
	}

	void DataTable::setName(const std::string& name)
	{
		name_ = name;
	}

	std::string DataTable::getName()
	{
		return name_;
	}

	size_t DataTable::getDataRowSize()
	{
		return rows_.front()->size();
	}

	size_t DataTable::getDataColumnSize()
	{
		return columns_->size();
	}

	void DataTable::clearDataRow()
	{
		if (rows_.empty() == false) {
			for (auto&& it : rows_) {
				delete it;
			}
			rows_.clear();
		}
	}

	void DataTable::clearDataColumn()
	{
		if (columns_ != NULL) {
			delete columns_;
		}
	}

	size_t DataTable::getNumberOfRow()
	{
		return rows_.size();
	}

	void DataTable::addDataRow(DataRow* r) {
		//TODO: Add DataRow to List row
		//push_back -> emplace_back -> Do not create new obj before adding them to the vector
		rows_.emplace_back(r);
	}

	DataRow* DataTable::getDataRowAt(size_t index)
	{
		if (index <= rows_.size()) {
			auto it = rows_.begin();
			std::advance(it, index);
			return *it;
		}
		return NULL;
	}

	void DataTable::removeDataRowAt(size_t index)
	{
		if (index <= rows_.size()) {
			auto it = rows_.begin();
			std::advance(it, index);
			rows_.erase(it);
		}
	}

	std::list<DataRow*> DataTable::getDataRows()
	{
		return rows_;
	}

	DataColumn* DataTable::getDataColumn()
	{
		return columns_;
	}

	void DataTable::setDataColumn(DataColumn* c)
	{
		columns_ = c;
	}

	void DataTable::addDataColumn(const std::string& name, ObjectTypes type)
	{
		columns_->add(name, type);
	}

	void DataTable::updateRowValueAt(size_t col_index, Object* value)
	{
		for (size_t i = 0; i < rows_.size(); i++) {
			auto row = getDataRowAt(i);
			row->updateValueAt(col_index, value);
		}
	}
}
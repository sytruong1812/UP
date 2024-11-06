#pragma once
#include <list>
#include <memory>
#include "row.h"
#include "column.h"

namespace GenericTable {

	class DataTable
	{
	private:
		std::string name_;
		DataColumn* columns_;
		std::list<DataRow*> rows_;
	public:
		DataTable(const std::string& name);
		~DataTable();

		/*
		- Purpose: Destructor to clean up resources in DataTable.
		- Parameters: None.
		- Return value: None.
		*/
		~DataTable();

		/*
		- Purpose: Clears all rows in the DataTable.
		- Parameters: None.
		- Return value: None.
		*/
		void clearDataRow();

		/*
		- Purpose: Clears all columns in the DataTable.
		- Parameters: None.
		- Return value: None.
		*/
		void clearDataColumn();

		/*
		- Purpose: Sets the name of the DataTable.
		- Parameters:
			1. [IN] name: The new name of the DataTable.
		- Return value: None.
		*/
		void setName(const std::string& name);

		/*
		- Purpose: Retrieves the name of the DataTable.
		- Parameters: None.
		- Return value: The name of the DataTable as a string.
		*/
		std::string getName();

		/*
		- Purpose: Retrieves the size of a single DataRow, representing the number of columns.
		- Parameters: None.
		- Return value: Size of a single DataRow as size_t.
		*/
		size_t getDataRowSize();

		/*
		- Purpose: Gets the number of columns in the DataTable.
		- Parameters: None.
		- Return value: The number of columns as size_t.
		*/
		size_t getDataColumnSize();

		/*
		- Purpose: Retrieves the total number of rows in the DataTable.
		- Parameters: None.
		- Return value: The total number of rows as size_t.
		*/
		size_t getNumberOfRow();

		/*
		- Purpose: Adds a new row to the DataTable.
		- Parameters:
			1. [IN] r: Pointer to the DataRow to add.
		- Return value: None.
		*/
		void addDataRow(DataRow* r);

		/*
		- Purpose: Retrieves the DataRow at the specified index.
		- Parameters:
			1. [IN] index: The index of the DataRow to retrieve.
		- Return value: Pointer to the DataRow at the specified index.
		*/
		DataRow* getDataRowAt(size_t index);

		/*
		- Purpose: Removes the DataRow at the specified index.
		- Parameters:
			1. [IN] index: The index of the DataRow to remove.
		- Return value: None.
		*/
		void removeDataRowAt(size_t index);

		/*
		- Purpose: Updates the value in a specific row and column.
		- Parameters:
			1. [IN] col_index: Index of the column to update.
			2. [IN] value: Pointer to the new Object value.
		- Return value: None.
		*/
		void updateRowValueAt(size_t col_index, Object* value);

		/*
		- Purpose: Retrieves all rows in the DataTable.
		- Parameters: None.
		- Return value: A list of pointers to DataRow representing all rows in the DataTable.
		*/
		std::list<DataRow*> getDataRows();

		/*
		- Purpose: Retrieves the DataColumn of the DataTable.
		- Parameters: None.
		- Return value: Pointer to the DataColumn of the DataTable.
		*/
		DataColumn* getDataColumn();

		/*
		- Purpose: Sets the DataColumn for the DataTable.
		- Parameters:
			1. [IN] c: Pointer to the DataColumn to set.
		- Return value: None.
		*/
		void setDataColumn(DataColumn* c);

		/*
		- Purpose: Adds a new column to the DataTable with specified name and type.
		- Parameters:
			1. [IN] name: The name of the column.
			2. [IN] type: The data type of the column (ObjectTypes).
		- Return value: None.
		*/
		void addDataColumn(const std::string& name, ObjectTypes type);
	};
}

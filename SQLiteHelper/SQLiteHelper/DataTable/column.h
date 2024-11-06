#pragma once
#include <vector>
#include <string>
#include "value.h"

namespace GenericTable {

	class DataColumn {
	private:
		std::vector<std::pair<std::string, ObjectTypes>> column;
	public:
		~DataColumn();

		/*
		- Purpose: Clears all columns from the DataColumn object.
		- Parameters: None.
		- Return value: None.
		*/
		void clear();

		/*
		- Purpose: Retrieves the number of columns currently stored.
		- Parameters: None.
		- Return value: The number of columns as a size_t value.
		*/
		size_t size();

		/*
		- Purpose: Checks if a column with the specified name exists.
		- Parameters:
			1. [IN] name: The name of the column to check for existence.
		- Return value: A boolean indicating whether the column exists (true) or not (false).
		*/
		bool contain(const std::string& name);

		/*
		- Purpose: Removes a column with the specified name from the DataColumn object.
		- Parameters:
			1. [IN] name: The name of the column to remove.
		- Return value: A boolean indicating success (true) or failure (false) of the removal operation.
		*/
		void removeColumn(const std::string& name);

		/*
		- Purpose: Retrieves the name of the column at the specified index.
		- Parameters:
			1. [IN] index: The index of the column whose name is to be retrieved.
		- Return value: A string representing the name of the column at the given index.
		*/
		std::string getNameAt(size_t index);

		/*
		- Purpose: Retrieves the type of the column at the specified index.
		- Parameters:
			1. [IN] index: The index of the column whose type is to be retrieved.
		- Return value: An ObjectTypes value representing the type of the column at the given index.
		*/
		ObjectTypes getTypeAt(size_t index);

		/*
		- Purpose: Removes the column at the specified index from the DataColumn object.
		- Parameters:
			1. [IN] index: The index of the column to remove.
		- Return value: None.
		*/
		void removeColumnAt(size_t index);

		/*
		- Purpose: Retrieves the index of a column with the specified name.
		- Parameters:
			1. [IN] name: The name of the column whose index is to be found.
		- Return value: The index of the column as a size_t value. If the column does not exist, it typically returns a value indicating not found (e.g., `size()`).
		*/
		size_t getColumnIndex(const std::string& name);

		/*
		- Purpose: Adds a new column with the specified name and type to the DataColumn object.
		- Parameters:
			1. [IN] name: The name of the column to add.
			2. [IN] type: The type of the column as defined by ObjectTypes.
		- Return value: None.
		*/
		void add(const std::string& name, ObjectTypes type);
	};
}


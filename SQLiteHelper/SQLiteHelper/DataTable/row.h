#pragma once
#include <memory>
#include <vector>
#include <string>
#include "value.h"

namespace GenericTable
{
	class DataRow {
	private:
		std::vector<std::unique_ptr<IValue>> row;
	public:
		~DataRow();

		/*
		- Purpose: Clears all values in the DataRow object.
		- Parameters: None.
		- Return value: None.
		*/
		void clear();

		/*
		- Purpose: Retrieves the number of values in the DataRow.
		- Parameters: None.
		- Return value: The number of values as a size_t.
		*/
		size_t size();

		/*
		- Purpose: Retrieves the value at the specified index.
		- Parameters:
			1. [IN] index: The index of the value to retrieve.
		- Return value: A pointer to the Object at the specified index.
		*/
		Object* getValueAt(size_t index);

		/*
		- Purpose: Removes the value at the specified index.
		- Parameters:
			1. [IN] index: The index of the value to remove.
		- Return value: None.
		*/
		void removeValueAt(size_t index);

		/*
		- Purpose: Updates the value at the specified index with a new Object.
		- Parameters:
			1. [IN] index: The index of the value to update.
			2. [IN] value: A pointer to the new Object.
		- Return value: None.
		*/
		void updateValueAt(size_t index, Object* value);

		/*
		- Purpose: Converts the DataRow to a vector of Object pointers.
		- Parameters: None.
		- Return value: A vector of Object pointers representing the values in the DataRow.
		*/
		std::vector<Object*> to_vector();

		/*
		- Purpose: Clears the provided vector of Object pointers.
		- Parameters:
			1. [IN] vec: A vector of Object pointers to clear.
		- Return value: None.
		*/
		void clear_vector(std::vector<Object*> vec);

		/*
		- Purpose: Adds a new Object pointer to the DataRow.
		- Parameters:
			1. [IN] obj: A pointer to the Object to add.
		- Return value: None.
		*/
		void add(Object* obj);

		/*
		- Purpose: Template function that adds a new value to the DataRow.
		- Parameters:
			1. [IN] data: The data of type T to add.
		- Return value: None.
		*/
		template <typename T>
		void add(T data);

		/*
		- Purpose: Template function that adds a new value with a specified size to the DataRow.
		- Parameters:
			1. [IN] data: The data of type T to add.
			2. [IN] size: The size of the data.
		- Return value: None.
		*/
		template <typename T>
		void add(T data, size_t size);
	}; 
}
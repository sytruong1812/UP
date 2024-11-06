#include "row.h"

namespace GenericTable
{
	DataRow::~DataRow() 
	{
		clear();
	}

	void DataRow::clear() {
		if (row.empty() == false) 
		{
			for (auto&& it : row) {
				it->~IValue();
			}
			row.clear();
		}
	}

	size_t DataRow::size() 
	{
		return row.size();
	}

	Object* DataRow::getValueAt(size_t index)
	{
		if (index <= row.size()) {
			return row[index]->get();	//TODO: Value::get() method -> new a Object -> delete Object
		}
		return NULL;
	}

	void DataRow::removeValueAt(size_t index)
	{
		if (index <= row.size()) {
			row[index]->~IValue();
			row.erase(row.begin() + index);
		}
	}

	void DataRow::updateValueAt(size_t index, Object* value)
	{
		if (index <= row.size()) {
			row[index]->set(value);
		}
	}

	std::vector<Object*> DataRow::to_vector()
	{
		std::vector<Object*> array(row.size());
		for (size_t i = 0; i < row.size(); i++) {
			array[i] = row[i]->get();	//TODO: Value::get() method -> new a Object -> add to vector -> delete Object
		}
		return array;
	}

	void DataRow::clear_vector(std::vector<Object*> vec)
	{
		if (vec.empty() == false) {
			for (auto it : vec) {
				delete it;
			}
		}
	}

	void DataRow::add(Object* obj) 
	{
		if (obj) 
		{
			switch (obj->object_type) {
			case ObjectTypes::TYPE_INT:
			{
				auto value = std::make_unique<Value<int>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_FLOAT:
			{
				auto value = std::make_unique<Value<float>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_LONG:
			{
				auto value = std::make_unique<Value<long>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_DOUBLE:
			{
				auto value = std::make_unique<Value<double>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_LLONG:
			{
				auto value = std::make_unique<Value<long long>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_ULLONG:
			{
				auto value = std::make_unique<Value<unsigned long long>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_STRING:
			{
				auto value = std::make_unique<Value<char*>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			case ObjectTypes::TYPE_BLOB:
			{
				auto value = std::make_unique<Value<uint8_t*>>();
				value->set(obj);
				row.push_back(std::move(value));
				break;
			}
			default:
				break;
			}
		}
	}
}
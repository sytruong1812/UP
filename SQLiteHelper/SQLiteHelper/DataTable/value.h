#pragma once
#include "object.h"

namespace GenericTable {

	/* Virtual Function:
	 *  - A virtual function is only different from a regular member function when called from a pointer.
	 *  - A pointer of the base class can hold the address of an object of the derived class,
	 *    but the opposite is not allowed.
	 *  - Use a virtual function when you want the member function of the class to be called
	 *    based on the object the pointer is pointing to, regardless of the pointer's type.
	*/
	class IValue
	{
	public:
		virtual ~IValue() {}
		virtual Object* get() = 0;
		virtual void set(Object*) = 0;
	};

	/* Templated Inheritance:
	 *  - Allows a derived class to inherit from a template base class.
	 *  - Helps increase flexibility and code reuse.
	*/
	template<class T>
	class Value : public IValue {
	private:
		T value_;
		size_t size_;
	public:
		Value() : value_(NULL), size_(0) {}
		~Value() override;
		Object* get() override;
		void set(Object* obj) override;
		void set(T object) { value_ = object; }
		void set(T object, size_t size) { value_ = object; size_ = size; }
	};

	/*=================[INT]===================*/
	template<>
	inline Value<int>::~Value() {
		value_ = 0;
	}
	template<>
	inline void Value<int>::set(Object* obj) {
		if (obj->object_type == TYPE_INT) {
			value_ = obj->object_value.int_;
		}
	}
	template<>
	inline Object* Value<int>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_INT;
		obj->object_value.int_ = value_;
		return obj;
	}
	/*=================[FLOAT]===================*/
	template<>
	inline Value<float>::~Value() {
		value_ = 0;
	}
	template<>
	inline void Value<float>::set(Object* obj) {
		if (obj->object_type == TYPE_FLOAT) {
			value_ = obj->object_value.float_;
		}
	}
	template<>
	inline Object* Value<float>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_FLOAT;
		obj->object_value.float_ = value_;
		return obj;
	}
	/*=================[LONG]===================*/
	template<>
	inline Value<long>::~Value() {
		value_ = 0;
	}
	template<>
	inline void Value<long>::set(Object* obj) {
		if (obj->object_type == TYPE_LONG) {
			value_ = obj->object_value.long_;
		}
	}
	template<>
	inline Object* Value<long>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_LONG;
		obj->object_value.long_ = value_;
		return obj;
	}
	/*=================[DOUBLE]===================*/
	template<>
	inline Value<double>::~Value() {
		value_ = 0;
	}
	template<>
	inline void Value<double>::set(Object* obj) {
		if (obj->object_type == TYPE_DOUBLE) {
			value_ = obj->object_value.double_;
		}
	}
	template<>
	inline Object* Value<double>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_DOUBLE;
		obj->object_value.double_ = value_;
		return obj;
	}
	/*=================[INT64_T]===================*/
	template<>
	inline Value<long long>::~Value() {
		value_ = 0;
	}
	template<>
	inline void Value<long long>::set(Object* obj) {
		if (obj->object_type == TYPE_LLONG) {
			value_ = obj->object_value.llong_;
		}
	}
	template<>
	inline Object* Value<long long>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_LLONG;
		obj->object_value.llong_ = value_;
		return obj;
	}
	/*=================[UINT64_T]===================*/
	template<>
	inline Value<unsigned long long>::~Value() {
		value_ = 0;
	}
	template<>
	inline void Value<unsigned long long>::set(Object* obj) {
		if (obj->object_type == TYPE_ULLONG) {
			value_ = obj->object_value.ullong_;
		}
	}
	template<>
	inline Object* Value<unsigned long long>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_ULLONG;
		obj->object_value.ullong_ = value_;
		return obj;
	}
	/*=================[STRING]===================*/
	template<>
	inline Value<char*>::~Value() {
		delete[] value_;
		size_ = 0;
	}
	template<>
	inline void Value<char*>::set(Object* obj) {
		if (obj->object_type == TYPE_STRING && obj->object_value.text_.str != NULL) {
			value_ = obj->object_value.text_.str;
			size_ = obj->object_value.text_.length;
		}
	}
	template<>
	inline Object* Value<char*>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_STRING;
		obj->object_value.text_.length = size_;
		obj->object_value.text_.str = new char[size_];
		memcpy(obj->object_value.text_.str, value_, size_);
		return obj;
	}
	/*==================[BLOB]===================*/
	template<>
	inline Value<uint8_t*>::~Value() {
		delete[] value_;
		size_ = 0;
	}
	template<>
	inline void Value<uint8_t*>::set(Object* obj) {
		if (obj->object_type == TYPE_BLOB && obj->object_value.blob_.data != NULL) {
			value_ = obj->object_value.blob_.data;
			size_ = obj->object_value.blob_.length;
		}
	}
	template<>
	inline Object* Value<uint8_t*>::get() {
		Object* obj = new Object();
		obj->object_type = ObjectTypes::TYPE_BLOB;
		obj->object_value.blob_.length = size_;
		obj->object_value.blob_.data = new uint8_t[size_];
		memcpy(obj->object_value.blob_.data, value_, size_);
		return obj;
	}
}
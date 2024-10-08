#pragma once
#include <vector>
#include <string>
#include "object.h"

namespace DATA_TABLE {

	/* Virtual Function:
	* 	- Hàm ảo chỉ khác hàm thành phần thông thường khi được gọi từ một con trỏ.
	* 	- Con trỏ của lớp cơ sở có thể chứa địa chỉ của đối tượng thuộc lớp dẫn xuất,
	*	nhưng ngược lại thì không được.
	*   - Sử dụng hàm ảo khi muốn con trỏ đang trỏ tới đối tượng của lớp nào thì hàm
	*	thành phần của lớp đó sẽ được gọi mà không xem xét đến kiểu của con trỏ.
	*/

	// Interface
	class IValue {
	public:
		virtual ~IValue() {}
		virtual void add(Object*, size_t) = 0;
		virtual Object get() = 0;
	};

	/* Templated Inheritance:
	*	- Templated inheritance cho phép một lớp con kế thừa từ một lớp cha có template,
	*	giúp tăng tính linh hoạt và tái sử dụng mã nguồn.
	*/
	template<class T>
	class Value : public IValue {
	private:
		T value_;
		size_t size_;
	public:
		Value() : size_(0) {}
		~Value() override {}

		void add(Object*, size_t) override;

		void add(T object) {
			value_ = object;
		}
		void add(ObjectTypes type) {
			value_ = static_cast<T>(type);
		}

		Object get() {
			Object obj;
			if (std::is_same<T, int>::value) {
				obj.object_type = TYPE_INT;
				obj.object_value.int_ = value_;
			}
			else if (std::is_same<T, float>::value) {
				obj.object_type = TYPE_FLOAT;
				obj.object_value.float_ = value_;
			}
			else if (std::is_same<T, long>::value) {
				obj.object_type = TYPE_LONG;
				obj.object_value.long_ = value_;
			}
			else if (std::is_same<T, double>::value) {
				obj.object_type = TYPE_DOUBLE;
				obj.object_value.double_ = value_;
			}
			else if (std::is_same<T, char*>::value) {
				obj.object_type = TYPE_STRING;
				obj.object_value.str_ = new char[size_];
				strcpy_s(obj.object_value.str_, size_, reinterpret_cast<const char*>(value_));
			}
			else if (std::is_same<T, uint8_t*>::value) {
				obj.object_type = TYPE_BLOB;
				obj.object_value.blob_ = new uint8_t[size_]; 
				memcpy(obj.object_value.blob_, reinterpret_cast<const char*>(value_), size_);
			}
			else if (std::is_same<T, time_t>::value) {
				obj.object_type = TYPE_TIME;
				obj.object_value.time_ = value_;
			}
			else {
				throw std::exception("Unsupported type!");
			}
			return obj;
		}
	};

	/*
	Template Specialization
	*/
	template<>
	void Value<int>::add(Object* obj, size_t size) {
		if (obj->object_type == TYPE_INT) {
			value_ = obj->object_value.int_;
		}
	}

	template<>
	void Value<float>::add(Object* obj, size_t size) {
		if (obj->object_type == TYPE_FLOAT) {
			value_ = obj->object_value.float_;
		}
	}

	template<>
	void Value<long>::add(Object* obj, size_t size) {
		if (obj->object_type == TYPE_LONG) {
			value_ = obj->object_value.long_;
		}
	}

	template<>
	void Value<double>::add(Object* obj, size_t size) {
		if (obj->object_type == TYPE_DOUBLE) {
			value_ = obj->object_value.double_;
		}
	}

	template<>
	void Value<char*>::add(Object* obj, size_t size) {
		if (value_ != nullptr) {
			delete[] value_;
			value_ = nullptr;
		}
		if (obj->object_type == TYPE_STRING && obj->object_value.str_ != nullptr) {
			value_ = new char[size + 1];
			size_ = size + 1;
			strcpy_s(value_, size + 1, obj->object_value.str_);
		}
		else {
			value_ = nullptr;
		}
	}

	template<>
	void Value<uint8_t*>::add(Object* obj, size_t size) {
		if (value_ != nullptr) {
			delete[] value_;
			value_ = nullptr;
		}
		if (obj->object_type == TYPE_BLOB && obj->object_value.blob_ != nullptr) {
			value_ = new uint8_t[size + 1];
			size_ = size + 1;
			memcpy(value_, obj->object_value.blob_, size + 1);
		}
		else {
			value_ = nullptr;
		}
	}

	template<>
	void Value<time_t>::add(Object* obj, size_t size) {
		if (obj->object_type == TYPE_TIME) {
			value_ = obj->object_value.time_;
		}
	}
}

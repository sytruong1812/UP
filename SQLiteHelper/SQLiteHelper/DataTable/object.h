#pragma once
#include <time.h>
#include <string>
#define _CRT_SECURE_NO_WARNINGS

namespace DATA_TABLE {

	typedef enum object_types_ {
		TYPE_NULL = 101,
		TYPE_INT = 102,
		TYPE_FLOAT = 103,
		TYPE_LONG = 104,
		TYPE_DOUBLE = 105,
		TYPE_STRING = 106,
		TYPE_BLOB = 107,
		TYPE_TIME = 108,
	} ObjectTypes;

	typedef union object_value_ {
		int int_;
		float float_;
		long long_;
		double double_;
		char* str_;
		uint8_t* blob_;
		time_t time_;
	} ObjectValue;

	struct Object {
		ObjectTypes object_type;
		ObjectValue object_value;
		Object() : object_type(TYPE_NULL) {
			object_value = {};
		}
		~Object() {
			cleanup();
		}

		Object(const Object& other) {
			copy(other);
		}

		// Copy assignment operator
		Object& operator=(const Object& other) {
			if (this != &other) {
				cleanup();  // Giải phóng dữ liệu cũ
				copy(other);
			}
			return *this;
		}

		void cleanup() {
			switch (object_type) {
			case TYPE_STRING:
				delete[] object_value.str_;
				break;
			case TYPE_BLOB:
				delete[] object_value.blob_;
				break;
			default:
				break;
			}
			object_type = TYPE_NULL;
		}

		void copy(const Object& other) {
			object_type = other.object_type;
			switch (other.object_type) {
			case TYPE_STRING:
				if (other.object_value.str_ != nullptr) {
					size_t len = strlen(other.object_value.str_) + 1; // Tính độ dài chuỗi, bao gồm cả ký tự null
					object_value.str_ = new char[len];
					strcpy_s(object_value.str_, len, other.object_value.str_); 
				}
				else {
					object_value.str_ = nullptr;
				}
				break;
			case TYPE_BLOB:
				if (other.object_value.blob_ != nullptr) {
					object_value.blob_ = new uint8_t[strlen(reinterpret_cast<const char*>(other.object_value.blob_)) + 1];
					memcpy(object_value.blob_, other.object_value.blob_, strlen(reinterpret_cast<const char*>(other.object_value.blob_)) + 1);
				}
				else {
					object_value.blob_ = nullptr;
				}
				break;
			default:
				object_value = other.object_value;
				break;
			}
		}
	};
}
#pragma once
#include <vector>
#include <string>
#include <stdexcept>

namespace DATA_TABLE {

	class DataColumn {
	private:
		std::vector<std::string> columnNames;
		std::vector<ObjectTypes> columnTypes;

	public:
		DataColumn() = default;
		// Không cần thiết khai báo destructor vì vector tự giải phóng bộ nhớ
		~DataColumn() = default;

		size_t size() const {
			return columnNames.size();
		}
		void clear() {
			columnNames.clear();
			columnTypes.clear();
		}

		void add(const std::string& name, ObjectTypes type) {
			columnNames.push_back(name);
			columnTypes.push_back(type);
		}

		// Lấy kiểu dữ liệu tại vị trí chỉ định
		ObjectTypes getTypeAt(size_t index) const {
			if (index < columnTypes.size()) {
				return columnTypes[index]; // Trả về kiểu dữ liệu của cột
			}
			throw std::out_of_range("Index out of range");
		}

		// Lấy tên cột tại vị trí chỉ định
		std::string getNameAt(size_t index) const {
			if (index < columnNames.size()) {
				return columnNames[index]; // Trả về tên cột
			}
			throw std::out_of_range("Index out of range");
		}
	};
}

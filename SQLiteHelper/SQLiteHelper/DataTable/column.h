#pragma once
#include <vector>
#include <string>
#include <stdexcept>

namespace DATA_TABLE {

	class DataColumn {
	private:
		// Vector lưu tên cột
		std::vector<std::string> columnNames;
		// Vector lưu kiểu dữ liệu tương ứng với tên cột
		std::vector<ObjectTypes> columnTypes;

	public:
		// Constructor mặc định
		DataColumn() = default;

		// Không cần thiết khai báo destructor vì vector tự giải phóng bộ nhớ
		~DataColumn() = default;

		// Trả về kích thước của cột
		size_t size() const {
			return columnNames.size();
		}

		// Xóa toàn bộ cột
		void clear() {
			columnNames.clear();
			columnTypes.clear();
		}

		// Thêm cột vào vector
		void add(const std::string& name, ObjectTypes type) {
			columnNames.push_back(name);
			columnTypes.push_back(type);
		}

		// Lấy kiểu dữ liệu tại vị trí chỉ định
		ObjectTypes getTypeAt(size_t index) const {
			if (index < columnTypes.size()) {
				return columnTypes[index]; // Trả về kiểu dữ liệu của cột
			}
			throw std::out_of_range("Index out of range"); // Ném ngoại lệ nếu chỉ số không hợp lệ
		}

		// Lấy tên cột tại vị trí chỉ định
		std::string getNameAt(size_t index) const {
			if (index < columnNames.size()) {
				return columnNames[index]; // Trả về tên cột
			}
			throw std::out_of_range("Index out of range"); // Ném ngoại lệ nếu chỉ số không hợp lệ
		}
	};
}

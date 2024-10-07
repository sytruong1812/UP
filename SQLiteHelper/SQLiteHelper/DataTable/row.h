#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include "value.h"

namespace DATA_TABLE
{
	class DataRow {
	private:
		std::vector<std::unique_ptr<IValue>> row; // Sử dụng unique_ptr để quản lý các giá trị trong hàng
	public:
		// Trả về size_t để phù hợp với kiểu trả về của std::vector::size()
		size_t size() const
		{
			return row.size();
		}

		// Xóa toàn bộ hàng
		void clear()
		{
			row.clear();
		}

		// Thêm dữ liệu vào hàng (sử dụng make_unique thay cho new)
		template <typename T>
		void add(T data, size_t size) {
			auto value = std::make_unique<Value<T>>(); // Tạo unique_ptr mới bằng make_unique
			value->add(data, size); // Thêm dữ liệu với kích thước
			row.push_back(std::move(value)); // Đưa vào vector
		}
		// Lấy giá trị tại index
		Object getValueAt(size_t index) const {
			if (index < row.size()) {
				return row[index]->get(); // Lấy giá trị từ IValue
			}
			throw std::out_of_range("Index out of range"); // Ném ngoại lệ nếu index không hợp lệ
		}
		// Tùy chọn: Trả về std::vector<Object> để tránh rò rỉ bộ nhớ
		std::vector<Object> to_vector() {
			std::vector<Object> array(row.size());
			for (size_t i = 0; i < row.size(); i++) {
				array[i] = row[i]->get(); // Lấy giá trị từ IValue và thêm vào vector
			}
			return array;
		}
	};
}

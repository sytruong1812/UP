#pragma once
#include <list>
#include <string>
#include <memory> // Để sử dụng smart pointer
#include <cassert>
#include "row.h"
#include "column.h"

namespace DATA_TABLE {

	class DataTable
	{
	private:
		std::string name_; // Tên bảng
		std::unique_ptr<DataColumn> columns_; // Sử dụng unique_ptr để quản lý bộ nhớ
		std::list<std::unique_ptr<DataRow>> rows_; // Sử dụng unique_ptr để quản lý hàng
	public:
		// Constructor với tham số tên bảng
		DataTable(const std::string& name)
			: name_(name), columns_(std::make_unique<DataColumn>()), rows_() {}

		// Destructor để giải phóng bộ nhớ tự động thông qua unique_ptr
		~DataTable() = default;

		// Đặt tên cho bảng
		void setTableName(const std::string& n) {
			name_ = n;
		}

		// Lấy tên bảng
		std::string getTableName() const {
			return name_;
		}

		// Thiết lập danh sách các hàng
		void setDataRow(std::list<std::unique_ptr<DataRow>> rows) {
			rows_ = std::move(rows);
		}

		// Lấy danh sách các hàng
		const std::list<std::unique_ptr<DataRow>>& getDataRow() const {
			return rows_;
		}

		// Thiết lập các cột của bảng
		void setDataColumn(std::unique_ptr<DataColumn> c) {
			columns_ = std::move(c);
		}

		// Lấy các cột của bảng
		DataColumn* getDataColumn() const {
			return columns_.get();
		}

		// Thêm cột mới vào bảng
		void addColumn(const std::string& name, ObjectTypes type) {
			columns_->add(name, type);
		}

		// Thêm hàng mới vào bảng (sử dụng emplace_back)
		void addRow(std::unique_ptr<DataRow> r) {
			rows_.emplace_back(std::move(r));
		}

		// Thêm dữ liệu vào hàng trong bảng
		template <typename T>
		void addDataToRow(size_t index, T data, size_t size) {
			assert(index < rows_.size()); // Kiểm tra chỉ số hàng hợp lệ
			auto it = std::next(rows_.begin(), index); // Tìm iterator đến hàng cần thêm dữ liệu
			(*it)->add(data, size); // Thêm dữ liệu vào hàng
		}
	};
}

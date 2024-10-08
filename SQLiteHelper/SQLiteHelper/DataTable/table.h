#pragma once
#include <list>
#include <string>
#include <memory> 
#include <cassert>
#include "row.h"
#include "column.h"

namespace DATA_TABLE {

	class DataTable
	{
	private:
		std::string name_;
		std::unique_ptr<DataColumn> columns_;
		std::list<std::unique_ptr<DataRow>> rows_;
	public:
		DataTable(const std::string& name)
			: name_(name), columns_(std::make_unique<DataColumn>()), rows_() {}

		~DataTable() = default;

		void setTableName(const std::string& n) {
			name_ = n;
		}

		std::string getTableName() const {
			return name_;
		}

		void setDataRow(std::list<std::unique_ptr<DataRow>> rows) {
			rows_ = std::move(rows);
		}

		const std::list<std::unique_ptr<DataRow>>& getDataRow() const {
			return rows_;
		}

		void setDataColumn(std::unique_ptr<DataColumn> c) {
			columns_ = std::move(c);
		}

		DataColumn* getDataColumn() const {
			return columns_.get();
		}

		void addColumn(const std::string& name, ObjectTypes type) {
			columns_->add(name, type);
		}

		// Thêm hàng mới vào bảng (sử dụng emplace_back)
		// Khi sử dụng push_back, bạn cần tạo sẵn đối tượng trước rồi truyền nó vào container. 
		// Điều này dẫn đến việc đối tượng được tạo ra trước, rồi sau đó sao chép hoặc di chuyển vào container.
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

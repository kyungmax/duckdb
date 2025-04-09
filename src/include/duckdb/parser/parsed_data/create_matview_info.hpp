#pragma once

#include "duckdb/parser/parsed_data/create_table_info.hpp"

namespace duckdb {

struct CreateMatViewInfo : CreateTableInfo {
	CreateMatViewInfo() {
		this->type = CatalogType::MATVIEW_ENTRY;
	}
};

} // namespace duckdb

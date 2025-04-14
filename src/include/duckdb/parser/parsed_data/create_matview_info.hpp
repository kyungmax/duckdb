#pragma once

#include "duckdb/parser/parsed_data/create_view_info.hpp"

namespace duckdb {

struct CreateMatViewInfo : CreateViewInfo {
	CreateMatViewInfo() : CreateViewInfo(CatalogType::MATVIEW_ENTRY) {
	}
};

} // namespace duckdb

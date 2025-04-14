#pragma once

#include "create_view_info.hpp"

namespace duckdb {

struct CreateMatViewInfo : CreateViewInfo {
	CreateMatViewInfo() {
		this->type = CatalogType::MATVIEW_ENTRY;
	}
};

} // namespace duckdb

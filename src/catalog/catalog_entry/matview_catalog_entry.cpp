#include "duckdb/catalog/catalog_entry/matview_catalog_entry.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"

#include <sstream>

namespace duckdb {
MatViewCatalogEntry::MatViewCatalogEntry(Catalog &catalog, SchemaCatalogEntry &schema, BoundCreateTableInfo &info,
                                         shared_ptr<DataTable> inherited_storage)
    : DuckTableEntry(catalog, schema, info, std::move(inherited_storage)), query(std::move(info.query)) {
	this->type = CatalogType::MATVIEW_ENTRY;
	if (query->type == LogicalOperatorType::LOGICAL_PROJECTION) {
		auto &projection = query->Cast<LogicalProjection>();
		for (auto &child : projection.children) {
			if (child->type == LogicalOperatorType::LOGICAL_GET) {
				auto &get = child->Cast<LogicalGet>();
				auto table = get.GetTable();
				if (table) {
					base_table_name = table->name; // Extract the table name
				}
			}
		}
	}
}
} // namespace duckdb

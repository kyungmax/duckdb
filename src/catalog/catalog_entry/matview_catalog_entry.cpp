#include "duckdb/catalog/catalog_entry/matview_catalog_entry.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"

#include <sstream>
#include <duckdb/planner/operator/logical_filter.hpp>

namespace duckdb {
MatViewCatalogEntry::MatViewCatalogEntry(Catalog &catalog, SchemaCatalogEntry &schema, BoundCreateTableInfo &info,
                                         shared_ptr<DataTable> inherited_storage)
    : DuckTableEntry(catalog, schema, info, std::move(inherited_storage)), query(std::move(info.query)) {
	this->type = CatalogType::MATVIEW_ENTRY;
	if (query != nullptr && query->type == LogicalOperatorType::LOGICAL_PROJECTION) {
		auto &projection = query->Cast<LogicalProjection>();
		for (auto &child : projection.children) {
			if (child->type == LogicalOperatorType::LOGICAL_GET) {
				auto &get = child->Cast<LogicalGet>();
				auto table = get.GetTable();
				if (table) {
					base_table_name = table->name; // Extract the table name
				}
				break;
			} else if (child->type == LogicalOperatorType::LOGICAL_FILTER) {
				auto &filter = child->Cast<LogicalFilter>();
				auto &get = filter.children.get(0);
				if (get->type == LogicalOperatorType::LOGICAL_GET) {
					auto &get_op = get->Cast<LogicalGet>();
					auto table = get_op.GetTable();
					if (table) {
						base_table_name = table->name; // Extract the table name
					}
					break;
				}
			}
		}
	}
}

unique_ptr<CatalogEntry> MatViewCatalogEntry::Copy(ClientContext &context) const {
	auto create_info = make_uniq<CreateTableInfo>(schema, name);
	create_info->comment = comment;
	create_info->tags = tags;
	create_info->columns = columns.Copy();

	for (idx_t i = 0; i < constraints.size(); i++) {
		auto constraint = constraints[i]->Copy();
		create_info->constraints.push_back(std::move(constraint));
	}

	auto binder = Binder::CreateBinder(context);
	auto bound_create_info = binder->BindCreateTableCheckpoint(std::move(create_info), schema);
	auto copy = make_uniq<MatViewCatalogEntry>(catalog, schema, *bound_create_info);
	copy->query = query->Copy(context);
	copy->base_table_name = base_table_name;
	return std::move(copy);
}

} // namespace duckdb

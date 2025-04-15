#include "duckdb/catalog/catalog_entry/matview_catalog_entry.hpp"

#include "duckdb/catalog/catalog.hpp"
#include "duckdb/main/connection.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"

#include <sstream>

namespace duckdb {

void MatViewCatalogEntry::Initialize(CreateMatViewInfo &info) {
	query = std::move(info.query);
	this->aliases = info.aliases;
	this->types = info.types;
	this->names = info.names;
	this->temporary = info.temporary;
	this->sql = info.sql;
	this->internal = info.internal;
	this->dependencies = info.dependencies;
	this->comment = info.comment;
	this->tags = info.tags;
	this->column_comments = info.column_comments;
}

MatViewCatalogEntry::MatViewCatalogEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateMatViewInfo &info)
    : StandardEntry(CatalogType::MATVIEW_ENTRY, schema, catalog, info.view_name) {
	Initialize(info);
}

string MatViewCatalogEntry::ToSQL() const {
	if (sql.empty()) {
		//! Return empty sql with view name so pragma view_tables don't complain
		return sql;
	}
	auto info = GetInfo();
	auto result = info->ToString();
	return result;
}
} // namespace duckdb

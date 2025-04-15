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

unique_ptr<CreateInfo> MatViewCatalogEntry::GetInfo() const {
	auto result = make_uniq<CreateMatViewInfo>();
	result->schema = schema.name;
	result->view_name = name;
	result->sql = sql;
	result->query = query ? unique_ptr_cast<SQLStatement, SelectStatement>(query->Copy()) : nullptr;
	result->aliases = aliases;
	result->names = names;
	result->types = types;
	result->temporary = temporary;
	result->dependencies = dependencies;
	result->comment = comment;
	result->tags = tags;
	result->column_comments = column_comments;
	return std::move(result);
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

const SelectStatement &MatViewCatalogEntry::GetQuery() {
	return *query;
}

unique_ptr<CatalogEntry> MatViewCatalogEntry::Copy(ClientContext &context) const {
	D_ASSERT(!internal);
	auto create_info = GetInfo();

	return make_uniq<MatViewCatalogEntry>(catalog, schema, create_info->Cast<CreateMatViewInfo>());
}

} // namespace duckdb

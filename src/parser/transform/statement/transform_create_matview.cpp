#include "duckdb/parser/parsed_data/create_matview_info.hpp"
#include "duckdb/parser/statement/create_statement.hpp"
#include "duckdb/parser/transformer.hpp"

namespace duckdb {

unique_ptr<CreateStatement> Transformer::TransformCreateMatView(duckdb_libpgquery::PGCreateMatViewStmt &stmt) {
	D_ASSERT(stmt.into->rel);
	if (stmt.relkind != duckdb_libpgquery::PG_OBJECT_MATVIEW) {
		throw NotImplementedException("TransformCreateMatView only support MATERIALIZED VIEW!");
	}
	if (stmt.is_select_into || stmt.into->colNames || stmt.into->options) {
		throw NotImplementedException("Unimplemented features for CREATE MATERIALIZED VIEW");
	}
	const auto qualifiedName = TransformQualifiedName(*stmt.into->rel);
	if (stmt.query->type != duckdb_libpgquery::T_PGSelectStmt) {
		throw ParserException("CREATE MATERIALIZED VIEW requires a SELECT clause");
	}
	auto query = Transformer::TransformSelectStmt(*stmt.query, false);
	auto result = make_uniq<CreateStatement>();
	auto info = make_uniq<CreateMatViewInfo>();
	info->schema = qualifiedName.schema;
	info->table = qualifiedName.name;
	info->on_conflict = TransformOnConflict(stmt.onconflict);
	info->temporary =
	    stmt.into->rel->relpersistence == duckdb_libpgquery::PGPostgresRelPersistence::PG_RELPERSISTENCE_TEMP;
	info->query = std::move(query);
	result->info = std::move(info);
	return result;
}

} // namespace duckdb

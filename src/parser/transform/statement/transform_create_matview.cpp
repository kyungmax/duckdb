#include "duckdb/parser/parsed_data/create_matview_info.hpp"
#include "duckdb/parser/statement/create_statement.hpp"
#include "duckdb/parser/transformer.hpp"

namespace duckdb {

unique_ptr<CreateStatement> Transformer::TransformCreateMatView(duckdb_libpgquery::PGCreateMatViewStmt &stmt) {
	D_ASSERT(stmt.type == duckdb_libpgquery::T_PGCreateMatViewStmt);
	D_ASSERT(stmt.view);

	if (stmt.query->type != duckdb_libpgquery::T_PGSelectStmt) {
		throw ParserException("CREATE MATERIALIZED VIEW requires a SELECT clause");
	}

	auto result = make_uniq<CreateStatement>();
	auto info = make_uniq<CreateViewInfo>();

	auto qname = TransformQualifiedName(*stmt.view);
	info->catalog = qname.catalog;
	info->schema = qname.schema;
	info->view_name = qname.name;
	info->temporary = !stmt.view->relpersistence;
	if (info->temporary && IsInvalidCatalog(info->catalog)) {
		info->catalog = TEMP_CATALOG;
	}
	info->on_conflict = TransformOnConflict(stmt.onconflict);
	info->query = TransformSelectStmt(*stmt.query, false);

	if (stmt.aliases && stmt.aliases->length > 0) {
		for (auto c = stmt.aliases->head; c != nullptr; c = lnext(c)) {
			auto val = PGPointerCast<duckdb_libpgquery::PGValue>(c->data.ptr_value);
			switch (val->type) {
			case duckdb_libpgquery::T_PGString: {
				info->aliases.emplace_back(val->val.str);
				break;
			}
			default:
				throw NotImplementedException("MatView projection type");
			}
		}
		if (info->aliases.empty()) {
			throw ParserException("Need at least one column name in CREATE MATERIALIZED VIEW projection list");
		}
	}

	if (stmt.options && stmt.options->length > 0) {
		throw NotImplementedException("MATERIALIZED VIEW options");
	}

	result->info = std::move(info);
	return result;
}

} // namespace duckdb

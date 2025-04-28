#include "duckdb/optimizer/matview_optimizer.hpp"

#include <duckdb/catalog/catalog_entry/duck_schema_entry.hpp>
#include <duckdb/catalog/catalog_entry/matview_catalog_entry.hpp>

namespace duckdb {

struct MViewMetadata {
	string schema_name;
	string catalog_name;
	string base_table_name;
	// vector<unique_ptr<Expression>> where_expressions;
};

unique_ptr<LogicalOperator> MatViewOptimizer::Optimize(unique_ptr<LogicalOperator> op) {
	// Step 1: 먼저 children부터 재귀적으로 Rewrite
	for (auto &child : op->children) {
		child = Optimize(std::move(child));
	}

	// Step 2: 현재 op가 LogicalGet인가 - select
	if (op->type == LogicalOperatorType::LOGICAL_GET) {

		auto &get = op->Cast<LogicalGet>();
		// 🔥 여기서 base table 이름, projection, where condition을 뽑아냄
		optional_ptr<TableCatalogEntry> table_catalog_entry = get.GetTable();
		if (table_catalog_entry->type != CatalogType::TABLE_ENTRY) {
			return op;
		}
		auto projection_columns = get.names;
		// auto where_condition = filter.expressions;
		MViewMetadata mview_metadata = {table_catalog_entry->schema.name, table_catalog_entry->catalog.GetName(),
		                                table_catalog_entry->name};
		if (ifMatchingMViewExists(context, mview_metadata)) {
			// 매칭 성공 시 MView LogicalGet 생성
			// TODO: 실제 mview 정보를 가져와야 함
			// auto mview_table = get.table; // 일단 임시로 같은 table 가정
			// auto mview_get = make_uniq<LogicalGet>(mview_table, get, get.names);
			//
			// auto new_projection = make_uniq<LogicalProjection>(projection.expressions, std::move(mview_get));
			// auto new_filter = make_uniq<LogicalFilter>(filter.expression.Copy(), std::move(new_projection));

			// return new_filter;
			return op;
		}
	}
	return op;
}

bool MatViewOptimizer::ifMatchingMViewExists(ClientContext &context, const MViewMetadata &mview) {
	auto &schema = Catalog::GetSchema(context, mview.catalog_name, mview.schema_name);
	vector<MatViewCatalogEntry *> matview_entries;
	schema.Scan(context, CatalogType::MATVIEW_ENTRY, [&](CatalogEntry &entry) {
		if (entry.type == CatalogType::MATVIEW_ENTRY) {
			MatViewCatalogEntry &mview_entry = entry.Cast<MatViewCatalogEntry>();
			if (mview.base_table_name == mview_entry.base_table_name) {
				matview_entries.push_back(&mview_entry);
			}
		}
	});
	// Base table for MView matches the base table in the query
	for (auto mview_entry : matview_entries) {

	}
	// auto table_names = schema.GetSimilarEntry()
	//
	//
	// auto &schema = Catalog::GetSchema(context, mview.base_table_name);
	// auto &catalog = schema.catalog;
	// Catalog::GetCatalogEntries() for (auto &entry : schema.GetEntry().find(CatalogType::TABLE_ENTRY)second) {
	// 	auto *table_entry = (TableCatalogEntry *)entry.second.get();
	//
	// 	if (!table_entry) {
	// 		continue;
	// 	}
	//
	// 	// TableCatalogEntry 안에 우리가 MView 구분할 수 있도록 flag를 넣어놨다고 가정
	// 	if (table_entry.IsMaterializedView()) {
	// 		auto *mview = dynamic_cast<MaterializedViewCatalogEntry *>(table_entry);
	// 		if (!mview) {
	// 			continue;
	// 		}
	//
	// 		// Base table 이름 일치하는지 체크
	// 		if (mview.base_table_name == base_table_name) {
	// 			result.push_back(mview);
	// 		}
	// 	}
	// }
	//
	// Catalog::GetEntry(context, CatalogType::MATVIEW_ENTRY, schema)
	//     // Step 1: Base Table 이름 일치
	//     if (mview.base_table_name != query_base_table) {
	// 	return false;
	// }
	//
	// // Step 2: Projection 수 일치
	// if (mview.projection_expressions.size() != query_projection_expressions.size()) {
	// 	return false;
	// }
	//
	// // Step 3: Projection Expression 하나하나 비교 (exact match)
	// for (size_t i = 0; i < mview.projection_expressions.size(); i++) {
	// 	if (!Expression::Equals(mview.projection_expressions[i].get(), query_projection_expressions[i].get())) {
	// 		return false;
	// 	}
	// }
	//
	// // (초기 버전) Step 4: WHERE 조건 없는 것만 지원

	return true;
}
};
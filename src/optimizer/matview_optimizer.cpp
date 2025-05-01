#include "duckdb/optimizer/matview_optimizer.hpp"

#include <duckdb/catalog/catalog_entry/duck_schema_entry.hpp>
#include <duckdb/catalog/catalog_entry/matview_catalog_entry.hpp>
#include <duckdb/function/table/table_scan.hpp>
#include <duckdb/planner/binder.hpp>

namespace duckdb {

struct MViewMetadata {
	std::string schema_name;
	std::string catalog_name;
	std::string base_table_name;
	std::vector<string> column_names;
	std::vector<unique_ptr<Expression>> where_expressions;
};

struct MViewQueryInfo {
	vector<string> column_names;
	vector<unique_ptr<Expression>> where_expressions;
};

unique_ptr<LogicalOperator> MatViewOptimizer::Optimize(unique_ptr<LogicalOperator> op, Binder &binder) {
	for (auto &child : op->children) {
		child = Optimize(std::move(child), binder);
	}
	if (op->type != LogicalOperatorType::LOGICAL_PROJECTION) {
		return op;
	}

	auto &projection = op->Cast<LogicalProjection>();

	LogicalFilter *logical_filter = nullptr;
	LogicalGet *logical_get = nullptr;

	auto &proj_child = projection.children[0];

	if (proj_child->type == LogicalOperatorType::LOGICAL_FILTER) {
		// FILTER 존재하는 경우
		logical_filter = &proj_child->Cast<LogicalFilter>();
		if (logical_filter->children.empty() || logical_filter->children[0]->type != LogicalOperatorType::LOGICAL_GET) {
			return op;
		}
		logical_get = &logical_filter->children[0]->Cast<LogicalGet>();
	} else if (proj_child->type == LogicalOperatorType::LOGICAL_GET) {
		// 바로 GET (WHERE 없는 경우)
		logical_get = &proj_child->Cast<LogicalGet>();
	} else {
		// 예상 외 구조
		return op;
	}

	auto &get = logical_get->Cast<LogicalGet>();
	optional_ptr<TableCatalogEntry> table_catalog_entry = get.GetTable();
	if (table_catalog_entry->type != CatalogType::TABLE_ENTRY) {
		return op;
	}
	auto projection_columns = get.names;
	auto where_condition = logical_get ? vector<unique_ptr<Expression>>() : std::move(logical_filter->expressions);
	MViewMetadata mview_metadata = {table_catalog_entry->schema.name, table_catalog_entry->catalog.GetName(),
	                                table_catalog_entry->name, std::move(projection_columns),
	                                std::move(where_condition)};
	auto matched_mview = getMatchingMView(context, mview_metadata);

	if (matched_mview) {
		auto &matview_entry = matched_mview->Cast<MatViewCatalogEntry>();
		auto table_index = binder.GenerateTableIndex();
		vector<string> mview_column_names = matview_entry.GetColumns().GetColumnNames();
		vector<LogicalType> mview_column_types = matview_entry.GetColumns().GetColumnTypes();

		// make a new LogicalGet operator for mview
		auto table_function = TableScanFunction::GetFunction();
		auto bind_data = make_uniq<TableScanBindData>(matched_mview->Cast<MatViewCatalogEntry>());
		auto new_get =
		    make_uniq<LogicalGet>(table_index,
		                          table_function, std::move(bind_data), mview_column_types,
		                          mview_column_names);

		// make a new LogicalFilter operator for mview
		unique_ptr<LogicalOperator> base_op = std::move(new_get);
		if (logical_filter) {
			if (!where_condition.empty()) {
				if (where_condition.size() != 1) {
					// 여러 개면 Conjunction으로 묶어야 함
					throw NotImplementedException("AND-multi-filter reconstruction not yet supported.");
				}
				auto filter_expr = where_condition[0]->Copy();
				auto filter_op = make_uniq<LogicalFilter>(std::move(filter_expr));
				filter_op->AddChild(std::move(base_op));
				base_op = std::move(filter_op);
			}
		}
		// make a new LogicalProjection operator for mview
		vector<unique_ptr<Expression>> projection_exprs;
		for (auto type: mview_column_types) {
			projection_exprs.emplace_back(make_uniq<BoundConstantExpression>(Value(type)));
		}

		auto projection_op = make_uniq<LogicalProjection>(binder.GenerateTableIndex(), std::move(projection_exprs));
		projection_op->AddChild(std::move(base_op));
		return projection_op;
	}
	// 	projection.table_index = mview_logical_get.table_index;
	// 	if (logical_filter) {
	// 		// projection -> filter -> new_get
	// 		logical_filter->children.clear();
	// 		logical_filter->children.push_back(std::move(new_get));
	// 		op->children.clear();
	// 		op->children.push_back(logical_filter->Copy(context));
	// 		return op;
	// 	}
	//
	// 	// projection -> new_get
	// 	op->children.clear();
	// 	op->children.push_back(std::move(new_get));
	// 	return op;
	// }
	return op;
}

CatalogEntry* MatViewOptimizer::getMatchingMView(ClientContext &context, const MViewMetadata &mview) {
	auto &schema = Catalog::GetSchema(context, mview.catalog_name, mview.schema_name);
	vector<CatalogEntry*> matview_entries;
	schema.Scan(context, CatalogType::MATVIEW_ENTRY, [&](CatalogEntry &entry) {
		if (entry.type == CatalogType::MATVIEW_ENTRY) {
			MatViewCatalogEntry &mview_entry = entry.Cast<MatViewCatalogEntry>();
			if (mview.base_table_name == entry.Cast<MatViewCatalogEntry>().base_table_name) {
				matview_entries.push_back(&entry);
			}
		}
	});
	if (matview_entries.size() == 0) {
		return nullptr;
	}
	// Base table for MView matches the base table in the query
	for (auto entry : matview_entries) {
		auto &mview_entry = entry->Cast<MatViewCatalogEntry>();
		auto mview_info = ExtractMViewInfoFromLogicalPlan(*mview_entry.query);
		if (mview.column_names.size() != mview_info.column_names.size()) {
			continue;
		}
		bool column_match = true;
		// compare with the column name

		for (size_t i = 0; i < mview.column_names.size(); i++) {
			if (mview.column_names[i] != mview_info.column_names[i]) {
				column_match = false;
				break;
			}
		}
		if (!column_match) {
			continue;
		}

		// 2. Where expression comparison
		if (mview.where_expressions.size() != mview_info.where_expressions.size()) {
			continue;
		}
		bool where_match = true;
		for (size_t i = 0; i < mview.where_expressions.size(); i++) {
			if (!mview.where_expressions[i].get()->Equals(*mview_info.where_expressions[i].get())) {
				where_match = false;
				break;
			}
		}
		if (!where_match) {
			continue;
		}

		// ✅ Match found
		return entry;
	}
	return nullptr;
}

MViewQueryInfo MatViewOptimizer::ExtractMViewInfoFromLogicalPlan(LogicalOperator &op) {
	MViewQueryInfo info;

	LogicalOperator *current = &op;

	if (current->type == LogicalOperatorType::LOGICAL_FILTER) {
		auto &filter = current->Cast<LogicalFilter>();
		info.where_expressions = std::move(filter.expressions);
		current = filter.children[0].get();
	}

	if (current->type == LogicalOperatorType::LOGICAL_PROJECTION) {
		auto &proj = current->Cast<LogicalProjection>();
		for (auto &expr : proj.expressions) {
			if (expr->type == ExpressionType::BOUND_REF) {
				auto &bound_ref = expr->Cast<BoundReferenceExpression>();
				info.column_names.push_back(bound_ref.alias.empty() ? bound_ref.ToString() : bound_ref.alias);
			} else {
				info.column_names.push_back(expr->ToString()); // fallback
			}
		}
	} else if (current->type == LogicalOperatorType::LOGICAL_GET) {
		auto &get = current->Cast<LogicalGet>();
		for (auto &name : get.names) {
			info.column_names.push_back(name);
		}
	}

	return info;
}
};
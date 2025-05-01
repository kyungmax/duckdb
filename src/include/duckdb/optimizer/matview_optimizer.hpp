//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/optimizer/matview_optimizer.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/planner/logical_operator_visitor.hpp"
#include "duckdb/planner/logical_operator.hpp"
#include "duckdb/common/optional_ptr.hpp"

#include <duckdb/catalog/catalog_entry/list.hpp>

namespace duckdb {
struct MViewMetadata;
struct MViewQueryInfo;
class ClientContext;
class Optimizer;

class MatViewOptimizer{
public:
	explicit MatViewOptimizer(ClientContext &context, Optimizer &optimizer) : context(context), optimizer(optimizer) {
	}

	ClientContext &context;
	Optimizer &optimizer;
	optional_ptr<LogicalOperator> current_op;
	unique_ptr<LogicalOperator> root;

public:

	unique_ptr<LogicalOperator> Optimize(unique_ptr<LogicalOperator> op, Binder &binder);

private:
	CatalogEntry* getMatchingMView(ClientContext &context, const MViewMetadata &mview);
	MViewQueryInfo ExtractMViewInfoFromLogicalPlan(LogicalOperator &op);
};

} // namespace duckdb

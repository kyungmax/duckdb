#pragma once

#include "duckdb/execution/physical_operator.hpp"
#include "duckdb/parser/parsed_data/create_matview_info.hpp"

namespace duckdb {

//! Physically CREATE Materialized view AS statement
class PhysicalCreateMatView : public PhysicalOperator {
public:
	static constexpr const PhysicalOperatorType TYPE = PhysicalOperatorType::CREATE_MATVIEW;

public:
	explicit PhysicalCreateMatView(unique_ptr<CreateMatViewInfo> info, idx_t estimated_cardinality)
	    : PhysicalOperator(PhysicalOperatorType::CREATE_MATVIEW, {LogicalType::BIGINT}, estimated_cardinality),
	      info(std::move(info)) {
	}

	unique_ptr<CreateMatViewInfo> info;

public:
	// Source interface
	SourceResultType GetData(ExecutionContext &context, DataChunk &chunk, OperatorSourceInput &input) const override;

	bool IsSource() const override {
		return true;
	}
};
} // namespace duckdb

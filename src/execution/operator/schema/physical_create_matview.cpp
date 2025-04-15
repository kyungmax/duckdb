#include "duckdb/execution/operator/schema/physical_create_matview.hpp"

namespace duckdb {

SourceResultType PhysicalCreateMatView::GetData(ExecutionContext &context, DataChunk &chunk,
                                                OperatorSourceInput &input) const {
	auto &catalog = Catalog::GetCatalog(context.client, info->catalog);
	catalog.CreateMatView(context.client, *info);

	return SourceResultType::FINISHED;
}

} // namespace duckdb

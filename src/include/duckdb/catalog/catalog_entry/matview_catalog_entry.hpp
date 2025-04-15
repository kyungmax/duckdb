#pragma once

namespace duckdb {

class DataTable;
struct CreateMatViewInfo;

//! A Materialized View catalog entry
class MatViewCatalogEntry : public StandardEntry {
public:
	static constexpr const CatalogType Type = CatalogType::MATVIEW_ENTRY;
	static constexpr const char *Name = "matview";
	//! Create a real TableCatalogEntry and initialize storage for it
	MatViewCatalogEntry(Catalog &catalog, SchemaCatalogEntry &schema, CreateMatViewInfo &info);

	//! The query of the view
	unique_ptr<SelectStatement> query;
	//! The SQL query (if any)
	string sql;
	//! The set of aliases associated with the view
	vector<string> aliases;
	//! The returned types of the view
	vector<LogicalType> types;
	//! The returned names of the view
	vector<string> names;
	//! The comments on the columns of the view: can be empty if there are no comments
	vector<Value> column_comments;
public:
	unique_ptr<CreateInfo> GetInfo() const override;

	unique_ptr<CatalogEntry> Copy(ClientContext &context) const override;

	virtual const SelectStatement &GetQuery();

	virtual bool HasTypes() const {
		return true;
	}

	string ToSQL() const override;
private:
	void Initialize(CreateMatViewInfo &info);
};
} // namespace duckdb

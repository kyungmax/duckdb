//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/enums/wal_type.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/constants.hpp"

namespace duckdb {

enum class WALType : uint8_t {
	INVALID = 0,
	// -----------------------------
	// Catalog
	// -----------------------------
	CREATE_TABLE = 1,
	DROP_TABLE = 2,

	CREATE_SCHEMA = 3,
	DROP_SCHEMA = 4,

	CREATE_VIEW = 5,
	DROP_VIEW = 6,

	CREATE_MATVIEW = 7,
	DROP_MATVIEW = 8,

	CREATE_SEQUENCE = 9,
	DROP_SEQUENCE = 10,
	SEQUENCE_VALUE = 11,

	CREATE_MACRO = 12,
	DROP_MACRO = 13,

	CREATE_TYPE = 14,
	DROP_TYPE = 15,

	ALTER_INFO = 20,

	CREATE_TABLE_MACRO = 21,
	DROP_TABLE_MACRO = 22,

	CREATE_INDEX = 23,
	DROP_INDEX = 24,

	// -----------------------------
	// Data
	// -----------------------------
	USE_TABLE = 25,
	INSERT_TUPLE = 26,
	DELETE_TUPLE = 27,
	UPDATE_TUPLE = 28,
	ROW_GROUP_DATA = 29,
	// -----------------------------
	// Flush
	// -----------------------------
	WAL_VERSION = 98,
	CHECKPOINT = 99,
	WAL_FLUSH = 100
};
}

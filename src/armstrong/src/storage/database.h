#pragma once

#include <sqlite3.h>

// the default sqlite on osx 10.4 doesnt have _prepare_v2
#if SQLITE_VERSION_NUMBER <= 3003009
#define sqlite3_prepare_v2 sqlite3_prepare
#endif

namespace sqlite {

// simple wrappers

struct statement {
	sqlite3_stmt* stmt;
	int prepareresult;
	int stepresult;

	statement();
	statement(sqlite3* db, std::string const& query);
	~statement();
	bool prepare(sqlite3* db, std::string const& query);
	bool execute();
	bool next();
	bool eof() const;
	void reset();
};

struct transaction {
	sqlite3* db;
	transaction(sqlite3* db);
	~transaction();
};

}

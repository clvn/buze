#include <iostream>
#include <string>
#include <cassert>
#include <sqlite3.h>
#include "database.h"

namespace sqlite {

// ---------------------------------------------------------------------------------------------------------------
// STATEMENT
// ---------------------------------------------------------------------------------------------------------------

statement::statement()
:
	stmt(0),
	prepareresult(0),
	stepresult(SQLITE_DONE)
{}

statement::statement(sqlite3* db, std::string const& query)
:
	stmt(0),
	stepresult(SQLITE_DONE)
{
	prepareresult = sqlite3_prepare_v2(db, query.c_str(), (int)query.length(), &stmt, 0);

	if (prepareresult != 0) {
		std::cerr << "Error in query:" << std::endl;
		std::cerr << query << std::endl;
		std::cerr << sqlite3_errmsg(db) << std::endl;
		assert(false);
	}
}

statement::~statement() {
	if (stmt) sqlite3_finalize(stmt);
}

bool statement::prepare(sqlite3* db, std::string const& query) {
	if (stmt) sqlite3_finalize(stmt);

	prepareresult = sqlite3_prepare_v2(db, query.c_str(), (int)query.length(), &stmt, 0);

	if (prepareresult != 0) {
		std::cerr << "Error in query:" << std::endl;
		std::cerr << query << std::endl;
		std::cerr << sqlite3_errmsg(db) << std::endl;
		assert(false);
		return false;
	}

	return true;
}

bool statement::execute() {
	assert(stmt != 0);
	if (prepareresult != 0) return false;
	return next();
}

bool statement::next() {
	assert(stmt != 0);
	stepresult = sqlite3_step(stmt);
	return stepresult == SQLITE_ROW || stepresult == SQLITE_DONE;
}

bool statement::eof() const {
	return stepresult == SQLITE_DONE;
}

// ---------------------------------------------------------------------------------------------------------------
// TRANSACTION
// ---------------------------------------------------------------------------------------------------------------

transaction::transaction(sqlite3* db) : db(db) {
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, "begin;", 0, 0, &zErrMsg);
	if (rc) assert(false);
}

transaction::~transaction() {
	char* zErrMsg = 0;
	int rc = sqlite3_exec(db, "commit;", 0, 0, &zErrMsg);
	if (rc) assert(false);
}

}

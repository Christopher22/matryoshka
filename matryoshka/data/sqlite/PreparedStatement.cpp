//
// Created by christopher on 13.03.2020.
//

#include "PreparedStatement.h"

#include <sqlite3.h>

namespace matryoshka::data::sqlite {

Result<PreparedStatement> PreparedStatement::Create(Database &database,
													std::string_view command) noexcept {
  sqlite3_stmt *prepared_statement;
  const Status result = Status(sqlite3_prepare_v3(database.Raw(),
												  command.data(),
												  command.size(),
												  SQLITE_PREPARE_PERSISTENT,
												  &prepared_statement,
												  nullptr));
  if (result) {
	return Result<PreparedStatement>(PreparedStatement(prepared_statement));
  } else {
	sqlite3_finalize(prepared_statement);
	return Result<PreparedStatement>(result);
  }
}

PreparedStatement::PreparedStatement(PreparedStatement &&other) noexcept: prepared_statement_(other
																								  .prepared_statement_) {
  other.prepared_statement_ = nullptr;
}

PreparedStatement::~PreparedStatement() noexcept {
  sqlite3_finalize(prepared_statement_);
}
}
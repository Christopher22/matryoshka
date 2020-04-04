//
// Created by christopher on 11.03.2020.
//

#include "Database.h"
#include "PreparedStatement.h"

#include <sqlite3.h>

#include <cassert>

namespace matryoshka::data::sqlite {

Database::Database(sqlite3 *database) noexcept: database_(database) {
  assert(database != nullptr);
  sqlite3_extended_result_codes(database_, true);
}

Result<Database> Database::Create(std::string_view path) noexcept {
  sqlite3 *database;
  Status status(sqlite3_open_v2(path.data(), &database, SQLITE_OPEN_READWRITE, nullptr));
  if (status) {
	return Result<Database>(Database(database));
  } else {
	sqlite3_close_v2(database);
	return Result<Database>(status);
  }
}

Database::Database(Database &&other) noexcept: database_(other.database_) {
  other.database_ = nullptr;
}

Database::~Database() noexcept {
  // nullptr is no-op.
  sqlite3_close_v2(database_);
}

std::string_view Database::ErrorCode() noexcept {
  return std::string_view(sqlite3_errmsg(database_));
}

Status Database::operator()(std::string_view sql) noexcept {
  if (database_ == nullptr) {
	return Status(SQLITE_ERROR);
  }

  auto prepared_statement = PreparedStatement::Create(*this, sql);
  if (PreparedStatement *statement = std::get_if<PreparedStatement>(&prepared_statement)) {
	return (*statement)([&](Query &query) {
	  return query();
	});
  } else {
	return std::get<Status>(prepared_statement);
  }
}

int Database::MaximalDataSize() const noexcept {
  return sqlite3_limit(database_, SQLITE_LIMIT_LENGTH, -1);
}

bool Database::SetMaximalDataSize(int new_size) noexcept {
  if (new_size <= 0) {
	return false;
  }
  sqlite3_limit(database_, SQLITE_LIMIT_LENGTH, new_size);
  return this->MaximalDataSize() == new_size;
}

}
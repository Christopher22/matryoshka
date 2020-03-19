//
// Created by christopher on 11.03.2020.
//

#include "Database.h"
#include "PreparedStatement.h"

#include <sqlite3.h>

namespace matryoshka::data::sqlite {

Database::Database(std::string_view path,
				   std::string meta_table,
				   std::string data_table,
				   unsigned int version) noexcept
	: database_(nullptr), meta_table_(std::move(meta_table)), data_table_(std::move(data_table)), version_(version) {

  if (sqlite3_open_v2(path.data(), &database_, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK) {
	sqlite3_close_v2(database_);
	database_ = nullptr;
  } else {
	sqlite3_extended_result_codes(database_, true);
  }
}

Database::Database(Database &&other) noexcept: database_(other.database_),
											   data_table_(std::move(other.data_table_)),
											   meta_table_(std::move(other.meta_table_)),
											   version_(other.version_) {
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

  auto prepared_statement = PreparedStatement::create(*this, sql);
  if (PreparedStatement *statement = std::get_if<PreparedStatement>(&prepared_statement)) {
	return (*statement)([&](Query &query) {
	  return query();
	});
  } else {
	return std::get<Status>(prepared_statement);
  }
}

}
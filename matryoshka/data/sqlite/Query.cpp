//
// Created by christopher on 13.03.2020.
//

#include "Query.h"

#include <sqlite3.h>

#include <cassert>

namespace matryoshka::data::sqlite {

Query::Query(sqlite3_stmt *prepared_statement) noexcept: prepared_statement_(prepared_statement) {
  assert(prepared_statement_ != nullptr);
}

Query::~Query() noexcept {
  this->reset();
}

Status Query::reset() noexcept {
  return Status(sqlite3_reset(prepared_statement_));
}

Status Query::unset() noexcept {
  return Status(sqlite3_clear_bindings(prepared_statement_));
}

Status Query::unset(int index) noexcept {
  return Status(sqlite3_bind_null(prepared_statement_, index + 1));
}

Status Query::set(int index, int value) noexcept {
  return Status(sqlite3_bind_int(prepared_statement_, index + 1, value));
}

Status Query::set(int index, std::int_fast64_t value) noexcept {
  return Status(sqlite3_bind_int64(prepared_statement_, index + 1, value));
}

Status Query::set(int index, std::string_view value) noexcept {
  return Status(sqlite3_bind_text(prepared_statement_, index + 1, value.data(), value.size(), SQLITE_TRANSIENT));
}

Status Query::set(int index, double value) noexcept {
  return Status(sqlite3_bind_double(prepared_statement_, index + 1, value));
}

Status Query::set(int index, Blob<true> &&value) {
  // Unique pointer is used for indicating the shifted ownership
  return Status(sqlite3_bind_blob(prepared_statement_, index + 1, value.release(), value.size(), &Query::_deleteBlob));
}

Status Query::set(int index, const Blob<false> &value) {
  // Unique pointer is used for indicating the shifted ownership
  return Status(sqlite3_bind_blob(prepared_statement_,
								  index + 1,
								  static_cast<const unsigned char *>(value),
								  value.size(),
								  SQLITE_TRANSIENT));
}

int Query::numParameters() const noexcept {
  return sqlite3_bind_parameter_count(prepared_statement_);
}

void Query::_deleteBlob(void *data) {
  delete[] static_cast<unsigned char *>(data);
}

int Query::_getIndex(std::string_view name) {
  return sqlite3_bind_parameter_index(prepared_statement_, name.data());
}

double Query::getDouble(int index) const {
  return sqlite3_column_double(prepared_statement_, index);
}

int Query::getInteger(int index) const {
  return sqlite3_column_int(prepared_statement_, index);
}

std::string_view Query::getText(int index) const {
  return std::string_view(reinterpret_cast<const char *>(sqlite3_column_text(prepared_statement_, index)),
						  sqlite3_column_bytes(
							  prepared_statement_,
							  index));
}

Blob<false> Query::getData(int index) const {
  return Blob<false>(static_cast<const unsigned char *>(sqlite3_column_blob(
	  prepared_statement_,
	  index)), sqlite3_column_bytes(prepared_statement_, index));
}

Status Query::operator()() noexcept {
  const Status status = Status(sqlite3_step(prepared_statement_));
  if (static_cast<int>(status) == SQLITE_DONE) {
	this->reset();
  }
  return status;
}

}
/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Query.h"

#include <sqlite3.h>

#include <cassert>

namespace matryoshka::data::sqlite {

Query::Query(sqlite3_stmt *prepared_statement) noexcept: prepared_statement_(prepared_statement) {
  assert(prepared_statement_ != nullptr);
}

Query::~Query() noexcept {
  this->Reset();
}

Status Query::Reset() noexcept {
  return Status(sqlite3_reset(prepared_statement_));
}

Status Query::Unset() noexcept {
  return Status(sqlite3_clear_bindings(prepared_statement_));
}

Status Query::Unset(int index) noexcept {
  return Status(sqlite3_bind_null(prepared_statement_, index + 1));
}

Status Query::Set(int index, int value) noexcept {
  return Status(sqlite3_bind_int(prepared_statement_, index + 1, value));
}

Status Query::Set(int index, std::int_fast64_t value) noexcept {
  return Status(sqlite3_bind_int64(prepared_statement_, index + 1, value));
}

Status Query::Set(int index, std::string_view value) noexcept {
  return Status(sqlite3_bind_text(prepared_statement_, index + 1, value.data(), value.size(), SQLITE_TRANSIENT));
}

Status Query::Set(int index, double value) noexcept {
  return Status(sqlite3_bind_double(prepared_statement_, index + 1, value));
}

Status Query::Set(int index, Blob<true> &&value) {
  // Unique pointer is used for indicating the shifted ownership
  return Status(sqlite3_bind_blob(prepared_statement_, index + 1, value.Release(), value.Size(), &Query::_deleteBlob));
}

Status Query::Set(int index, const Blob<false> &value) {
  // Unique pointer is used for indicating the shifted ownership
  return Status(sqlite3_bind_blob(prepared_statement_,
								  index + 1,
								  static_cast<const unsigned char *>(value),
								  value.Size(),
								  SQLITE_TRANSIENT));
}

int Query::NumParameter() const noexcept {
  return sqlite3_bind_parameter_count(prepared_statement_);
}

void Query::_deleteBlob(void *data) {
  delete[] static_cast<unsigned char *>(data);
}

int Query::_getIndex(std::string_view name) {
  return sqlite3_bind_parameter_index(prepared_statement_, name.data());
}

double Query::GetDouble(int index) const {
  return sqlite3_column_double(prepared_statement_, index);
}

int Query::GetInteger(int index) const {
  return sqlite3_column_int(prepared_statement_, index);
}

std::string_view Query::GetText(int index) const {
  return std::string_view(reinterpret_cast<const char *>(sqlite3_column_text(prepared_statement_, index)),
						  sqlite3_column_bytes(
							  prepared_statement_,
							  index));
}

Blob<false> Query::GetData(int index) const {
  return Blob<false>(static_cast<const unsigned char *>(sqlite3_column_blob(
	  prepared_statement_,
	  index)), sqlite3_column_bytes(prepared_statement_, index));
}

Status Query::operator()() noexcept {
  const Status status = Status(sqlite3_step(prepared_statement_));
  if (static_cast<int>(status) == SQLITE_DONE) {
	this->Reset();
  }
  return status;
}

Query::ValueType Query::Type(int index) const noexcept {
  switch (sqlite3_column_type(prepared_statement_, index)) {
	case SQLITE_INTEGER: return ValueType::Integer;
	case SQLITE_FLOAT: return ValueType::Float;
	case SQLITE_TEXT: return ValueType::Text;
	case SQLITE_BLOB: return ValueType::Blob;
	default: return ValueType::Null;
  }
}

}
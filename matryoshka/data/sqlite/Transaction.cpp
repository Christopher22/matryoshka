/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Transaction.h"

#include "Database.h"

#include <sqlite3.h>
#include <cassert>

namespace matryoshka::data::sqlite {

Result<Transaction> Transaction::Open(Database *database) noexcept {
  assert(database != nullptr);
  const Status status = (*database)("BEGIN;");
  if (status.IsSuccessful()) {
	return Result<Transaction>(Transaction(database));
  } else {
	return Result<Transaction>(status);
  }
}

Transaction::Transaction(Transaction &&other) noexcept: database_(other.database_) {
  other.database_ = nullptr;
}

Transaction::~Transaction() noexcept {
  this->Rollback();
}

Status Transaction::_callDatabase(std::string_view command) noexcept {
  if (database_ == nullptr) {
	return Status();
  }

  const Status status = (*database_)(command);
  database_ = nullptr;
  return status;
}

}
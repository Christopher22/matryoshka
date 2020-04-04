//
// Created by christopher on 04.04.2020.
//

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
//
// Created by christopher on 04.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_TRANSACTION_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_TRANSACTION_H_

#include "Result.h"

namespace matryoshka::data::sqlite {
class Database;

class Transaction {
 public:
  static Result<Transaction> Open(Database *database) noexcept;
  inline Status Commit() noexcept {
	return this->_callDatabase("COMMIT;");
  }
  Status Rollback() noexcept {
	return this->_callDatabase("ROLLBACK;");
  }

  ~Transaction() noexcept;
  Transaction(Transaction &&other) noexcept;
  Transaction(Transaction const &) = delete;
  Transaction &operator=(Transaction const &) = delete;

 protected:
  constexpr explicit Transaction(Database *database) noexcept: database_(database) {}

 private:
  Status _callDatabase(std::string_view command) noexcept;

  Database *database_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_TRANSACTION_H_

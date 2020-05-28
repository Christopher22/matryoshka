/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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

/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_

#include "Result.h"

#include <string_view>

class sqlite3;

namespace matryoshka::data::sqlite {
class Database {
 public:
  using RowId = std::int_fast64_t;

  static Result<Database> Create(std::string_view path = ":memory:") noexcept;
  Database(Database &&other) noexcept;
  ~Database() noexcept;
  Database(Database const &) = delete;
  Database &operator=(Database const &) = delete;

  [[nodiscard]] RowId LastInsertedRow() const noexcept;

  [[nodiscard]] int MaximalDataSize() const noexcept;
  bool SetMaximalDataSize(int new_size) noexcept;

  Status operator()(std::string_view sql) noexcept;
  [[nodiscard]] std::string_view ErrorCode() noexcept;

  [[nodiscard]] inline sqlite3 *Raw() const noexcept {
	return database_;
  }

  inline explicit operator sqlite3 *() noexcept {
	return database_;
  }

 protected:
  explicit Database(sqlite3 *database) noexcept;

 private:
  sqlite3 *database_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_

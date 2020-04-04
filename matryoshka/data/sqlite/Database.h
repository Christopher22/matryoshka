//
// Created by christopher on 11.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_

#include "Status.h"

#include <string_view>
#include <variant>

class sqlite3;

namespace matryoshka::data::sqlite {
class Database {
 public:
  static std::variant<Database, Status> create(std::string_view path) noexcept;
  Database(Database &&other) noexcept;
  ~Database() noexcept;
  Database(Database const &) = delete;
  Database &operator=(Database const &) = delete;

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

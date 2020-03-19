//
// Created by christopher on 11.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_

#include "Status.h"

#include <string_view>

class sqlite3;

namespace matryoshka::data::sqlite {
class Database {
 public:
  explicit Database(std::string_view path,
					std::string meta_table = "matryoshka",
					std::string data_table = "matryoshka_meta",
					unsigned int version = 0) noexcept;
  Database(Database &&other) noexcept;
  ~Database() noexcept;
  Database(Database const &) = delete;
  Database &operator=(Database const &) = delete;

  Status operator()(std::string_view sql) noexcept;
  [[nodiscard]] std::string_view ErrorCode() noexcept;
  [[nodiscard]] inline std::string_view MetaTable() const noexcept {
	return meta_table_;
  }
  [[nodiscard]] inline std::string_view DataTable() const noexcept {
	return data_table_;
  }

  inline explicit operator bool() noexcept {
	return database_ != nullptr;
  }

  inline explicit operator sqlite3 *() noexcept {
	return database_;
  }

 private:
  sqlite3 *database_;
  std::string meta_table_, data_table_;
  const unsigned int version_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_DATABASE_H_

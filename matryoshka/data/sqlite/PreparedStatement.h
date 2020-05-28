/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_PREPAREDSTATEMENT_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_PREPAREDSTATEMENT_H_

#include "Database.h"
#include "Query.h"
#include "Result.h"

#include <string_view>
#include <variant>
#include <functional>
#include <optional>
#include <utility>
#include <initializer_list>

class sqlite3_stmt;

namespace matryoshka::data::sqlite {
class PreparedStatement {
 public:
  static Result<PreparedStatement> Create(Database &database, std::string_view command) noexcept;
  static Result<PreparedStatement> Insert(Database &database,
										  std::string_view table,
										  std::initializer_list<std::string_view> columns) noexcept;

  PreparedStatement(PreparedStatement &&other) noexcept;
  ~PreparedStatement() noexcept;
  PreparedStatement(PreparedStatement const &) = delete;
  PreparedStatement &operator=(PreparedStatement const &) = delete;

  inline explicit operator bool() const noexcept {
	return prepared_statement_ != nullptr;
  }

  template<typename C>
  inline Status operator()(C callback) const {
	static_assert(std::is_constructible<std::function<Status(Query &)>, C>::value);

	if (prepared_statement_ == nullptr) {
	  return Status(1);
	}
	// ToDo: Insert mutex here to restrict concurrent access on single prepared statement
	Query query(prepared_statement_);
	return callback(query);
  }

  template<typename T, class... Args>
  [[nodiscard]] std::optional<T> Execute(Args &&... args) {
	std::optional<T> result;
	(*this)([&](sqlite::Query &query) {
	  return query.SetMulti(0, std::forward<Args>(args)...).Than(query).Than([&]() {
		// Check if any data was reported
		if (query.Type(0) != Query::ValueType::Null) {
		  result = query.Get<T>(0);
		}
		return Status();
	  });
	});
	return result;
  }

  template<class... Args>
  [[nodiscard]] inline Status Execute(Args &&... args) {
	return (*this)([&](sqlite::Query &query) {
	  return query.SetMulti(0, std::forward<Args>(args)...).Than(query);
	});
  }

 protected:
  constexpr explicit PreparedStatement(sqlite3_stmt *prepared_statement) noexcept: prepared_statement_(
	  prepared_statement) {}

 private:
  sqlite3_stmt *prepared_statement_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_PREPAREDSTATEMENT_H_

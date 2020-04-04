//
// Created by christopher on 13.03.2020.
//

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

class sqlite3_stmt;

namespace matryoshka::data::sqlite {
class PreparedStatement {
 public:
  static Result<PreparedStatement> Create(Database &database, std::string_view command) noexcept;

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
  std::optional<T> Execute(Args &&... args) {
	std::optional<T> result;
	(*this)([&](sqlite::Query &query) {
	  return query.SetMulti(0, std::forward<Args>(args)...).Than(query).Than([&]() {
		result = query.Get<T>(0);
		return Status();
	  });
	});
	return result;
  }

  template<class... Args>
  inline Status Execute(Args &&... args) {
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

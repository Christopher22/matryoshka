//
// Created by christopher on 13.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_PREPAREDSTATEMENT_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_PREPAREDSTATEMENT_H_

#include "Database.h"
#include "Query.h"

#include <string_view>
#include <variant>
#include <functional>

class sqlite3_stmt;

namespace matryoshka::data::sqlite {
class PreparedStatement {
 public:
  static std::variant<PreparedStatement, Status> create(Database &database, std::string_view command) noexcept;

  PreparedStatement(PreparedStatement &&other) noexcept;
  ~PreparedStatement() noexcept;
  PreparedStatement(PreparedStatement const &) = delete;
  PreparedStatement &operator=(PreparedStatement const &) = delete;

  inline explicit operator bool() const noexcept {
	return prepared_statement_ != nullptr;
  }

  template<typename C>
  inline Status operator()(C &callback) {
	static_assert(std::is_constructible<std::function<Status(Query &)>, C>::value);

	if (prepared_statement_ == nullptr) {
	  return Status(1);
	}
	// ToDo: Insert mutex here to restrict concurrent access on single prepared statement
	Query query(prepared_statement_);
	return callback(query);
  }

 protected:
  constexpr explicit PreparedStatement(sqlite3_stmt *prepared_statement) noexcept: prepared_statement_(
	  prepared_statement) {}

 private:
  sqlite3_stmt *prepared_statement_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_PREPAREDSTATEMENT_H_

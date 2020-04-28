//
// Created by christopher on 13.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_

class sqlite3_stmt;

#include <string_view>
#include <cstdint>
#include <memory>
#include <tuple>

#include "Status.h"
#include "Blob.h"

namespace matryoshka::data::sqlite {
class Query {
 public:
  enum class ValueType {
	Integer,
	Float,
	Text,
	Blob,
	Null
  };

  explicit Query(sqlite3_stmt *prepared_statement) noexcept;
  Query(Query &&other) = delete;
  Query(Query const &) = delete;
  Query &operator=(Query const &) = delete;
  ~Query() noexcept;

  [[nodiscard]] int NumParameter() const noexcept;
  Status Reset() noexcept;
  Status operator()() noexcept;

  Status Unset() noexcept;
  Status Unset(int index) noexcept;
  Status Set(int index, int value) noexcept;
  Status Set(int index, std::int_fast64_t value) noexcept;
  Status Set(int index, std::string_view value) noexcept;
  Status Set(int index, double value) noexcept;
  Status Set(int index, Blob<true> &&value);
  Status Set(int index, const Blob<false> &value);

  template<typename T>
  inline Status SetByName(std::string_view name, T value) noexcept {
	const int index = this->_getIndex(name);
	if (index == 0) {
	  return Status(1);
	}
	return this->Set(index - 1, value);
  }

  template<typename Arg>
  inline Status SetMulti(int index, Arg &&current) {
	return this->Set(index, current);
  }

  template<typename Arg, typename... Args>
  inline Status SetMulti(int index, Arg &&current, Args &&... rest) {
	return this->Set(index, current).Than([&]() {
	  return this->SetMulti(index + 1, std::forward<Args>(rest)...);
	});
  }

  template<typename T>
  [[nodiscard]] inline T Get(int index) const {
	static_assert(false, "Reading of this type is not supported.");
  }

  template<>
  [[nodiscard]] inline int Get(int index) const {
	return this->GetInteger(index);
  }

  template<>
  [[nodiscard]] inline double Get(int index) const {
	return this->GetDouble(index);
  }

  template<>
  [[nodiscard]] inline std::string_view Get(int index) const {
	return this->GetText(index);
  }

  template<>
  [[nodiscard]] inline std::string Get(int index) const {
	return std::string(this->GetText(index));
  }

  template<>
  [[nodiscard]] inline Blob<false> Get(int index) const {
	return this->GetData(index);
  }

  template<>
  [[nodiscard]] inline Blob<true> Get(int index) const {
	return Blob<true>(this->GetData(index));
  }

  [[nodiscard]] ValueType Type(int index) const noexcept;

 protected:
  [[nodiscard]] double GetDouble(int index) const;
  [[nodiscard]] int GetInteger(int index) const;
  [[nodiscard]] std::string_view GetText(int index) const;
  [[nodiscard]] Blob<false> GetData(int index) const;

 private:
  static void _deleteBlob(void *data);
  int _getIndex(std::string_view name);

  sqlite3_stmt *prepared_statement_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_

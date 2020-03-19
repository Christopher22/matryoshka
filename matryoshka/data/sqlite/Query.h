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
  explicit Query(sqlite3_stmt *prepared_statement) noexcept;
  Query(Query &&other) = delete;
  Query(Query const &) = delete;
  Query &operator=(Query const &) = delete;
  ~Query() noexcept;

  [[nodiscard]] int numParameters() const noexcept;
  Status reset() noexcept;
  Status operator()() noexcept;

  Status unset() noexcept;
  Status unset(int index) noexcept;
  Status set(int index, int value) noexcept;
  Status set(int index, std::int_fast64_t value) noexcept;
  Status set(int index, std::string_view value) noexcept;
  Status set(int index, double value) noexcept;
  Status set(int index, Blob<true> &&value);
  Status set(int index, const Blob<false> &value);

  template<typename T>
  inline Status set(std::string_view name, T value) noexcept {
	const int index = this->_getIndex(name);
	if (index == 0) {
	  return Status(1);
	}
	return this->set(index - 1, value);
  }

  template<typename T>
  [[nodiscard]] inline T get(int index) const {
	static_assert(false, "Reading of this type is not supported.");
  }

  template<>
  [[nodiscard]] inline int get(int index) const {
	return this->getInteger(index);
  }

  template<>
  [[nodiscard]] inline double get(int index) const {
	return this->getDouble(index);
  }

  template<>
  [[nodiscard]] inline std::string_view get(int index) const {
	return this->getText(index);
  }

  template<>
  [[nodiscard]] inline std::string get(int index) const {
	return std::string(this->getText(index));
  }

  template<>
  [[nodiscard]] inline Blob<false> get(int index) const {
	return this->getData(index);
  }

  template<>
  [[nodiscard]] inline Blob<true> get(int index) const {
	return Blob<true>(this->getData(index));
  }

 protected:
  [[nodiscard]] double getDouble(int index) const;
  [[nodiscard]] int getInteger(int index) const;
  [[nodiscard]] std::string_view getText(int index) const;
  [[nodiscard]] Blob<false> getData(int index) const;

 private:
  static void _deleteBlob(void *data);
  int _getIndex(std::string_view name);

  sqlite3_stmt *prepared_statement_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_

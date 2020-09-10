/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_

class sqlite3_stmt;

#include <string_view>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>

#include "Status.h"
#include "Blob.h"

namespace matryoshka::data::sqlite {

namespace values {
template<typename T>
struct Value : std::false_type {};
}

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
	static_assert(values::Value<T>::value, "The requested type is not supported.");
	return values::Value<T>::Read(this, index);
  }

  [[nodiscard]] ValueType Type(int index) const noexcept;

 protected:
  [[nodiscard]] double GetDouble(int index) const;
  [[nodiscard]] int GetInteger(int index) const;
  [[nodiscard]] std::string_view GetText(int index) const;
  [[nodiscard]] Blob<false> GetData(int index) const;

  friend class values::Value<int>;
  friend class values::Value<double>;
  friend class values::Value<std::string_view>;
  friend class values::Value<Blob<false>>;
  friend class values::Value<std::string>;
  friend class values::Value<Blob<true>>;

 private:
  static void _deleteBlob(void *data);
  int _getIndex(std::string_view name);

  sqlite3_stmt *prepared_statement_;
};

/**
 * This namespace contains all the values which might be read directly from the database.
 */
namespace values {
template<>
struct Value<int> : std::true_type {
  [[nodiscard]] static inline int Read(const Query *query, int index) {
	return query->GetInteger(index);
  }
};

template<>
struct Value<double> : std::true_type {
  [[nodiscard]] static inline double Read(const Query *query, int index) {
	return query->GetDouble(index);
  }
};

template<>
struct Value<std::string_view> : std::true_type {
  [[nodiscard]] static inline std::string_view Read(const Query *query, int index) {
	return query->GetText(index);
  }
};

template<>
struct Value<Blob<false>> : std::true_type {
  [[nodiscard]] static inline Blob<false> Read(const Query *query, int index) {
	return query->GetData(index);
  }
};

template<>
struct Value<std::string> : std::true_type {
  [[nodiscard]] static inline std::string Read(const Query *query, int index) {
	return std::string(query->GetText(index));
  }
};

template<>
struct Value<Blob<true>> : std::true_type {
  [[nodiscard]] static inline Blob<true> Read(const Query *query, int index) {
	return Blob<true>(query->GetData(index));
  }
};
}

}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_QUERY_H_

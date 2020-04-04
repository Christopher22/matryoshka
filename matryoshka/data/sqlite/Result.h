//
// Created by christopher on 04.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_

#include "Status.h"

#include <variant>

namespace matryoshka::data::sqlite {

template<typename T>
class Result : public std::variant<T, Status> {
 public:
  constexpr explicit Result(T &&data) noexcept: std::variant<T, Status>(std::move(data)) {}
  constexpr explicit Result(Status status) noexcept: std::variant<T, Status>(status) {}

  template<typename C>
  inline Status Than(C &callback) {
	T *value = std::get_if<T>(this);
	return value != nullptr ? callback(std::move(*value)) : std::get<Status>(*this);
  }

  explicit inline operator Status() const noexcept {
	const Status *status = std::get_if<Status>(this);
	return status != nullptr ? *status : Status();
  }

  inline explicit operator bool() const noexcept {
	return std::holds_alternative<T>(*this);
  }

  inline T *operator->() noexcept {
	return std::get_if<T>(this);
  }
};
}
#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_

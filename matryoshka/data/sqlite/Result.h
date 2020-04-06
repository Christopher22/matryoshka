//
// Created by christopher on 04.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_

#include "Status.h"

#include <variant>
#ifndef NDEBUG
#include <iostream>
#endif

namespace matryoshka::data::sqlite {

template<typename T = std::monostate, typename S = Status>
class Result : public std::variant<T, S> {
 public:
  constexpr explicit Result(T &&data) noexcept: std::variant<T, S>(std::move(data)) {}
  constexpr explicit Result(S status) noexcept: std::variant<T, S>(status) {}

  template<typename... Args>
  static inline Result<T, S> Ok(Args &&... args) {
	return Result<T, S>(std::move(T(std::forward<Args>(args)...)));
  }

  template<typename... Args>
  static inline Result<T, S> Fail(Args &&... args) {
	const S status_code(std::forward<Args>(args)...);
#ifndef NDEBUG
	std::cerr << "[WARNING] Failure occurred: " << status_code << std::endl;
#endif
	return Result<T, S>(status_code);
  }

  template<typename X>
  static inline X Get(Result<X> &&result) {
	return std::get<X>(std::move(result));
  }

  template<typename Head, typename... Tail>
  static inline S Check(const Head &result, const Tail &...results) noexcept {
	if (result) {
	  if constexpr(sizeof...(results) > 0) {
		return Check(results...);
	  } else {
		return S();
	  }
	} else {
	  return static_cast<S>(result);
	}
  }

  template<typename C>
  inline S Than(C &callback) {
	T *value = std::get_if<T>(this);
	return value != nullptr ? callback(std::move(*value)) : std::get<S>(*this);
  }

  explicit inline operator S() const noexcept {
	const S *status = std::get_if<S>(this);
	return status != nullptr ? *status : S();
  }

  inline explicit operator bool() const noexcept {
	return std::holds_alternative<T>(*this);
  }

  inline T *operator->() noexcept {
	return std::get_if<T>(this);
  }

  inline bool operator==(const S &value) const {
	const S *inner_value = std::get_if<S>(this);
	return inner_value != nullptr && *inner_value == value;
  }

  inline bool operator!=(const S &value) const {
	return !(rhs == *this);
  }

  inline bool operator==(const T &value) const {
	const T *inner_value = std::get_if<T>(this);
	return inner_value != nullptr && *inner_value == value;
  }

  inline bool operator!=(const T &value) const {
	return !(rhs == *this);
  }

  bool operator==(const Result<T, S> &rhs) const {
	const T *value1 = std::get_if<T>(this), value_other = std::get_if<T>(&rhs);
	if (value1 != nullptr && value_other != nullptr && *value1 == *value_other) {
	  return true;
	}
	return value1 == nullptr && value_other == nullptr && *std::get_if<S>(this) == *std::get_if<S>(&rhs);
  }

  inline bool operator!=(const Result<T, S> &rhs) const {
	return !(rhs == *this);
  }
};
}
#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_

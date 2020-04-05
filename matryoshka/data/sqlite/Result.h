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
  constexpr explicit Result(S status) noexcept: std::variant<T, S>(status) {
#ifndef NDEBUG
	std::cerr << "[WARNING] Failure occurred: " << status << std::endl;
#endif
  }

  template<typename... Args>
  static inline Result<T, S> Ok(Args &&... args) {
	return Result<T, S>(std::move(T(std::forward<Args>(args)...)));
  }

  template<typename... Args>
  static inline Result<T, S> Fail(Args &&... args) {
	return Result<T, S>(S(std::forward<Args>(args)...));
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
};
}
#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_RESULT_H_

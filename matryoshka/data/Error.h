//
// Created by christopher on 19.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_ERROR_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_ERROR_H_

#include "sqlite/Status.h"
#include "sqlite/Result.h"

#include <variant>
#include <iostream>

namespace matryoshka::data {
namespace errors {
using Backend = sqlite::Status;
enum class Io {
  InvalidDatabaseVersion,
  FileNotFound,
  FileExists,
  OutOfBounds,
  NotImplemented
};
}

class Error : public std::variant<errors::Backend, errors::Io> {
 public:
  class ErrorPrinter {
   public:
	constexpr ErrorPrinter() = default;
	std::string_view operator()(errors::Backend &sqlite_error) const noexcept;
	std::string_view operator()(errors::Io &io_error) const noexcept;
  };

  constexpr explicit Error(errors::Backend backend_error) noexcept: std::variant<errors::Backend, errors::Io>(
	  backend_error) {}
  constexpr explicit Error(errors::Io io_error) noexcept: std::variant<errors::Backend, errors::Io>(io_error) {}

  template<typename T>
  constexpr explicit Error(sqlite::Result<T> &result) noexcept: std::variant<errors::Backend,
																			 errors::Io>(static_cast<sqlite::Status>(result)) {}

  [[nodiscard]] inline static std::string_view Message(Error &&error) noexcept {
	return std::visit(ErrorPrinter(), error);
  }

  inline friend std::ostream &operator<<(std::ostream &output, const Error &error) {
	output << Error::Message(Error(error));
	return output;
  }
};

}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_ERROR_H_

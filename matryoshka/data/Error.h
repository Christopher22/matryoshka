/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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
  FileCreationFailed,
  DirectoryCreationFailed,
  FileExists,
  ReadingError,
  WritingError,
  OutOfBounds,
  NotImplemented
};

struct ArgumentError {
  constexpr ArgumentError() noexcept {}
  constexpr bool operator==(const ArgumentError &b) const noexcept { return true; }
  constexpr bool operator!=(const ArgumentError &b) const noexcept { return false; }
};
}

class Error {
 public:
  constexpr explicit Error(errors::ArgumentError backend_error) noexcept: data_(backend_error) {}

  constexpr explicit Error(errors::Backend backend_error) noexcept: data_(backend_error) {}

  constexpr explicit Error(errors::Io io_error) noexcept: data_(io_error) {}

  template<typename T>
  constexpr explicit Error(sqlite::Result<T> &result) noexcept: data_(static_cast<sqlite::Status>(result)) {}

  [[nodiscard]] inline static std::string_view Message(Error &&error) noexcept {
	return std::visit(ErrorPrinter(), error.data_);
  }

  inline friend std::ostream &operator<<(std::ostream &output, const Error &error) {
	output << Error::Message(Error(error));
	return output;
  }

  inline bool operator==(const Error &rhs) const {
	return data_ == rhs.data_;
  }

  inline bool operator!=(const Error &rhs) const {
	return !(rhs == *this);
  }

  template<typename T>
  [[nodiscard]] inline T *get() noexcept {
	return std::get_if<T>(&data_);
  }

 private:
  class ErrorPrinter {
   public:
	constexpr ErrorPrinter() = default;
	std::string_view operator()(errors::Backend &sqlite_error) const noexcept;
	std::string_view operator()(errors::Io &io_error) const noexcept;
	std::string_view operator()(errors::ArgumentError &io_error) const noexcept;
  };

  std::variant<errors::Backend, errors::Io, errors::ArgumentError> data_;
};

}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_ERROR_H_

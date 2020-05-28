/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_

#include <string_view>
#include <iostream>

namespace matryoshka::data::sqlite {
class Status {
 public:
  constexpr inline explicit Status(int status = 0) : status_(status) {}

  [[nodiscard]] static Status Aborted() noexcept;

  template<typename C>
  inline Status Than(C &callback) const noexcept {
	return *this ? callback() : *this;
  }

  [[nodiscard]] std::string_view Message() const noexcept;
  [[nodiscard]] bool IsSuccessful() const noexcept;
  [[nodiscard]] bool DataAvailable() const noexcept;
  [[nodiscard]] bool ConstraintViolated() const noexcept;

  inline explicit operator int() const noexcept {
	return status_;
  }

  inline explicit operator bool() const noexcept {
	return this->IsSuccessful();
  }

  inline friend std::ostream &operator<<(std::ostream &output, const Status &status) {
	output << status.Message();
	return output;
  }

  inline bool operator==(const Status &rhs) const {
	return status_ == rhs.status_;
  }

  inline bool operator!=(const Status &rhs) const {
	return !(rhs == *this);
  }

 private:
  int status_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_

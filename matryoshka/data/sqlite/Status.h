//
// Created by christopher on 17.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_

#include <string_view>
#include <iostream>

namespace matryoshka::data::sqlite {
class Status {
 public:
  constexpr inline explicit Status(int status = 0) : status_(status) {}

  template<typename C>
  inline Status Than(C &callback) const noexcept {
	return *this ? callback() : *this;
  }

  [[nodiscard]] std::string_view Message() const noexcept;
  [[nodiscard]] bool IsSuccessful() const noexcept;
  [[nodiscard]] bool DataAvailable() const noexcept;

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

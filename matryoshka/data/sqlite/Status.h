//
// Created by christopher on 17.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_

#include <string_view>

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

 private:
  int status_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_STATUS_H_

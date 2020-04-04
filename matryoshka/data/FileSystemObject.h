//
// Created by christopher on 11.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEMOBJECT_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEMOBJECT_H_

#include <cstdint>
#include <string>
#include <string_view>

namespace matryoshka::data {

class FileSystemObject {
 public:
  using HandleType = std::int_fast64_t;

  inline bool operator==(const FileSystemObject &other) const noexcept {
	return id_ == other.id_;
  }

  inline bool operator!=(const FileSystemObject &other) const noexcept {
	return id_ != other.id_;
  }

  [[nodiscard]] inline HandleType Handle() const noexcept {
	return id_;
  }

 protected:
  constexpr explicit FileSystemObject(HandleType id) noexcept : id_(id) {}

  HandleType id_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_FILESYSTEMOBJECT_H_
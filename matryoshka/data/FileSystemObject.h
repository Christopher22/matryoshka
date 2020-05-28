/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEMOBJECT_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEMOBJECT_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <cassert>

namespace matryoshka::data {

using FileSystemObjectType = int;

template<FileSystemObjectType ObjectType = -1>
class FileSystemObject {
 public:
  using HandleType = std::int_fast64_t;
  static constexpr FileSystemObjectType Type = ObjectType;

  FileSystemObject(FileSystemObject const &) = delete;
  FileSystemObject &operator=(FileSystemObject const &) = delete;

  template<FileSystemObjectType T>
  inline bool operator==(const FileSystemObject<T> &other) const noexcept {
	return id_ == other.id_;
  }

  template<FileSystemObjectType T>
  inline bool operator!=(const FileSystemObject<T> &other) const noexcept {
	return id_ != other.id_;
  }

  [[nodiscard]] inline HandleType Handle() const noexcept {
	// Check for valid handle on debug
	assert(id_ >= 0);
	return id_;
  }

  inline void Invalidate() noexcept {
	id_ = -1;
  }

 protected:
  constexpr explicit FileSystemObject(HandleType id) noexcept: id_(id) {}

  HandleType id_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_FILESYSTEMOBJECT_H_

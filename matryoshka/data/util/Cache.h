/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CACHE_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CACHE_H_

#include "../sqlite/Blob.h"

#include <queue>

namespace matryoshka::data::util {
class Cache {
 public:
  using Chunk = sqlite::Blob<true>;

  explicit Cache() noexcept;
  Cache(Cache const &) = delete;
  Cache &operator=(Cache const &) = delete;
  
  [[nodiscard]] int Size() const noexcept;
  [[nodiscard]] bool IsEmpty() const noexcept;

  void Push(Chunk &&data);
  Chunk Pop(int size);

  inline explicit operator bool() const noexcept {
	return !this->IsEmpty();
  }

 private:
  std::queue<Chunk> cache_;
  int size_, current_index_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CACHE_H_

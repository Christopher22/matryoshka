//
// Created by christopher on 11.05.2020.
//

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

//
// Created by christopher on 11.05.2020.
//

#include "Cache.h"

namespace matryoshka::data::util {

Cache::Cache() noexcept: size_(0), current_index_(0) {

}

int Cache::Size() const noexcept {
  return size_ - current_index_;
}

bool Cache::IsEmpty() const noexcept {
  return cache_.empty();
}

void Cache::Push(Cache::Chunk &&data) {
  size_ += data.Size();
  cache_.push(std::move(data));
}

Cache::Chunk Cache::Pop(int size) {
  if (size <= 0 && size > this->Size()) {
	return Cache::Chunk();
  }

  Chunk data(size);
  int bytes_written = 0;
  for (int chunk_onset = 0; bytes_written < size && !cache_.empty();) {
	const int remaining_size = size - chunk_onset;
	const int current_chunk_size = cache_.front().Size() - current_index_;

	// If the current blob in cache hold more than the required data
	if (remaining_size < current_chunk_size) {
	  data.Set(chunk_onset, &cache_.front(), remaining_size, current_index_);
	  current_index_ += remaining_size;
	  bytes_written += remaining_size;
	  chunk_onset += remaining_size;
	} else {
	  data.Set(chunk_onset, &cache_.front(), current_chunk_size, current_index_);
	  size_ -= cache_.front().Size();
	  cache_.pop();
	  current_index_ = 0;
	  bytes_written += current_chunk_size;
	  chunk_onset += current_chunk_size;
	}
  }

  return data;
}

}
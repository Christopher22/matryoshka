//
// Created by christopher on 28.04.2020.
//

#include "ChunkReader.h"

#include <utility>
#include <cassert>

namespace matryoshka::data::util {

ChunkReader::ChunkReader(int length, ChunkReader::Callback callback, int start)
	: Reader(start), length_(length), callback_(std::move(callback)) {
  assert(length > 0);
}

int ChunkReader::Length() const noexcept {
  return length_;
}

sqlite::Status ChunkReader::HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) {
  sqlite::Blob<true> data(num_bytes);
  blob.Read(data, blob_offset, 0, num_bytes);
  return callback_(std::move(data));
}

}
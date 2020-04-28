//
// Created by christopher on 28.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CHUNKREADER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CHUNKREADER_H_

#include "Reader.h"

#include <functional>

namespace matryoshka::data::util {
class ChunkReader : public Reader {
 public:
  using Callback = std::function<sqlite::Status(sqlite::Blob<true> &&)>;

  ChunkReader(int length, Callback callback, int start = 0);
  [[nodiscard]] int Length() const noexcept override;

 protected:
  sqlite::Status HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) override;

 private:
  Callback callback_;
  int length_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CHUNKREADER_H_

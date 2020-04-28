//
// Created by christopher on 02.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CONTINUOUSREADER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CONTINUOUSREADER_H_

#include "Reader.h"

namespace matryoshka::data::util {
class ContinuousReader : public Reader {
 public:
  static sqlite::Blob<true> Release(ContinuousReader &&reader);

  explicit ContinuousReader(int length, int start = 0);
  [[nodiscard]] int Length() const noexcept override;

 protected:
  sqlite::Status HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) override;

 private:
  sqlite::Blob<true> data_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CONTINUOUSREADER_H_

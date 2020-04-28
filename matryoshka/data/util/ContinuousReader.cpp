//
// Created by christopher on 02.04.2020.
//

#include "ContinuousReader.h"

namespace matryoshka::data::util {

ContinuousReader::ContinuousReader(int length, int start) : Reader(start), data_(length) {

}

sqlite::Blob<true> ContinuousReader::Release(ContinuousReader &&reader) {
  assert(reader);
  return std::move(reader.data_);
}

int ContinuousReader::Length() const noexcept {
  return data_.Size();
}

sqlite::Status ContinuousReader::HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) {
  return blob.Read(data_, blob_offset, bytes_read, num_bytes);
}

}
//
// Created by christopher on 02.04.2020.
//

#include "BlobReader.h"

#include <sqlite3.h>
#include <cassert>

namespace matryoshka::data::sqlite {

BlobReader::~BlobReader() noexcept {
  sqlite3_blob_close(handle_);
}

BlobReader::BlobReader(BlobReader &&other) noexcept: handle_(other.handle_) {
  other.handle_ = nullptr;
}

Result<BlobReader> BlobReader::Open(const Database &database,
									BlobReader::RowId blob_id,
									std::string_view table,
									std::string_view column) noexcept {

  sqlite3_blob *handle;
  const auto status = Status(sqlite3_blob_open(
	  database.Raw(),
	  "main",
	  table.data(),
	  column.data(),
	  blob_id,
	  0,
	  &handle));

  if (status) {
	return Result<BlobReader>(BlobReader(handle));
  } else {
	return Result<BlobReader>(status);
  }
}

Result<BlobReader> BlobReader::Open(BlobReader &&old_handle, BlobReader::RowId blob_id) noexcept {
  const auto status = Status(sqlite3_blob_reopen(old_handle.handle_, blob_id));
  if (status) {
	return Result<BlobReader>(BlobReader(old_handle.handle_));
  } else {
	return Result<BlobReader>(status);
  }
}

int BlobReader::Size() const noexcept {
  return sqlite3_blob_bytes(handle_);
}

Blob<true> BlobReader::Read(int length, int offset) const {
  assert(length > 0);
  Blob<true> data(length);
  if (this->Read(data, offset)) {
	return data;
  } else {
	return Blob<true>();
  }
}

Status BlobReader::Read(Blob<true> &destination, int offset, int destination_offset, int num_bytes) const {
  assert(destination.Size() > 0);
  assert(destination_offset >= 0);

  unsigned char *data = destination.Data();
  return Status(sqlite3_blob_read(
	  handle_,
	  static_cast<void *>(&data[destination_offset]),
	  num_bytes <= 0 ? destination.Size() - destination_offset : num_bytes,
	  offset)
  );
}

}
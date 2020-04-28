//
// Created by christopher on 28.04.2020.
//

#include "Reader.h"

#include <algorithm>
#include <cassert>

namespace matryoshka::data::util {

Reader::Reader(int start) : bytes_read_(0), start_offset_(start), blob_index_(0), error_(errors::Io::OutOfBounds) {

}

Reader &Reader::operator()(sqlite::BlobReader &&blob) {
  int num_bytes = std::min(blob.Size(), this->Length() - bytes_read_);
  sqlite::Status status;
  if (blob_index_ == 0) {
	num_bytes = std::min(blob.Size() - start_offset_, num_bytes);
	if (num_bytes == 0) {
	  // Handle the out-of-bound case, when the chunks are not all completely filled
	  error_ = data::Error(errors::Io::OutOfBounds);
	  return *this;
	}
	status = this->HandleBlob(blob, start_offset_, 0, num_bytes);
  } else {
	status = this->HandleBlob(blob, 0, bytes_read_, num_bytes);
  }
  bytes_read_ += num_bytes;
  ++blob_index_;

  if (!status) {
	(*this)(status);
  } else if (!*this && blob_indices_.size() > blob_index_) {
	std::visit(*this, sqlite::BlobReader::Open(std::move(blob), blob_indices_[blob_index_]));
  }
  return *this;
}

Reader &Reader::operator()(sqlite::Status status) {
  error_ = data::Error(status);
  return *this;
}

sqlite::Status Reader::Add(sqlite::Query &query) {
  bool set_offset = false;
  while (true) {
	sqlite::Status result = query();
	if (!result.DataAvailable()) {
	  return result;
	}

	this->Add(query.Get<int>(0));
	if (!set_offset) {
	  const int chunk_num = query.Get<int>(1);
	  const int chunk_size = query.Get<int>(2);
	  start_offset_ -= chunk_num * chunk_size;
	  assert(start_offset_ >= 0);
	  set_offset = true;
	}
  }
}

sqlite::Result<sqlite::PreparedStatement> Reader::PrepareStatement(sqlite::Database &database,
																   MetaTable &meta) {
  static constexpr std::string_view SQL_GET_CHUNKS = R"(
	SELECT chunk_id, chunk_num, {meta}.chunk_size FROM {data}
	INNER JOIN {meta} ON {meta}.id={data}.file_id
	WHERE file_id = :handle AND chunk_num BETWEEN cast((:index / {meta}.chunk_size) as int) AND cast(((:index + :size - 1) / {meta}.chunk_size) as int)
	ORDER BY chunk_num ASC
  )";
  return sqlite::PreparedStatement::Create(database, meta.Format(SQL_GET_CHUNKS));
}

}
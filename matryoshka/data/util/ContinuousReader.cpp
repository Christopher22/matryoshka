//
// Created by christopher on 02.04.2020.
//

#include "ContinuousReader.h"

#include <algorithm>
#include <cassert>

namespace matryoshka::data::util {

ContinuousReader::ContinuousReader(int length, int start)
	: data_(length), bytes_read_(0), start_offset_(start), blob_index_(0), error_(errors::Io::OutOfBounds) {

}

ContinuousReader &ContinuousReader::operator()(sqlite::BlobReader &&blob) {
  int num_bytes = std::min(blob.Size(), data_.Size() - bytes_read_);

  sqlite::Status status;
  if (blob_index_ == 0) {
	num_bytes = std::min(blob.Size() - start_offset_, num_bytes);
	if (num_bytes == 0) {
	  // Handle the out-of-bound case, when the chunks are not all completely filled
	  error_ = data::Error(errors::Io::OutOfBounds);
	  return *this;
	}
	status = blob.Read(data_, start_offset_, 0, num_bytes);
  } else {
	status = blob.Read(data_, 0, bytes_read_, num_bytes);
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

ContinuousReader &ContinuousReader::operator()(sqlite::Status status) {
  error_ = data::Error(status);
  return *this;
}

sqlite::Status ContinuousReader::Add(sqlite::Query &query) {
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

sqlite::Blob<true> ContinuousReader::Release(ContinuousReader &&reader) {
  assert(reader);
  return std::move(reader.data_);
}

sqlite::Result<sqlite::PreparedStatement> ContinuousReader::PrepareStatement(sqlite::Database &database,
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
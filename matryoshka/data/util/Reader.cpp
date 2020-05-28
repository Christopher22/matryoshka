/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Reader.h"

#include <algorithm>
#include <cassert>

namespace matryoshka::data::util {

Reader::Reader(int start)
	: current_blob_(std::nullopt),
	  bytes_read_(0),
	  start_offset_(start),
	  blob_index_(0) {

}

std::optional<Error> Reader::operator()() {
  if (!current_blob_.has_value()) {
	return data::Error(errors::Io::OutOfBounds);
  }

  // Get the current blob and reset its former handle
  auto blob = std::move(current_blob_.value());
  current_blob_.reset();

  int num_bytes = std::min(blob.Size(), this->Length() - bytes_read_);
  sqlite::Status status;
  if (blob_index_ == 0) {
	num_bytes = std::min(blob.Size() - start_offset_, num_bytes);
	if (num_bytes == 0) {
	  // Handle the out-of-bound case, when the chunks are not all completely filled
	  return data::Error(errors::Io::OutOfBounds);
	}
	status = this->HandleBlob(blob, start_offset_, 0, num_bytes);
  } else {
	status = this->HandleBlob(blob, 0, bytes_read_, num_bytes);
  }
  bytes_read_ += num_bytes;
  ++blob_index_;

  if (!status) {
	return data::Error(status);
  } else if (!*this && blob_indices_.size() > blob_index_) {
	auto next_blob = sqlite::BlobReader::Open(std::move(blob), blob_indices_[blob_index_]);
	if (!next_blob) {
	  return Error(std::get<sqlite::Status>(next_blob));
	}
	current_blob_.emplace(std::move(std::get<sqlite::BlobReader>(next_blob)));
  }
  return std::nullopt;
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
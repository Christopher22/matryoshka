/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_READER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_READER_H_

#include "../sqlite/BlobReader.h"
#include "../sqlite/Blob.h"
#include "../sqlite/Query.h"
#include "../sqlite/PreparedStatement.h"
#include "../sqlite/Result.h"
#include "../Error.h"

#include "MetaTable.h"

#include <variant>
#include <vector>

namespace matryoshka::data::util {
class Reader {
 public:
  [[nodiscard]] virtual int Length() const noexcept = 0;

  static sqlite::Result<sqlite::PreparedStatement> PrepareStatement(sqlite::Database &database, MetaTable &meta);

  explicit Reader(int start = 0);
  std::optional<Error> operator()();

  sqlite::Status Add(sqlite::Query &query);
  inline void Add(sqlite::Database::RowId blob_id) {
	blob_indices_.emplace_back(blob_id);
  }

  explicit inline operator bool() const noexcept {
	const int length = this->Length();
	return length == bytes_read_ && length > 0;
  }

  [[nodiscard]] inline sqlite::Database::RowId First() const noexcept {
	return !blob_indices_.empty() ? blob_indices_[0] : -1;
  }

  [[nodiscard]] inline int StartOffset() const {
	return start_offset_;
  }

  inline void SetStartOffset(int start_offset) {
	if (start_offset >= 0 && bytes_read_ == 0) {
	  start_offset_ = start_offset;
	}
  }

  inline void SetFirstBlob(sqlite::BlobReader &&first_blob) {
	current_blob_.emplace(std::move(first_blob));
  }

 protected:
  virtual sqlite::Status HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) = 0;

 private:
  std::optional<sqlite::BlobReader> current_blob_;
  int bytes_read_, start_offset_, blob_index_;
  std::vector<sqlite::Database::RowId> blob_indices_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_READER_H_

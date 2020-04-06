//
// Created by christopher on 02.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CONTINUOUSREADER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CONTINUOUSREADER_H_

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
class ContinuousReader {
 public:
  static sqlite::Result<sqlite::PreparedStatement> PrepareStatement(sqlite::Database &database, MetaTable &meta);
  static sqlite::Blob<true> Release(ContinuousReader &&reader);

  explicit ContinuousReader(int length, int start = 0);
  ContinuousReader &operator()(sqlite::BlobReader &&blob);
  ContinuousReader &operator()(sqlite::Status status);

  sqlite::Status Add(sqlite::Query &query);
  inline void Add(sqlite::Database::RowId blob_id) {
	blob_indices_.emplace_back(blob_id);
  }

  inline ContinuousReader &operator()(std::variant<sqlite::BlobReader, sqlite::Status> &&first_blob) {
	return std::visit(*this, std::move(first_blob));
  }

  explicit inline operator bool() const noexcept {
	return data_.Size() == bytes_read_ && data_.Size() > 0;
  }

  [[nodiscard]] inline sqlite::Database::RowId First() const noexcept {
	return !blob_indices_.empty() ? blob_indices_[0] : -1;
  }

  [[nodiscard]] inline Error Error() const noexcept {
	return error_;
  }

  [[nodiscard]] inline int StartOffset() const {
	return start_offset_;
  }

  inline void SetStartOffset(int start_offset) {
	if (start_offset >= 0) {
	  start_offset_ = start_offset;
	}
  }

 private:
  sqlite::Blob<true> data_;
  int bytes_read_, start_offset_, blob_index_;
  class Error error_;
  std::vector<sqlite::Database::RowId> blob_indices_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CONTINUOUSREADER_H_

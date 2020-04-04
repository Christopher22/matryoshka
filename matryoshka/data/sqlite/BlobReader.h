//
// Created by christopher on 02.04.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOBREADER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOBREADER_H_

#include "Status.h"
#include "Database.h"
#include "Blob.h"

#include <variant>

class sqlite3_blob;

namespace matryoshka::data::sqlite {
class BlobReader {
 public:
  using RowId = std::int_fast64_t;

  static std::variant<BlobReader, Status> Open(const Database &database,
											   RowId blob_id,
											   std::string_view table,
											   std::string_view column) noexcept;
  static std::variant<BlobReader, Status> Open(BlobReader &&old_handle, RowId blob_id) noexcept;

  BlobReader(BlobReader &&other) noexcept;
  ~BlobReader() noexcept;
  BlobReader(BlobReader const &) = delete;
  BlobReader &operator=(BlobReader const &) = delete;

  [[nodiscard]] int Size() const noexcept;
  Status Read(Blob<true> &destination, int offset = 0, int destination_offset = 0, int num_bytes = -1) const;
  [[nodiscard]] Blob<true> Read(int length, int offset = 0) const;

 protected:
  explicit constexpr BlobReader(sqlite3_blob *handle) noexcept: handle_(handle) {}

 private:
  sqlite3_blob *handle_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOBREADER_H_

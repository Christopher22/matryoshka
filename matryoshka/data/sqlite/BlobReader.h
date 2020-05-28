/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOBREADER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_SQLITE_BLOBREADER_H_

#include "Status.h"
#include "Database.h"
#include "Blob.h"
#include "Result.h"

class sqlite3_blob;

namespace matryoshka::data::sqlite {
class BlobReader {
 public:
  static Result<BlobReader> Open(const Database &database,
								 Database::RowId blob_id,
								 std::string_view table,
								 std::string_view column) noexcept;
  static Result<BlobReader> Open(BlobReader &&old_handle, Database::RowId blob_id) noexcept;

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

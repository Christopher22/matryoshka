/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CHUNKREADER_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CHUNKREADER_H_

#include "Reader.h"

#include <functional>

namespace matryoshka::data::util {
class ChunkReader : public Reader {
 public:
  using Callback = std::function<sqlite::Status(sqlite::Blob<true> &&)>;

  ChunkReader(int length, Callback callback, int start = 0);
  [[nodiscard]] int Length() const noexcept override;

 protected:
  sqlite::Status HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) override;

 private:
  Callback callback_;
  int length_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_CHUNKREADER_H_

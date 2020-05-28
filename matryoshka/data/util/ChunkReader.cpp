/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ChunkReader.h"

#include <utility>
#include <cassert>

namespace matryoshka::data::util {

ChunkReader::ChunkReader(int length, ChunkReader::Callback callback, int start)
	: Reader(start), length_(length), callback_(std::move(callback)) {
  assert(length > 0);
}

int ChunkReader::Length() const noexcept {
  return length_;
}

sqlite::Status ChunkReader::HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) {
  sqlite::Blob<true> data(num_bytes);
  blob.Read(data, blob_offset, 0, num_bytes);
  return callback_(std::move(data));
}

}
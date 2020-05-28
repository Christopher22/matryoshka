/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ContinuousReader.h"

namespace matryoshka::data::util {

ContinuousReader::ContinuousReader(int length, int start) : Reader(start), data_(length) {

}

sqlite::Blob<true> ContinuousReader::Release(ContinuousReader &&reader) {
  assert(reader);
  return std::move(reader.data_);
}

int ContinuousReader::Length() const noexcept {
  return data_.Size();
}

sqlite::Status ContinuousReader::HandleBlob(sqlite::BlobReader &blob, int blob_offset, int bytes_read, int num_bytes) {
  return blob.Read(data_, blob_offset, bytes_read, num_bytes);
}

}
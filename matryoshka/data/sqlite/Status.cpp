/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Status.h"

#include <sqlite3.h>

namespace matryoshka::data::sqlite {

std::string_view Status::Message() const noexcept {
  return std::string_view(sqlite3_errstr(status_));
}

bool Status::IsSuccessful() const noexcept {
  return status_ == SQLITE_OK || status_ == SQLITE_DONE || status_ == SQLITE_ROW;
}

bool Status::DataAvailable() const noexcept {
  return status_ == SQLITE_ROW;
}

bool Status::ConstraintViolated() const noexcept {
  // Use the lower 8 bits for getting the primary error code from an extended one, as defined in the documentation.
  return (status_ & 0xFF) == SQLITE_CONSTRAINT;
}

Status Status::Aborted() noexcept {
  return Status(SQLITE_ABORT);
}

}
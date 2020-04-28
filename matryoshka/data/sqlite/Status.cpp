//
// Created by christopher on 17.03.2020.
//

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
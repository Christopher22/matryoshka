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

}
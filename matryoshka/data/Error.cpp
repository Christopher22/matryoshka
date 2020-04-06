//
// Created by christopher on 04.04.2020.
//

#include "Error.h"

namespace matryoshka::data {

std::string_view Error::ErrorPrinter::operator()(errors::Backend &sqlite_error) const noexcept {
  return sqlite_error.Message();
}

std::string_view Error::ErrorPrinter::operator()(errors::Io &io_error) const noexcept {
  switch (io_error) {
	case errors::Io::FileNotFound: return "File not found";
	case errors::Io::OutOfBounds: return "Out of bounds";
	case errors::Io::InvalidDatabaseVersion: return "Database version not supported";
	case errors::Io::NotImplemented: return "Not implemented";
	default: return "Unknown error occurred";
  }
}

}
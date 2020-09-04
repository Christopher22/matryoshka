/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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
	case errors::Io::FileExists: return "File does already exists";
	case errors::Io::ReadingError: return "Unable to read file from local file system";
	case errors::Io::WritingError: return "Unable to write file to local file system";
	case errors::Io::FileCreationFailed: return "Unable to create the file";
	case errors::Io::DirectoryCreationFailed: return "Unable to create parent directories";
	default: return "Unknown error occurred";
  }
}

std::string_view Error::ErrorPrinter::operator()(errors::ArgumentError &io_error) const noexcept {
  return "Invalid argument provided.";
}

}
/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_PATH_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_PATH_H_

#include <string>
#include <string_view>
#include <vector>
#include <iostream>

namespace matryoshka::data {
class Path {
 public:
  explicit Path(std::string_view raw_path);
  Path(Path &&other) noexcept;
  Path(Path const &) = delete;
  Path &operator=(Path const &) = delete;

  [[nodiscard]] std::string AbsolutePath(int parts = -1) const;

  inline explicit operator bool() const noexcept {
	return !parts_.empty();
  }

  inline bool operator==(const Path &rhs) const noexcept {
	return parts_ == rhs.parts_;
  }

  inline bool operator!=(const Path &rhs) const noexcept {
	return !(rhs == *this);
  }

  inline bool operator==(std::string_view rhs) const noexcept {
	return parts_ == Path(rhs).parts_;
  }

  inline bool operator!=(std::string_view rhs) const noexcept {
	return !(parts_ == Path(rhs).parts_);
  }

  inline friend std::ostream &operator<<(std::ostream &output, const Path &error) {
	output << error.AbsolutePath();
	return output;
  }

 private:
  std::vector<std::string> parts_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_PATH_H_

/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Path.h"

#include <numeric>

namespace matryoshka::data {

Path::Path(std::string_view path) : parts_() {
  bool is_parsing = true;
  std::string_view::size_type index, old_index = 0;

  // Parse the path by splitting it into its parts
  while (is_parsing) {
	// Ensure that the remaining part is properly parsed
	if ((index = path.find('/', old_index)) == std::string_view::npos) {
	  index = path.size();
	  is_parsing = false;
	}

	std::string_view part = path.substr(old_index, index - old_index);
	if (part == "..") {
	  if (!parts_.empty()) {
		parts_.pop_back();
	  }
	} else if (!part.empty() && part != "." && part != "/") {
	  parts_.emplace_back(part);
	}
	old_index = index + 1;
  }
}

Path::Path(Path &&other) noexcept: parts_(std::move(other.parts_)) {}

std::string Path::AbsolutePath(int parts) const {
  std::string result_path;
  result_path.reserve(std::accumulate(parts_.begin(), parts_.end(), std::size_t(0),
									  [](std::string::size_type num_elements, const std::string &value) {
										return num_elements + value.size() + 1;
									  }));

  for (std::vector<std::string>::size_type i = 0, size = (parts <= parts_.size() ? parts : parts_.size());
	   i < size;
	   ++i) {
	if (i > 0) {
	  result_path.push_back('/');
	}
	result_path.append(parts_[i]);
  }

  return result_path;
}

}
/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "MetaTable.h"
#include "../sqlite/PreparedStatement.h"

#include <string>
#include <cassert>
#include <limits>
#include <charconv>
#include <algorithm>

namespace matryoshka::data::util {

MetaTable::MetaTable(MetaTable::Version version) noexcept : name_(std::string(META_PREFIX) + std::to_string(version)) {

}

MetaTable::MetaTable(std::string_view name) : name_(name) {
  assert(name_.find(META_PREFIX) == 0);
}

std::vector<MetaTable> MetaTable::Load(sqlite::Database &database) noexcept {
  std::vector<MetaTable> result;
  auto prepared_statement = sqlite::PreparedStatement::Create(
	  database,
	  "SELECT name FROM sqlite_master WHERE type='table' AND name LIKE 'Matryoshka_Meta_%'"
  );

  if (auto *statement = std::get_if<sqlite::PreparedStatement>(&prepared_statement)) {
	(*statement)([&](sqlite::Query &query) {
	  while (query().DataAvailable()) {
		result.emplace_back(query.Get<std::string_view>(0));
	  }
	  return sqlite::Status();
	});

	std::sort(result.begin(), result.end(), [](const MetaTable &a, const MetaTable &b) {
	  return a > b;
	});
  }

  return result;
}

MetaTable::Version MetaTable::Id() const noexcept {
  Version version = std::numeric_limits<Version>::max();
  auto index = name_.rfind('_');
  if (index == std::string::npos || index + 1 >= name_.size()) {
	return version;
  }
  std::from_chars(name_.data() + index + 1, name_.data() + name_.size(), version, 10);
  return version;
}

bool MetaTable::operator==(const MetaTable &rhs) const noexcept {
  return name_ == rhs.name_;
}

bool MetaTable::operator!=(const MetaTable &rhs) const noexcept {
  return !(rhs == *this);
}

bool MetaTable::operator<(const MetaTable &rhs) const noexcept {
  return this->Id() < rhs.Id();
}

bool MetaTable::operator>(const MetaTable &rhs) const noexcept {
  return rhs < *this;
}

bool MetaTable::operator<=(const MetaTable &rhs) const noexcept {
  return !(rhs < *this);
}

bool MetaTable::operator>=(const MetaTable &rhs) const noexcept {
  return !(*this < rhs);
}

std::string_view MetaTable::Meta() const noexcept {
  return name_;
}

std::string_view MetaTable::Data() const noexcept {
  return "Matryoshka_Data";
}

std::string MetaTable::Format(std::string_view input) const {
  std::string result;
  result.reserve(input.size() + this->Meta().size() + this->Data().size());

  std::string_view::size_type last_index = 0;
  while (last_index < input.size()) {
	auto index_meta = input.find(MetaTable::FORMAT_META, last_index),
		index_data = input.find(MetaTable::FORMAT_DATA, last_index);

	if (index_meta < index_data && index_meta != std::string_view::npos) {
	  result.append(input, last_index, index_meta - last_index);
	  result.append(this->Meta());
	  last_index = index_meta + MetaTable::FORMAT_META.size();
	} else if (index_data < index_meta && index_data != std::string_view::npos) {
	  result.append(input, last_index, index_data - last_index);
	  result.append(this->Data());
	  last_index = index_data + MetaTable::FORMAT_DATA.size();
	} else {
	  result.append(input, last_index);
	  last_index = std::string_view::npos;
	}
  }

  return result;
}

}
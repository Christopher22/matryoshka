/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_UTIL_METATABLE_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_UTIL_METATABLE_H_

#include "../sqlite/Database.h"

#include <vector>

namespace matryoshka::data::util {
class MetaTable {
 public:
  static constexpr std::string_view FORMAT_META = "{meta}";
  static constexpr std::string_view FORMAT_DATA = "{data}";
  using Version = unsigned int;

  explicit MetaTable(Version version) noexcept;
  explicit MetaTable(std::string_view name);
  static std::vector<MetaTable> Load(sqlite::Database &database) noexcept;
  [[nodiscard]] Version Id() const noexcept;
  [[nodiscard]] std::string_view Meta() const noexcept;
  [[nodiscard]] std::string_view Data() const noexcept;
  [[nodiscard]] std::string Format(std::string_view input) const;

  bool operator==(const MetaTable &rhs) const noexcept;
  bool operator!=(const MetaTable &rhs) const noexcept;
  bool operator<(const MetaTable &rhs) const noexcept;
  bool operator>(const MetaTable &rhs) const noexcept;
  bool operator<=(const MetaTable &rhs) const noexcept;
  bool operator>=(const MetaTable &rhs) const noexcept;

 private:
  static constexpr std::string_view META_PREFIX = "Matryoshka_Meta_";

  std::string name_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_UTIL_METATABLE_H_

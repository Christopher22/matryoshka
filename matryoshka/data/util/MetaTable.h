//
// Created by christopher on 24.03.2020.
//

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

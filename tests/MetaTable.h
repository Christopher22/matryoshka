/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_TESTS_METATABLE_H_
#define MATRYOSHKA_TESTS_METATABLE_H_

#include <doctest/doctest.h>

#include "../matryoshka/data/util/MetaTable.h"

using matryoshka::data::util::MetaTable;

TEST_SUITE ("MetaTable") {
TEST_CASE ("ID") {
  MetaTable meta(42);
  CHECK(meta.Meta() == "Matryoshka_Meta_42");
  CHECK(meta.Data() == "Matryoshka_Data");
  CHECK(meta.Id() == 42);
}

TEST_CASE ("Parsing") {
  MetaTable meta("Matryoshka_Meta_42");
  CHECK(meta.Id() == 42);
}

TEST_CASE ("Order") {
  MetaTable meta1("Matryoshka_Meta_1"), meta42("Matryoshka_Meta_42");
  CHECK(meta42 > meta1);
}

TEST_CASE ("Format") {
  MetaTable meta(0);
  CHECK(meta.Format("abc") == "abc");
  CHECK(meta.Format("{meta} abc") == "Matryoshka_Meta_0 abc");
  CHECK(meta.Format("{meta} abc {meta}") == "Matryoshka_Meta_0 abc Matryoshka_Meta_0");
  CHECK(meta.Format("{meta} abc {meta}{meta}") == "Matryoshka_Meta_0 abc Matryoshka_Meta_0Matryoshka_Meta_0");
  CHECK(meta.Format("{meta} abc {data}") == "Matryoshka_Meta_0 abc Matryoshka_Data");
  CHECK(meta.Format("{meta} abc {data} {data}") == "Matryoshka_Meta_0 abc Matryoshka_Data Matryoshka_Data");
  CHECK(meta.Format("{meta} abc {data}{data}") == "Matryoshka_Meta_0 abc Matryoshka_DataMatryoshka_Data");
}
}

#endif //MATRYOSHKA_TESTS_METATABLE_H_

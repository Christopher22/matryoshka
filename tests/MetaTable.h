//
// Created by christopher on 24.03.2020.
//

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

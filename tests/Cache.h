/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_TESTS_CACHE_H_
#define MATRYOSHKA_TESTS_CACHE_H_

#include "../matryoshka/data/util/Cache.h"

using namespace matryoshka::data::util;

TEST_SUITE ("Cache") {
TEST_CASE ("General") {
  Cache cache;
  CHECK(cache.IsEmpty());
  CHECK(!cache);
  CHECK(cache.Size() == 0);
}

TEST_CASE ("Writing") {
  Cache cache;
  cache.Push(Cache::Chunk::Filled(3, 42));
  cache.Push(Cache::Chunk::Filled(3, 66));
  CHECK(!cache.IsEmpty());
  CHECK(cache);
  CHECK(cache.Size() == 6);

  SUBCASE("Complete read") {
    auto data = cache.Pop(6);
    CHECK(data.Size() == 6);
    for(int i = 0; i < data.Size(); ++i) {
      CHECK(data[i] == (i < 3 ? 42 : 66));
    }
  }

  SUBCASE("Single element read") {
    for(int i = 5; i >= 0; --i) {
	  auto data = cache.Pop(1);
	  CHECK(data.Size() == 1);
	  CHECK(data[0] == (i > 2 ? 42 : 66));
	  CHECK(cache.Size() == i);
    }
  }

  SUBCASE("Equal parts") {
	auto data = cache.Pop(3);
	CHECK(data.Size() == 3);
	auto data2 = cache.Pop(3);
	CHECK(data == Cache::Chunk::Filled(3, 42));
	CHECK(data2 == Cache::Chunk::Filled(3, 66));
  }

  SUBCASE("Unequal parts") {
	auto data = cache.Pop(1);
	CHECK(data.Size() == 1);
	CHECK(data[0] == 42);
	CHECK(cache.Size() == 5);

	auto data2 = cache.Pop(2);
	CHECK(data2.Size() == 2);
	CHECK(data2 == Cache::Chunk::Filled(2, 42));
	CHECK(cache.Size() == 3);

	auto data3 = cache.Pop(3);
	CHECK(data3.Size() == 3);
	CHECK(data3 == Cache::Chunk::Filled(3, 66));
  }

  CHECK(cache.IsEmpty());
  CHECK(!cache);
  CHECK(cache.Size() == 0);
}
}

#endif //MATRYOSHKA_TESTS_CACHE_H_

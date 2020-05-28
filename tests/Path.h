/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_TESTS_PATH_H_
#define MATRYOSHKA_TESTS_PATH_H_

#include <doctest/doctest.h>

#include "../matryoshka/data/Path.h"

using namespace matryoshka::data;

TEST_SUITE ("Path") {
TEST_CASE ("Parsing") {
  // Check special chars
  CHECK(Path("/") == "");
  CHECK(Path(".") == "");
  CHECK(Path("..") == "");
  
  // Support multiple writing variants
  CHECK(Path("42") == "42");
  CHECK(Path("/42") == "42");
  CHECK(Path("42/") == "42");
  CHECK(Path("/42/") == "42");
  
  // Support multiple parts
  CHECK(Path("42/PI") == "42/PI");
  CHECK(Path("/42/PI/") == "42/PI");
  CHECK(Path("/42/PI") == "42/PI");
  
  // Support current directory part
  CHECK(Path("/42/.") == "42");
  CHECK(Path("/42/./") == "42");
  CHECK(Path("/42/./PI") == "42/PI");
  
  // Support parent directory
  CHECK(Path("/42/..") == "");
  CHECK(Path("42/..") == "");
  CHECK(Path("./..") == "");
  CHECK(Path("42/../") == "");
  CHECK(Path("42/../PI") == "PI");
  CHECK(Path("42/./../PI/") == "PI");
  CHECK(Path("42/43/../PI/") == "42/PI");
}

TEST_CASE ("Path parts") {
  auto path = Path("a/b/c/");
  CHECK(path.AbsolutePath() == "a/b/c");
  CHECK(path.AbsolutePath(1) == "a");
  CHECK(path.AbsolutePath(2) == "a/b");
  // It is save to specify to much parts
  CHECK(path.AbsolutePath(42) == "a/b/c");
}
}

#endif //MATRYOSHKA_TESTS_PATH_H_

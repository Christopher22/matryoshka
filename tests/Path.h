//
// Created by christopher on 04.04.2020.
//

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

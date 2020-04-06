//
// Created by christopher on 30.03.2020.
//

#ifndef MATRYOSHKA_TESTS_FILESYSTEM_H_
#define MATRYOSHKA_TESTS_FILESYSTEM_H_

#include <doctest/doctest.h>

#include "../matryoshka/data/FileSystem.h"
#include "../matryoshka/data/Path.h"

TEST_SUITE ("FileSystem") {
TEST_CASE ("Reading") {
  auto database = std::get<Database>(Database::Create());
  auto file_system_container = FileSystem::Open(std::move(database));
  REQUIRE_MESSAGE(file_system_container, file_system_container);
  auto file_system = std::get<FileSystem>(std::move(file_system_container));

  sqlite::Blob<true> data(42);
  for (unsigned char i = 0, size = data.Size(); i < size; ++i) {
	data[i] = i;
  }

  Path path("example_folder/example_file.txt");
  matryoshka::data::Result<File> file_container((Error(errors::Io::NotImplemented)));

  SUBCASE("One chunk") {
	file_container = file_system.Create(path, data.Copy());
  }

  SUBCASE("Oversized chunk") {
	file_container = file_system.Create(path, data.Copy(), data.Size() + 42);
  }

  SUBCASE("Multiple chunks - Last chunk == chunk size") {
	file_container = file_system.Create(path, data.Copy(), 14);
  }

  SUBCASE("Multiple chunks - Last chunk != chunk size") {
	file_container = file_system.Create(path, data.Copy(), 16);
  }

  REQUIRE_MESSAGE(file_container, file_container);
  auto file = std::get<File>(file_container);

  // Read full data
  matryoshka::data::Result<matryoshka::data::FileSystem::Chunk> read_blob = file_system.Read(file, 0, data.Size());
  CHECK(read_blob == data);

  // Read first byte
  read_blob = file_system.Read(file, 0, 1);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 1);
  CHECK(read_blob->operator[](0) == 0);

  // Read last byte
  read_blob = file_system.Read(file, 41, 1);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 1);
  CHECK(read_blob->operator[](0) == 41);

  // Check out-of-bounds
  read_blob = file_system.Read(file, 42, 1);
  CHECK(read_blob == Error(errors::Io::OutOfBounds));
  read_blob = file_system.Read(file, 40, 4);
  CHECK(read_blob == Error(errors::Io::OutOfBounds));

  // Read at chunk borders
  read_blob = file_system.Read(file, 15, 1);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 1);
  CHECK(read_blob->operator[](0) == 15);

  read_blob = file_system.Read(file, 15, 2);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 2);
  CHECK(read_blob->operator[](0) == 15);
  CHECK(read_blob->operator[](1) == 16);
}
}

#endif //MATRYOSHKA_TESTS_FILESYSTEM_H_

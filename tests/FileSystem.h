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

TEST_CASE ("Multiple files") {
  auto database = std::get<Database>(Database::Create());
  auto file_system_container = FileSystem::Open(std::move(database));
  REQUIRE_MESSAGE(file_system_container, file_system_container);
  auto file_system = std::get<FileSystem>(std::move(file_system_container));

  sqlite::Blob<true> data(42);
  const Path path_1 = Path("folder/example_file_1.txt");
  REQUIRE(file_system.Create(path_1, data.Copy()));
  REQUIRE(file_system.Create(path_1, data.Copy()) == Error(errors::Io::FileExists));

  // Create an example content
  const Path path_2 = Path("folder/example_file_2.txt"),
	path_3 = Path("folder/nested_folder1/file1.txt"),
	path_4 = Path("folder/nested_folder1/file2.txt"),
	path_5 = Path("folder/nested_folder2/file1.txt");
  REQUIRE(file_system.Create(path_2, data.Copy()));
  REQUIRE(file_system.Create(path_3, data.Copy()));
  REQUIRE(file_system.Create(path_4, data.Copy()));
  REQUIRE(file_system.Create(path_5, data.Copy()));

  std::vector<Path> paths;

  // Check non-existing paths
  file_system.Find(Path("folder"), paths);
  CHECK(paths.empty());

  // Check existing paths - makes no real sense, but...
  file_system.Find(path_1, paths);
  CHECK(paths.size() == 1);
  CHECK(paths.at(0) == path_1);

  // Check single char wildcard
  paths.clear();
  file_system.Find(Path("folder/example_file_?.txt"), paths);
  CHECK(paths.size() == 2);
  CHECK(std::find(paths.begin(), paths.end(), path_1) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_2) != paths.end());

  // Check multiple char wildcard
  paths.clear();
  file_system.Find(Path("folder/example_*.txt"), paths);
  CHECK(paths.size() == 2);
  CHECK(std::find(paths.begin(), paths.end(), path_1) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_2) != paths.end());

  // Check multiple char wildcard in folders
  paths.clear();
  file_system.Find(Path("folder/*/*"), paths);
  CHECK(paths.size() == 3);
  CHECK(std::find(paths.begin(), paths.end(), path_3) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_4) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_5) != paths.end());

  // Check general wildcard
  std::vector<Path> all_paths;
  paths.clear();
  file_system.Find(Path("*"), paths);
  file_system.Find(all_paths);
  CHECK(paths.size() == 5);
  CHECK(paths == all_paths);
  CHECK(std::find(paths.begin(), paths.end(), path_1) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_2) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_3) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_4) != paths.end());
  CHECK(std::find(paths.begin(), paths.end(), path_5) != paths.end());
}
}

#endif //MATRYOSHKA_TESTS_FILESYSTEM_H_

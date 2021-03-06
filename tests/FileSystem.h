/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_TESTS_FILESYSTEM_H_
#define MATRYOSHKA_TESTS_FILESYSTEM_H_

#include <filesystem>

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
  std::string local_file_path = "test.tmp";

  // Create a local file on disk for the file IO checks
  REQUIRE(data.Save(local_file_path, false));

  SUBCASE("One chunk") {
	file_container = file_system.Create(path, data.Copy());
  }

  SUBCASE("One chunk - Callback style") {
	file_container = file_system.Create(path, [&] (int) {
		return data.Copy();
	}, data.Size());
  }

  SUBCASE("One chunk - File") {
	file_container = file_system.Create(path, local_file_path);
  }

  SUBCASE("Oversized chunk") {
	file_container = file_system.Create(path, data.Copy(), data.Size() + 42);
  }

  SUBCASE("Oversized chunk - Callback style") {
	file_container = file_system.Create(path, [&] (int) {
	  return data.Copy();
	}, data.Size(), data.Size() + 42);
  }

  SUBCASE("Oversized chunk - File") {
	file_container = file_system.Create(path, local_file_path, data.Size() + 42);
  }

  SUBCASE("Multiple chunks - Last chunk == chunk size") {
	file_container = file_system.Create(path, data.Copy(), 14);
  }

  SUBCASE("Multiple chunks - Last chunk == chunk size - Callback style") {
    int bytes_written = 0;
	file_container = file_system.Create(path, [&] (int chunk_size) {
	  auto result = sqlite::Blob<true>(data.Part(chunk_size, bytes_written));
	  bytes_written += chunk_size;
	  return result;
	}, data.Size(), 14);
  }

  SUBCASE("Multiple chunks - Last chunk == chunk size - File") {
	file_container = file_system.Create(path, local_file_path, 14);
  }

  SUBCASE("Multiple chunks - Last chunk != chunk size") {
	file_container = file_system.Create(path, data.Copy(), 16);
  }

  SUBCASE("Multiple chunks - Last chunk == chunk size - Callback style - Callback style") {
	int bytes_written = 0;
	file_container = file_system.Create(path, [&] (int chunk_size) {
	  auto result = sqlite::Blob<true>(data.Part(chunk_size, bytes_written));
	  bytes_written += chunk_size;
	  return result;
	}, data.Size(), 16);
  }

  SUBCASE("Multiple chunks - Last chunk != chunk size - File") {
	file_container = file_system.Create(path, local_file_path, 16);
  }

  REQUIRE_MESSAGE(file_container, file_container);
  auto file = std::get<File>(std::move(file_container));

  // Check the reported file size
  CHECK(file_system.Size(file) == data.Size());

  // Check direct read from database to local file system
  auto read_status = file_system.Read(file, "test2.tmp", 0, data.Size(), true);
  REQUIRE_MESSAGE(!read_status.has_value(), read_status);
  CHECK(Blob<true>("test2.tmp") == data);

  // Check direct read from database to local file system with an non-existing folder
  CHECK(!file_system.Read(file, "nonexisting_folder/test2.tmp", 0, data.Size(), true, true).has_value());
  CHECK(Blob<true>("nonexisting_folder/test2.tmp") == data);
  auto read_missing_parent = file_system.Read(file, "nonexisting_folder2/test2.tmp", 0, data.Size(), true, false);
  CHECK(read_missing_parent.has_value());
  CHECK(read_missing_parent.value() == matryoshka::data::Error(matryoshka::data::errors::Io::FileCreationFailed));

  // Read full data
  matryoshka::data::Result<matryoshka::data::FileSystem::Chunk> read_blob = file_system.Read(file, 0, data.Size());
  CHECK(read_blob == data);

  // Read first byte
  read_blob = file_system.Read(file, 0, 1);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 1);
  CHECK(read_blob->operator[](0) == 0);

  CHECK(!file_system.Read(file, 0, 1, [] (auto &&chunk) {
	CHECK(chunk.Size() == 1);
	CHECK(chunk.operator[](0) == 0);
	return true;
  }));

  // Read last byte
  read_blob = file_system.Read(file, 41, 1);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 1);
  CHECK(read_blob->operator[](0) == 41);

  CHECK(!file_system.Read(file, 41, 1, [] (auto &&chunk) {
	CHECK(chunk.Size() == 1);
	CHECK(chunk.operator[](0) == 41);
	return true;
  }));

  // Check out-of-bounds
  read_blob = file_system.Read(file, 42, 1);
  CHECK(read_blob == Error(errors::Io::OutOfBounds));
  read_blob = file_system.Read(file, 40, 4);
  CHECK(read_blob == Error(errors::Io::OutOfBounds));

  CHECK(file_system.Read(file, 42, 1, [] (auto &&chunk) {
	CHECK(false);
	return true;
  }).value() == Error(errors::Io::OutOfBounds));

  CHECK(file_system.Read(file, 40, 4, [] (auto &&chunk) {
    // The chunk reader reads all data available but returns OutOfBounds nevertheless.
	CHECK(true);
	return true;
  }).value() == Error(errors::Io::OutOfBounds));

  // Read at chunk borders
  read_blob = file_system.Read(file, 15, 1);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 1);
  CHECK(read_blob->operator[](0) == 15);

  CHECK(!file_system.Read(file, 15, 1, [] (auto &&chunk) {
	CHECK(chunk.Size() == 1);
	CHECK(chunk.operator[](0) == 15);
	return true;
  }));

  read_blob = file_system.Read(file, 15, 2);
  REQUIRE(read_blob);
  CHECK(read_blob->Size() == 2);
  CHECK(read_blob->operator[](0) == 15);
  CHECK(read_blob->operator[](1) == 16);

  // Check delete in all different chunked conditions
  Path second_file("second_file/no_delete");
  REQUIRE(file_system.Create(second_file, data.Copy()));
  REQUIRE(file_system.Delete(std::move(file)));
  CHECK(file_system.Open(path) == Error(errors::Io::FileNotFound));
  CHECK(file_system.Open(second_file));
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

  // Check for opening
  CHECK(file_system.Open(path_1));
  CHECK(!file_system.Open(Path("a_nonexisting_file")));

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

TEST_CASE ("Empty files") {
auto database = std::get<Database>(Database::Create());
auto file_system_container = FileSystem::Open(std::move(database));
REQUIRE_MESSAGE(file_system_container, file_system_container);
auto file_system = std::get<FileSystem>(std::move(file_system_container));

Blob<true> empty_blob;
REQUIRE(empty_blob.Save("empty_file_1"));

auto file_container = file_system.Create(Path("data_empty"), "empty_file_1", 0);
REQUIRE_MESSAGE(file_container, file_container);
auto file = std::get<File>(std::move(file_container));

CHECK(file_system.Size(file) == 0);

REQUIRE(!std::filesystem::exists("empty_file_2"));
auto result = file_system.Read(file, "empty_file_2", 0, 0);
REQUIRE(!result.has_value());
REQUIRE(std::filesystem::exists("empty_file_2"));
}
}

#endif //MATRYOSHKA_TESTS_FILESYSTEM_H_

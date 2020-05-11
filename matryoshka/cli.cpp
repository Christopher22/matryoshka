//
// Created by christopher on 14.04.2020.
//

#include "data/FileSystem.h"

#include <CLI/CLI.hpp>

#include <limits>

using namespace matryoshka::data;

enum class ReturnCode : int {
  Success = 0,
  SQLiteInvalid = 1,
  FilesystemInvalid = 2,
  FileNotFound = 3,
  FilePushFailed = 4,
  FilePullFailed = 5
};

FileSystem Open(std::string_view path) {
  auto database = sqlite::Database::Create(path);
  if (!database) {
	throw CLI::RuntimeError("Unable to open the SQLite database", static_cast<int>(ReturnCode::SQLiteInvalid));
  }

  auto file_system = FileSystem::Open(std::move(std::get<sqlite::Database>(database)));
  if (!file_system) {
	throw CLI::RuntimeError("Unable to open the file system", static_cast<int>(ReturnCode::FilesystemInvalid));
  }

  return std::move(std::get<FileSystem>(file_system));
}

int main(int argc, char **argv) {
  std::string container_file, source, destination;
  int chunk_size = 8192;

  CLI::App app("Matryoshka - Command line interface");
  app.add_option("container_file", container_file, "The Matryoshka file")->check(CLI::ExistingFile);

  // "list" command
  app.add_subcommand("list", "Show all files")->alias("ls")->final_callback([&]() {
	FileSystem file_system = Open(container_file);
	std::vector<Path> paths;
	file_system.Find(paths);
	for (auto &path: paths) {
	  std::cout << path << std::endl;
	}
	return ReturnCode::Success;
  });

  // "push" command
  auto push = app.add_subcommand("push", "Push a file to the Matryoshka file")->final_callback([&]() {
	FileSystem file_system = Open(container_file);
	auto result = file_system.Create(Path(destination), source, chunk_size);
	if (!result) {
	  throw CLI::RuntimeError(std::string(Error::Message(std::get<Error>(std::move(result)))),
							  static_cast<int>(ReturnCode::FilePushFailed));
	}
  });
  push->add_option("source", source, "The file to be pushed")->required()->check(CLI::ExistingFile);
  push->add_option("destination", destination, "The inner path in the Matryoshka file")->required();
  push->add_option("chunk_size", chunk_size, "The chunk size used internally.")
	  ->check(CLI::Range(1, std::numeric_limits<int>::max()));

  // "pull" command
  auto pull = app.add_subcommand("pull", "Pull a file from the Matryoshka file")->final_callback([&]() {
	FileSystem file_system = Open(container_file);

	// Open the file
	auto file_container = file_system.Open(Path(source));
	if (!file_container) {
	  throw CLI::RuntimeError("Unable to access file from the database. Does it exist?",
							  static_cast<int>(ReturnCode::FileNotFound));
	}

	// Read its content into the memory
	auto file = Result<File>::Get(std::move(file_container));
	auto result = file_system.Read(file, destination, 0, file_system.Size(file), true);
	if (result.has_value()) {
	  throw CLI::RuntimeError(std::string(Error::Message(Error(result.value()))),
							  static_cast<int>(ReturnCode::FilePullFailed));
	}
  });
  pull->add_option("source", source, "The inner path in the Matryoshka file")->required();
  pull->add_option("destination", destination, "The destination file")->required()->check(CLI::NonexistentPath);

  CLI11_PARSE(app, argc, argv);
  return static_cast<int>(ReturnCode::Success);
}

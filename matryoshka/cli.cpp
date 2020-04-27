//
// Created by christopher on 14.04.2020.
//

#include "data/FileSystem.h"

#include <CLI/CLI.hpp>

using namespace matryoshka::data;

FileSystem Open(std::string_view path) {
  auto database = sqlite::Database::Create(path);
  if (!database) {
	throw CLI::RuntimeError("Unable to open the SQLite database", 1);
  }

  auto file_system = FileSystem::Open(std::move(std::get<sqlite::Database>(database)));
  if (!file_system) {
	throw CLI::RuntimeError("Unable to open the file system", 2);
  }

  return std::move(std::get<FileSystem>(file_system));
}

int main(int argc, char **argv) {
  std::string container_file, source, destination;

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
	return 0;
  });

  // "push" command
  auto push = app.add_subcommand("push", "Push a file to the Matryoshka file")->final_callback([&]() {
	FileSystem file_system = Open(container_file);

	// Read the file into the memory
	FileSystem::Chunk chunk(source);
	if (!chunk) {
	  throw CLI::RuntimeError("Unable to read file from file system", 3);
	}

	// Write the file to the database
	if (!file_system.Create(Path(destination), std::move(chunk))) {
	  throw CLI::RuntimeError("Unable to write file to the database", 4);
	}
  });
  push->add_option("source", source, "The file to be pushed")->required()->check(CLI::ExistingFile);
  push->add_option("destination", destination, "The inner path in the Matryoshka file")->required();

  // "pull" command
  auto pull = app.add_subcommand("pull", "Pull a file from the Matryoshka file")->final_callback([&]() {
	FileSystem file_system = Open(container_file);

	// Open the file
	auto file_container = file_system.Open(Path(source));
	if (!file_container) {
	  throw CLI::RuntimeError("Unable to access file from the database. Does it exist?", 5);
	}

	// Read its content into the memory
	auto file = Result<File>::Get(std::move(file_container));
	auto data = file_system.Read(file, 0, file_system.Size(file));
	if (!data) {
	  throw CLI::RuntimeError("Unable to read file into the memory.", 6);
	}

	// Save it to the disk
	if (!data->Save(destination, false)) {
	  throw CLI::RuntimeError("Unable to write file to disk.", 7);
	}
  });
  pull->add_option("source", source, "The inner path in the Matryoshka file")->required();
  pull->add_option("destination", destination, "The destination file")->required()->check(CLI::NonexistentPath);

  CLI11_PARSE(app, argc, argv);
  return 0;
}

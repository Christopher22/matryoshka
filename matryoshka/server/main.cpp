//
// Created by christopher on 27.04.2020.
//

#include <restinio/all.hpp>
#include <CLI/CLI.hpp>

#include "Server.h"

using namespace matryoshka::data;
using namespace matryoshka::server;

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
  std::string container_file;

  CLI::App app("Matryoshka - WebDav");
  app.add_option("container_file", container_file, "The Matryoshka file")->check(CLI::ExistingFile)->required();

  try {
	(app).parse((argc), (argv));
	Server server(Open(container_file));
	restinio::run(
		restinio::on_this_thread()
			.port(8080)
			.address("localhost")
			.request_handler([&server](auto req) { return server(req); }));
  } catch (const CLI::ParseError &e) {
	return (app).exit(e);
  }

  return 0;
}
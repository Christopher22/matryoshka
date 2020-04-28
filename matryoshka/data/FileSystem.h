//
// Created by christopher on 19.03.2020.
//

#ifndef MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEM_H_
#define MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEM_H_

#include "Error.h"
#include "Folder.h"
#include "File.h"
#include "Path.h"
#include "util/MetaTable.h"
#include "util/Reader.h"
#include "sqlite/Database.h"
#include "sqlite/PreparedStatement.h"
#include "sqlite/Blob.h"
#include "sqlite/Result.h"

#include <variant>
#include <optional>
#include <string_view>
#include <functional>

namespace matryoshka::data {
template<typename T>
using Result = sqlite::Result<T, Error>;

class FileSystem {
 public:
  constexpr static util::MetaTable::Version CURRENT_VERSION = 0;
  using Chunk = sqlite::Blob<true>;

  static Result<FileSystem> Open(sqlite::Database &&database) noexcept;
  FileSystem(FileSystem &&other) noexcept;
  FileSystem(FileSystem const &) = delete;
  FileSystem &operator=(FileSystem const &) = delete;

  [[nodiscard]] Result<File> Open(const Path &path) noexcept;
  [[nodiscard]] Result<Chunk> Read(const File &file, int start, int length) const;
  std::optional<Error> Read(const File &file, int start, int length, std::function<bool(Chunk&&)> callback) const;

  [[nodiscard]] int Size(const File& file);
  Result<File> Create(const Path &path, Chunk &&data, int chunk_size = -1);
  void Find(const Path &path, std::vector<Path> &files) const noexcept;
  inline void Find(std::vector<Path> &files) const noexcept {
	this->Find(Path("*"), files);
  }

 protected:
  FileSystem(sqlite::Database &&database,
			 sqlite::PreparedStatement &&handle_statement,
			 sqlite::PreparedStatement &&chunk_statement,
			 sqlite::PreparedStatement &&header_statement,
			 sqlite::PreparedStatement &&blob_statement,
			 sqlite::PreparedStatement &&glob_statement_,
			 sqlite::PreparedStatement &&size_statement_,
			 util::MetaTable meta_table) noexcept;

  sqlite::Result<sqlite::Database::RowId, sqlite::Status> CreateHeader(const Path &path,
																	   int chunk_size,
																	   FileSystemObjectType type) noexcept;
 private:

  std::optional<Error> Read(const File &file, util::Reader *reader, int start) const;
  sqlite::Database database_;
  sqlite::PreparedStatement handle_statement_, chunk_statement_, header_statement_, blob_statement_, glob_statement_, size_statement_;
  util::MetaTable meta_;
};
}

#endif //MATRYOSHKA_MATRYOSHKA_DATA_FILESYSTEM_H_

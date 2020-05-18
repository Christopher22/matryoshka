//
// Created by christopher on 19.03.2020.
//

#include "FileSystem.h"
#include "sqlite/PreparedStatement.h"
#include "sqlite/BlobReader.h"
#include "sqlite/Transaction.h"
#include "util/ContinuousReader.h"
#include "util/ChunkReader.h"
#include "util/Cache.h"

#include <cassert>
#include <sstream>
#include <utility>
#include <numeric>
#include <algorithm>
#include <fstream>

using namespace matryoshka::data::sqlite;

namespace matryoshka::data {

FileSystem::FileSystem(sqlite::Database &&database,
					   sqlite::PreparedStatement &&handle_statement,
					   sqlite::PreparedStatement &&chunk_statement,
					   sqlite::PreparedStatement &&header_statement,
					   sqlite::PreparedStatement &&blob_statement,
					   sqlite::PreparedStatement &&glob_statement,
					   sqlite::PreparedStatement &&size_statement,
					   util::MetaTable meta_table) noexcept
	: database_(std::move(database)),
	  handle_statement_(std::move(handle_statement)),
	  chunk_statement_(std::move(chunk_statement)),
	  header_statement_(std::move(header_statement)),
	  blob_statement_(std::move(blob_statement)),
	  glob_statement_(std::move(glob_statement)),
	  size_statement_(std::move(size_statement)),
	  meta_(std::move(meta_table)) {
  assert(handle_statement_ && chunk_statement_ && header_statement_ && blob_statement_ && glob_statement_
			 && size_statement_);
}

FileSystem::FileSystem(FileSystem &&other) noexcept: database_(std::move(other.database_)),
													 handle_statement_(std::move(other.handle_statement_)),
													 chunk_statement_(std::move(other.chunk_statement_)),
													 header_statement_(std::move(other.header_statement_)),
													 blob_statement_(std::move(other.blob_statement_)),
													 glob_statement_(std::move(other.glob_statement_)),
													 size_statement_(std::move(other.size_statement_)),
													 meta_(std::move(other.meta_)) {
}

Result<FileSystem> FileSystem::Open(sqlite::Database &&database) noexcept {
  static constexpr std::string_view SQL_CREATE_META =
	  "CREATE TABLE {meta} (id INTEGER PRIMARY KEY, path TEXT UNIQUE NOT NULL, type INTEGER, flags INTEGER, chunk_size INTEGER NOT NULL)";
  static constexpr std::string_view SQL_CREATE_DATA =
	  "CREATE TABLE IF NOT EXISTS {data} (chunk_id INTEGER PRIMARY KEY, file_id INTEGER NOT NULL, chunk_num INTEGER NOT NULL, data BLOB NOT NULL, CONSTRAINT unq UNIQUE (file_id, chunk_num), FOREIGN KEY(file_id) REFERENCES {meta} (id))";
  static constexpr std::string_view SQL_GET_HANDLE = "SELECT id FROM {meta} WHERE path = ? AND type = ?";
  static constexpr std::string_view SQL_GLOB = "SELECT path FROM {meta} WHERE path GLOB ? AND type = ?";
  static constexpr std::string_view SQL_SIZE = "SELECT COALESCE(SUM(LENGTH(data)), -1) FROM {data} WHERE file_id = ?";

  auto meta = util::MetaTable::Load(database);
  if (!meta.empty() && meta[0].Id() != CURRENT_VERSION) {
	return Result<FileSystem>::Fail(errors::Io::InvalidDatabaseVersion);
  } else if (meta.empty()) {
	meta.emplace_back(CURRENT_VERSION);

	// Create meta table
	auto status = database(meta[0].Format(SQL_CREATE_META));
	if (!status) {
	  return Result<FileSystem>::Fail(status);
	}

	// Create data table
	if (!(status = database(meta[0].Format(SQL_CREATE_DATA)))) {
	  return Result<FileSystem>::Fail(status);
	}
  }

  // Prepare the statement for querying the id
  auto handle_statement = sqlite::PreparedStatement::Create(database, meta[0].Format(SQL_GET_HANDLE));
  auto chunk_statement = util::Reader::PrepareStatement(database, meta[0]);
  auto insert_header_statement =
	  sqlite::PreparedStatement::Insert(database, meta[0].Meta(), {"path", "type", "chunk_size"});
  auto insert_blob_statement =
	  sqlite::PreparedStatement::Insert(database, meta[0].Data(), {"file_id", "chunk_num", "data"});
  auto glob_statement = sqlite::PreparedStatement::Create(database, meta[0].Format(SQL_GLOB));
  auto size_statement = sqlite::PreparedStatement::Create(database, meta[0].Format(SQL_SIZE));

  Status status = sqlite::Result<>::Check(handle_statement,
										  chunk_statement,
										  insert_header_statement,
										  insert_blob_statement,
										  glob_statement,
										  size_statement);
  if (status) {
	// Protected constructor enforce external setup
	return Result<FileSystem>(FileSystem(std::move(database),
										 sqlite::Result<>::Get(std::move(handle_statement)),
										 sqlite::Result<>::Get(std::move(chunk_statement)),
										 sqlite::Result<>::Get(std::move(insert_header_statement)),
										 sqlite::Result<>::Get(std::move(insert_blob_statement)),
										 sqlite::Result<>::Get(std::move(glob_statement)),
										 sqlite::Result<>::Get(std::move(size_statement)),
										 meta[0]));
  } else {
	return Result<FileSystem>::Fail(status);
  }
}

Result<File> FileSystem::Open(const Path &path) noexcept {
  const std::string clean_path = path.AbsolutePath();
  std::optional<int> handle = handle_statement_.Execute<int, std::string_view, int>(
	  clean_path,
	  static_cast<int>(File::Type)
  );

  if (handle.has_value()) {
	return Result<File>::Ok(handle.value());
  } else {
	return Result<File>::Fail(errors::Io::FileNotFound);
  }
}

Result<FileSystem::Chunk> FileSystem::Read(const File &file, int start, int length) const {
  util::ContinuousReader reader(length, start);
  auto error = this->Read(file, &reader, start);
  if (!error) {
	return Result<FileSystem::Chunk>::Ok(util::ContinuousReader::Release(std::move(reader)));
  } else {
	return Result<FileSystem::Chunk>::Fail(error.value());
  }
}

std::optional<Error> FileSystem::Read(const File &file,
									  int start,
									  int length,
									  std::function<bool(Chunk &&)> callback) const {
  util::ChunkReader reader(length, [&](auto &&chunk) {
	return callback(std::forward<decltype(chunk)>(chunk)) ? Status() : Status::Aborted();
  }, start);

  // Read the chunks. Aborting by user is not a error worth reporting.
  auto error = this->Read(file, &reader, start);
  if (!error || error.value() == Error(Status::Aborted())) {
	return std::nullopt;
  } else {
	return error;
  }

}

Result<File> FileSystem::Create(const Path &path,
								std::function<sqlite::Status(sqlite::Database::RowId, int)> file_creation,
								int file_size,
								int chunk_size) {
  // Define a appropriate chunk size
  if (chunk_size <= 0 || chunk_size > file_size) {
	chunk_size = file_size;
  }
  if (chunk_size >= database_.MaximalDataSize()) {
	chunk_size = database_.MaximalDataSize() - 64;
  }

  // Open a transaction ensuring the correct content
  auto transaction = Transaction::Open(&database_);
  if (!transaction) {
	return Result<File>::Fail(static_cast<Status>(transaction));
  }

  // Create the header entry and get the file handle
  auto header_container = this->CreateHeader(path, chunk_size, File::Type);
  if (!header_container) {
	auto status = static_cast<Status>(header_container);
	if (status.ConstraintViolated()) {
	  return Result<File>(Error(errors::Io::FileExists));
	} else {
	  return Result<File>::Fail(status);
	}
  }

  // Create the actual file and fail if that was not sucessfull
  auto file = std::get<sqlite::Database::RowId>(header_container);
  const Status status = file_creation(file, chunk_size);
  if (!status) {
	return Result<File>::Fail(status);
  }

  // Ensure data is correctly written into the database
  const Status is_commit_succesful = transaction->Commit();
  if (!is_commit_succesful) {
	return Result<File>::Fail(status);
  }

  return Result<File>::Ok(file);
}

Result<File> FileSystem::Create(const Path &path, FileSystem::Chunk &&data, int proposed_chunk_size) {
  return this->Create(path, [&](sqlite::Database::RowId file_id, int chunk_size) {
	// Write the data to SQlite, most efficiently if it is only a single chunk
	Status status;
	if (chunk_size == data.Size()) {
	  status = blob_statement_([&](Query &query) {
		return query.Set(0, file_id)
			.Than([&]() {
			  return query.Set(1, 0);
			}).Than([&]() {
			  return query.Set(2, std::move(data));
			}).Than(query);
	  });
	} else {
	  for (int part_index = 0, c = 0, size = data.Size(); part_index < size && status; part_index += chunk_size, ++c) {
		status = blob_statement_([&](Query &query) {
		  return query.Set(0, file_id)
			  .Than([&]() {
				return query.Set(1, c);
			  }).Than([&]() {
				return query.Set(2, data.Part(std::min(chunk_size, size - part_index), part_index));
			  }).Than(query);
		});
	  }
	}
	return status;
  }, data.Size(), proposed_chunk_size);
}

Result<File> FileSystem::Create(const Path &path,
								std::function<Chunk(int)> data_source,
								int file_size,
								int proposed_chunk_size) {
  return this->Create(path, [&](sqlite::Database::RowId file_id, int chunk_size) {
	util::Cache cache;
	int bytes_written = 0, chunk_num = 0;
	Status result = Status();

	while (result && bytes_written < file_size) {
	  const int required_bytes = std::min(chunk_size, file_size - bytes_written);
	  auto chunk = data_source(required_bytes);

	  // Callback might be aborted at any time
	  if (!chunk) {
		result = Status::Aborted();
		break;
	  }

	  // The optimal case: No data cached, new chunk of optimal size -> no copy involved
	  if (chunk.size() == required_bytes && !cache) {
		result = blob_statement_([&](Query &query) {
		  return query.Set(0, file_id)
			  .Than([&]() {
				return query.Set(1, chunk_num++);
			  }).Than([&]() {
				return query.Set(2, std::move(chunk));
			  }).Than(query);
		});
		bytes_written += required_bytes;
		continue;
	  }

	  // Place the chunk on the cache and use it
	  cache.Push(std::move(chunk));
	  if (cache.Size() >= required_bytes) {
		result = blob_statement_([&](Query &query) {
		  return query.Set(0, file_id)
			  .Than([&]() {
				return query.Set(1, chunk_num++);
			  }).Than([&]() {
				return query.Set(2, cache.Pop(required_bytes));
			  }).Than(query);
		});
		bytes_written += required_bytes;
	  }
	}

	return result;
  }, file_size, proposed_chunk_size);
}

Result<File> FileSystem::Create(const Path &path, std::string_view file_path, int chunk_size) {
  std::ifstream file(file_path.data(), std::ifstream::in | std::ifstream::binary);
  if (file) {
	// Get file length
	file.seekg(0, std::ifstream::end);
	const int length = file.tellg();
	file.seekg(0, std::ifstream::beg);
	if (!file) {
	  return Result<File>::Fail(errors::Io::ReadingError);
	}

	// Copy the file chunkwise into the buffer
	auto result = this->Create(path, [&](int chunk_size) {
	  Chunk data(chunk_size);
	  file.read(reinterpret_cast<char *>(data.Data()), chunk_size);
	  if (file.gcount() == chunk_size) {
		return data;
	  } else {
		return Chunk();
	  }
	}, length, chunk_size);

	// Check if file was read successfully
	Error *error = nullptr;
	if ((error = std::get_if<Error>(&result)) != nullptr) {
	  Status *code = nullptr;
	  if ((code = std::get_if<Status>(error)) != nullptr && *code == Status::Aborted()) {
		return Result<File>::Fail(errors::Io::ReadingError);
	  }
	}

	return result;
  } else {
	return Result<File>::Fail(errors::Io::FileNotFound);
  }
}

sqlite::Result<sqlite::Database::RowId, sqlite::Status> FileSystem::CreateHeader(const Path &path,
																				 int chunk_size,
																				 FileSystemObjectType type) noexcept {
  sqlite::Database::RowId id = -1;
  const Status status = header_statement_([&](Query &query) {
	return query.Set(0, path.AbsolutePath())
		.Than([&]() { return query.Set(1, static_cast<int>(type)); })
		.Than([&]() { return query.Set(2, chunk_size); })
		.Than(query)
		.Than([&]() {
		  id = database_.LastInsertedRow();
		  return Status();
		});
  });

  if (status) {
	return sqlite::Result<sqlite::Database::RowId, sqlite::Status>::Ok(id);
  } else {
	return sqlite::Result<sqlite::Database::RowId, sqlite::Status>::Fail(status);
  }
}

void FileSystem::Find(const Path &path, std::vector<Path> &files) const noexcept {
  std::string full_path = path.AbsolutePath();
  glob_statement_([&](Query &query) {
	query.Set(0, full_path);
	query.Set(1, File::Type);
	while (query().DataAvailable()) {
	  files.emplace_back(query.Get<std::string_view>(0));
	}
	return Status();
  });
}

int FileSystem::Size(const File &file) {
  return size_statement_.Execute<int>(file.Handle()).value();
}

std::optional<Error> FileSystem::Read(const File &file, util::Reader *reader, int start) const {
  // Load the chunks
  const auto chunk_status = chunk_statement_([&](Query &query) {
	return query.SetByName(":handle", file.Handle())
		.Than([&query, start] {
		  return query.SetByName(":index", start);
		}).Than([&query, &reader] {
		  return query.SetByName(":size", reader->Length());
		}).Than([&query, &reader] {
		  return reader->Add(query);
		});
  });

  if (!chunk_status) {
	return Error(chunk_status);
  } else if (reader->First() == -1) {
	return Error(errors::Io::OutOfBounds);
  }

  // Try to read the first blob
  auto first_blob = BlobReader::Open(database_, reader->First(), meta_.Data(), "data");
  if (first_blob) {
	reader->SetFirstBlob(std::move(std::get<BlobReader>(first_blob)));
  } else {
	return Error(std::get<Status>(first_blob));
  }

  // Read the blobs sequentially
  do {
	auto tmp_result = (*reader)();
	if (tmp_result.has_value()) {
	  return tmp_result.value();
	}
  } while (!*reader);
  return std::nullopt;
}

std::optional<Error> FileSystem::Read(const File &file,
									  std::string_view file_path,
									  int start,
									  int length,
									  bool truncate) const {
  std::ofstream output_file(file_path.data(),
							std::ifstream::out | std::ifstream::binary
								| (truncate ? std::ifstream::trunc : std::ifstream::app));
  if (output_file) {
	auto result = this->Read(file, start, length, [&](Chunk data) {
	  output_file.write(reinterpret_cast<const char *>(data.Data()), data.Size());
	  return static_cast<bool>(output_file);
	});

	// Aborting does not count as error, we have to set it manually
	if (!result.has_value() && !output_file) {
	  return Error(errors::Io::WritingError);
	}
	return result;
  } else {
	return Error(errors::Io::FileCreationFailed);
  }
}
}
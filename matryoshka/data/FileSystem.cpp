//
// Created by christopher on 19.03.2020.
//

#include "FileSystem.h"
#include "sqlite/PreparedStatement.h"
#include "sqlite/BlobReader.h"
#include "sqlite/Transaction.h"
#include "util/ContinuousReader.h"

#include <cassert>
#include <sstream>
#include <utility>
#include <numeric>
#include <algorithm>

using namespace matryoshka::data::sqlite;

namespace matryoshka::data {

FileSystem::FileSystem(sqlite::Database &&database,
					   sqlite::PreparedStatement &&handle_statement,
					   sqlite::PreparedStatement &&chunk_statement,
					   sqlite::PreparedStatement &&header_statement,
					   sqlite::PreparedStatement &&blob_statement,
					   util::MetaTable meta_table) noexcept
	: database_(std::move(database)),
	  handle_statement_(std::move(handle_statement)),
	  chunk_statement_(std::move(chunk_statement)),
	  header_statement_(std::move(header_statement)),
	  blob_statement_(std::move(blob_statement)),
	  meta_(std::move(meta_table)) {
  assert(handle_statement_ && chunk_statement_ && header_statement_ && blob_statement_);
}

FileSystem::FileSystem(FileSystem &&other) noexcept: database_(std::move(other.database_)),
													 handle_statement_(std::move(other.handle_statement_)),
													 chunk_statement_(std::move(other.chunk_statement_)),
													 header_statement_(std::move(other.header_statement_)),
													 blob_statement_(std::move(other.blob_statement_)),
													 meta_(std::move(other.meta_)) {
}

Result<FileSystem> FileSystem::Open(sqlite::Database &&database) noexcept {
  static constexpr std::string_view SQL_CREATE_META =
	  "CREATE TABLE {meta} (id INTEGER PRIMARY KEY, path TEXT UNIQUE NOT NULL, type INTEGER, flags INTEGER, chunk_size INTEGER NOT NULL)";
  static constexpr std::string_view SQL_CREATE_DATA =
	  "CREATE TABLE IF NOT EXISTS {data} (chunk_id INTEGER PRIMARY KEY, file_id INTEGER NOT NULL, chunk_num INTEGER NOT NULL, data BLOB NOT NULL, CONSTRAINT unq UNIQUE (file_id, chunk_num), FOREIGN KEY(file_id) REFERENCES {meta} (id))";
  static constexpr std::string_view SQL_GET_HANDLE = "SELECT id FROM {meta} WHERE path = ? AND type = ?";

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
  auto chunk_statement = util::ContinuousReader::PrepareStatement(database, meta[0]);
  auto insert_header_statement =
	  sqlite::PreparedStatement::Insert(database, meta[0].Meta(), {"path", "type", "chunk_size"});
  auto insert_blob_statement =
	  sqlite::PreparedStatement::Insert(database, meta[0].Data(), {"file_id", "chunk_num", "data"});

  Status status =
	  sqlite::Result<>::Check(handle_statement, chunk_statement, insert_header_statement, insert_blob_statement);
  if (status) {
	// Protected constructor enforce external setup
	return Result<FileSystem>(FileSystem(std::move(database),
										 sqlite::Result<>::Get(std::move(handle_statement)),
										 sqlite::Result<>::Get(std::move(chunk_statement)),
										 sqlite::Result<>::Get(std::move(insert_header_statement)),
										 sqlite::Result<>::Get(std::move(insert_blob_statement)),
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

  // Load the chunks
  const auto chunk_status = chunk_statement_([&](Query &query) {
	return query.SetByName(":handle", file.Handle())
		.Than([&query, start] {
		  return query.SetByName(":index", start);
		}).Than([&query, length] {
		  return query.SetByName(":size", length);
		}).Than([&query, &reader] {
		  return reader.Add(query);
		});
  });

  if (!chunk_status) {
	return Result<FileSystem::Chunk>::Fail(chunk_status);
  } else if (reader.First() == -1) {
	return Result<FileSystem::Chunk>::Fail(errors::Io::OutOfBounds);
  }

  // Read the blobs sequentially
  if (reader(BlobReader::Open(database_, reader.First(), meta_.Data(), "data"))) {
	return Result<FileSystem::Chunk>::Ok(util::ContinuousReader::Release(std::move(reader)));
  } else {
	return Result<FileSystem::Chunk>::Fail(reader.Error());
  }
}

Result<File> FileSystem::Create(const Path &path, FileSystem::Chunk &&data, int chunk_size) {
  // Define a appropriate chunk size
  if (chunk_size <= 0 || chunk_size > data.Size()) {
	chunk_size = data.Size();
  }
  if (chunk_size >= database_.MaximalDataSize()) {
	chunk_size = database_.MaximalDataSize() - 64;
  }

  // Open a transaction ensuring the correct content
  auto transaction = Transaction::Open(&database_);
  if (!transaction) {
	return Result<File>::Fail(static_cast<Status>(transaction));
  }

  // Create the header enty and get the file handle
  auto header_container = this->CreateHeader(path, chunk_size, File::Type);
  if (!header_container) {
	auto status = static_cast<Status>(header_container);
	return status.ConstraintViolated() ? Result<File>(Error(errors::Io::FileExists)) : Result<File>::Fail(status);
  }
  auto file_id = sqlite::Result<>::Get(std::move(header_container));

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
  if (!status) {
	return Result<File>::Fail(status);
  }

  // Ensure data is correctly written into the database
  const Status is_commit_succesful = transaction->Commit();
  if (!is_commit_succesful) {
	return Result<File>::Fail(status);
  }

  return Result<File>::Ok(file_id);
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

  return status ? sqlite::Result<sqlite::Database::RowId, sqlite::Status>::Ok(id)
				: sqlite::Result<sqlite::Database::RowId, sqlite::Status>::Fail(status);
}

}
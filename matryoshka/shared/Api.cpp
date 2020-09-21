/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Api.h"

#include "../data/FileSystem.h"

struct FileSystem {
  matryoshka::data::FileSystem file_system_;
  explicit FileSystem(matryoshka::data::FileSystem &&file_system) : file_system_(std::move(file_system)) {}
};

struct Status {
  matryoshka::data::Error error_;
  explicit Status(matryoshka::data::Error error) : error_(error) {}
};

struct FileHandle {
  matryoshka::data::File file_;
  explicit FileHandle(matryoshka::data::File &&file) : file_(std::move(file)) {}
};

void DestroyFileSystem(FileSystem *file_system) {
  delete file_system;
}

void DestroyStatus(Status *status) {
  delete status;
}

void DestroyFileHandle(FileHandle *file_handle) {
  delete file_handle;
}

template<typename R, typename T>
inline R *HandleError(Status **status, T
value) {
  if (status != nullptr) {
	*status = new Status(matryoshka::data::Error(value));
  }
  return nullptr;
}

template<typename R, typename T>
inline R *HandleError(Status **status,
					  matryoshka::data::sqlite::Result<T, matryoshka::data::sqlite::Status> &&value) {
  if (status != nullptr) {
	*status = new Status(matryoshka::data::Error(std::get<matryoshka::data::sqlite::Status>(value)));
  }
  return nullptr;
}

template<typename R, typename T>
inline R *HandleError(Status **status, matryoshka::data::Result<T> &&value) {
  if (status != nullptr) {
	*status = new Status(std::get<matryoshka::data::Error>(value));
  }
  return nullptr;
}

FileSystem *Load(const char *path, Status **status) {
  auto database = matryoshka::data::sqlite::Database::Create(path);
  if (!database) {
	return HandleError<FileSystem>(status, std::move(database));
  }

  auto file_system =
	  matryoshka::data::FileSystem::Open(std::move(std::get<matryoshka::data::sqlite::Database>(database)));
  if (!file_system) {
	return HandleError<FileSystem>(status, std::move(file_system));
  }
  return new FileSystem(std::move(std::move(std::get<matryoshka::data::FileSystem>(file_system))));
}

const char *GetMessage(Status *status) {
  if (status == nullptr) {
	return "";
  }
  return matryoshka::data::Error::Message(matryoshka::data::Error(status->error_)).data();
}

FileHandle *Open(FileSystem *file_system,
				 const char *path,
				 Status **status) {
  if (file_system == nullptr || path == nullptr) {
	return HandleError<FileHandle>(status, matryoshka::data::errors::ArgumentError());
  }

  auto parsed_path = matryoshka::data::Path(path);
  auto result = file_system->file_system_.Open(parsed_path);
  if (!result) {
	return HandleError<FileHandle>(status, std::move(result));
  }

  return new FileHandle(std::move(std::get<matryoshka::data::File>(result)));
}

FileHandle *Push(FileSystem *file_system,
				 const char *inner_path,
				 const char *file_path,
				 int chunk_size,
				 Status
				 **status) {
  if (file_system == nullptr || inner_path == nullptr || file_path == nullptr) {
	return HandleError<FileHandle>(status, matryoshka::data::errors::ArgumentError());
  }

  const auto path = matryoshka::data::Path(inner_path);
  auto result = file_system->file_system_.Create(path, file_path, chunk_size);
  if (!result) {
	return HandleError<FileHandle>(status, std::move(result));
  }

  return new FileHandle(std::move(std::get<matryoshka::data::File>(result)));
}

Status *Pull(FileSystem *file_system, FileHandle *file, const char *file_path) {
  if (file_system == nullptr || file == nullptr || file_path == nullptr || !static_cast<bool>(file->file_)) {
	return new Status(matryoshka::data::Error(matryoshka::data::errors::ArgumentError()));
  }

  const auto length = file_system->file_system_.Size(file->file_);
  auto result = file_system->file_system_.Read(file->file_, file_path, 0, length);
  return result ? new Status(result.value()) : nullptr;
}

int Find(FileSystem *file_system, const char *path, void (*callback)(const char *)) {
  if (file_system == nullptr) {
	return 0;
  }

  std::vector<matryoshka::data::Path> paths;
  if (path == nullptr) {
	file_system->file_system_.Find(paths);
  } else {
	const auto parsed_path = matryoshka::data::Path(path);
	file_system->file_system_.Find(parsed_path, paths);
  }

  for (auto &found_path: paths) {
	callback(found_path.AbsolutePath().c_str());
  }

  return paths.size();
}

int GetSize(FileSystem *file_system, FileHandle *file) {
  if (file == nullptr || file_system == nullptr || !static_cast<bool>(file->file_)) {
	return 0;
  }
  return file_system->file_system_.Size(file->file_);
}

int Delete(FileSystem *file_system, FileHandle *file) {
  return (file == nullptr || file_system == nullptr || !static_cast<bool>(file->file_)
	  || !file_system->file_system_.Delete(std::move(file->file_))
		 ) ? 0 : 1;
}


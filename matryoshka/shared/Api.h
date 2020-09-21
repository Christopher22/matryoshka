/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MATRYOSHKA_MATRYOSHKA_SHARED_API_H_
#define MATRYOSHKA_MATRYOSHKA_SHARED_API_H_

#include <matryoshka_export.h>

extern "C" {
struct FileSystem;
struct Status;
struct FileHandle;

/**
 * Open a SQlite database containing the Matryoshka virtual file system.
 * @param path The path to the Matryoshka SQlite database.
 * @param status Contains the error code of the failure if and only if the return value is nullptr. Setting this value to nullptr is safe and will not save the error code.
 * @return A pointer to the virtual file system or nullptr on failure.
 */
MATRYOSHKA_EXPORT FileSystem *Load(const char *path, Status **status);

/**
 * Destroy a file system.
 * @param file_system The virtual file system. Passing nullptr is a safe no-op.
 */
MATRYOSHKA_EXPORT void DestroyFileSystem(FileSystem *file_system);

/**
 * Destroy a status.
 * @param status The status. Passing nullptr is a safe no-op.
 */
MATRYOSHKA_EXPORT void DestroyStatus(Status *status);

/**
 * Destroy a file handle.
 * @param file_handle The file handle. Passing nullptr is a safe no-op.
 */
MATRYOSHKA_EXPORT void DestroyFileHandle(FileHandle *file_handle);

/**
 * Return the error message associated with a status.
 * @param status The status of interest.
 * @return A human-readable description of the failure.
 */
MATRYOSHKA_EXPORT const char *GetMessage(Status *status);

/**
 * Open a existing file on the virtual file system.
 * @param file_system A pointer to the virtual file system.
 * @param path The (inner) path on the virtual file system (mind the forward slashes as separators!)
 * @param status Contains the error code of the failure if and only if the return value is nullptr. Setting this value to nullptr is safe and will not save the error code.
 * @return A handle to the file or nullptr at failure.
 */
MATRYOSHKA_EXPORT FileHandle *Open(FileSystem *file_system, const char *path, Status **status);

/**
 * Push a file to the virtual file system.
 * @param file_system A pointer to the virtual file system.
 * @param inner_path The inner path on the virtual file system (mind the forward slashes as separators!)
 * @param file_path The path on the real file system.
 * @param chunk_size The proposed chunk size. Negative values will let the virtual file system choose.
 * @param status Contains the error code of the failure if and only if the return value is nullptr. Setting this value to nullptr is safe and will not save the error code.
 * @return A handle to the newly created file or nullptr on failure.
 */
MATRYOSHKA_EXPORT FileHandle *Push(FileSystem *file_system,
								   const char *inner_path,
								   const char *file_path,
								   int chunk_size,
								   Status **status);

/**
 * Pull a file from the database into the virtual file system.
 * @param file_system A pointer to the virtual file system.
 * @param inner_path The inner path on the virtual file system (mind the forward slashes as separators!)
 * @param file_path The path on the real file system.
 * @return A error ocurring during operation or nullptr on success.
 */
MATRYOSHKA_EXPORT Status *Pull(FileSystem *file_system,
							   FileHandle *inner_path,
							   const char *file_path);

/**
 * Search for a specific file(s).
 * @param file_system A pointer to the virtual file system.
 * @param path The path supporting glob-like palceholders.
 * @param callback A callback for each path found.
 * @return The number of paths found.
 */
MATRYOSHKA_EXPORT int Find(FileSystem *file_system,
						   const char *path,
						   void (*callback)(const char *));

/**
 * Returns the size of a file.
 * @param file_system A pointer to the virtual file system.
 * @param file A handle to the file.
 * @return File size in bytes.
 */
MATRYOSHKA_EXPORT int GetSize(FileSystem *file_system, FileHandle *file);

/**
 * Delete a file. The file handle must not be used after the call but still needs to be freed.
 * @param file_system A pointer to the virtual file system.
 * @param file A handle to the file.
 * @return 1 if operation was successful, 0 otherwise.
 */
MATRYOSHKA_EXPORT int Delete(FileSystem *file_system, FileHandle *file);

};

#endif //MATRYOSHKA_MATRYOSHKA_SHARED_API_H_

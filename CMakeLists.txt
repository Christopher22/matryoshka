cmake_minimum_required(VERSION 3.15)
project(Matryoshka)
include(CTest)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_TESTING OFF)
set(BUILD_CLI ON)

# Specify the dependencies
set(MATRYOSHKA_DEPENDENCIES sqlite3/3.31.1)
if (BUILD_TESTING)
    list(APPEND MATRYOSHKA_DEPENDENCIES doctest/2.3.7)
endif ()
if (BUILD_CLI)
    list(APPEND MATRYOSHKA_DEPENDENCIES CLI11/1.9.0@cliutils/stable)
endif ()

# Use conan with cmake
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_check(REQUIRED)
conan_cmake_run(REQUIRES ${MATRYOSHKA_DEPENDENCIES} BASIC_SETUP CMAKE_TARGETS BUILD missing)

# Build Matryoshka library
add_library(Matryoshka matryoshka/data/sqlite/Database.cpp matryoshka/data/sqlite/Database.h matryoshka/data/sqlite/PreparedStatement.cpp matryoshka/data/sqlite/PreparedStatement.h matryoshka/data/sqlite/Query.cpp matryoshka/data/sqlite/Query.h matryoshka/data/sqlite/Blob.h matryoshka/data/sqlite/Status.h matryoshka/data/sqlite/Status.cpp matryoshka/data/sqlite/BlobReader.cpp matryoshka/data/sqlite/BlobReader.h matryoshka/data/Path.cpp matryoshka/data/Path.h matryoshka/data/FileSystemObject.h matryoshka/data/File.h matryoshka/data/Folder.h matryoshka/data/util/MetaTable.cpp matryoshka/data/util/MetaTable.h matryoshka/data/sqlite/Result.h matryoshka/data/sqlite/Transaction.cpp matryoshka/data/sqlite/Transaction.h matryoshka/data/Error.cpp matryoshka/data/Error.h matryoshka/data/util/ContinuousReader.cpp matryoshka/data/util/ContinuousReader.h matryoshka/data/FileSystem.cpp matryoshka/data/FileSystem.h matryoshka/data/util/Reader.cpp matryoshka/data/util/Reader.h matryoshka/data/util/ChunkReader.cpp matryoshka/data/util/ChunkReader.h)
target_link_libraries(Matryoshka CONAN_PKG::sqlite3)

# Build tests, if required
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    MESSAGE(STATUS "Build tests")

    add_executable(MatryoshkaTest tests/main.cpp tests/Sqlite.h tests/MetaTable.h tests/FileSystem.h)
    target_link_libraries(MatryoshkaTest Matryoshka CONAN_PKG::doctest)
    add_test(NAME MatryoshkaTestCmake COMMAND MatryoshkaTest)
endif ()

# Build command line interface, if required
if (BUILD_CLI)
    add_executable(MatryoshkaCLI matryoshka/cli.cpp)
    target_link_libraries(MatryoshkaCLI Matryoshka CONAN_PKG::CLI11)
endif ()
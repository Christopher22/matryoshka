#[[
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
#]]

cmake_minimum_required(VERSION 3.15)
project(Matryoshka VERSION 0.2.0.0 DESCRIPTION "Embedding files into SQLite databases" HOMEPAGE_URL "https://github.com/Christopher22/matryoshka")
set(Matryoshka_AUTHOR "Christopher Gundler")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Check if link time optimization might be enabled
include(CheckIPOSupported)
check_ipo_supported(RESULT INTERPROCEDURAL_IS_SUPPORTED)
if (INTERPROCEDURAL_IS_SUPPORTED)
    message(STATUS "Using interprocedual optimizations")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif ()

# Create a nice build dir without any cache files on CMake
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Matryoshka_${Matryoshka_VERSION_MAJOR}_${Matryoshka_VERSION_MINOR}_${Matryoshka_VERSION_PATCH}_${Matryoshka_VERSION_TWEAK}")

option(BUILD_TESTING "Build tests" OFF)
option(BUILD_CLI "Build CLI client" OFF)
option(BUILD_WEBDAV "Build WebDAV server" OFF)
option(BUILD_SHARED "Build shared library" ON)

# Use conan with cmake
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_check(REQUIRED)

# Specify the dependencies
set(MATRYOSHKA_DEPENDENCIES sqlite3/3.31.1)
if (BUILD_TESTING)
    list(APPEND MATRYOSHKA_DEPENDENCIES doctest/2.3.7)
endif ()
if (BUILD_CLI)
    list(APPEND MATRYOSHKA_DEPENDENCIES CLI11/1.9.0@cliutils/stable)
endif ()
if (BUILD_WEBDAV)
    conan_add_remote(NAME stiffstream URL https://api.bintray.com/conan/stiffstream/public)
    list(APPEND MATRYOSHKA_DEPENDENCIES restinio/0.6.6@stiffstream/stable CLI11/1.9.0@cliutils/stable)
endif ()
conan_cmake_run(REQUIRES ${MATRYOSHKA_DEPENDENCIES} BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS BUILD missing)

# Build Matryoshka library
add_library(Matryoshka matryoshka/data/sqlite/Database.cpp matryoshka/data/sqlite/Database.h matryoshka/data/sqlite/PreparedStatement.cpp matryoshka/data/sqlite/PreparedStatement.h matryoshka/data/sqlite/Query.cpp matryoshka/data/sqlite/Query.h matryoshka/data/sqlite/Blob.h matryoshka/data/sqlite/Status.h matryoshka/data/sqlite/Status.cpp matryoshka/data/sqlite/BlobReader.cpp matryoshka/data/sqlite/BlobReader.h matryoshka/data/Path.cpp matryoshka/data/Path.h matryoshka/data/FileSystemObject.h matryoshka/data/File.h matryoshka/data/Folder.h matryoshka/data/util/MetaTable.cpp matryoshka/data/util/MetaTable.h matryoshka/data/sqlite/Result.h matryoshka/data/sqlite/Transaction.cpp matryoshka/data/sqlite/Transaction.h matryoshka/data/Error.cpp matryoshka/data/Error.h matryoshka/data/util/ContinuousReader.cpp matryoshka/data/util/ContinuousReader.h matryoshka/data/FileSystem.cpp matryoshka/data/FileSystem.h matryoshka/data/util/Reader.cpp matryoshka/data/util/Reader.h matryoshka/data/util/ChunkReader.cpp matryoshka/data/util/ChunkReader.h matryoshka/data/util/Cache.cpp matryoshka/data/util/Cache.h)
target_link_libraries(Matryoshka CONAN_PKG::sqlite3)
set_target_properties(Matryoshka PROPERTIES PREFIX "static_")

# Build tests, if required
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    include(CTest)
    MESSAGE(STATUS "Building tests")

    add_executable(MatryoshkaTest tests/main.cpp tests/Sqlite.h tests/MetaTable.h tests/FileSystem.h tests/Cache.h)
    target_link_libraries(MatryoshkaTest Matryoshka CONAN_PKG::doctest)
    add_test(NAME CMakeMatryoshkaTest COMMAND MatryoshkaTest WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif ()

# Build command line interface, if required
if (BUILD_CLI)
    MESSAGE(STATUS "Building CLI")

    add_executable(MatryoshkaCLI matryoshka/cli.cpp)
    target_link_libraries(MatryoshkaCLI Matryoshka CONAN_PKG::CLI11)
endif ()

# Build library for the command language runtime, if required
if (BUILD_SHARED)
    MESSAGE(STATUS "Building shared library")

    include(GenerateExportHeader)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

    # Write the set the current version of the DLL
    configure_file(matryoshka/shared/resources.rc.in ${CMAKE_CURRENT_BINARY_DIR}/resources.rc @ONLY)

    add_library(MatryoshkaShared SHARED ${CMAKE_CURRENT_BINARY_DIR}/resources.rc matryoshka/shared/Api.cpp matryoshka/shared/Api.h)
    target_link_libraries(MatryoshkaShared Matryoshka)
    set_target_properties(MatryoshkaShared PROPERTIES OUTPUT_NAME "Matryoshka")
    generate_export_header(MatryoshkaShared BASE_NAME Matryoshka)
    # Required for 'matryoshka_export.h'
    target_include_directories(MatryoshkaShared PRIVATE ${PROJECT_BINARY_DIR})
endif ()

# Build WebDAV, if required
if (BUILD_WEBDAV)
    MESSAGE(STATUS "Building web server")

    # ASIO wants the current windows version - set it.
    # Adopted from https://stackoverflow.com/questions/9742003/platform-detection-in-cmake
    if (WIN32)
        macro(get_WIN32_WINNT version)
            if (CMAKE_SYSTEM_VERSION)
                set(ver ${CMAKE_SYSTEM_VERSION})
                string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
                string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
                # Check for Windows 10, b/c we'll need to convert to hex 'A'.
                if ("${verMajor}" MATCHES "10")
                    set(verMajor "A")
                    string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
                endif ()
                # Remove all remaining '.' characters.
                string(REPLACE "." "" ver ${ver})
                # Prepend each digit with a zero.
                string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
                set(${version} "0x${ver}")
            endif ()
        endmacro()
        get_WIN32_WINNT(ver)
        add_definitions(-D_WIN32_WINNT=${ver})
    endif ()

    add_executable(MatryoshkaServer matryoshka/server/main.cpp matryoshka/server/Server.cpp matryoshka/server/Server.h)
    target_link_libraries(MatryoshkaServer Matryoshka CONAN_PKG::restinio CONAN_PKG::CLI11)
endif ()
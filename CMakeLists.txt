cmake_minimum_required(VERSION 3.15)
project(Matryoshka)
include(CTest)

set(CMAKE_CXX_STANDARD 17)

# Use conan with cmake
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()
include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_check(REQUIRED)
conan_cmake_run(REQUIRES sqlite3/3.31.1 doctest/2.3.7 BASIC_SETUP CMAKE_TARGETS BUILD missing)

# Build Matryoshka library
add_library(Matryoshka matryoshka/data/sqlite/Database.cpp matryoshka/data/sqlite/Database.h nk.cpp matryoshka/data/sqlite/PreparedStatement.cpp matryoshka/data/sqlite/PreparedStatement.h matryoshka/data/sqlite/Query.cpp matryoshka/data/sqlite/Query.h matryoshka/data/sqlite/Blob.h matryoshka/data/sqlite/Status.h matryoshka/data/sqlite/Status.cpp)
target_link_libraries(Matryoshka CONAN_PKG::sqlite3)

# Build tests, if required
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    MESSAGE(STATUS "Build tests")

    add_executable(MatryoshkaTest tests/main.cpp tests/sqlite.h)
    target_link_libraries(MatryoshkaTest Matryoshka CONAN_PKG::doctest)
    add_test(NAME MatryoshkaTestCmake COMMAND MatryoshkaTest)
else ()
    add_executable(MatryoshkaExecutable matryoshka/main.cpp)
    target_link_libraries(MatryoshkaExecutable Matryoshka)
endif ()
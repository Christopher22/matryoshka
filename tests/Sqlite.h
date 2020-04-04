//
// Created by christopher on 24.03.2020.
//

#ifndef MATRYOSHKA_TESTS_SQLITE_H_
#define MATRYOSHKA_TESTS_SQLITE_H_

#include <doctest/doctest.h>

#include "../matryoshka/data/sqlite/Database.h"
#include "../matryoshka/data/sqlite/PreparedStatement.h"
#include "../matryoshka/data/sqlite/Query.h"
#include "../matryoshka/data/sqlite/Status.h"
#include "../matryoshka/data/sqlite/BlobReader.h"

using namespace matryoshka::data::sqlite;

TEST_SUITE ("SQLite") {
TEST_CASE ("Testing database") {
  auto database_creation = Database::Create(":memory:");

	  REQUIRE_MESSAGE(
	  std::holds_alternative<Database>(database_creation),
	  std::get<Status>(database_creation).Message()
  );
  Database data = std::move(std::get<Database>(database_creation));

  const Status status_table_creation =
	  data("CREATE TABLE test (id integer PRIMARY KEY, name text NOT NULL, concurrency double, data blob)");
	  REQUIRE_MESSAGE(status_table_creation, status_table_creation.Message()
  );

	  SUBCASE("IO") {
	const std::string EXAMPLE_STRING(R"(Max\0 Mustermann)");
	const int EXAMPLE_INT = 32;
	const double EXAMPLE_DOUBLE = 4.321;
	auto EXAMPLE_BLOB = Blob<true>::Filled(32);
	static_cast<unsigned char *>(EXAMPLE_BLOB)[7] = 42;

	// Write data
	{
	  auto statement_container = PreparedStatement::Create(data, "INSERT INTO test VALUES(?, ?, ?, ?)");
		  REQUIRE(std::holds_alternative<PreparedStatement>(statement_container));

		  SUBCASE("Manual") {
			REQUIRE(std::get<PreparedStatement>(statement_container)
						([&](
							Query &query
						) {
						  CHECK(query.Set(0, EXAMPLE_INT));
						  CHECK(query.Set(1, EXAMPLE_STRING));
						  CHECK(query.Set(2, EXAMPLE_DOUBLE));
						  CHECK(query.Set(3, static_cast<Blob<false>>(EXAMPLE_BLOB)));
						  return query();
						}));
	  }

		  SUBCASE("Auto") {
		Status result = std::get<PreparedStatement>(statement_container).Execute(
			EXAMPLE_INT, EXAMPLE_STRING, EXAMPLE_DOUBLE, static_cast<Blob<false>>(EXAMPLE_BLOB)
		);
			REQUIRE(result);
	  }
	}

	// Read the data
	{
	  auto statement_container = PreparedStatement::Create(data, "SELECT id, name, concurrency, data FROM test");
		  REQUIRE(std::holds_alternative<PreparedStatement>(statement_container));

	  std::get<PreparedStatement>(statement_container)([&](Query &query) {
			REQUIRE(query());
			CHECK(query.Get<int>(0) == EXAMPLE_INT);
			CHECK(query.Get<std::string_view>(1) == EXAMPLE_STRING);
			CHECK(query.Get<std::string>(1) == EXAMPLE_STRING);
			CHECK(query.Get<double>(2) == EXAMPLE_DOUBLE);
			CHECK(query.Get<Blob<true>>(3) == EXAMPLE_BLOB);
			CHECK(query.Get<Blob<false>>(3) == EXAMPLE_BLOB);
		return Status();
	  });
	}

	// Read the data via BlobReader
	{
	  auto reader_container = BlobReader::Open(data, EXAMPLE_INT, "test", "data");
		  REQUIRE_MESSAGE(std::holds_alternative<BlobReader>(reader_container),
						  std::get<Status>(reader_container).Message());

	  BlobReader reader = std::move(std::get<BlobReader>(reader_container));
	  Blob<true> raw_blob = reader.Read(reader.Size());
		  CHECK(raw_blob == EXAMPLE_BLOB);
	}
  }
}
};

#endif //MATRYOSHKA_TESTS_SQLITE_H_

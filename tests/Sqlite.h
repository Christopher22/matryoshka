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
#include "../matryoshka/data/sqlite/Transaction.h"

using namespace matryoshka::data::sqlite;

TEST_SUITE ("SQLite") {
TEST_CASE ("Testing database") {
  // Some example data for the database
  const std::string EXAMPLE_STRING(R"(Max\0 Mustermann)");
  const int EXAMPLE_INT = 32;
  const double EXAMPLE_DOUBLE = 4.321;
  auto EXAMPLE_BLOB = Blob<true>::Filled(32);
  static_cast<unsigned char *>(EXAMPLE_BLOB)[7] = 42;

  // Create the database
  auto database_creation = Database::Create(":memory:");
	  REQUIRE_MESSAGE(
	  database_creation,
	  static_cast<Status>(database_creation).Message()
  );

  // Create the table
  Database data = std::move(std::get<Database>(database_creation));
  const Status status_table_creation =
	  data("CREATE TABLE test (id integer PRIMARY KEY, name text NOT NULL, concurrency double, data blob)");
	  REQUIRE_MESSAGE(status_table_creation, status_table_creation.Message()
  );

  // Create the insert statement
  auto insert_statement = PreparedStatement::Insert(data, "test", { "id", "name", "concurrency", "data" });
  REQUIRE(insert_statement);

  SUBCASE("IO") {
	// Write data
	{
		  SUBCASE("Manual") {
			REQUIRE(std::get<PreparedStatement>(insert_statement)
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
		Status result = insert_statement->Execute(
			EXAMPLE_INT, EXAMPLE_STRING, EXAMPLE_DOUBLE, static_cast<Blob<false>>(EXAMPLE_BLOB)
		);
			REQUIRE_MESSAGE(result, result.Message());
	  }

		  SUBCASE("Transaction") {
		// Create the transact
		Result<Transaction> transaction = Transaction::Open(&data);
			REQUIRE_MESSAGE(transaction, static_cast<Status>(transaction).Message());

		// Insert the data - it is not written by now!
		Status result = insert_statement->Execute(
			EXAMPLE_INT, EXAMPLE_STRING, EXAMPLE_DOUBLE, static_cast<Blob<false>>(EXAMPLE_BLOB)
		);
			REQUIRE_MESSAGE(result, result.Message());

		SUBCASE("Commit") {
		  // Commit
		  transaction->Commit();
		}

		/* Should fail
		SUBCASE("Rollback") {
		  // Rollback
		  transaction->Rollback();
		}

		SUBCASE("Implicit Rollback") {
		}
		*/
	  }
	}

	// Read the data
	{
	  auto statement_container = PreparedStatement::Create(data, "SELECT id, name, concurrency, data FROM test");
	  REQUIRE(statement_container);

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
		  REQUIRE_MESSAGE(reader_container, static_cast<Status>(reader_container).Message());

	  BlobReader reader = std::move(std::get<BlobReader>(reader_container));
	  Blob<true> raw_blob = reader.Read(reader.Size());
		  CHECK(raw_blob == EXAMPLE_BLOB);
	}
  }

	  SUBCASE("Transactions") {

  }
}
};

#endif //MATRYOSHKA_TESTS_SQLITE_H_

/*
This file is part of Matryoshka.
Copyright (C) 2020 Christopher Gundler <christopher@gundler.de>
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

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
TEST_CASE ("Database") {
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
  auto insert_statement = PreparedStatement::Insert(data, "test", {"id", "name", "concurrency", "data"});
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

			// Check types of result
			std::vector<Query::ValueType> values = {Query::ValueType::Integer, Query::ValueType::Text, Query::ValueType::Float, Query::ValueType::Blob};
			for(int i = 0; i < values.size(); ++i) {
			  for(int j = 0; j < values.size(); ++j) {
			    if(i == j) {
			      CHECK(query.Type(j) == values[i]);
			    } else {
			      CHECK(query.Type(j) != values[i]);
			    }
			  }
			}

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
}

TEST_CASE ("Blob") {
  auto example = Blob<true>::Filled(42, 7), example_copy = example.Copy();
  auto example_view = static_cast<Blob<false>>(example);
  Blob<true> example2(new unsigned char[42], 42);
  Blob<true> invalid;

  // Check writing access
  CHECK(!invalid);
  example2[5] = 3;
  CHECK(example2[5] == 3);

  // Check equality
  CHECK(example == example);
  CHECK(example == example_copy);
  CHECK(example == example_view);
  CHECK(example != example2);
  CHECK(example != invalid);

  // File system
  CHECK(example.Save("test.tmp", false));
  CHECK(Blob<true>("test.tmp") == example);

  // Copy from other blob
  auto other_example = Blob<true>::Filled(3, 66);
  other_example[1] = 77;

  CHECK(!example.Set(41, &other_example, 2)); // Check out-of-bounds source
  CHECK(!example.Set(38, &other_example, -1)); // Check out of bounds source (destination too small)
  CHECK(!example.Set(0, &other_example, 4));  // Check out-of-bounds destination
  CHECK(!example.Set(0, &other_example, 3, 1)); // Check out-of-bounds destination (invalid onset)

  CHECK(example.Set(1, &other_example, 2, 1));
  CHECK(example[0] == 7);
  CHECK(example[1] == 77);
  CHECK(example[2] == 66);
  CHECK(example[3] == 7);
};
}

#endif //MATRYOSHKA_TESTS_SQLITE_H_

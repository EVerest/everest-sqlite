// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 - 2025 Pionix GmbH and Contributors to EVerest

#include <database/database_exceptions.hpp>
#include <database/sqlite/sqlite_connection.hpp>
#include <database/sqlite/sqlite_statement.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

class SQLiteStatementTest : public ::testing::Test {
protected:
    std::unique_ptr<DatabaseConnection> db;

    void SetUp() override {
        fs::path db_path = "file::memory:?cache=shared";
        db = std::make_unique<DatabaseConnection>(db_path);
        ASSERT_TRUE(db->open_connection());

        ASSERT_TRUE(db->execute_statement(
            "CREATE TABLE test_table (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, value INTEGER, score REAL);"));
    }

    void TearDown() override {
        db->close_connection();
    }
};

TEST_F(SQLiteStatementTest, InsertAndQueryRow) {
    auto insert_stmt = db->new_statement("INSERT INTO test_table (name, value, score) VALUES (:name, :value, :score);");

    insert_stmt->bind_text(1, "test_name");
    insert_stmt->bind_int(2, 42);
    insert_stmt->bind_double(3, 98.6);

    ASSERT_EQ(insert_stmt->step(), SQLITE_DONE);

    auto select_stmt = db->new_statement("SELECT name, value, score FROM test_table WHERE id = 1;");
    ASSERT_EQ(select_stmt->step(), SQLITE_ROW);

    EXPECT_EQ(select_stmt->column_text(0), "test_name");
    EXPECT_EQ(select_stmt->column_int(1), 42);
    EXPECT_DOUBLE_EQ(select_stmt->column_double(2), 98.6);
}

TEST_F(SQLiteStatementTest, NullBindingAndOptionalResult) {
    auto insert_stmt = db->new_statement("INSERT INTO test_table (name, value, score) VALUES (?, ?, ?);");
    insert_stmt->bind_null(1);
    insert_stmt->bind_int(2, 100);
    insert_stmt->bind_null(3);
    ASSERT_EQ(insert_stmt->step(), SQLITE_DONE);

    auto select_stmt = db->new_statement("SELECT name, score FROM test_table WHERE id = 1;");
    ASSERT_EQ(select_stmt->step(), SQLITE_ROW);

    EXPECT_FALSE(select_stmt->column_text_nullable(0).has_value());
    EXPECT_FALSE(select_stmt->column_text_nullable(1).has_value());
}

TEST_F(SQLiteStatementTest, InvalidParameterThrows) {
    auto stmt = db->new_statement("SELECT * FROM test_table WHERE name = :name;");
    EXPECT_THROW(stmt->bind_int(":invalid", 1), std::out_of_range);
}

TEST_F(SQLiteStatementTest, ResetAndReuseStatement) {
    auto insert_stmt = db->new_statement("INSERT INTO test_table (name, value, score) VALUES (?, ?, ?);");

    insert_stmt->bind_text(1, "row1");
    insert_stmt->bind_int(2, 1);
    insert_stmt->bind_double(3, 1.1);
    ASSERT_EQ(insert_stmt->step(), SQLITE_DONE);

    ASSERT_EQ(insert_stmt->reset(), SQLITE_OK);

    insert_stmt->bind_text(1, "row2");
    insert_stmt->bind_int(2, 2);
    insert_stmt->bind_double(3, 2.2);
    ASSERT_EQ(insert_stmt->step(), SQLITE_DONE);

    auto select_stmt = db->new_statement("SELECT COUNT(*) FROM test_table;");
    ASSERT_EQ(select_stmt->step(), SQLITE_ROW);
    EXPECT_EQ(select_stmt->column_int(0), 2);
}

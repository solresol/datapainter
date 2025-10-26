#include <gtest/gtest.h>
#include "database.h"
#include <filesystem>
#include <fstream>

using namespace datapainter;

// Test that we can create an in-memory database
TEST(DatabaseTest, OpenInMemoryDatabase) {
    Database db(":memory:");
    EXPECT_TRUE(db.is_open());
    EXPECT_EQ(db.path(), ":memory:");
}

// Test that we can create a file-based database
TEST(DatabaseTest, OpenFileDatabaseCreatesNewFile) {
    const std::string test_db = "test_create.db";

    // Clean up if exists
    if (std::filesystem::exists(test_db)) {
        std::filesystem::remove(test_db);
    }

    {
        Database db(test_db);
        EXPECT_TRUE(db.is_open());
        EXPECT_EQ(db.path(), test_db);
    }

    // Verify file was created
    EXPECT_TRUE(std::filesystem::exists(test_db));

    // Clean up
    std::filesystem::remove(test_db);
}

// Test that we can open an existing database
TEST(DatabaseTest, OpenExistingDatabase) {
    const std::string test_db = "test_existing.db";

    // Create database first
    {
        Database db(test_db);
        EXPECT_TRUE(db.is_open());
    }

    // Open it again
    {
        Database db(test_db);
        EXPECT_TRUE(db.is_open());
    }

    // Clean up
    std::filesystem::remove(test_db);
}

// Test that invalid path fails gracefully
TEST(DatabaseTest, InvalidPathFailsGracefully) {
    Database db("/nonexistent/directory/cannot/create.db");
    EXPECT_FALSE(db.is_open());
    EXPECT_FALSE(db.last_error().empty());
}

// Test move constructor
TEST(DatabaseTest, MoveConstructor) {
    Database db1(":memory:");
    EXPECT_TRUE(db1.is_open());

    Database db2(std::move(db1));
    EXPECT_TRUE(db2.is_open());
    EXPECT_FALSE(db1.is_open());  // db1 should be in moved-from state
}

// Test move assignment
TEST(DatabaseTest, MoveAssignment) {
    Database db1(":memory:");
    Database db2(":memory:");

    EXPECT_TRUE(db1.is_open());
    EXPECT_TRUE(db2.is_open());

    db2 = std::move(db1);
    EXPECT_TRUE(db2.is_open());
    EXPECT_FALSE(db1.is_open());
}

// Test execute() with valid SQL
TEST(DatabaseTest, ExecuteValidSQL) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    bool result = db.execute("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT)");
    EXPECT_TRUE(result);
}

// Test execute() with invalid SQL
TEST(DatabaseTest, ExecuteInvalidSQL) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    bool result = db.execute("INVALID SQL STATEMENT");
    EXPECT_FALSE(result);
    EXPECT_FALSE(db.last_error().empty());
}

// Test execute() on closed database
TEST(DatabaseTest, ExecuteOnClosedDatabase) {
    Database db("/nonexistent/path.db");
    ASSERT_FALSE(db.is_open());

    bool result = db.execute("CREATE TABLE test (id INTEGER)");
    EXPECT_FALSE(result);
}

// Test that connection() returns valid pointer
TEST(DatabaseTest, ConnectionReturnsValidPointer) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    sqlite3* conn = db.connection();
    EXPECT_NE(conn, nullptr);
}

// Test that connection() returns nullptr for closed database
TEST(DatabaseTest, ConnectionReturnsNullForClosedDatabase) {
    Database db("/nonexistent/path.db");
    ASSERT_FALSE(db.is_open());

    sqlite3* conn = db.connection();
    EXPECT_EQ(conn, nullptr);
}

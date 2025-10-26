#include <gtest/gtest.h>
#include "database.h"
#include <sqlite3.h>

using namespace datapainter;

// Test that unsaved_changes table can be created
TEST(UnsavedChangesTableTest, CreateUnsavedChangesTable) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    bool result = db.ensure_unsaved_changes_table();
    EXPECT_TRUE(result);
}

// Test that unsaved_changes table is idempotent
TEST(UnsavedChangesTableTest, EnsureUnsavedChangesTableIsIdempotent) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    EXPECT_TRUE(db.ensure_unsaved_changes_table());
    EXPECT_TRUE(db.ensure_unsaved_changes_table());  // Should succeed again
}

// Test that unsaved_changes table has correct schema
TEST(UnsavedChangesTableTest, UnsavedChangesTableHasCorrectSchema) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    ASSERT_TRUE(db.ensure_unsaved_changes_table());

    // Query the schema
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "PRAGMA table_info(unsaved_changes)";
    int rc = sqlite3_prepare_v2(db.connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    // Expected columns
    std::vector<std::string> expected_cols = {
        "id", "table_name", "action", "data_id", "x", "y",
        "old_target", "new_target", "meta_field", "old_value", "new_value", "is_active"
    };

    std::vector<std::string> actual_cols;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* col_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        actual_cols.push_back(col_name);
    }

    sqlite3_finalize(stmt);

    EXPECT_EQ(actual_cols.size(), expected_cols.size());
    for (size_t i = 0; i < expected_cols.size() && i < actual_cols.size(); i++) {
        EXPECT_EQ(actual_cols[i], expected_cols[i]) << "Column " << i << " mismatch";
    }
}

// Test that action column has CHECK constraint
TEST(UnsavedChangesTableTest, ActionColumnHasCheckConstraint) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    ASSERT_TRUE(db.ensure_unsaved_changes_table());

    // Valid actions should work
    const char* valid_sql =
        "INSERT INTO unsaved_changes (table_name, action) VALUES ('test', 'insert')";
    EXPECT_TRUE(db.execute(valid_sql));

    const char* valid_sql2 =
        "INSERT INTO unsaved_changes (table_name, action) VALUES ('test', 'delete')";
    EXPECT_TRUE(db.execute(valid_sql2));

    const char* valid_sql3 =
        "INSERT INTO unsaved_changes (table_name, action) VALUES ('test', 'update')";
    EXPECT_TRUE(db.execute(valid_sql3));

    const char* valid_sql4 =
        "INSERT INTO unsaved_changes (table_name, action) VALUES ('test', 'meta')";
    EXPECT_TRUE(db.execute(valid_sql4));

    // Invalid action should fail
    const char* invalid_sql =
        "INSERT INTO unsaved_changes (table_name, action) VALUES ('test', 'invalid')";
    EXPECT_FALSE(db.execute(invalid_sql));
}

// Test that index exists on table_name and id
TEST(UnsavedChangesTableTest, IndexExistsOnTableNameAndId) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    ASSERT_TRUE(db.ensure_unsaved_changes_table());

    // Check for index
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='index' AND name='uc_table'";
    int rc = sqlite3_prepare_v2(db.connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    bool found_index = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        found_index = true;
    }

    sqlite3_finalize(stmt);
    EXPECT_TRUE(found_index) << "Index uc_table should exist";
}

// Test table_exists recognizes unsaved_changes
TEST(UnsavedChangesTableTest, TableExistsRecognizesUnsavedChanges) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    EXPECT_FALSE(db.table_exists("unsaved_changes"));  // Before creation

    ASSERT_TRUE(db.ensure_unsaved_changes_table());

    EXPECT_TRUE(db.table_exists("unsaved_changes"));   // After creation
}

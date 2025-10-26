#include <gtest/gtest.h>
#include "database.h"
#include <sqlite3.h>

using namespace datapainter;

// Test that metadata table can be created
TEST(MetadataTableTest, CreateMetadataTable) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    bool result = db.ensure_metadata_table();
    EXPECT_TRUE(result);
}

// Test that metadata table is idempotent (can call multiple times)
TEST(MetadataTableTest, EnsureMetadataTableIsIdempotent) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    EXPECT_TRUE(db.ensure_metadata_table());
    EXPECT_TRUE(db.ensure_metadata_table());  // Should succeed again
    EXPECT_TRUE(db.ensure_metadata_table());  // And again
}

// Test that metadata table has correct schema
TEST(MetadataTableTest, MetadataTableHasCorrectSchema) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    ASSERT_TRUE(db.ensure_metadata_table());

    // Query the schema
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "PRAGMA table_info(metadata)";
    int rc = sqlite3_prepare_v2(db.connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    // Expected columns in order
    std::vector<std::string> expected_cols = {
        "table_name", "x_axis_name", "y_axis_name", "target_col_name",
        "x_meaning", "o_meaning", "valid_x_min", "valid_x_max",
        "valid_y_min", "valid_y_max", "show_zero_bars"
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

// Test that table_name is PRIMARY KEY
TEST(MetadataTableTest, TableNameIsPrimaryKey) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    ASSERT_TRUE(db.ensure_metadata_table());

    // Query the schema for primary key info
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "PRAGMA table_info(metadata)";
    int rc = sqlite3_prepare_v2(db.connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    bool found_pk = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* col_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int pk = sqlite3_column_int(stmt, 5);  // pk column in PRAGMA table_info

        if (std::string(col_name) == "table_name" && pk == 1) {
            found_pk = true;
            break;
        }
    }

    sqlite3_finalize(stmt);
    EXPECT_TRUE(found_pk) << "table_name should be PRIMARY KEY";
}

// Test that show_zero_bars has default value of 0
TEST(MetadataTableTest, ShowZeroBarsHasDefaultValue) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    ASSERT_TRUE(db.ensure_metadata_table());

    // Insert a row without specifying show_zero_bars
    const char* sql =
        "INSERT INTO metadata (table_name, x_axis_name, y_axis_name, "
        "target_col_name, x_meaning, o_meaning) "
        "VALUES ('test', 'x', 'y', 'target', 'cat', 'dog')";

    ASSERT_TRUE(db.execute(sql));

    // Check that show_zero_bars is 0
    sqlite3_stmt* stmt = nullptr;
    const char* query = "SELECT show_zero_bars FROM metadata WHERE table_name = 'test'";
    int rc = sqlite3_prepare_v2(db.connection(), query, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    int show_zero_bars = sqlite3_column_int(stmt, 0);
    EXPECT_EQ(show_zero_bars, 0);

    sqlite3_finalize(stmt);
}

// Test table_exists helper function
TEST(MetadataTableTest, TableExistsReturnsTrueForMetadata) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    EXPECT_FALSE(db.table_exists("metadata"));  // Before creation

    ASSERT_TRUE(db.ensure_metadata_table());

    EXPECT_TRUE(db.table_exists("metadata"));   // After creation
}

// Test table_exists for non-existent table
TEST(MetadataTableTest, TableExistsReturnsFalseForNonExistent) {
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    EXPECT_FALSE(db.table_exists("nonexistent_table"));
}

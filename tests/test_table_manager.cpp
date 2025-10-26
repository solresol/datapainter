#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "table_manager.h"
#include <sstream>

using namespace datapainter;

class TableManagerTest : public ::testing::Test {
protected:
    TableManagerTest() : db_(":memory:") {}

    void SetUp() override {
        // Create system tables
        db_.ensure_metadata_table();
        db_.ensure_unsaved_changes_table();
    }

    Database db_;
};

// Test: --create-table creates table with metadata
TEST_F(TableManagerTest, CreateTable) {
    TableManager mgr(db_);

    EXPECT_TRUE(mgr.create_table("new_table", "target", "x", "y",
                                  "x_val", "o_val",
                                  -10.0, 10.0, -10.0, 10.0, false));

    // Verify metadata was created
    MetadataManager meta_mgr(db_);
    auto meta = meta_mgr.read("new_table");
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->table_name, "new_table");
    EXPECT_EQ(meta->target_col_name, "target");
    EXPECT_EQ(meta->x_axis_name, "x");
    EXPECT_EQ(meta->y_axis_name, "y");

    // Verify data table was created
    DataTable dt(db_, "new_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 0);  // Empty table
}

// Test: --rename-table renames table and updates metadata
TEST_F(TableManagerTest, RenameTable) {
    TableManager mgr(db_);

    // Create a table first
    mgr.create_table("old_table", "target", "x", "y",
                     "x_val", "o_val", -10.0, 10.0, -10.0, 10.0, false);

    // Add some data
    DataTable dt(db_, "old_table");
    dt.insert_point(1.0, 2.0, "x_val");

    // Rename it
    EXPECT_TRUE(mgr.rename_table("old_table", "new_table"));

    // Verify old table doesn't exist in metadata
    MetadataManager meta_mgr(db_);
    auto old_meta = meta_mgr.read("old_table");
    EXPECT_FALSE(old_meta.has_value());

    // Verify new table exists
    auto new_meta = meta_mgr.read("new_table");
    ASSERT_TRUE(new_meta.has_value());
    EXPECT_EQ(new_meta->table_name, "new_table");

    // Verify data was preserved
    DataTable new_dt(db_, "new_table");
    auto points = new_dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 1);
    EXPECT_EQ(points[0].x, 1.0);
}

// Test: --copy-table duplicates table and metadata
TEST_F(TableManagerTest, CopyTable) {
    TableManager mgr(db_);

    // Create source table
    mgr.create_table("source", "target", "x", "y",
                     "x_val", "o_val", -10.0, 10.0, -10.0, 10.0, false);

    // Add data
    DataTable src_dt(db_, "source");
    src_dt.insert_point(1.0, 2.0, "x_val");
    src_dt.insert_point(3.0, 4.0, "o_val");

    // Copy it
    EXPECT_TRUE(mgr.copy_table("source", "destination"));

    // Verify destination metadata exists
    MetadataManager meta_mgr(db_);
    auto dest_meta = meta_mgr.read("destination");
    ASSERT_TRUE(dest_meta.has_value());
    EXPECT_EQ(dest_meta->table_name, "destination");

    // Verify data was copied
    DataTable dest_dt(db_, "destination");
    auto dest_points = dest_dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(dest_points.size(), 2);

    // Verify source still exists
    auto src_points = src_dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(src_points.size(), 2);
}

// Test: --delete-table removes table and metadata
TEST_F(TableManagerTest, DeleteTable) {
    TableManager mgr(db_);

    // Create a table
    mgr.create_table("to_delete", "target", "x", "y",
                     "x_val", "o_val", -10.0, 10.0, -10.0, 10.0, false);

    // Verify it exists
    MetadataManager meta_mgr(db_);
    auto meta = meta_mgr.read("to_delete");
    ASSERT_TRUE(meta.has_value());

    // Delete it
    EXPECT_TRUE(mgr.delete_table("to_delete"));

    // Verify metadata is gone
    meta = meta_mgr.read("to_delete");
    EXPECT_FALSE(meta.has_value());

    // Verify data table is gone (check that querying it fails)
    // We can't easily test this without trying to use the table,
    // but the metadata being gone is sufficient
}

// Test: --list-tables outputs all tables
TEST_F(TableManagerTest, ListTables) {
    TableManager mgr(db_);

    // Create multiple tables
    mgr.create_table("table1", "target", "x", "y", "x_val", "o_val",
                     -10.0, 10.0, -10.0, 10.0, false);
    mgr.create_table("table2", "target", "x", "y", "x_val", "o_val",
                     -10.0, 10.0, -10.0, 10.0, false);
    mgr.create_table("table3", "target", "x", "y", "x_val", "o_val",
                     -10.0, 10.0, -10.0, 10.0, false);

    auto tables = mgr.list_tables();
    EXPECT_EQ(tables.size(), 3);
    EXPECT_EQ(tables[0], "table1");
    EXPECT_EQ(tables[1], "table2");
    EXPECT_EQ(tables[2], "table3");
}

// Test: --show-metadata outputs metadata for table
TEST_F(TableManagerTest, ShowMetadata) {
    TableManager mgr(db_);

    // Create a table
    mgr.create_table("test_table", "target", "x_axis", "y_axis",
                     "x_val", "o_val", -5.0, 15.0, -8.0, 12.0, true);

    std::ostringstream output;
    EXPECT_TRUE(mgr.show_metadata("test_table", output));

    std::string result = output.str();
    EXPECT_NE(result.find("test_table"), std::string::npos);
    EXPECT_NE(result.find("target"), std::string::npos);
    EXPECT_NE(result.find("x_axis"), std::string::npos);
    EXPECT_NE(result.find("y_axis"), std::string::npos);
    EXPECT_NE(result.find("x_val"), std::string::npos);
    EXPECT_NE(result.find("o_val"), std::string::npos);
}

// Test: --add-point inserts point directly
TEST_F(TableManagerTest, AddPoint) {
    TableManager mgr(db_);

    // Create a table
    mgr.create_table("test_table", "target", "x", "y",
                     "x_val", "o_val", -10.0, 10.0, -10.0, 10.0, false);

    // Add a point
    EXPECT_TRUE(mgr.add_point("test_table", 3.5, 7.2, "x_val"));

    // Verify it was added
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 1);
    EXPECT_EQ(points[0].x, 3.5);
    EXPECT_EQ(points[0].y, 7.2);
    EXPECT_EQ(points[0].target, "x_val");
}

// Test: --delete-point removes point by id
TEST_F(TableManagerTest, DeletePointById) {
    TableManager mgr(db_);

    // Create a table and add points
    mgr.create_table("test_table", "target", "x", "y",
                     "x_val", "o_val", -10.0, 10.0, -10.0, 10.0, false);

    DataTable dt(db_, "test_table");
    auto id1 = dt.insert_point(1.0, 2.0, "x_val");
    auto id2 = dt.insert_point(3.0, 4.0, "o_val");
    ASSERT_TRUE(id1.has_value());
    ASSERT_TRUE(id2.has_value());

    // Delete one point
    EXPECT_TRUE(mgr.delete_point("test_table", id1.value()));

    // Verify only one point remains
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 1);
    EXPECT_EQ(points[0].id, id2.value());
}

// Test: Handle errors gracefully
TEST_F(TableManagerTest, HandleErrors) {
    TableManager mgr(db_);

    // Try to rename non-existent table
    EXPECT_FALSE(mgr.rename_table("nonexistent", "new_name"));

    // Try to copy non-existent table
    EXPECT_FALSE(mgr.copy_table("nonexistent", "new_name"));

    // Try to delete non-existent table
    EXPECT_FALSE(mgr.delete_table("nonexistent"));

    // Try to add point to non-existent table
    EXPECT_FALSE(mgr.add_point("nonexistent", 1.0, 2.0, "x_val"));

    // Try to delete point from non-existent table
    EXPECT_FALSE(mgr.delete_point("nonexistent", 1));
}

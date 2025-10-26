#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"

using namespace datapainter;

// Test fixture for metadata tests
class MetadataTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
        ASSERT_TRUE(db->is_open());
        ASSERT_TRUE(db->ensure_metadata_table());
        mgr = std::make_unique<MetadataManager>(*db);
    }

    std::unique_ptr<Database> db;
    std::unique_ptr<MetadataManager> mgr;
};

// Test inserting metadata
TEST_F(MetadataTest, InsertMetadata) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    EXPECT_TRUE(mgr->insert(meta));
}

// Test inserting duplicate table name fails
TEST_F(MetadataTest, InsertDuplicateFails) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    ASSERT_TRUE(mgr->insert(meta));
    EXPECT_FALSE(mgr->insert(meta));  // Duplicate should fail
}

// Test reading metadata
TEST_F(MetadataTest, ReadMetadata) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "class";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";
    meta.valid_x_min = -10.0;
    meta.valid_x_max = 10.0;
    meta.valid_y_min = -5.0;
    meta.valid_y_max = 5.0;
    meta.show_zero_bars = true;

    ASSERT_TRUE(mgr->insert(meta));

    auto read_meta = mgr->read("test_table");
    ASSERT_TRUE(read_meta.has_value());

    EXPECT_EQ(read_meta->table_name, "test_table");
    EXPECT_EQ(read_meta->x_axis_name, "time");
    EXPECT_EQ(read_meta->y_axis_name, "value");
    EXPECT_EQ(read_meta->target_col_name, "class");
    EXPECT_EQ(read_meta->x_meaning, "positive");
    EXPECT_EQ(read_meta->o_meaning, "negative");
    ASSERT_TRUE(read_meta->valid_x_min.has_value());
    EXPECT_DOUBLE_EQ(read_meta->valid_x_min.value(), -10.0);
    ASSERT_TRUE(read_meta->valid_x_max.has_value());
    EXPECT_DOUBLE_EQ(read_meta->valid_x_max.value(), 10.0);
    ASSERT_TRUE(read_meta->valid_y_min.has_value());
    EXPECT_DOUBLE_EQ(read_meta->valid_y_min.value(), -5.0);
    ASSERT_TRUE(read_meta->valid_y_max.has_value());
    EXPECT_DOUBLE_EQ(read_meta->valid_y_max.value(), 5.0);
    EXPECT_TRUE(read_meta->show_zero_bars);
}

// Test reading non-existent metadata
TEST_F(MetadataTest, ReadNonExistentReturnsNullopt) {
    auto result = mgr->read("nonexistent");
    EXPECT_FALSE(result.has_value());
}

// Test updating metadata
TEST_F(MetadataTest, UpdateMetadata) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    ASSERT_TRUE(mgr->insert(meta));

    // Update the metadata
    meta.x_meaning = "feline";
    meta.o_meaning = "canine";
    meta.show_zero_bars = true;

    EXPECT_TRUE(mgr->update(meta));

    // Verify update
    auto read_meta = mgr->read("test_table");
    ASSERT_TRUE(read_meta.has_value());
    EXPECT_EQ(read_meta->x_meaning, "feline");
    EXPECT_EQ(read_meta->o_meaning, "canine");
    EXPECT_TRUE(read_meta->show_zero_bars);
}

// Test updating non-existent metadata fails
TEST_F(MetadataTest, UpdateNonExistentFails) {
    Metadata meta;
    meta.table_name = "nonexistent";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    EXPECT_FALSE(mgr->update(meta));
}

// Test removing metadata
TEST_F(MetadataTest, RemoveMetadata) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    ASSERT_TRUE(mgr->insert(meta));
    EXPECT_TRUE(mgr->remove("test_table"));

    // Verify removal
    auto result = mgr->read("test_table");
    EXPECT_FALSE(result.has_value());
}

// Test removing non-existent metadata fails
TEST_F(MetadataTest, RemoveNonExistentFails) {
    EXPECT_FALSE(mgr->remove("nonexistent"));
}

// Test listing tables
TEST_F(MetadataTest, ListTables) {
    Metadata meta1;
    meta1.table_name = "table1";
    meta1.x_axis_name = "x";
    meta1.y_axis_name = "y";
    meta1.target_col_name = "target";
    meta1.x_meaning = "cat";
    meta1.o_meaning = "dog";

    Metadata meta2;
    meta2.table_name = "table2";
    meta2.x_axis_name = "x";
    meta2.y_axis_name = "y";
    meta2.target_col_name = "target";
    meta2.x_meaning = "yes";
    meta2.o_meaning = "no";

    ASSERT_TRUE(mgr->insert(meta1));
    ASSERT_TRUE(mgr->insert(meta2));

    auto tables = mgr->list_tables();
    EXPECT_EQ(tables.size(), 2);
    EXPECT_TRUE(std::find(tables.begin(), tables.end(), "table1") != tables.end());
    EXPECT_TRUE(std::find(tables.begin(), tables.end(), "table2") != tables.end());
}

// Test listing tables when empty
TEST_F(MetadataTest, ListTablesWhenEmpty) {
    auto tables = mgr->list_tables();
    EXPECT_TRUE(tables.empty());
}

// Test creating data table
TEST_F(MetadataTest, CreateDataTable) {
    EXPECT_TRUE(mgr->create_data_table("my_data"));
    EXPECT_TRUE(db->table_exists("my_data"));
}

// Test data table has correct schema
TEST_F(MetadataTest, DataTableHasCorrectSchema) {
    ASSERT_TRUE(mgr->create_data_table("my_data"));

    // Verify columns
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "PRAGMA table_info(my_data)";
    int rc = sqlite3_prepare_v2(db->connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    std::vector<std::string> expected_cols = {"id", "x", "y", "target"};
    std::vector<std::string> actual_cols;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* col_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        actual_cols.push_back(col_name);
    }

    sqlite3_finalize(stmt);

    EXPECT_EQ(actual_cols, expected_cols);
}

// Test data table has indexes
TEST_F(MetadataTest, DataTableHasIndexes) {
    ASSERT_TRUE(mgr->create_data_table("my_data"));

    // Check for xy index
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='index' AND tbl_name='my_data'";
    int rc = sqlite3_prepare_v2(db->connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);

    std::vector<std::string> indexes;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* idx_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        indexes.push_back(idx_name);
    }

    sqlite3_finalize(stmt);

    EXPECT_TRUE(std::find(indexes.begin(), indexes.end(), "my_data_xy") != indexes.end());
    EXPECT_TRUE(std::find(indexes.begin(), indexes.end(), "my_data_target") != indexes.end());
}

// Test renaming table
TEST_F(MetadataTest, RenameTable) {
    // Create metadata and data table
    Metadata meta;
    meta.table_name = "old_name";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    ASSERT_TRUE(mgr->insert(meta));
    ASSERT_TRUE(mgr->create_data_table("old_name"));

    // Rename
    EXPECT_TRUE(mgr->rename_table("old_name", "new_name"));

    // Verify old doesn't exist
    EXPECT_FALSE(mgr->read("old_name").has_value());
    EXPECT_FALSE(db->table_exists("old_name"));

    // Verify new exists
    auto new_meta = mgr->read("new_name");
    ASSERT_TRUE(new_meta.has_value());
    EXPECT_EQ(new_meta->table_name, "new_name");
    EXPECT_TRUE(db->table_exists("new_name"));
}

// Test copying table
TEST_F(MetadataTest, CopyTable) {
    // Create metadata and data table
    Metadata meta;
    meta.table_name = "source";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    ASSERT_TRUE(mgr->insert(meta));
    ASSERT_TRUE(mgr->create_data_table("source"));

    // Insert some data
    db->execute("INSERT INTO source (x, y, target) VALUES (1.0, 2.0, 'cat')");

    // Copy
    EXPECT_TRUE(mgr->copy_table("source", "dest"));

    // Verify both exist
    EXPECT_TRUE(mgr->read("source").has_value());
    EXPECT_TRUE(mgr->read("dest").has_value());
    EXPECT_TRUE(db->table_exists("source"));
    EXPECT_TRUE(db->table_exists("dest"));

    // Verify data was copied
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT COUNT(*) FROM dest";
    int rc = sqlite3_prepare_v2(db->connection(), sql, -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    int count = sqlite3_column_int(stmt, 0);
    EXPECT_EQ(count, 1);
    sqlite3_finalize(stmt);
}

// Test deleting table
TEST_F(MetadataTest, DeleteTable) {
    // Create metadata and data table
    Metadata meta;
    meta.table_name = "to_delete";
    meta.x_axis_name = "x";
    meta.y_axis_name = "y";
    meta.target_col_name = "target";
    meta.x_meaning = "cat";
    meta.o_meaning = "dog";

    ASSERT_TRUE(mgr->insert(meta));
    ASSERT_TRUE(mgr->create_data_table("to_delete"));

    // Delete
    EXPECT_TRUE(mgr->delete_table("to_delete"));

    // Verify both are gone
    EXPECT_FALSE(mgr->read("to_delete").has_value());
    EXPECT_FALSE(db->table_exists("to_delete"));
}

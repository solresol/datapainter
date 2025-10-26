#include <gtest/gtest.h>
#include "database.h"
#include "unsaved_changes.h"

using namespace datapainter;

// Test fixture for unsaved changes tests
class UnsavedChangesTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
        ASSERT_TRUE(db->is_open());
        ASSERT_TRUE(db->ensure_unsaved_changes_table());
        changes = std::make_unique<UnsavedChanges>(*db);
    }

    std::unique_ptr<Database> db;
    std::unique_ptr<UnsavedChanges> changes;
};

// Test recording insert action
TEST_F(UnsavedChangesTest, RecordInsert) {
    auto id = changes->record_insert("test_table", 1.5, 2.5, "x");
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(id.value(), 0);
}

// Test recording multiple inserts
TEST_F(UnsavedChangesTest, RecordMultipleInserts) {
    auto id1 = changes->record_insert("test_table", 1.0, 2.0, "x");
    auto id2 = changes->record_insert("test_table", 3.0, 4.0, "o");
    auto id3 = changes->record_insert("other_table", 5.0, 6.0, "x");

    ASSERT_TRUE(id1.has_value());
    ASSERT_TRUE(id2.has_value());
    ASSERT_TRUE(id3.has_value());

    // IDs should be unique and increasing
    EXPECT_NE(id1.value(), id2.value());
    EXPECT_NE(id2.value(), id3.value());
    EXPECT_LT(id1.value(), id2.value());
    EXPECT_LT(id2.value(), id3.value());
}

// Test recording delete action
TEST_F(UnsavedChangesTest, RecordDelete) {
    auto id = changes->record_delete("test_table", 42, 1.5, 2.5, "x");
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(id.value(), 0);
}

// Test recording update action
TEST_F(UnsavedChangesTest, RecordUpdate) {
    auto id = changes->record_update("test_table", 42, "x", "o");
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(id.value(), 0);
}

// Test recording metadata change
TEST_F(UnsavedChangesTest, RecordMetadataChange) {
    auto id = changes->record_metadata_change("test_table", "x_meaning", "cat", "dog");
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(id.value(), 0);
}

// Test getting changes for empty table
TEST_F(UnsavedChangesTest, GetChangesEmpty) {
    auto records = changes->get_changes("test_table");
    EXPECT_TRUE(records.empty());
}

// Test getting changes for specific table
TEST_F(UnsavedChangesTest, GetChangesForTable) {
    changes->record_insert("test_table", 1.0, 2.0, "x");
    changes->record_insert("other_table", 3.0, 4.0, "o");
    changes->record_insert("test_table", 5.0, 6.0, "x");

    auto records = changes->get_changes("test_table");
    EXPECT_EQ(records.size(), 2);

    // Verify table name filtering
    for (const auto& rec : records) {
        EXPECT_EQ(rec.table_name, "test_table");
    }
}

// Test getting all changes
TEST_F(UnsavedChangesTest, GetAllChanges) {
    changes->record_insert("table1", 1.0, 2.0, "x");
    changes->record_insert("table2", 3.0, 4.0, "o");
    changes->record_delete("table1", 1, 5.0, 6.0, "x");

    auto records = changes->get_all_changes();
    EXPECT_EQ(records.size(), 3);
}

// Test insert record contains correct data
TEST_F(UnsavedChangesTest, InsertRecordHasCorrectData) {
    auto change_id = changes->record_insert("test_table", 1.5, 2.5, "x");
    ASSERT_TRUE(change_id.has_value());

    auto records = changes->get_changes("test_table");
    ASSERT_EQ(records.size(), 1);

    EXPECT_EQ(records[0].id, change_id.value());
    EXPECT_EQ(records[0].table_name, "test_table");
    EXPECT_EQ(records[0].action, "insert");
    EXPECT_FALSE(records[0].data_id.has_value());  // inserts don't have data_id yet
    ASSERT_TRUE(records[0].x.has_value());
    EXPECT_DOUBLE_EQ(records[0].x.value(), 1.5);
    ASSERT_TRUE(records[0].y.has_value());
    EXPECT_DOUBLE_EQ(records[0].y.value(), 2.5);
    EXPECT_FALSE(records[0].old_target.has_value());
    ASSERT_TRUE(records[0].new_target.has_value());
    EXPECT_EQ(records[0].new_target.value(), "x");
}

// Test delete record contains correct data
TEST_F(UnsavedChangesTest, DeleteRecordHasCorrectData) {
    auto change_id = changes->record_delete("test_table", 42, 1.5, 2.5, "o");
    ASSERT_TRUE(change_id.has_value());

    auto records = changes->get_changes("test_table");
    ASSERT_EQ(records.size(), 1);

    EXPECT_EQ(records[0].action, "delete");
    ASSERT_TRUE(records[0].data_id.has_value());
    EXPECT_EQ(records[0].data_id.value(), 42);
    ASSERT_TRUE(records[0].old_target.has_value());
    EXPECT_EQ(records[0].old_target.value(), "o");
}

// Test update record contains correct data
TEST_F(UnsavedChangesTest, UpdateRecordHasCorrectData) {
    auto change_id = changes->record_update("test_table", 42, "x", "o");
    ASSERT_TRUE(change_id.has_value());

    auto records = changes->get_changes("test_table");
    ASSERT_EQ(records.size(), 1);

    EXPECT_EQ(records[0].action, "update");
    ASSERT_TRUE(records[0].data_id.has_value());
    EXPECT_EQ(records[0].data_id.value(), 42);
    ASSERT_TRUE(records[0].old_target.has_value());
    EXPECT_EQ(records[0].old_target.value(), "x");
    ASSERT_TRUE(records[0].new_target.has_value());
    EXPECT_EQ(records[0].new_target.value(), "o");
}

// Test metadata record contains correct data
TEST_F(UnsavedChangesTest, MetadataRecordHasCorrectData) {
    auto change_id = changes->record_metadata_change("test_table", "x_meaning", "cat", "dog");
    ASSERT_TRUE(change_id.has_value());

    auto records = changes->get_changes("test_table");
    ASSERT_EQ(records.size(), 1);

    EXPECT_EQ(records[0].action, "meta");
    EXPECT_FALSE(records[0].data_id.has_value());
    EXPECT_FALSE(records[0].x.has_value());
    EXPECT_FALSE(records[0].y.has_value());
    ASSERT_TRUE(records[0].meta_field.has_value());
    EXPECT_EQ(records[0].meta_field.value(), "x_meaning");
    ASSERT_TRUE(records[0].old_value.has_value());
    EXPECT_EQ(records[0].old_value.value(), "cat");
    ASSERT_TRUE(records[0].new_value.has_value());
    EXPECT_EQ(records[0].new_value.value(), "dog");
}

// Test clearing changes for specific table
TEST_F(UnsavedChangesTest, ClearChangesForTable) {
    changes->record_insert("table1", 1.0, 2.0, "x");
    changes->record_insert("table2", 3.0, 4.0, "o");

    EXPECT_TRUE(changes->clear_changes("table1"));

    auto table1_records = changes->get_changes("table1");
    auto table2_records = changes->get_changes("table2");

    EXPECT_TRUE(table1_records.empty());
    EXPECT_EQ(table2_records.size(), 1);
}

// Test clearing all changes
TEST_F(UnsavedChangesTest, ClearAllChanges) {
    changes->record_insert("table1", 1.0, 2.0, "x");
    changes->record_insert("table2", 3.0, 4.0, "o");

    EXPECT_TRUE(changes->clear_all_changes());

    auto all_records = changes->get_all_changes();
    EXPECT_TRUE(all_records.empty());
}

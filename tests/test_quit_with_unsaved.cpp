// Test to verify quit behavior when there are unsaved changes
#include <gtest/gtest.h>
#include "../include/database.h"
#include "../include/unsaved_changes.h"
#include "../include/point_editor.h"
#include "../include/metadata.h"

using namespace datapainter;

class QuitWithUnsavedTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_ = std::make_unique<Database>(":memory:");
        ASSERT_TRUE(db_->is_open());

        // Create test table
        const char* create_table_sql = R"(
            CREATE TABLE test_data (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                x REAL NOT NULL,
                y REAL NOT NULL,
                label TEXT NOT NULL
            )
        )";
        ASSERT_TRUE(db_->execute(create_table_sql));

        // Create unsaved_changes table
        ASSERT_TRUE(db_->ensure_unsaved_changes_table());

        // Create metadata table and add entry
        ASSERT_TRUE(db_->ensure_metadata_table());

        const char* insert_meta_sql = R"(
            INSERT INTO metadata (table_name, target_col_name, x_axis_name, y_axis_name,
                                 x_meaning, o_meaning, valid_x_min, valid_x_max, valid_y_min, valid_y_max)
            VALUES ('test_data', 'label', 'x', 'y', 'positive', 'negative', -10, 10, -10, 10)
        )";
        ASSERT_TRUE(db_->execute(insert_meta_sql));
    }

    std::unique_ptr<Database> db_;
};

TEST_F(QuitWithUnsavedTest, HasUnsavedChangesReturnsTrueWhenChangesExist) {
    // Create some unsaved changes
    UnsavedChanges uc(*db_);
    auto change_id = uc.record_insert("test_data", 5.0, 5.0, "positive");
    ASSERT_TRUE(change_id.has_value());

    // Check that we have unsaved changes
    auto changes = uc.get_all_changes();
    EXPECT_GT(changes.size(), 0) << "Should have unsaved changes after insert";

    // Count active changes
    int active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 1) << "Should have 1 active unsaved change";
}

TEST_F(QuitWithUnsavedTest, HasUnsavedChangesReturnsFalseWhenNoChanges) {
    UnsavedChanges uc(*db_);
    auto changes = uc.get_all_changes();
    EXPECT_EQ(changes.size(), 0) << "Should have no unsaved changes initially";
}

TEST_F(QuitWithUnsavedTest, HasUnsavedChangesIgnoresInactiveChanges) {
    // Create an insert and then mark it inactive (deleted)
    UnsavedChanges uc(*db_);
    auto change_id = uc.record_insert("test_data", 5.0, 5.0, "positive");
    ASSERT_TRUE(change_id.has_value());

    // Mark it inactive (simulating deletion)
    ASSERT_TRUE(uc.mark_change_inactive(change_id.value()));

    // Check that we have no active unsaved changes
    auto changes = uc.get_all_changes();
    EXPECT_GT(changes.size(), 0) << "Should still have records in unsaved_changes";

    int active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0) << "Should have 0 active unsaved changes after marking inactive";
}

TEST_F(QuitWithUnsavedTest, CountUnsavedChanges) {
    UnsavedChanges uc(*db_);

    // Initially no changes
    auto changes = uc.get_all_changes();
    int active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) active_count++;
    }
    EXPECT_EQ(active_count, 0);

    // Add 3 changes
    uc.record_insert("test_data", 1.0, 1.0, "positive");
    uc.record_insert("test_data", 2.0, 2.0, "negative");
    uc.record_insert("test_data", 3.0, 3.0, "positive");

    // Count again
    changes = uc.get_all_changes();
    active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) active_count++;
    }
    EXPECT_EQ(active_count, 3) << "Should have 3 active unsaved changes";
}

TEST_F(QuitWithUnsavedTest, UnsavedChangesAcrossMultipleTables) {
    // Create a second table
    const char* create_table2_sql = R"(
        CREATE TABLE test_data2 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            x REAL NOT NULL,
            y REAL NOT NULL,
            label TEXT NOT NULL
        )
    )";
    ASSERT_TRUE(db_->execute(create_table2_sql));

    const char* insert_meta2_sql = R"(
        INSERT INTO metadata (table_name, target_col_name, x_axis_name, y_axis_name,
                             x_meaning, o_meaning, valid_x_min, valid_x_max, valid_y_min, valid_y_max)
        VALUES ('test_data2', 'label', 'x', 'y', 'positive', 'negative', -10, 10, -10, 10)
    )";
    ASSERT_TRUE(db_->execute(insert_meta2_sql));

    UnsavedChanges uc(*db_);

    // Add changes to both tables
    uc.record_insert("test_data", 1.0, 1.0, "positive");
    uc.record_insert("test_data2", 2.0, 2.0, "negative");

    // Count all changes
    auto all_changes = uc.get_all_changes();
    int active_count = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) active_count++;
    }
    EXPECT_EQ(active_count, 2) << "Should have 2 active unsaved changes across both tables";

    // Count changes for first table only
    auto table1_changes = uc.get_changes("test_data");
    int table1_active = 0;
    for (const auto& change : table1_changes) {
        if (change.is_active) table1_active++;
    }
    EXPECT_EQ(table1_active, 1) << "Should have 1 active unsaved change in test_data";
}

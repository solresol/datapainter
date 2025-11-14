#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "point_editor.h"
#include "unsaved_changes.h"
#include "save_manager.h"

using namespace datapainter;

class QuitCountsOnlyActiveChangesTest : public ::testing::Test {
protected:
    QuitCountsOnlyActiveChangesTest() : db_(":memory:") {}

    void SetUp() override {
        // Create metadata table
        db_.ensure_metadata_table();
        db_.ensure_unsaved_changes_table();

        // Create test table
        MetadataManager mgr(db_);
        mgr.create_data_table("test_table");

        Metadata meta;
        meta.table_name = "test_table";
        meta.target_col_name = "target_col";
        meta.x_axis_name = "x_axis";
        meta.y_axis_name = "y_axis";
        meta.x_meaning = "x_meaning";
        meta.o_meaning = "o_meaning";
        meta.valid_x_min = -10.0;
        meta.valid_x_max = 10.0;
        meta.valid_y_min = -10.0;
        meta.valid_y_max = 10.0;
        meta.show_zero_bars = false;
        mgr.insert(meta);
    }

    Database db_;
};

// Test: After save, get_all_changes should only return active changes
TEST_F(QuitCountsOnlyActiveChangesTest, GetAllChangesOnlyReturnsActive) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');
    editor.create_point(5.0, 6.0, 'x');

    // Verify we have 3 active changes
    auto all_changes = uc.get_all_changes();
    int active_count = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 3) << "Before save: should have 3 active changes";

    // Save changes
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // After save, get_all_changes should return no active changes
    all_changes = uc.get_all_changes();
    active_count = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0) << "After save: should have 0 active changes";
}

// Test: Create, save, create more, count should only show new changes
TEST_F(QuitCountsOnlyActiveChangesTest, OnlyNewChangesAfterSave) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create and save some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // Verify no active changes after save
    auto all_changes = uc.get_all_changes();
    int active_count = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0) << "After first save: should have 0 active changes";

    // Create more points
    editor.create_point(5.0, 6.0, 'x');
    editor.create_point(7.0, 8.0, 'o');

    // Should only count the 2 new changes
    all_changes = uc.get_all_changes();
    active_count = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 2) << "After creating 2 more points: should have 2 active changes";
}

// Test: get_changes for specific table only returns active changes
TEST_F(QuitCountsOnlyActiveChangesTest, GetChangesForTableOnlyReturnsActive) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create and save some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // After save, get_changes should return empty or only active changes
    auto changes = uc.get_changes("test_table");
    int active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0) << "get_changes should only count active changes";
}

// Test: Saved changes are deleted from unsaved_changes table
TEST_F(QuitCountsOnlyActiveChangesTest, SavedChangesAreDeleted) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');

    // Verify changes exist before save
    auto all_changes_before = uc.get_all_changes();
    EXPECT_EQ(all_changes_before.size(), 2) << "Should have 2 changes before save";

    // Save (deletes the change records from unsaved_changes table)
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // After save, the changes are deleted from unsaved_changes table
    auto all_changes_after = uc.get_all_changes();

    // Count total changes vs active changes
    int total_changes = static_cast<int>(all_changes_after.size());
    int active_changes = 0;
    for (const auto& change : all_changes_after) {
        if (change.is_active) {
            active_changes++;
        }
    }

    // Design choice: save() calls clear_changes() which DELETEs records
    EXPECT_EQ(total_changes, 0) << "After save, changes are deleted from unsaved_changes";
    EXPECT_EQ(active_changes, 0) << "No active changes remain";
}

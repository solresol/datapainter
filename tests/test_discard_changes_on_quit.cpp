#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "point_editor.h"
#include "unsaved_changes.h"

using namespace datapainter;

class DiscardChangesOnQuitTest : public ::testing::Test {
protected:
    DiscardChangesOnQuitTest() : db_(":memory:") {}

    void SetUp() override {
        db_.ensure_metadata_table();
        db_.ensure_unsaved_changes_table();

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

// Test: Discarding changes should clear unsaved_changes table
TEST_F(DiscardChangesOnQuitTest, DiscardClearsUnsavedChanges) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');
    editor.create_point(5.0, 6.0, 'x');

    // Verify unsaved changes exist
    auto changes_before = uc.get_changes("test_table");
    int active_before = 0;
    for (const auto& change : changes_before) {
        if (change.is_active) {
            active_before++;
        }
    }
    EXPECT_EQ(active_before, 3) << "Should have 3 unsaved changes";

    // User chooses to discard changes and quit
    // This should clear the unsaved_changes table for this table
    bool cleared = uc.clear_changes("test_table");
    EXPECT_TRUE(cleared) << "clear_changes should succeed";

    // After discarding, unsaved_changes should be empty
    auto changes_after = uc.get_changes("test_table");
    EXPECT_EQ(changes_after.size(), 0)
        << "After discarding changes on quit, unsaved_changes should be empty";
}

// Test: Discard doesn't affect the main data table
TEST_F(DiscardChangesOnQuitTest, DiscardDoesntAffectDataTable) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);
    DataTable dt(db_, "test_table");

    // Add an existing saved point directly to data table
    dt.insert_point(10.0, 20.0, "x_meaning");

    // Verify it's in the data table
    auto points_before = dt.query_viewport(-100.0, 100.0, -100.0, 100.0);
    EXPECT_EQ(points_before.size(), 1) << "Should have 1 saved point";

    // Create unsaved points via editor
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');

    // Discard unsaved changes (mimics quit without save)
    uc.clear_changes("test_table");

    // Data table should still have the original saved point
    auto points_after = dt.query_viewport(-100.0, 100.0, -100.0, 100.0);
    EXPECT_EQ(points_after.size(), 1)
        << "Discarding unsaved changes shouldn't affect saved data";
    EXPECT_DOUBLE_EQ(points_after[0].x, 10.0);
    EXPECT_DOUBLE_EQ(points_after[0].y, 20.0);
}

// Test: After discard and restart, no phantom changes
TEST_F(DiscardChangesOnQuitTest, NoPhantomChangesAfterDiscard) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create changes
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');

    // Discard (mimics quitting without save)
    uc.clear_changes("test_table");

    // Simulate restarting the program - check for unsaved changes
    auto all_changes = uc.get_all_changes();
    int active_changes = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_changes++;
        }
    }

    EXPECT_EQ(active_changes, 0)
        << "After discarding, restart should show no unsaved changes";
}

// Test: Discard only affects specified table
TEST_F(DiscardChangesOnQuitTest, DiscardOnlyAffectsSpecifiedTable) {
    // Create a second table
    MetadataManager mgr(db_);
    mgr.create_data_table("table2");
    Metadata meta2;
    meta2.table_name = "table2";
    meta2.target_col_name = "target_col";
    meta2.x_axis_name = "x_axis";
    meta2.y_axis_name = "y_axis";
    meta2.x_meaning = "x_meaning";
    meta2.o_meaning = "o_meaning";
    meta2.valid_x_min = -10.0;
    meta2.valid_x_max = 10.0;
    meta2.valid_y_min = -10.0;
    meta2.valid_y_max = 10.0;
    meta2.show_zero_bars = false;
    mgr.insert(meta2);

    // Create changes in both tables
    PointEditor editor1(db_, "test_table");
    PointEditor editor2(db_, "table2");
    UnsavedChanges uc(db_);

    editor1.create_point(1.0, 2.0, 'x');
    editor2.create_point(3.0, 4.0, 'o');

    // Discard only test_table changes
    uc.clear_changes("test_table");

    // test_table should have no changes
    auto changes1 = uc.get_changes("test_table");
    EXPECT_EQ(changes1.size(), 0) << "test_table changes should be discarded";

    // table2 should still have its change
    auto changes2 = uc.get_changes("table2");
    int active2 = 0;
    for (const auto& change : changes2) {
        if (change.is_active) {
            active2++;
        }
    }
    EXPECT_EQ(active2, 1) << "table2 changes should remain";
}

#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "point_editor.h"
#include "unsaved_changes.h"
#include "save_manager.h"

using namespace datapainter;

class SaveResetsUnsavedCountTest : public ::testing::Test {
protected:
    SaveResetsUnsavedCountTest() : db_(":memory:") {}

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

// Test: After saving, unsaved changes count should be zero
TEST_F(SaveResetsUnsavedCountTest, SaveClearsUnsavedCount) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');
    editor.create_point(5.0, 6.0, 'x');

    // Verify unsaved changes exist
    auto changes = uc.get_changes("test_table");
    int active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 3);

    // Save changes
    SaveManager save_manager(db_, "test_table");
    bool save_success = save_manager.save();
    EXPECT_TRUE(save_success);

    // Verify unsaved changes count is now zero
    changes = uc.get_changes("test_table");
    active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0) << "After save, unsaved changes count should be 0";
}

// Test: After save, points should be in database
TEST_F(SaveResetsUnsavedCountTest, SavePersistsPointsToDatabase) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');

    // Save changes
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // Verify points are in database
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 2);
}

// Test: Multiple saves don't create duplicate unsaved changes
TEST_F(SaveResetsUnsavedCountTest, MultipleSavesDontDuplicate) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create and save points
    editor.create_point(1.0, 2.0, 'x');
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // Verify no active unsaved changes
    auto changes = uc.get_changes("test_table");
    int active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0);

    // Create more points
    editor.create_point(3.0, 4.0, 'o');
    editor.create_point(5.0, 6.0, 'x');

    // Should have 2 new unsaved changes
    changes = uc.get_changes("test_table");
    active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 2);

    // Save again
    save_manager.save();

    // Should have 0 unsaved changes again
    changes = uc.get_changes("test_table");
    active_count = 0;
    for (const auto& change : changes) {
        if (change.is_active) {
            active_count++;
        }
    }
    EXPECT_EQ(active_count, 0);
}

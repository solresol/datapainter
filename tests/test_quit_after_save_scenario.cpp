#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "point_editor.h"
#include "unsaved_changes.h"
#include "save_manager.h"

using namespace datapainter;

class QuitAfterSaveScenarioTest : public ::testing::Test {
protected:
    QuitAfterSaveScenarioTest() : db_(":memory:") {}

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

// Test: User scenario - create, save, then quit should show 0 changes
TEST_F(QuitAfterSaveScenarioTest, CreateSaveQuitShowsZero) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // User creates some points
    editor.create_point(1.0, 2.0, 'x');
    editor.create_point(3.0, 4.0, 'o');
    editor.create_point(5.0, 6.0, 'x');

    // User presses 's' to save
    SaveManager save_manager(db_, "test_table");
    bool saved = save_manager.save();
    ASSERT_TRUE(saved) << "Save should succeed";

    // User presses 'q' to quit - should show 0 unsaved changes
    // This mimics what main.cpp does on quit:
    auto all_changes = uc.get_all_changes();
    int active_changes = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_changes++;
        }
    }

    EXPECT_EQ(active_changes, 0)
        << "After save, quit should report 0 unsaved changes, not "
        << all_changes.size() << " total changes";
}

// Test: Exact reproduction of user's complaint
TEST_F(QuitAfterSaveScenarioTest, ReproduceUserComplaint) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);

    // Create 5 points
    for (int i = 0; i < 5; i++) {
        editor.create_point(i * 1.0, i * 2.0, 'x');
    }

    // Verify 5 unsaved changes
    auto all_changes = uc.get_all_changes();
    int active_before_save = 0;
    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_before_save++;
        }
    }
    EXPECT_EQ(active_before_save, 5) << "Should have 5 unsaved changes before save";

    // Save
    SaveManager save_manager(db_, "test_table");
    save_manager.save();

    // Check unsaved changes count again (like quit dialog does)
    all_changes = uc.get_all_changes();
    int active_after_save = 0;
    int total_after_save = static_cast<int>(all_changes.size());

    for (const auto& change : all_changes) {
        if (change.is_active) {
            active_after_save++;
        }
    }

    // The user's complaint: quit shows wrong count
    EXPECT_EQ(active_after_save, 0)
        << "After save, should show 0 active changes. "
        << "User complaint: quit shows " << total_after_save
        << " instead of " << active_after_save;
}

// Test: Multiple save cycles
TEST_F(QuitAfterSaveScenarioTest, MultipleSaveCycles) {
    PointEditor editor(db_, "test_table");
    UnsavedChanges uc(db_);
    SaveManager save_manager(db_, "test_table");

    // Cycle 1: Create and save
    editor.create_point(1.0, 1.0, 'x');
    editor.create_point(2.0, 2.0, 'o');
    save_manager.save();

    // After first save, should have 0 active changes
    auto all_changes = uc.get_all_changes();
    int active = 0;
    for (const auto& c : all_changes) {
        if (c.is_active) active++;
    }
    EXPECT_EQ(active, 0) << "After first save";

    // Cycle 2: Create more and save
    editor.create_point(3.0, 3.0, 'x');
    save_manager.save();

    // After second save, should still have 0 active changes
    all_changes = uc.get_all_changes();
    active = 0;
    for (const auto& c : all_changes) {
        if (c.is_active) active++;
    }
    EXPECT_EQ(active, 0) << "After second save";

    // Cycle 3: Create more (but don't save)
    editor.create_point(4.0, 4.0, 'o');
    editor.create_point(5.0, 5.0, 'x');

    // Before save, should have exactly 2 active changes
    all_changes = uc.get_all_changes();
    active = 0;
    for (const auto& c : all_changes) {
        if (c.is_active) active++;
    }
    EXPECT_EQ(active, 2) << "Should have exactly 2 new unsaved changes";
}

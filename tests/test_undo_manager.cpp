#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "unsaved_changes.h"
#include "undo_manager.h"
#include <optional>

using namespace datapainter;

class UndoManagerTest : public ::testing::Test {
protected:
    UndoManagerTest() : db_(":memory:") {}

    void SetUp() override {
        // Create metadata and unsaved_changes tables
        db_.ensure_metadata_table();
        db_.ensure_unsaved_changes_table();

        // Create test table
        MetadataManager mgr(db_);
        mgr.create_data_table("test_table");

        Metadata meta;
        meta.table_name = "test_table";
        meta.target_col_name = "target";
        meta.x_axis_name = "x";
        meta.y_axis_name = "y";
        meta.x_meaning = "x_val";
        meta.o_meaning = "o_val";
        meta.valid_x_min = -10.0;
        meta.valid_x_max = 10.0;
        meta.valid_y_min = -10.0;
        meta.valid_y_max = 10.0;
        meta.show_zero_bars = false;
        mgr.insert(meta);

        changes_ = std::make_unique<UnsavedChanges>(db_);
    }

    Database db_;
    std::unique_ptr<UnsavedChanges> changes_;
};

// Test: Track current position in unsaved_changes
TEST_F(UndoManagerTest, TrackCurrentPosition) {
    UndoManager undo_mgr(db_, "test_table");

    // Initially, position should be at the end (no changes)
    EXPECT_EQ(undo_mgr.current_position(), 0);

    // Add some changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");

    // Position should update when we refresh
    undo_mgr.refresh();
    EXPECT_EQ(undo_mgr.current_position(), 2);
}

// Test: Press 'u' undoes last action
TEST_F(UndoManagerTest, UndoLastAction) {
    UndoManager undo_mgr(db_, "test_table");

    // Add a change
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    EXPECT_TRUE(undo_mgr.can_undo());
    EXPECT_FALSE(undo_mgr.can_redo());

    // Undo the change
    EXPECT_TRUE(undo_mgr.undo());
    EXPECT_EQ(undo_mgr.current_position(), 0);

    // Should not be able to undo further
    EXPECT_FALSE(undo_mgr.can_undo());
    EXPECT_TRUE(undo_mgr.can_redo());
}

// Test: Multiple undo steps backward
TEST_F(UndoManagerTest, MultipleUndoSteps) {
    UndoManager undo_mgr(db_, "test_table");

    // Add multiple changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");
    changes_->record_delete("test_table", 1, 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    EXPECT_EQ(undo_mgr.current_position(), 3);

    // Undo once
    EXPECT_TRUE(undo_mgr.undo());
    EXPECT_EQ(undo_mgr.current_position(), 2);

    // Undo twice
    EXPECT_TRUE(undo_mgr.undo());
    EXPECT_EQ(undo_mgr.current_position(), 1);

    // Undo three times
    EXPECT_TRUE(undo_mgr.undo());
    EXPECT_EQ(undo_mgr.current_position(), 0);

    // Can't undo anymore
    EXPECT_FALSE(undo_mgr.can_undo());
    EXPECT_FALSE(undo_mgr.undo());
}

// Test: Enable redo after undo
TEST_F(UndoManagerTest, RedoAfterUndo) {
    UndoManager undo_mgr(db_, "test_table");

    // Add changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");
    undo_mgr.refresh();

    // Undo
    EXPECT_TRUE(undo_mgr.undo());
    EXPECT_EQ(undo_mgr.current_position(), 1);
    EXPECT_TRUE(undo_mgr.can_redo());

    // Redo
    EXPECT_TRUE(undo_mgr.redo());
    EXPECT_EQ(undo_mgr.current_position(), 2);
    EXPECT_FALSE(undo_mgr.can_redo());
}

// Test: Clear redo stack on new edit
TEST_F(UndoManagerTest, ClearRedoStackOnNewEdit) {
    UndoManager undo_mgr(db_, "test_table");

    // Add changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");
    undo_mgr.refresh();

    // Undo one change
    EXPECT_TRUE(undo_mgr.undo());
    EXPECT_EQ(undo_mgr.current_position(), 1);
    EXPECT_TRUE(undo_mgr.can_redo());

    // Add a new change (should clear redo stack)
    changes_->record_insert("test_table", 5.0, 6.0, "x_val");
    undo_mgr.refresh(true);  // Clear inactive changes

    // Redo should no longer be available
    EXPECT_FALSE(undo_mgr.can_redo());
    EXPECT_EQ(undo_mgr.current_position(), 2);
}

// Test: Display undo/redo availability in UI
TEST_F(UndoManagerTest, DisplayAvailability) {
    UndoManager undo_mgr(db_, "test_table");

    // Initially, nothing available
    EXPECT_FALSE(undo_mgr.can_undo());
    EXPECT_FALSE(undo_mgr.can_redo());

    // Add a change
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    // Undo available, redo not
    EXPECT_TRUE(undo_mgr.can_undo());
    EXPECT_FALSE(undo_mgr.can_redo());

    // Undo
    undo_mgr.undo();

    // Redo available, undo not
    EXPECT_FALSE(undo_mgr.can_undo());
    EXPECT_TRUE(undo_mgr.can_redo());
}

// Test: Undo point insert (mark as undone)
TEST_F(UndoManagerTest, UndoPointInsert) {
    UndoManager undo_mgr(db_, "test_table");

    // Add an insert change
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    // Verify the change is active
    auto recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_TRUE(recs[0].is_active);

    // Undo
    undo_mgr.undo();

    // Change should still exist but be marked as inactive
    recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_FALSE(recs[0].is_active);
}

// Test: Undo point delete
TEST_F(UndoManagerTest, UndoPointDelete) {
    UndoManager undo_mgr(db_, "test_table");

    // Add a delete change
    changes_->record_delete("test_table", 1, 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    // Verify the change is active
    auto recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_TRUE(recs[0].is_active);

    // Undo
    undo_mgr.undo();

    // Change should be marked as inactive
    recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_FALSE(recs[0].is_active);
}

// Test: Undo point update
TEST_F(UndoManagerTest, UndoPointUpdate) {
    UndoManager undo_mgr(db_, "test_table");

    // Add an update change
    changes_->record_update("test_table", 1, "x_val", "o_val");
    undo_mgr.refresh();

    // Verify the change is active
    auto recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_TRUE(recs[0].is_active);

    // Undo
    undo_mgr.undo();

    // Change should be marked as inactive
    recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_FALSE(recs[0].is_active);
}

// Test: Undo metadata change
TEST_F(UndoManagerTest, UndoMetadataChange) {
    UndoManager undo_mgr(db_, "test_table");

    // Add a metadata change
    changes_->record_metadata_change("test_table", "x_axis_name", "old_x", "new_x");
    undo_mgr.refresh();

    // Verify the change is active
    auto recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_TRUE(recs[0].is_active);

    // Undo
    undo_mgr.undo();

    // Change should be marked as inactive
    recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 1);
    EXPECT_FALSE(recs[0].is_active);
}

// Test: Redo undone operation
TEST_F(UndoManagerTest, RedoUndoneOperation) {
    UndoManager undo_mgr(db_, "test_table");

    // Add a change
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    // Undo it
    undo_mgr.undo();
    auto recs = changes_->get_changes("test_table");
    EXPECT_FALSE(recs[0].is_active);

    // Redo it
    undo_mgr.redo();
    recs = changes_->get_changes("test_table");
    EXPECT_TRUE(recs[0].is_active);
}

// Test: Get number of undoable/redoable steps
TEST_F(UndoManagerTest, GetUndoRedoCounts) {
    UndoManager undo_mgr(db_, "test_table");

    // Add 3 changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");
    changes_->record_delete("test_table", 1, 1.0, 2.0, "x_val");
    undo_mgr.refresh();

    // Should have 3 undoable, 0 redoable
    EXPECT_EQ(undo_mgr.undo_count(), 3);
    EXPECT_EQ(undo_mgr.redo_count(), 0);

    // Undo 2 times
    undo_mgr.undo();
    undo_mgr.undo();

    // Should have 1 undoable, 2 redoable
    EXPECT_EQ(undo_mgr.undo_count(), 1);
    EXPECT_EQ(undo_mgr.redo_count(), 2);
}

// Test: Undo/redo with mixed operations
TEST_F(UndoManagerTest, MixedOperations) {
    UndoManager undo_mgr(db_, "test_table");

    // Add various changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");
    changes_->record_update("test_table", 1, "x_val", "o_val");
    changes_->record_delete("test_table", 2, 3.0, 4.0, "o_val");
    changes_->record_metadata_change("test_table", "x_axis_name", "x", "new_x");
    undo_mgr.refresh();

    EXPECT_EQ(undo_mgr.undo_count(), 5);

    // Undo all
    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(undo_mgr.undo());
    }
    EXPECT_FALSE(undo_mgr.can_undo());
    EXPECT_EQ(undo_mgr.redo_count(), 5);

    // Redo all
    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(undo_mgr.redo());
    }
    EXPECT_FALSE(undo_mgr.can_redo());
    EXPECT_EQ(undo_mgr.undo_count(), 5);
}

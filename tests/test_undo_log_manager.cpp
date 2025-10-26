#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "unsaved_changes.h"
#include "undo_log_manager.h"
#include <sstream>

using namespace datapainter;

class UndoLogManagerTest : public ::testing::Test {
protected:
    UndoLogManagerTest() : db_(":memory:") {}

    void SetUp() override {
        // Create system tables
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

// Test: --clear-undo-log removes unsaved_changes for table
TEST_F(UndoLogManagerTest, ClearUndoLogForTable) {
    UndoLogManager mgr(db_);

    // Add changes to test_table
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");

    // Verify changes exist
    auto recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 2);

    // Clear undo log
    EXPECT_TRUE(mgr.clear_undo_log("test_table"));

    // Verify changes were cleared
    recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 0);
}

// Test: --clear-all-undo-log removes all unsaved_changes
TEST_F(UndoLogManagerTest, ClearAllUndoLogs) {
    UndoLogManager mgr(db_);

    // Create another table
    MetadataManager meta_mgr(db_);
    meta_mgr.create_data_table("table2");
    Metadata meta;
    meta.table_name = "table2";
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
    meta_mgr.insert(meta);

    // Add changes to both tables
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("table2", 3.0, 4.0, "o_val");

    // Verify changes exist
    auto recs1 = changes_->get_changes("test_table");
    auto recs2 = changes_->get_changes("table2");
    EXPECT_EQ(recs1.size(), 1);
    EXPECT_EQ(recs2.size(), 1);

    // Clear all undo logs
    EXPECT_TRUE(mgr.clear_all_undo_logs());

    // Verify all changes were cleared
    recs1 = changes_->get_changes("test_table");
    recs2 = changes_->get_changes("table2");
    EXPECT_EQ(recs1.size(), 0);
    EXPECT_EQ(recs2.size(), 0);
}

// Test: --commit-unsaved-changes applies and clears for table
TEST_F(UndoLogManagerTest, CommitUnsavedChanges) {
    UndoLogManager mgr(db_);

    // Add unsaved changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");

    // Commit changes
    EXPECT_TRUE(mgr.commit_unsaved_changes("test_table"));

    // Verify data was inserted
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-100, 100, -100, 100);
    EXPECT_EQ(points.size(), 2);

    // Verify unsaved changes were cleared
    auto recs = changes_->get_changes("test_table");
    EXPECT_EQ(recs.size(), 0);
}

// Test: --list-unsaved-changes outputs pending changes
TEST_F(UndoLogManagerTest, ListUnsavedChanges) {
    UndoLogManager mgr(db_);

    // Add various changes
    changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");
    changes_->record_delete("test_table", 1, 1.0, 2.0, "x_val");

    std::ostringstream output;
    EXPECT_TRUE(mgr.list_unsaved_changes("test_table", output));

    std::string result = output.str();
    // Should contain information about the changes
    EXPECT_NE(result.find("insert"), std::string::npos);
    EXPECT_NE(result.find("delete"), std::string::npos);
}

// Test: Handle table with no unsaved changes
TEST_F(UndoLogManagerTest, HandleNoChanges) {
    UndoLogManager mgr(db_);

    // Try to clear undo log for table with no changes
    EXPECT_TRUE(mgr.clear_undo_log("test_table"));

    // Try to commit with no changes
    EXPECT_TRUE(mgr.commit_unsaved_changes("test_table"));

    // Try to list with no changes
    std::ostringstream output;
    EXPECT_TRUE(mgr.list_unsaved_changes("test_table", output));
    // Output should indicate no changes
    std::string result = output.str();
    EXPECT_NE(result.find("No"), std::string::npos);
}

// Test: Handle non-existent table
TEST_F(UndoLogManagerTest, HandleNonExistentTable) {
    UndoLogManager mgr(db_);

    // Try to clear undo log for non-existent table (should succeed but do nothing)
    EXPECT_TRUE(mgr.clear_undo_log("nonexistent"));

    // Try to commit for non-existent table (should fail)
    EXPECT_FALSE(mgr.commit_unsaved_changes("nonexistent"));

    // Try to list for non-existent table (should succeed but show nothing)
    std::ostringstream output;
    EXPECT_TRUE(mgr.list_unsaved_changes("nonexistent", output));
}

// Test: List shows active and inactive changes
TEST_F(UndoLogManagerTest, ListShowsActiveStatus) {
    UndoLogManager mgr(db_);

    // Add changes
    auto id1 = changes_->record_insert("test_table", 1.0, 2.0, "x_val");
    changes_->record_insert("test_table", 3.0, 4.0, "o_val");

    // Mark one as inactive
    const char* sql = "UPDATE unsaved_changes SET is_active = 0 WHERE id = ?";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id1.value());
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    std::ostringstream output;
    mgr.list_unsaved_changes("test_table", output);

    std::string result = output.str();
    // Should show both active and inactive status
    EXPECT_NE(result.find("Active:"), std::string::npos);
}

// Test to verify that deletion marks insert as inactive in database
#include <gtest/gtest.h>
#include "../include/database.h"
#include "../include/point_editor.h"
#include "../include/unsaved_changes.h"
#include <filesystem>

using namespace datapainter;

class DeletionDatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_path_ = ":memory:";
        db_ = std::make_unique<Database>(db_path_);
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
        const char* create_changes_sql = R"(
            CREATE TABLE IF NOT EXISTS unsaved_changes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                table_name TEXT NOT NULL,
                action TEXT NOT NULL,
                data_id INTEGER,
                x REAL,
                y REAL,
                old_target TEXT,
                new_target TEXT,
                meta_field TEXT,
                old_value TEXT,
                new_value TEXT,
                is_active INTEGER DEFAULT 1
            )
        )";
        ASSERT_TRUE(db_->execute(create_changes_sql));

        // Create metadata table and add entry
        const char* create_meta_sql = R"(
            CREATE TABLE IF NOT EXISTS metadata (
                table_name TEXT PRIMARY KEY,
                target_column_name TEXT NOT NULL,
                x_axis_name TEXT NOT NULL,
                y_axis_name TEXT NOT NULL,
                x_meaning TEXT NOT NULL,
                o_meaning TEXT NOT NULL,
                valid_x_min REAL,
                valid_x_max REAL,
                valid_y_min REAL,
                valid_y_max REAL
            )
        )";
        ASSERT_TRUE(db_->execute(create_meta_sql));

        const char* insert_meta_sql = R"(
            INSERT INTO metadata (table_name, target_column_name, x_axis_name, y_axis_name,
                                 x_meaning, o_meaning, valid_x_min, valid_x_max, valid_y_min, valid_y_max)
            VALUES ('test_data', 'label', 'x', 'y', 'positive', 'negative', -10, 10, -10, 10)
        )";
        ASSERT_TRUE(db_->execute(insert_meta_sql));

        editor_ = std::make_unique<PointEditor>(*db_, "test_data");
    }

    std::string db_path_;
    std::unique_ptr<Database> db_;
    std::unique_ptr<PointEditor> editor_;
};

TEST_F(DeletionDatabaseTest, DeleteUnsavedPointMarksInsertInactive) {
    // Create a point (this creates an insert in unsaved_changes)
    ASSERT_TRUE(editor_->create_point(5.0, 5.0, 'x'));

    // Check that insert was recorded
    UnsavedChanges uc(*db_);
    auto changes = uc.get_changes("test_data");
    ASSERT_EQ(changes.size(), 1);
    EXPECT_EQ(changes[0].action, "insert");
    EXPECT_TRUE(changes[0].is_active);
    EXPECT_DOUBLE_EQ(changes[0].x.value(), 5.0);
    EXPECT_DOUBLE_EQ(changes[0].y.value(), 5.0);

    int insert_change_id = changes[0].id;

    // Delete the point (should mark insert as inactive)
    double cell_size = 1.0;
    int deleted = editor_->delete_points_at_cursor(5.0, 5.0, cell_size);
    EXPECT_EQ(deleted, 1);

    // Check that insert is now inactive
    changes = uc.get_changes("test_data");
    ASSERT_EQ(changes.size(), 1);  // Still 1 record
    EXPECT_EQ(changes[0].id, insert_change_id);
    EXPECT_EQ(changes[0].action, "insert");
    EXPECT_FALSE(changes[0].is_active) << "Insert should be marked inactive after deletion";
}

TEST_F(DeletionDatabaseTest, GetPointsAtCursorSkipsInactiveInserts) {
    // Create a point
    ASSERT_TRUE(editor_->create_point(5.0, 5.0, 'x'));

    // Should find the point before deletion
    double cell_size = 1.0;
    auto points_before = editor_->get_points_at_cursor(5.0, 5.0, cell_size);
    EXPECT_EQ(points_before.size(), 1);

    // Delete the point
    int deleted = editor_->delete_points_at_cursor(5.0, 5.0, cell_size);
    EXPECT_EQ(deleted, 1);

    // Should NOT find the point after deletion
    auto points_after = editor_->get_points_at_cursor(5.0, 5.0, cell_size);
    EXPECT_EQ(points_after.size(), 0) << "Should not find inactive inserted point";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Test to verify flip functionality works for both saved and unsaved points
#include <gtest/gtest.h>
#include "../include/database.h"
#include "../include/point_editor.h"
#include "../include/unsaved_changes.h"
#include "../include/metadata.h"
#include "../include/data_table.h"

using namespace datapainter;

class FlipFunctionalityTest : public ::testing::Test {
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
        ASSERT_TRUE(db_->ensure_unsaved_changes_table());

        // Create metadata table and add entry
        ASSERT_TRUE(db_->ensure_metadata_table());

        const char* insert_meta_sql = R"(
            INSERT INTO metadata (table_name, target_col_name, x_axis_name, y_axis_name,
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

TEST_F(FlipFunctionalityTest, FlipUnsavedPoint) {
    // Create an unsaved point (x type = positive)
    ASSERT_TRUE(editor_->create_point(5.0, 5.0, 'x'));

    double cell_size = 1.0;

    // Verify the point exists and is 'positive'
    auto points_before = editor_->get_points_at_cursor(5.0, 5.0, cell_size);
    ASSERT_EQ(points_before.size(), 1);
    EXPECT_EQ(points_before[0].target, "positive");

    // Flip the point
    int flipped = editor_->flip_points_at_cursor(5.0, 5.0, cell_size);
    EXPECT_EQ(flipped, 1);

    // Verify the point is now 'negative'
    auto points_after = editor_->get_points_at_cursor(5.0, 5.0, cell_size);
    ASSERT_EQ(points_after.size(), 1);
    EXPECT_EQ(points_after[0].target, "negative") << "Point should be flipped to 'negative'";
}

TEST_F(FlipFunctionalityTest, FlipSavedPoint) {
    // Insert a point directly into the database
    DataTable dt(*db_, "test_data");
    auto id = dt.insert_point(3.0, 3.0, "positive");
    ASSERT_TRUE(id.has_value());

    double cell_size = 1.0;

    // Verify the point exists and is 'positive'
    auto points_before = editor_->get_points_at_cursor(3.0, 3.0, cell_size);
    ASSERT_EQ(points_before.size(), 1);
    EXPECT_EQ(points_before[0].target, "positive");

    // Flip the point
    int flipped = editor_->flip_points_at_cursor(3.0, 3.0, cell_size);
    EXPECT_EQ(flipped, 1);

    // Verify the point is now 'negative'
    auto points_after = editor_->get_points_at_cursor(3.0, 3.0, cell_size);
    ASSERT_EQ(points_after.size(), 1);
    EXPECT_EQ(points_after[0].target, "negative") << "Saved point should be flipped to 'negative'";

    // Verify it's recorded as an update in unsaved_changes
    UnsavedChanges uc(*db_);
    auto changes = uc.get_changes("test_data");
    bool found_update = false;
    for (const auto& change : changes) {
        if (change.action == "update" && change.old_target == "positive" &&
            change.new_target == "negative") {
            found_update = true;
        }
    }
    EXPECT_TRUE(found_update) << "Should record flip as an update in unsaved_changes";
}

TEST_F(FlipFunctionalityTest, FlipMultiplePoints) {
    // Create two unsaved points at the same location
    ASSERT_TRUE(editor_->create_point(7.0, 7.0, 'x'));
    ASSERT_TRUE(editor_->create_point(7.0, 7.0, 'o'));

    double cell_size = 1.0;

    // Verify both points exist
    auto points_before = editor_->get_points_at_cursor(7.0, 7.0, cell_size);
    ASSERT_EQ(points_before.size(), 2);

    // Flip both points
    int flipped = editor_->flip_points_at_cursor(7.0, 7.0, cell_size);
    EXPECT_EQ(flipped, 2);

    // Verify both are flipped
    auto points_after = editor_->get_points_at_cursor(7.0, 7.0, cell_size);
    ASSERT_EQ(points_after.size(), 2);

    // Should have one positive and one negative (flipped from the originals)
    int positive_count = 0;
    int negative_count = 0;
    for (const auto& point : points_after) {
        if (point.target == "positive") positive_count++;
        if (point.target == "negative") negative_count++;
    }
    EXPECT_EQ(positive_count, 1);
    EXPECT_EQ(negative_count, 1);
}

TEST_F(FlipFunctionalityTest, FlipBackAndForth) {
    // Create an unsaved point
    ASSERT_TRUE(editor_->create_point(2.0, 2.0, 'x'));

    double cell_size = 1.0;

    // Original: positive
    auto points1 = editor_->get_points_at_cursor(2.0, 2.0, cell_size);
    ASSERT_EQ(points1.size(), 1);
    EXPECT_EQ(points1[0].target, "positive");

    // First flip: should be negative
    editor_->flip_points_at_cursor(2.0, 2.0, cell_size);
    auto points2 = editor_->get_points_at_cursor(2.0, 2.0, cell_size);
    ASSERT_EQ(points2.size(), 1);
    EXPECT_EQ(points2[0].target, "negative");

    // Second flip: should be back to positive
    editor_->flip_points_at_cursor(2.0, 2.0, cell_size);
    auto points3 = editor_->get_points_at_cursor(2.0, 2.0, cell_size);
    ASSERT_EQ(points3.size(), 1);
    EXPECT_EQ(points3[0].target, "positive");
}

TEST_F(FlipFunctionalityTest, FlipEmptyCell) {
    // Try to flip where there are no points
    double cell_size = 1.0;
    int flipped = editor_->flip_points_at_cursor(9.0, 9.0, cell_size);
    EXPECT_EQ(flipped, 0) << "Should return 0 when no points to flip";
}
